1. [Visual LSH](https://randorithms.com/2019/09/19/Visual-LSH.html)  
2. [Locality Sensitive Hashing (LSH): The Illustrated Guide](https://www.pinecone.io/learn/series/faiss/locality-sensitive-hashing/)  
3. [Random Projection for Locality Sensitive Hashing](https://www.pinecone.io/learn/series/faiss/locality-sensitive-hashing-random-projection/)  
4. [Sparse Implementation](https://github.com/pinecone-io/examples/tree/main/learn/search/faiss-ebook/locality-sensitive-hashing-traditional)  
5. [Random Projections](https://github.com/pinecone-io/examples/tree/main/learn/search/faiss-ebook/locality-sensitive-hashing-random-projection)

---

## **Observations**

**a)** Printing canditate set size gave me - **10k for 10k dataset, 100k .., 1m..** -> issue pointed by this is **LSH behaving at best equal to brute force.** Why would all vectors land in same bucket.  

`normal[k] = Random::get(-0.05f, 0.05f);`

This made **hyperplanes degernate** hence all vectors ended up to be at same side of all plane, ence same hash for all.  

*Soln* spread planes - `(0, 1)` or experimnet with other vals.  

---

**b)** Still scaling linearly. Print candidate set size again — **800 average at 10K, 8000 at 100K, 100K at 1M. Exactly proportional.** Basically our hash is unable to different finely.  

*Soln:* **Increase no of planes per table.**

---

**c)** Low recall, this is solved by relation from artcile above. Hence prpvides a tradeoff between **Recall and QPs.** Need to find a balance.  

Acc to my debugs, **table 8-10 and proj 10-12 proved as middle ground.**