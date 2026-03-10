## **References**

1. [Pinecone article about HNSW](https://www.pinecone.io/learn/series/faiss/hnsw/).  
2. [Approximate nearest neighbor algorithm based on navigable small world graphs (2014)](https://publications.hse.ru/mirror/pubs/share/folder/x5p6h7thif/direct/128296059).    
3. [Approximate Nearest Neighbor Search Small World Approach(2011)](https://www.iiis.org/CDs2011/CD2011IDI/ICTA_2011/PapersPdf/CT175ON.pdf).   
4. [Visual explanation and high level overview by alex chi](https://skyzh.github.io/write-you-a-vector-db/cpp-06-01-nsw.html).    
5. [All pinecone code for FAISS](https://github.com/pinecone-io/examples/tree/main/learn/search/faiss-ebook)


## **Observations** 
**1.** Biggest issue, i got is, when connecting new incoming vector to already established graph. Problem was, suppose incoming is A, and established vector is X. Upon finding nearest vectors to A, we got X also. We happily connect A<->X, but now X has M+1 neighbors. So it must prune 1 edge. let that edge be connected to Y. And Y is a distant node, and the edge X<->Y acts as a bridge between the two components. Now if i remove this edge blindly beacuse it is far. Then our graph becomes disconnected and we are doooooomed.    
**2** Biggest need for NSW, was i think Curse of Dimensionality.
