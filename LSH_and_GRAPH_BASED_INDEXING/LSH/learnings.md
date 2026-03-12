# LSH Experiments

## Benchmarks

### Index Size: 1M vectors
---
| Tables | Projections | MsPQ (ms) | Recall@10 | QPS |
| ------ | ----------- | --------- | --------- | --- |
| 10     | 12          | 33.98     | 85%       | 29  |
| 10     | 12          | 30.30     | 82%       | 33  |
| 8      | 12          | 26.58     | 78%       | 37  |
| 8      | 8           | 54.19     | 90%       | 18  |
| 8      | 16          | 7.93      | 55%       | 126 |
| 8      | 14          | 16.04     | 67%       | 62  |
| 9      | 16          | 11.70     | 61%       | 85  |
| 9      | 18          | 5.99      | 52%       | 166 |
| 9      | 17          | 7.65      | 54%       | 130 |
| 10     | 20          | 4.33      | 50%       | 230 |
| 10     | 16          | 14.28     | 67%       | 70  |
| 4      | 4           | 140.35    | 96%       | 7   |
| 4      | 8           | 33.02     | 72%       | 30  |
| 4      | 10          | 23.63     | 67%       | 42  |
---

# References

1. [Visual LSH](https://randorithms.com/2019/09/19/Visual-LSH.html)
2. [Locality Sensitive Hashing (LSH): The Illustrated Guide](https://www.pinecone.io/learn/series/faiss/locality-sensitive-hashing/)
3. [Random Projection for Locality Sensitive Hashing](https://www.pinecone.io/learn/series/faiss/locality-sensitive-hashing-random-projection/)
4. [Sparse Implementation](https://github.com/pinecone-io/examples/tree/main/learn/search/faiss-ebook/locality-sensitive-hashing-traditional)
5. [Random Projections](https://github.com/pinecone-io/examples/tree/main/learn/search/faiss-ebook/locality-sensitive-hashing-random-projection)

---

# Observations

## a) Degenerate Hyperplanes

While debugging, I printed the **candidate set size** and observed:

```
10K dataset  → ~10K candidates
100K dataset → ~100K candidates
1M dataset   → ~1M candidates
```

This indicated that **LSH was behaving like brute force search**.

### Root Cause

Hyperplanes were generated using:

```cpp
normal[k] = Random::get(-0.05f, 0.05f);
```

This caused **degenerate hyperplanes**.

Since all projection values were extremely small:

* Hyperplanes were almost flat
* All vectors fell on the **same side of the planes**
* Every vector produced **the same hash**

Result:

```
All vectors landed in the same bucket
```

### Solution

Increase hyperplane spread.

Example:

```
Random(0,1)
```

or experiment with other wider ranges.

---

# b) Candidate Set Still Scaling Linearly

Even after fixing the plane distribution, candidate size behaved as:

```
10K dataset  → ~800 candidates
100K dataset → ~8000 candidates
1M dataset   → ~100000 candidates
```

This shows **linear growth with dataset size**, meaning the hash was still **not discriminative enough**.

### Solution

Increase the **number of projections per table**.

More projections:

* increases hash selectivity
* reduces bucket size
* reduces candidate set

---

# c) Recall vs QPS Tradeoff

Increasing projections improves **selectivity**, but also:

* reduces recall
* reduces candidate overlap across tables

Therefore we get the classic LSH tradeoff:

```
More projections → higher speed, lower recall
Fewer projections → higher recall, slower search
```

Based on experiments:

```
Tables: 8–10
Projections: 10–12
```

gave the best **middle ground between recall and QPS**.