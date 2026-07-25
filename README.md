# vectorDB

`vectorDB` is a from-scratch C++20 vector search project. It implements exact
search, approximate search indexes, binary persistence, dataset importers,
benchmarks, and tests for understanding how vector databases work below the
API layer.

The project is intentionally educational and systems-oriented. It does not hide
the core algorithms behind external ANN libraries. The code shows the tradeoffs
directly: exact scan correctness, hash selectivity, graph navigability,
construction cost, memory layout, SIMD distance kernels, persistence integrity,
and benchmark methodology.

## Contents

- Exact in-memory vector store with L2, cosine, and dot-product search.
- Binary archive format with CRC-32 verification and append support.
- Locality Sensitive Hashing index using random projections.
- Navigable Small World graph index.
- Header-only Hierarchical Navigable Small World index used by the benchmark.
- SIFT1M and GLoVE import helpers.
- Benchmarks for brute force, LSH, NSW, and HNSW.
- GoogleTest coverage for core containers, archive, vector store, LSH, and NSW.
- A custom `SafeVector<T>` used as a learning exercise for RAII and exception safety.

## Why This Exists

Vector search looks simple from the outside: store embeddings and ask for the
nearest vectors. The difficult parts appear when the dataset grows.

Exact search gives perfect recall, but every query touches every vector. At
1M vectors with 128 float dimensions, that is 128M float operations per query
before considering memory traffic. Approximate indexes reduce the number of
vectors inspected by accepting a tunable recall/speed tradeoff.

This repository follows that path:

1. Start with brute force search so correctness is measurable.
2. Add LSH to avoid scanning the full dataset through hash collisions.
3. Add NSW to route through a proximity graph instead of hash buckets.
4. Add HNSW to give the graph long-range express layers.
5. Measure everything against SIFT ground truth using recall and QPS.

## Repository Layout

```text
include/vectorDB/
  VectorStore.hpp              Exact in-memory vector store API
  archive.hpp                  CRC-protected binary archive API
  LSH_Index.hpp                Random-projection LSH index API
  NSW_Index.hpp                NSW graph index API
  HNSW_Index.hpp               Header-only HNSW index used by benchmark
  safeVector.h                 Custom vector implementation
  utils/
    Vector.hpp                 Vector wrapper with cached norm
    Metric.hpp                 L2, cosine, dot-product enum
    distances.hpp              Distance kernels, including AVX2 L2
    Importer.hpp               GLoVE and SIFT readers
    Result.hpp                 Result<Ok, Err> helper
    Serializer_De.hpp          Binary serialization helpers
    Node.hpp                   NSW graph node
    HNSW/Node.hpp              HNSW graph node
    Timer.hpp                  Millisecond timer
    Random_engine.hpp          Random number helper

src/
  VectorStore.cpp              Exact search, save, load
  archive.cpp                  CRC archive implementation
  LSH_Index.cpp                LSH build, query, save, load
  NSW_Index.cpp                NSW insert, query, save, load
  generate_crc_tab.cpp         Utility used to generate the CRC table

benchmarks/
  benchmark_brute.cpp          Exact search benchmark
  benchmark_lsh.cpp            LSH benchmark
  benchmark_nsw.cpp            NSW benchmark
  benchmark_hnsw.cpp           HNSW benchmark

tests/
  test_safeVector.cpp          SafeVector behavior
  test_archive.cpp             Archive save/load/verify/append
  test_unit.cpp                VectorStore unit tests
  test_vdb.cpp                 Dataset-backed VectorStore tests
  test_LSH.cpp                 LSH persistence smoke tests
  test_NSW.cpp                 NSW persistence round trip

docs/
  *_learnings.md               Development notes, experiments, and references
```

## Build

Requirements:

- CMake 3.10 or newer.
- C++20 compiler.
- AVX2 and FMA support, because the build adds `-mavx2 -mfma`.
- Network access during the first test configure if GoogleTest is not cached,
  because `tests/CMakeLists.txt` fetches GoogleTest with `FetchContent`.

Release build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Debug build:

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j
```

The top-level CMake currently builds four libraries:

- `Archive_lib`
- `VectorStore_lib`
- `LSH_Index`
- `NSW_Index`

`HNSW_Index.hpp` is header-only and included directly by
`benchmarks/benchmark_hnsw.cpp`.

## Run

Example archive CLI:

```bash
./build/examples/cli_demo create vectors.bin 128 1000 --random
./build/examples/cli_demo info vectors.bin
./build/examples/cli_demo verify vectors.bin
./build/examples/cli_demo load vectors.bin --verify
./build/examples/cli_demo append vectors.bin 1000
```

CLI caveat: the current `append` command requires a third argument, but the code
does not use its value yet. It reads the existing file info and appends a
zero-filled batch with the same dimension and count as the current archive.

Tests:

```bash
./build/tests/test_safeVector
./build/tests/test_archive
./build/tests/test_unit
./build/tests/test_vdb
./build/tests/test_LSH
./build/tests/test_NSW
```

Important test caveat: several tests currently use absolute local paths such as
`/home/sp27022003/vector-db/...` for datasets and output files. Those tests need
the same files in those paths or small path edits before they are portable.

Benchmarks:

```bash
./build/benchmarks/benchmark_brute
./build/benchmarks/benchmark_lsh
./build/benchmarks/benchmark_nsw
./build/benchmarks/benchmark_hnsw
```

Benchmark caveat: benchmark source files also contain absolute dataset and
output paths. Update `dir` and output file paths before running on a new
machine.

## Datasets

The code supports two dataset families:

- SIFT `.fvecs` / `.ivecs`: used for nearest-neighbor recall benchmarking.
- GLoVE text embeddings: used for semantic vector search experiments.

SIFT import expects:

```text
sift_base.fvecs
sift_query.fvecs
sift_groundtruth.ivecs
```

The importer assumes SIFT vectors are 128-dimensional. It reads base vectors,
query vectors, and ground-truth neighbor IDs.

GLoVE import currently assumes 100-dimensional vectors. It maps words to
integer IDs and stores the vectors in insertion order.

## Core Types

### `Vector`

`Vector` is a small wrapper around `std::vector<float>`:

```cpp
struct Vector {
    std::vector<float> data;
    float norm_data{0.0f};
    bool normalized{false};
};
```

The cached norm exists because cosine similarity needs vector norms repeatedly.
Vectors compute their norm once through `compute_norm()`, then reuse it.

### Metrics

`Metric` supports:

- `Metric::L2`: squared Euclidean distance. Smaller is better.
- `Metric::Cosine`: cosine similarity. Larger is better.
- `Metric::DotProduct`: dot-product similarity. Larger is better.

L2 intentionally returns squared distance, not square-rooted Euclidean distance.
For nearest-neighbor ordering this is equivalent and avoids an expensive
`sqrt()` per candidate.

### Distance Kernel Caveat

The L2 specialization in `include/vectorDB/utils/distances.hpp` uses AVX2 and
processes 8 floats per loop. The current implementation does not include a
scalar tail loop, so dimensions should be a multiple of 8. SIFT's 128 dimensions
are safe. Arbitrary dimensions should be padded or the kernel should be updated.

## Exact Search: `VectorStore`

`VectorStore` is the correctness baseline. It stores `(id, Vector)` pairs in a
flat vector and keeps an `unordered_set` of IDs to reject duplicate inserts.

Public operations:

```cpp
Result<Unit, DBError> insert(uint64_t id, Vector vector);
Result<Unit, DBError> remove(uint64_t id);
Result<std::vector<float>, DBError> get(uint64_t id) const;
Result<std::vector<std::pair<uint64_t, float>>, DBError>
    query(const Vector& q, uint64_t k = 10, Metric metric = Metric::L2) const;
Result<Unit, DBError> save(const std::string& filename) const;
Result<Unit, DBError> load(const std::string& filename);
Result<uint64_t, DBError> size() const;
Result<uint64_t, DBError> dimensions() const;
Result<Info, DBError> info() const;
```

Why exact search matters:

- It provides a ground truth implementation inside the repo.
- It validates distance functions before approximate indexes are trusted.
- It gives 100% recall for benchmark comparison.

Query implementation:

1. Validate the database is non-empty.
2. Validate query dimensionality.
3. Scan every vector.
4. Maintain a heap of size `k`.
5. Return the best `k` IDs and scores.

For L2, the heap is a max-heap because the worst current neighbor is the largest
distance. For cosine and dot product, the heap is a min-heap because the worst
current neighbor is the smallest similarity.

Complexity:

- Insert: amortized `O(1)` plus norm computation.
- Query: `O(N * D)` distance work and `O(N log k)` heap work.
- Save/load: `O(N * D)` sequential binary I/O.

Current limitations:

- `remove()` erases from the vector storage but does not erase the ID from the
  ID set, so reinserting the same ID after removal is still rejected.
- `get()` is still a linear scan after checking the ID set.
- The `VectorStore` binary format does not currently include CRC validation.

## Error Handling: `Result`

Expected failures in `VectorStore` use `Result<OkT, ErrT>` instead of
exceptions. This keeps normal failure paths explicit:

```cpp
auto result = store.query(query, 10, Metric::L2);
if (result.is_ok()) {
    auto neighbors = result.ok_value();
} else {
    auto error = result.err_value();
}
```

`Result<Unit, ErrT>` is used for operations that return no value on success,
similar to `Result<(), E>` in Rust.

`DBError` includes:

- `MetricError`
- `DimensionError`
- `IdNotFoundError`
- `ZeroNormError`
- `DataBaseEmptyError`
- `FileCorrupted`
- `IdAlreadyPresent`
- `FileNotFound`

Archive and index serialization helpers use exceptions derived from
`ArchiveError` for I/O and corruption failures.

## Binary Persistence

The repository has multiple binary formats. They are intentionally separate.

### `VectorStore::save/load`

Stores float vectors for the exact in-memory store:

```text
magic bytes: uint32_t, 0x56454344 ("VECD")
version:     uint32_t
dimension:   uint32_t
count:       uint64_t
records:
  id:        uint64_t
  data:      dimension * float
  norm:      float
```

This format is simple and fast, but CRC fields are commented out in the current
implementation.

### `VectorArchive`

`VectorArchive` stores `std::vector<std::vector<double>>` and includes CRC-32
checks for both header and data:

```text
magic bytes: uint32_t, 0x56454344 ("VECD")
version:     uint32_t
dimension:   uint32_t
count:       uint64_t
header_crc:  uint32_t
data:        count * dimension * double
data_crc:    uint32_t
```

Why separate header and data CRCs:

- A single CRC over header plus data is simple but makes append expensive.
- Updating the count in the header would require rereading all data to compute
  one new combined CRC.
- Separate CRCs let append update the header CRC and continue the data CRC from
  the previous value.

`save()` writes data in chunks of 32 vectors to avoid building one huge byte
buffer. `verify()` recomputes both CRCs. `append()` updates count, header CRC,
data bytes, and data CRC in-place.

Measured archive notes from `docs/archive_learnings.md`:

| Operation / Experiment | Time |
| --- | ---: |
| Create 128D x 1 random vector | 0.245 ms |
| Create 128D x 10 random vectors | 0.430 ms |
| Create 128D x 1,000 random vectors | 10.012 ms |
| Create 128D x 100,000 random vectors | 743.062 ms |
| Create 128D x 1,000,000 random vectors | 7684.73 ms |

The development notes show CRC computation dominated archive write time at
large scale.

### LSH Index Format

`LSHIndex::save()` writes:

```text
magic bytes and version
num_tables
num_projections
dimension
count
hyperplanes for each table
hash table contents:
  active hash count
  hash key
  bucket size
  vector indices
raw vectors:
  id
  vector data
```

This saves both the random hyperplanes and the populated buckets, so loading
does not need to rebuild the index.

### NSW Index Format

`NSW_Index::save()` writes:

```text
magic bytes and version
efConstruction
efSearch
M
num_nodes
dimension
nodes:
  id
  vector size
  vector data
  edge count
  neighbors as (distance, node_index)
entry point
```

`test_NSW.cpp` verifies that a saved and loaded graph compares equal to the
original.

## Approximate Index: LSH

`LSHIndex` implements random-projection Locality Sensitive Hashing.

Parameters:

- `num_tables`: number of independent hash tables.
- `num_projections`: number of random hyperplanes per table.

Why LSH:

Brute force search scans every vector. LSH tries to produce a smaller candidate
set by hashing vectors so nearby vectors are likely to collide. After candidate
collection, the implementation reranks candidates exactly with L2 distance.

Build pipeline:

1. Generate `num_tables * num_projections` random hyperplanes.
2. For each vector, compute one hash per table.
3. Store the vector's internal index in each table bucket.

Query pipeline:

1. Hash the query in every table.
2. Collect vectors from matching buckets.
3. If fewer than `k` candidates are found, flip one hash bit at a time and check
   neighboring buckets.
4. Deduplicate candidate indices.
5. Exact-rerank candidates with L2.
6. Return the top `k`.

Why multiple tables:

A single projection hash can miss true neighbors. Multiple independent tables
increase the chance that a true neighbor collides in at least one table.

Why projections matter:

- More projections create more selective hashes and smaller buckets.
- Smaller buckets improve speed.
- Too many projections reduce collisions and hurt recall.
- Too few projections create huge buckets and behave like brute force.

Recorded SIFT1M LSH experiments from `docs/LSH_learnings.md`:

| Tables | Projections | MsPQ | Recall@10 | QPS |
| ---: | ---: | ---: | ---: | ---: |
| 10 | 12 | 33.98 | 85% | 29 |
| 10 | 12 | 30.30 | 82% | 33 |
| 8 | 12 | 26.58 | 78% | 37 |
| 8 | 8 | 54.19 | 90% | 18 |
| 8 | 16 | 7.93 | 55% | 126 |
| 9 | 18 | 5.99 | 52% | 166 |
| 10 | 20 | 4.33 | 50% | 230 |
| 4 | 4 | 140.35 | 96% | 7 |
| 4 | 10 | 23.63 | 67% | 42 |

Main lesson:

The useful operating region was around 8-10 tables and 10-12 projections for
the recorded SIFT1M runs. More projections raised QPS but reduced recall.

## Approximate Index: NSW

`NSW_Index` implements a Navigable Small World graph.

Parameters:

- `M`: target maximum number of neighbors.
- `efConstruction`: search width used during insertion.
- `efSearch`: default search width used during query.

Why NSW:

High-dimensional vector search is hard for tree structures because pruning
becomes weak as distances concentrate. NSW uses a proximity graph instead. Each
node stores links to nearby nodes, and search navigates through those links.

Insertion:

1. The first vector becomes the entry point.
2. For every new vector, search the current graph with `efConstruction`.
3. Connect the new node to selected nearby nodes.
4. Add reciprocal links.
5. Prune neighbor lists to keep graph degree bounded.

Search:

1. Start at the entry point.
2. Maintain candidate and result heaps.
3. Expand neighbors while candidates can improve the current result set.
4. Return the closest `k` results after translating internal node indices back
   to external IDs.

Why degree limits matter:

Without a degree limit, early nodes become huge hubs. Traversal then spends too
much time scanning high-degree neighbor lists, and the graph loses the point of
approximate search.

Why heuristic pruning matters:

Keeping only the closest neighbors can remove long-range bridge edges. The
implemented heuristic rejects a candidate if an already selected neighbor is
closer to that candidate than the query is. That preserves more diverse edges
and improves graph navigability.

Visited-set optimization:

NSW uses a generation counter in `m_visited` instead of clearing a visited set
for every query. This avoids an `O(N)` clear before each graph search.

Recorded SIFT1M NSW experiments from `docs/NSW_learnings.md`:

Naive degree limit, `M = 16`, `efConstruction = 200`, build time 1215.16 s:

| efSearch | MsPQ | Recall@10 | QPS |
| ---: | ---: | ---: | ---: |
| 10 | 0.172457 | 31% | 5798 |
| 25 | 0.310062 | 46% | 3225 |
| 50 | 0.493521 | 63% | 2026 |
| 100 | 0.750665 | 72% | 1332 |
| 200 | 1.28716 | 80% | 776 |
| 500 | 2.67246 | 86% | 374 |

Higher connectivity, `maxEdges = 1.5 * M`, build time 1433.71 s:

| efSearch | MsPQ | Recall@10 | QPS |
| ---: | ---: | ---: | ---: |
| 10 | 0.23625 | 53% | 4232 |
| 25 | 0.373767 | 73% | 2675 |
| 50 | 0.591347 | 85% | 1691 |
| 100 | 0.923099 | 92% | 1083 |
| 200 | 1.57705 | 96% | 634 |
| 500 | 3.37052 | 98% | 296 |

Heuristic pruning, `M = 16`, `efConstruction = 200`, build time 1195.29 s:

| efSearch | MsPQ | Recall@10 | QPS |
| ---: | ---: | ---: | ---: |
| 10 | 0.205985 | 53% | 4854 |
| 25 | 0.319483 | 72% | 3130 |
| 50 | 0.498650 | 84% | 2005 |
| 100 | 0.788994 | 93% | 1267 |
| 200 | 1.387550 | 97% | 720 |
| 500 | 2.974480 | 99% | 336 |

Main lesson:

`efSearch` directly trades latency for recall. Low values are fast and miss more
neighbors. High values inspect more of the graph and recover recall.

## Approximate Index: HNSW

`HNSW_Index` extends NSW with hierarchy.

Parameters:

- `M`: base graph connectivity target.
- `Mmax`: maximum edges above layer 0, currently `M`.
- `Mmax0`: maximum edges at layer 0, currently `2 * M`.
- `efConstruction`: insertion search width.
- `efSearch`: query search width.
- `mL`: layer normalization factor, currently `1 / log(M)`.

Why HNSW:

NSW has one graph. Search can still spend time moving from a poor entry region
to the right region. HNSW adds sparse upper layers. Higher layers contain fewer
nodes and act like express lanes. Search starts high, moves greedily toward the
query region, then descends to denser layers.

Layer selection:

Each inserted node receives a random maximum layer from an exponential
distribution. Most nodes exist only at layer 0. A small number of nodes become
long-range routing points in upper layers.

Insertion:

1. Select the new node's maximum layer.
2. If this is the first node, make it the entry point.
3. From the current top layer down to the new node's top layer, run greedy
   search with `ef = 1`.
4. From the new node's top layer down to layer 0, run wider search with
   `efConstruction`.
5. Select neighbors with the diversity heuristic.
6. Add reciprocal edges.
7. Prune overflowing neighbor lists.
8. If the new node is taller than the current entry point, promote it.

Query:

1. Start from the global entry point at the top layer.
2. Run greedy search with `ef = 1` down to layer 1.
3. Run layer-0 search with `efSearch`.
4. Return the first `k` candidates.

Current status:

- Header-only implementation in `include/vectorDB/HNSW_Index.hpp`.
- Used by `benchmarks/benchmark_hnsw.cpp`.
- Not currently exposed as a separate CMake library target.
- No HNSW save/load implementation yet.

Recorded HNSW notes:

- An early 10K-vector implementation reached 99% recall but only about
  127-131 QPS because query time stayed around 7.6-7.9 ms.
- A lean-node experiment storing only neighbor IDs improved 10K-vector QPS by
  about 30%, but the gain disappeared at 1M scale because recomputing distances
  caused large RAM fetches.
- A bug in distance hydration caused recall to collapse to about 69%; fixing the
  reference point restored recall to 99-100% in the development notes.
- With production-scale settings described in the notes, reducing
  `efConstruction` from 200 to 100 and compiling with aggressive optimization
  reduced build time from about 1190 s to about 760 s.

The old README recorded a high-level HNSW SIFT1M result of about 5794 QPS with
greater than 95% recall and 760.18 s build time for an optimized `-Ofast` run.
Treat that as a recorded project result, not a value recomputed by the current
benchmark in this checkout.

## Benchmark Metrics

The benchmark files use these metrics:

### MsPQ

Milliseconds per query.

```text
MsPQ = total query time in milliseconds / number of queries
```

### QPS

Queries per second.

```text
QPS = 1000 / MsPQ
```

The benchmark code writes integer QPS.

### Recall@10

For each query, compare the returned top 10 IDs against SIFT ground truth.

```text
Recall@10 = matching returned IDs / (number of queries * 10)
```

The ANN benchmarks currently evaluate 100 queries. The brute-force benchmark can
use the query count from the imported query set.

### Build Time

Wall-clock time in seconds to construct the index after importing the dataset.
Import time is not the main value being reported.

## Benchmark Summary

Recorded project-level SIFT observations:

| Method | Dataset | Representative Parameters | Build Time | Recall@10 | QPS | Why It Behaves This Way |
| --- | --- | --- | ---: | ---: | ---: | --- |
| Brute force | SIFT1M notes | Exact L2 scan | ~0 s index build | 100% | ~6 | Perfect recall but every query scans all vectors. |
| LSH | SIFT1M | 10 tables, 12 projections | ~6.38 s in old README notes | 82-85% | 29-33 | Hashing reduces candidates, but high-dimensional buckets still grow large. |
| NSW | SIFT1M | `M=16`, `efConstruction=200`, heuristic pruning | 1195.29 s | 93% at `efSearch=100` | 1267 | Graph routing is much faster than exhaustive scan; higher `efSearch` buys recall. |
| NSW | SIFT1M | same, `efSearch=25` | same build | 72% | 3130 | Lower search width improves QPS but loses recall. |
| HNSW | SIFT1M recorded notes | optimized `-Ofast`, `efConstruction=100`, `efSearch=50` | 760.18 s | >95% | 5794 | Hierarchy shortens routing path; memory layout still limits throughput. |

External libraries such as Faiss and nmslib are not benchmarked by this repo's
current CMake targets. The comparison notes in `why_not_apt.md` explain why they
can be faster: arena allocation, flatter memory layout, fewer cache misses,
prefetching, quantization, and more tuned SIMD.

## Why Production Libraries Are Still Faster

The implementation uses standard containers such as:

```cpp
std::vector<Node>
std::vector<std::pair<float, uint64_t>>
```

That is simple and readable, but at 1M nodes it causes many separate heap
allocations. Graph search is pointer-heavy: each hop loads a node, then follows
the node's neighbor-list allocation, then loads neighbor IDs. When the graph no
longer fits in cache, those random accesses stall on RAM.

Production systems often use:

- Arena allocation for graph data.
- Fixed-stride or compressed neighbor storage.
- Explicit prefetching.
- Quantized vector storage.
- Hand-tuned SIMD kernels.
- Parallel query execution.
- More careful batch construction.

The algorithmic idea can match HNSW, while the memory layout still leaves a
large performance gap.

## Importers

### SIFT

`Importer::import_sift1m()` reads:

```cpp
Result<SiftRes, ImporterError> import_sift1m(
    const std::string& data_file,
    const std::string& query_file,
    const std::string& truth_file,
    std::uint32_t k_imports = -1
);
```

`SiftRes` contains:

- `vectors`: base vectors as `Vector`.
- `ids`: generated numeric IDs.
- `queries`: query vectors.
- `truths`: ground-truth nearest-neighbor IDs.
- `truth_k`: number of ground-truth IDs per query.

The parser expects each vector record to begin with a 32-bit dimension field and
currently rejects dimensions other than 128.

### GLoVE

`Importer::import_glove()` reads text embeddings:

```cpp
Result<GloveRes, ImporterError> import_glove(const std::string& filename);
```

`GloveRes` contains:

- `word_to_id`
- `id_to_word`
- `vectors`
- `ids`

The parser uses `std::from_chars` for numeric conversion and expects 100 floats
after each word token.

## `SafeVector<T>`

`SafeVector<T>` is not used as the main storage engine. It is included as a
learning component for implementing vector-like ownership correctly.

It demonstrates:

- RAII with `std::unique_ptr<T[]>`.
- Copy constructor.
- Move constructor.
- Copy-and-swap assignment.
- `push_back` overloads for lvalues and rvalues.
- Bounds-checked `operator[]` and `at()`.
- `reserve`, `resize`, `clear`, `front`, `back`, `begin`, `end`.

Why it matters:

Implementing this container exposes the same ownership and exception-safety
issues that appear in larger storage systems. The tests verify copying, moving,
self-assignment, reallocation, iteration, and exception behavior.

## Development Notes

The `docs/` directory contains the reasoning trail behind many implementation
choices:

- `docs/archive_learnings.md`: CRC, append design, binary I/O, archive timings.
- `docs/brute_search_learnings.md`: result type, metric dispatch, import speed.
- `docs/LSH_learnings.md`: hash selectivity, hyperplane distribution, recall/QPS.
- `docs/NSW_learnings.md`: graph degree, heuristic pruning, efSearch tradeoffs.
- `docs/HNSW_learnings.md`: cache experiments, hydration bug, memory layout.
- `docs/safeVector_learnings.md`: Rule of Five and exception safety.
- `why_not_apt.md`: why this implementation trails Faiss/nmslib in throughput.

## Known Limitations

- Dataset and output paths in tests and benchmarks are hard-coded.
- The AVX2 L2 kernel needs dimensions divisible by 8 or a scalar tail fix.
- `VectorStore::remove()` does not currently remove the ID from `m_id_set`.
- `VectorStore::get()` is `O(N)` because there is no ID-to-position map.
- `VectorStore::save/load` does not currently verify CRCs.
- LSH and NSW serialization use exceptions, while `VectorStore` uses `Result`.
- HNSW is header-only and benchmark-only in CMake.
- HNSW persistence is not implemented.
- Query benchmarks are single-threaded.
- Graph storage uses heap-backed vectors rather than a flat arena layout.
- No package install/export target is defined yet.

## Roadmap

High-impact next steps:

1. Add a scalar tail path to the AVX2 L2 kernel.
2. Replace hard-coded benchmark/test paths with command-line arguments or CMake
   cache variables.
3. Fix `VectorStore::remove()` to erase IDs from `m_id_set`.
4. Add an ID-to-index map for faster `get()` and remove.
5. Add CRC verification to `VectorStore::save/load` or route persistence through
   one common format.
6. Add HNSW save/load tests.
7. Add a CMake library target for HNSW.
8. Introduce flat arena storage for graph neighbors.
9. Add multi-query parallel benchmark runners.
10. Record reproducible benchmark metadata: CPU, compiler, flags, dataset path,
    commit hash, parameters, query count, and random seed.

## Minimal API Example

```cpp
#include "vectorDB/VectorStore.hpp"

int main() {
    VectorStore store;

    store.insert(1, Vector{{1.0f, 0.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 0.0f}});
    store.insert(2, Vector{{0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 0.0f}});

    Vector query{{1.0f, 0.1f, 0.0f, 0.0f,
                  0.0f, 0.0f, 0.0f, 0.0f}};

    auto result = store.query(query, 1, Metric::L2);
    if (result.is_ok()) {
        auto neighbors = result.ok_value();
        // neighbors[0].first is the nearest ID
        // neighbors[0].second is the squared L2 distance
    }
}
```

## Minimal LSH Example

```cpp
#include "vectorDB/LSH_Index.hpp"

int main() {
    std::vector<std::pair<std::uint64_t, Vector>> data;
    data.push_back({1, Vector{{1, 0, 0, 0, 0, 0, 0, 0}}});
    data.push_back({2, Vector{{0, 1, 0, 0, 0, 0, 0, 0}}});

    LSHIndex index(10, 12);
    index.build(std::move(data));

    auto result = index.query(Vector{{1, 0, 0, 0, 0, 0, 0, 0}}, 1);
}
```

## Minimal NSW Example

```cpp
#include "vectorDB/NSW_Index.hpp"

int main() {
    NSW_Index index(16, 100, 50);
    index.insert(1, Vector{{1, 0, 0, 0, 0, 0, 0, 0}});
    index.insert(2, Vector{{0, 1, 0, 0, 0, 0, 0, 0}});

    auto result = index.query(Vector{{1, 0, 0, 0, 0, 0, 0, 0}}, 1);
}
```

## Minimal Archive Example

```cpp
#include "vectorDB/archive.hpp"

int main() {
    VectorArchive archive;

    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };

    archive.save("vectors.vecd", data);

    if (archive.verify("vectors.vecd")) {
        auto loaded = archive.load("vectors.vecd", true);
    }

    archive.append("vectors.vecd", {{7.0, 8.0, 9.0}});
}
```
