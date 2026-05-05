My first hnsw implmentaation had these results:
Parameters(M = 16, efConst = 200)
Build time: 13.9057 secs 
efSearch = 10
MsPQ for 10000 vectors : 7.58731 Milliseconds
Recall@10 for 10000 vectors : 99%
QPS for 10000 vectors : 131 queries
efSearch = 25
MsPQ for 10000 vectors : 7.7442 Milliseconds
Recall@10 for 10000 vectors : 99%
QPS for 10000 vectors : 129 queries
efSearch = 50
MsPQ for 10000 vectors : 7.86582 Milliseconds
Recall@10 for 10000 vectors : 99%
QPS for 10000 vectors : 127 queries
efSearch = 100
MsPQ for 10000 vectors : 7.75081 Milliseconds
Recall@10 for 10000 vectors : 99%
QPS for 10000 vectors : 129 queries
efSearch = 200
MsPQ for 10000 vectors : 7.72989 Milliseconds
Recall@10 for 10000 vectors : 99%
QPS for 10000 vectors : 129 queries
efSearch = 500
MsPQ for 10000 vectors : 7.6614 Milliseconds
Recall@10 for 10000 vectors : 99%
QPS for 10000 vectors : 130 queries

## 1. The "Lean Node" Experiment & CPU Cache Mechanics
**Goal:** Maximize L1 cache efficiency by stripping the float distance from the permanent graph edges, storing only raw `uint64_t` IDs.

### The "Hydration" Pattern
To make this work, I implemented a temporary workspace pattern. During the insert shrinkage phase, the engine temporarily "hydrated" the node IDs by recalculating their distances on the fly to perform heuristic sorting, then "dehydrated" them back to raw IDs before saving.

### The 10K Vector Victory
At the 10,000 vector scale, this resulted in a ~30% increase in QPS. Because the entire dataset fit inside the CPU's L2/L3 cache, the AVX2 SIMD distance calculations were practically free. Packing twice as many IDs into a 64-byte CPU cache line meant the CPU spent less time stalling and more time routing.

---

## 2. Profiling and The Standard Library Paradox

**Goal:** Eliminate the massive CPU time (34%) spent in `std::ranges::sort` during the hydration phase.

### The Bubble-Up Hypothesis
Since an overflowing neighbor list of size `M_max + 1` is already 97% sorted, I hypothesized that an `O(N)` single-pass insertion (Bubble-Up) would outperform the `O(N log N)` generic `std::sort`.

### The Reality Check
The custom Bubble-Up loop was actually slower.

### The Lesson
The C++ Standard Library is written by wizards. Even after removing dynamic heap allocations (`std::vector::reserve`) and replacing `std::swap` with memory shifts, the STL’s intrinsic fallback to highly-unrolled insertion sort for small arrays (<32 elements) is nearly impossible to beat.

> Measure everything; trust the profiler over theoretical Big-O notation.

---

## 3. Spatial Locality and Graph Corruption

**Goal:** Fix the massive Recall collapse (down to 69%) during early 1M vector benchmarks.

### The Bug
During the Lean Node hydration phase, distances were accidentally calculated relative to the newly inserted vector rather than the existing base node.

### The Impact
This completely destroyed the spatial locality of the graph. The heuristic pruning kept edges that pointed across the entire dataset, destroying the "navigable small world" property.

### The Lesson
HNSW relies entirely on accurate local neighborhoods. Fixing this single distance calculation instantly restored Recall to 99–100%.

---

## 4. The 1M Scale Wall (Cache vs. RAM Thrashing)

**Goal:** Scale the Lean Node architecture to 1 million vectors (500MB+ dataset).

### The Wall
At 1M vectors, the Lean Node's build time actually increased compared to the Fat Node, and the QPS advantage completely vanished.

### The Hardware Reality
At this scale, the dataset exceeded the CPU's L3 cache. Every hydration required fetching a 512-byte payload (128-dim float vector) from slow DDR RAM.

The 4 bytes saved per edge were insignificant compared to memory latency costs.

### The Pivot
Reverted to the standard **Fat Node (distance + id)** architecture for production-scale systems to avoid repeated memory fetches.

---

## 5. Compiler Magic & Diminishing Algorithmic Returns

**Goal:** Close the performance gap with the official `nmslib` implementation.

### Parameter Tuning
Reduced `efConstruction` from 200 to 100, based on guidance from the original HNSW paper.

- Result: ~34% faster build time

### Compiler Optimizations
Enabled:
- `-Ofast`
- `-ffast-math`

These allowed:
- Loop unrolling
- Better AVX2 SIMD utilization
- Floating-point reordering

### The Result
- Build time: **1190s → 760s**
- Query throughput: **~10% increase (~3500 QPS)**

---

## 6. The Ultimate Bottleneck: Memory Layout

**Goal:** Understand why `nmslib` and `Faiss` still achieve ~2× higher QPS.

### Pointer Chasing Problem
Using `std::vector` inside each node resulted in:
- 1 million heap allocations
- Random memory distribution
- Frequent cache misses during traversal

### Arena Allocation (Flat Memory)
Libraries like Faiss use contiguous memory blocks:
- Single large allocation
- Structured memory layout

### Hardware Prefetcher Advantage
Flat memory enables the CPU to:
- Detect access patterns
- Prefetch data into L1 cache
- Hide memory latency

---
