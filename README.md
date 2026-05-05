# vectorDB

A vector database built from scratch in C++20, implementing four search strategies over high-dimensional float vectors. Built on the SIFT1M benchmark dataset (1 million 128-dimensional vectors).

---

# Benchmark Comparison (1 Million Vectors)
| Implementation | QPS (1M Vectors) | Recall@10 | Build Time | Major Bottleneck | Improvement |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1. Brute Force** | 6 QPS | 100% | 0.00 secs | $O(N)$ complexity; computing 1 million exact L2 distances per query saturates the CPU. | **Baseline:** Perfect accuracy, but completely unscalable. |
| **2. LSH** | ~29 QPS | 85% | ~6.38 secs | Curse of dimensionality; high-dimensional grid partitioning leads to massive collision overhead. | **Sub-linear Search:** Replaced exhaustive $O(N)$ loops with Locality-Sensitive Hashing and bitwise projection. |
| **3. NSW** | 3,130 QPS | 94% | 266.99 secs | Polylogarithmic scaling; greedy routing suffers from a long "zoom-out" phase and can get stuck in local minima. | **Graph Routing:** Replaced hash buckets with a Navigable Small World graph, unlocking massive speed multipliers. |
| **4. HNSW** | 5,794 QPS | >95% | 760.18 secs | Pointer chasing; `std::vector` heap allocations fragment memory, destroying L1 cache hit rates during traversals. | **Logarithmic Search & SIMD:** Added hierarchy layers. Used `-Ofast` to unlock AVX2 SIMD, boosting query throughput heavily. |
| **5. nmslib (Official)** | ~10,000 QPS | ~95% | Highly Opt. | Physical RAM bandwidth limits (waiting for memory to physically travel to the CPU). | **Arena Allocation:** Used C-style flat, contiguous memory arrays to perfectly feed the CPU's Hardware Prefetcher. |
| **6. Annoy (Spotify)** | ~1,000 QPS | ~95% | Variable | Unpredictable memory branching during Random Projection Tree traversals. | *(Historical Standard)*: Used heavily before graph algorithms took over the industry. |
| **7. Faiss (Facebook)** | ~11,000 QPS | ~95% | Highly Opt. | Physical RAM bandwidth limits. | **Production Scale:** Best-in-class memory alignment, multi-threading, and hardware-specific SIMD instruction unrolling. |

---

### Data Notes:
*   *LSH Metrics:* Selected from `Table: 10, Proj: 12` as a representative balance of speed and recall.
*   *NSW Metrics:* Selected at `efSearch = 100`.
*   *HNSW Metrics:* Sourced from your optimized `-Ofast` / `efConst = 100` run at `efSearch = 50`.
*   *nmslib / Faiss / Annoy:* Benchmarks drawn from the official HNSW paper and standardized industry tracking on the SIFT1M dataset.

## What It Does

### Data Storage — [How it works](#vectorstore-and-archive)

Two storage layers: `VectorStore` holds vectors in memory with insert, remove, get, and query operations. `VectorArchive` handles binary persistence to disk with CRC-32 integrity checks, buffered writes, and an append operation that updates the file in-place.

### Brute Force Search — [How it works](#brute-force-search)

Exhaustive nearest neighbor search supporting L2, cosine, and dot product metrics. Serves as the correctness baseline. Uses AVX2 SIMD intrinsics for the L2 distance kernel.

### LSH Index — [How it works](#locality-sensitive-hashing-lsh)

Approximate nearest neighbor search using random projection hashing. Multiple hash tables reduce the candidate set from the full dataset to a small bucket, trading recall for speed. A bit-flip fallback ensures minimum candidate set size.

### NSW Index — [How it works](#navigable-small-world-nsw)

Graph-based approximate search. Vectors are inserted as nodes; each node connects to its nearest neighbors at insertion time. Search traverses the graph greedily from a fixed entry point. Includes heuristic neighbor pruning to preserve graph navigability.

### HNSW Index — [How it works](#hierarchical-navigable-small-world-hnsw)

Extends NSW with a multi-layer hierarchy. Higher layers act as express lanes for long-distance navigation; layer 0 contains the full graph. Entry point adapts as taller nodes are inserted. Supports tunable `M`, `efConstruction`, and `efSearch` parameters.

### Serialization — [How it works](#serialization)

Binary save/load for both LSH and NSW/HNSW indexes. File format includes magic bytes, version, parameters, hyperplanes or graph structure, and raw vector data.

### Result Type — [How it works](#error-handling-and-result-type)

A `Result<Ok, Err>` type modeled after Rust's Result, using `std::variant` internally. Avoids exceptions for expected failure paths. Includes a `Unit` specialization for void-returning operations.

### SafeVector — [How it works](#safevector)

A custom `std::vector` replacement built as a learning exercise, implementing the Rule of Five, copy-and-swap idiom, strong exception safety in `push_back`, and bounds-checked access.

### Thread-Safe VectorStore — [How it works](#concurrency-and-thread-safe-vectorstore)
 
A wrapper around `VectorStore` that adds a `shared_mutex` for concurrent access: multiple threads can query simultaneously, while insert/remove/save/load take exclusive ownership. Includes a `condition_variable` that blocks queries until the database reaches a minimum size.

---

## Getting Started

### Dependencies

- CMake 3.10+
- A compiler with C++20 and AVX2 support (GCC or Clang)
- SIFT1M dataset

### Clone

```bash
git clone https://github.com/draxBlob27/vector-db.git
cd vector-db
```

### Download SIFT1M

```bash
wget ftp://ftp.irisa.fr/local/texmex/corpus/sift.tar.gz
tar -xzf sift.tar.gz
# Place the sift/ folder at ~/vector-db/sift1M/
```

For the smaller 10K subset used in tests:

```bash
# Extract just the first 10K vectors using the same .fvecs files
# or download sift_small from http://corpus-texmex.irisa.fr/
```

### Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Run Benchmarks

```bash
./benchmarks/benchmark_brute
./benchmarks/benchmark_lsh
./benchmarks/benchmark_nsw
./benchmarks/benchmark_hnsw
```

Results are written to `benchmarks/benchmark_<name>.txt`.

### Run Tests

```bash
cd build
./tests/test_safeVector
./tests/test_archive
./tests/test_unit
./tests/test_vdb       # requires GLoVE and SIFT10K datasets
./tests/test_LSH
./tests/test_NSW
```

GLoVE dataset: https://nlp.stanford.edu/projects/glove/ (use the 100-dimensional version)

---

## How It Works

### VectorStore and Archive

`VectorStore` stores vectors as a flat `std::vector<pair<id, Vector>>` alongside an `unordered_set` for O(1) duplicate detection. Dimension consistency is enforced at insertion. Norms are computed once at insertion time and cached for reuse during cosine queries.

`VectorArchive` writes a fixed binary header (magic bytes, version, dimension, count, CRC-32) followed by data in chunks. Chunked writes avoid allocating the entire dataset in one buffer. The append operation seeks to the count field, updates it in-place, then seeks to the end of existing data to write new vectors, carrying the existing CRC state forward rather than recomputing it from scratch. Separate CRCs for header and data avoid the need to re-read all data on every append.

The magic bytes `0x56454344` spell "VECD" — used to reject foreign files quickly.

---

### Brute Force Search

For L2 distance, the inner loop uses AVX2 SIMD: loads 8 floats at a time from each vector, computes the difference, and accumulates squared differences using a fused multiply-add instruction. This keeps the hot loop in vector registers and avoids the overhead of scalar iteration.

The query maintains a max-heap of size k. For each candidate, if its distance is less than the heap's top (the current worst of the best-k), it replaces it. This avoids sorting the full result set.

Cosine and dot product metrics share the same heap structure but use different comparison directions (higher is better).

---

### Locality Sensitive Hashing (LSH)

Random projection LSH works by generating random hyperplanes through the origin. For each vector, the sign of the dot product with each hyperplane produces one bit; the concatenation of bits across all projections in a table forms the hash key.

Two nearby vectors (in L2 space) are more likely to land on the same side of each hyperplane, so they tend to share the same hash. Multiple independent hash tables increase recall: a true neighbor only needs to collide in at least one table.

The key failure mode discovered during development was using too-small random values for hyperplane normals, which produced near-flat planes where almost all vectors had the same projection sign. This collapsed all vectors into one bucket. The fix was using values drawn from a normal distribution.

A bit-flip fallback handles queries that return fewer than k candidates: it XORs one bit at a time from each table's hash, checking adjacent buckets until enough candidates are found.

The recall-speed tradeoff is direct: more projections per table means smaller buckets (faster, lower recall); fewer projections means larger buckets (slower, higher recall). Empirically, 8-10 tables with 10-12 projections gave the best balance on SIFT1M.

---

### Navigable Small World (NSW)

NSW builds a proximity graph incrementally. When a new vector arrives, the graph is searched greedily from the entry point to find the ef nearest current nodes. The new node connects to the best M of those, and those neighbors reciprocally connect back.

The challenge is preventing any node from accumulating too many edges. Without a degree limit, the first-inserted node (the entry point) can attract edges from nearly every subsequent insertion, turning it into a hub that dominates search traversal. The fix is enforcing a maximum edge count per node: after connecting a neighbor, if its degree exceeds the limit, prune to the closest M neighbors.

A simple "keep closest" pruning strategy was tried first. This gave ~98% recall ceiling even at high efSearch because important long-range bridge edges were pruned when a closer neighbor was present. The heuristic from the original paper addresses this: when considering a new edge, reject a candidate if there already exists a result node that is closer to the candidate than the candidate is to the query. This preserves diversity and maintains navigability.

The visited set uses a generation counter rather than clearing a boolean array on each query, avoiding O(n) overhead per search.

---

### Hierarchical Navigable Small World (HNSW)

HNSW assigns each inserted node a random maximum layer drawn from an exponential distribution (controlled by `mL = 1/ln(M)`). Most nodes exist only at layer 0; a small fraction reach higher layers.

Insertion proceeds top-down: starting from the entry point at the highest layer, a greedy search with ef=1 descends through each layer above the new node's layer, narrowing in on the insertion region. From the new node's layer down to layer 0, the full `efConstruction`-wide search runs at each layer and the heuristic neighbor selection determines which edges to create.

The "lean node" experiment (storing only IDs in edges, recomputing distances on the fly) showed a ~30% QPS gain at 10K vectors where the dataset fit in L2/L3 cache, but no gain at 1M vectors where every recomputation fetched from RAM. The fat node (storing distance alongside ID) was reverted for production scale.

A subtle bug during development was computing hydration distances relative to the new node instead of the base node. This destroyed local neighborhood structure silently — the graph structure appeared valid but recall dropped to 69%. Restoring the correct reference point brought recall back to 99-100%.

---

### Serialization

The file format for NSW/HNSW writes each node sequentially: ID, vector size, vector data, neighbor count, neighbor list (each neighbor stored as a distance+ID pair). This flat layout allows a single forward read on load.

LSH serialization writes all hyperplane vectors first (one table at a time), then the hash table contents (active hash count, then for each bucket: hash key, vector count, vector indices), then the raw vectors. This ordering allows the hyperplanes to be loaded and the hash tables to be reconstructed without seeking.

Magic bytes and version fields at the start of every file allow fast rejection of wrong file types and format mismatches.

---

### Error Handling and Result Type

`Result<OkT, ErrT>` wraps `std::variant<Ok<OkT>, Err<ErrT>>`. The `Ok` and `Err` wrapper types use explicit constructors to prevent implicit conversions. Two access patterns are provided: `ok_value()` returns a copy (safe to call multiple times) and `take_ok_value()` moves out the value (invalidates the Result).

The `Result<Unit, ErrT>` specialization handles operations that succeed without returning a value, analogous to `Result<(), E>` in Rust. `take_ok_value()` is deleted on this specialization since there is nothing to move.

This pattern avoids exceptions for expected failure cases (wrong dimensions, missing IDs, file not found) while keeping exceptions for truly unexpected I/O failures in the archive layer.

---

### SafeVector

The move constructor takes ownership of the source's heap allocation and resets the source's size and capacity to zero — required to avoid double-free when the source is later destroyed.

The copy-and-swap assignment operator takes its argument by value (triggering either a copy or move construction depending on the call site), then swaps with `*this`. This handles self-assignment correctly, provides strong exception safety (the copy happens before any modification to `*this`), and collapses copy and move assignment into one function.

`push_back` achieves strong exception safety by allocating new memory, moving all existing elements, and inserting the new value before committing the new buffer to `m_data`. If the assignment throws, `m_data` is unchanged.

---
 
### Concurrency and Thread-Safe VectorStore
 
The first question when adding concurrency is where it actually helps. Several obvious candidates turn out to be wrong:
 
- **Building the index**: inserting a vector is a critical section on shared state. Spawning threads only to serialize them at the same lock buys nothing.
- **Norm computation at insertion**: the per-vector cost is small enough that thread spawn overhead would exceed the savings.
- **Saving to disk**: writing a binary file is inherently sequential. Splitting it across threads creates coordination problems with no throughput benefit.
- **Loading from disk**: threads would spend most of their time waiting on I/O, not computing. The bottleneck is the disk, not the CPU.
This leaves **concurrent querying** as the meaningful target. Many read threads can safely scan the same immutable vector data simultaneously, as long as no write is happening.
 
`ThreadSafeVectorStore` wraps `VectorStore` with a `std::shared_mutex`. Read operations (`query`, `get`, `size`, `dimensions`, `info`) acquire a shared lock, allowing any number of concurrent readers. Write operations (`insert`, `remove`, `save`, `load`) acquire a unique lock, blocking all readers and other writers until they complete.
 
The `condition_variable_any` adds a wait condition to `query`: if the database has not yet reached a minimum population (used in scenarios where a background thread is loading data while query threads are already running), the query thread sleeps and releases the shared lock rather than spinning or returning an empty result. When an insert completes successfully, it calls `notify_all` to wake any waiting query threads.
 
The main difficulty with concurrent code is that bugs are timing-dependent and often invisible under light load. A few hard lessons from this:
 
- Using `std::shared_mutex` is not free. Lock acquisition and contention have overhead. If the critical section is very short (e.g. a single integer read), the locking cost can exceed the benefit. The right question is whether the protected work is large enough to justify the synchronization cost.
- A `condition_variable` must always be used with a predicate. Spurious wakeups (where the thread wakes without being notified) are real and can cause a query to proceed on an empty database. The predicate re-checks the condition before proceeding.
- The mental model for correctness: at any point in time, either one writer holds the mutex exclusively, or any number of readers hold it shared. These two states must never overlap. Getting this wrong produces data races that corrupt results silently rather than crashing.
