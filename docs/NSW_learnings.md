# HNSW / NSW Implementation Notes

## References

1. [Pinecone article about HNSW](https://www.pinecone.io/learn/series/faiss/hnsw/)
2. [Approximate Nearest Neighbor Algorithm Based on Navigable Small World Graphs (2014)](https://publications.hse.ru/mirror/pubs/share/folder/x5p6h7thif/direct/128296059)
3. [Approximate Nearest Neighbor Search Small World Approach (2011)](https://www.iiis.org/CDs2011/CD2011IDI/ICTA_2011/PapersPdf/CT175ON.pdf)
4. [Visual explanation and high level overview by Alex Chi](https://skyzh.github.io/write-you-a-vector-db/cpp-06-01-nsw.html)
5. [Pinecone FAISS example code](https://github.com/pinecone-io/examples/tree/main/learn/search/faiss-ebook)
6. [Similarity Search, Hierarchical Navigable Small World (HNSW) -- explains heurisitc](https://towardsdatascience.com/similarity-search-part-4-hierarchical-navigable-small-world-hnsw-2aad4fe87d37/)
---

# Observations

## 1. Graph Connectivity Issue During Insertion

The biggest issue encountered was **connecting a new incoming vector into the existing graph**.

Suppose:

* Incoming vector: **A**
* Existing node: **X**

While inserting **A**, we search for nearest neighbors and **X** is among them.

We connect:

```
A <-> X
```

However, if **X already has M neighbors**, the new connection causes:

```
deg(X) = M + 1
```

So we must **prune one edge**.

Consider that the removed edge is:

```
X <-> Y
```

If **Y is a distant node but acts as a bridge between two graph regions**, removing it may cause:

* **Graph disconnection**
* Search becoming **trapped in local regions**

Result:

> The ANN graph becomes partially disconnected → search quality collapses.

---

# Motivation Behind NSW

The key motivation behind NSW is addressing the **Curse of Dimensionality**.

Traditional structures fail in high dimensions because:

* Tree pruning becomes ineffective
* Distances concentrate
* Exhaustive search becomes expensive

NSW solves this by building a **navigable proximity graph**.

---

# Iteration History

## First Iteration — Unbounded Degree Graph

The initial implementation **did not limit the number of edges per node**.

This created a serious issue:

* Entry point node accumulated **a huge number of neighbors**
* Some nodes had **> 50% of dataset as neighbors**

This resulted in:

* Extremely **dense graph**
* Massive computation during traversal
* Very slow search

### Result

On **SIFT1M** dataset:

```
QPS ≈ 4
```

Which is **worse than exhaustive search**.

---

# Fix 1 — Limit Maximum Edges

To fix the above problem, I introduced a **maximum number of edges per node**.

```
maxEdges = M
```

---

# Benchmarks (maxEdges = M)

Dataset: **SIFT1M**
Parameters: **M = 16, efConst = 200**

Build Time: **1215.16 seconds**

| efSearch | MsPQ (ms) | Recall@10 | QPS  |
| -------- | --------- | --------- | ---- |
| 10       | 0.172457  | 31%       | 5798 |
| 25       | 0.310062  | 46%       | 3225 |
| 50       | 0.493521  | 63%       | 2026 |
| 100      | 0.750665  | 72%       | 1332 |
| 200      | 1.28716   | 80%       | 776  |
| 500      | 2.67246   | 86%       | 374  |

---

# Fix 2 — Increase Maximum Edges

Next experiment:

```
maxEdges = 1.5 * M
```

This produced significantly better recall.

---

# Benchmarks (maxEdges = 1.5M)

Dataset: **SIFT1M**
Parameters: **M = 16, efConst = 200**

Build Time: **1433.71 seconds**

| efSearch | MsPQ (ms) | Recall@10 | QPS  |
| -------- | --------- | --------- | ---- |
| 10       | 0.23625   | 53%       | 4232 |
| 25       | 0.373767  | 73%       | 2675 |
| 50       | 0.591347  | 85%       | 1691 |
| 100      | 0.923099  | 92%       | 1083 |
| 200      | 1.57705   | 96%       | 634  |
| 500      | 3.37052   | 98%       | 296  |

---

# Tradeoff Observed

Increasing graph connectivity:

* improves **recall**
* increases **build time**
* reduces **QPS**

Through experiments:

```
maxEdges = 1.5M
```

gave the **best balance between recall and search speed**.

---

# Problem with Naive Pruning

The pruning strategy used so far:

> Keep the **closest neighbors** until the node has `1.5M` edges.

However this **does not consider graph connectivity**.

This caused:

* Important **long-range bridges** to be removed
* Reduced graph navigability

Observed effect:

```
Recall ceiling ≈ 98%
```

Even with very large `efSearch`.

---

# Solution — Heuristic Pruning (Paper Method)

The original **HNSW paper proposes heuristic neighbor selection**.

Instead of simply picking the closest neighbors, the algorithm:

* Preserves **diversity of neighbors**
* Keeps **long-range edges**
* Improves **navigability of the graph**

---
# Benchmarks (Heuristic Pruning)

Dataset: **SIFT1M**
Parameters: **M = 16, efConst = 200**

Build Time: **1195.29 seconds**

| efSearch | MsPQ (ms) | Recall@10 | QPS  |
| -------- | --------- | --------- | ---- |
| 10       | 0.205985  | 53%       | 4854 |
| 25       | 0.319483  | 72%       | 3130 |
| 50       | 0.498650  | 84%       | 2005 |
| 100      | 0.788994  | 93%       | 1267 |
| 200      | 1.387550  | 97%       | 720  |
| 500      | 2.974480  | 99%       | 336  |

---

# Observation on Heuristic Pruning

The **heuristic neighbor selection** proposed in the paper is primarily designed to improve performance at **high recall operating points**.

Its main purpose is to:

* preserve **diverse neighbors**
* maintain **long-range graph connections**
* improve **navigability of the graph**

However, the paper notes that this heuristic is **not optimized for low `efSearch` settings**, where the goal is **very fast approximate search**.

At low `efSearch`:

* search explores only a **small portion of the graph**
* benefits of improved graph connectivity are **less pronounced**

As `efSearch` increases:

* the search explores **larger candidate sets**
* the **diverse neighbor structure** created by heuristic pruning becomes more beneficial
* resulting in **higher recall**

This matches the experimental observations where recall improvements become most visible at **higher `efSearch` values**.

### Additional Observations from Experiments

After introducing heuristic pruning, the following effects were observed:

* **Faster build time**
* **Higher QPS**
* **Improved recall at high-recall operating points**

This indicates that heuristic neighbor selection not only improves **graph navigability**, but also leads to a **better balanced graph structure**, reducing unnecessary edges while preserving important long-range connections.
