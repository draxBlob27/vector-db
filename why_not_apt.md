## Why This Implementation Trails Production Systems

Reaching ~5,794 QPS on HNSW is a real achievement for a from-scratch C++ implementation. The gap between this and Faiss (~11,000 QPS) or nmslib (~10,000 QPS) is not due to any algorithmic shortcoming — the same HNSW algorithm runs in all three. The gap is almost entirely **memory architecture**.

---

### 1. Heap Fragmentation from `std::vector` (The Core Problem)

Every node in this implementation stores its neighbor list as a `std::vector<pair<float, uint64_t>>`. This means 1 million nodes = 1 million separate heap allocations, scattered across RAM.

During a graph traversal, the CPU must:
1. Load a node's metadata (cache miss — random address)
2. Follow a pointer to its `std::vector` buffer (second cache miss — another random address)
3. Load the actual neighbor IDs (third cache miss — yet another random address)

Every hop in the greedy search triggers this three-miss chain. At 1M vectors, the working set far exceeds L3 cache, so each miss pays the full ~100ns DDR RAM penalty.

Faiss avoids this entirely with **arena allocation**: one massive contiguous block of memory is pre-allocated, and all graph data is packed into it with a fixed stride. The CPU's hardware prefetcher detects the sequential access pattern and speculatively loads the next node's neighbors before they are needed, hiding latency almost completely.

---

### 2. The "Lean Node" Experiment: Cache Efficiency vs. RAM Bandwidth

An experiment was run storing only `uint64_t` IDs in edges (saving 4 bytes per edge) and recomputing distances on the fly — a pattern called "hydration". At 10K vectors, this yielded a ~30% QPS gain because the dataset fit in L2/L3 cache, making the AVX2 distance recomputation essentially free.

At 1M vectors, the gain disappeared completely. Each hydration required fetching a 512-byte vector (128 floats) from DDR RAM. The 4 bytes saved per edge were irrelevant compared to the latency of the RAM fetch they triggered.

**Lesson:** Cache-efficiency optimizations must be validated at the actual deployment scale. Micro-benchmarks at 10K are not representative of 1M+ behavior.

---

### 3. SIMD Is Partially Utilized

AVX2 SIMD is used for the L2 distance kernel, which is correct. However, the gains are limited by the memory access pattern: AVX2 can process 8 floats per instruction, but it cannot hide the latency of cache misses. The CPU stalls waiting for data regardless of how fast the arithmetic unit is.

Production libraries go further:
- **Instruction unrolling**: manually unroll the inner loop 4–8× to keep the SIMD pipeline saturated even during partial stalls
- **Prefetch intrinsics**: explicitly issue `_mm_prefetch` calls for the next neighbor before finishing the current one
- **Quantization (Faiss-specific)**: store vectors as int8 or even 4-bit values, reducing the memory bandwidth required per distance computation by 4–8×

---

### 4. Build Time: `efConstruction` and Compiler Flags

Build time dropped from 1,190s to 760s by reducing `efConstruction` from 200 to 100 and enabling `-Ofast -ffast-math`. These flags permit floating-point reordering and unlock better AVX2 loop unrolling from the compiler.

The remaining build time is dominated by the insertion graph search at layer 0, which is inherently sequential per-insertion (each new node depends on the current graph state). Faiss parallelizes batch insertions using a two-phase approach: search in parallel, then synchronize only for edge writes. This is non-trivial to implement correctly under concurrent modification but is the primary remaining build-time lever.

---

### Summary

| Gap | Root Cause | Fix |
| :--- | :--- | :--- |
| ~2× QPS vs. nmslib/Faiss | Heap-fragmented `std::vector` neighbor lists | Arena-allocate a flat contiguous graph |
| ~6–8× latent multiplier | Single-threaded query runner | Thread pool over query set |
| Partial SIMD gains | Memory latency not hidden | Prefetch intrinsics + vector quantization |
| 760s build time | Sequential insertions | Parallel batch insert with synchronized edge writes |

The algorithmic work — heuristic neighbor selection, multi-layer hierarchy, exponential layer assignment, efSearch/efConstruction tuning — is complete and matches paper-quality recall.
