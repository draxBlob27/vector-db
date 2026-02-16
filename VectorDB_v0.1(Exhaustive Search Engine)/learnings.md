1. [Erase Remove idiom](https://www.geeksforgeeks.org/cpp/erase-remove-idiom-in-cpp/)

2. std::variant handling was confusing, required help from articles + AI. [Modern C++: Error Handling with Result Type](https://yegor.pomortsev.com/post/result-type/), [C++ void template parameter](https://lifecs.likai.org/2010/04/c-void-template-parameter.html)

3. To handle result<void, E> -> need to write template specialization. 

4. I was mixing, runtime polymorphism and compile time polymorhism.
I declared distance functions as templated -> template paramter Metric must be provided at compile time
My API or method in VectorStore was expecting metric from user -> runtime variable
Hence that was a design issue.
So for runtime dispatch i would use switch statements.
[when-is-virtual-dispatch-faster-than-function-templates-in-c-runtime](https://stackoverflow.com/questions/79219324/when-is-virtual-dispatch-faster-than-function-templates-in-c-runtime)

5. std::from_chars very strealined value extraction from text files. (https://learnmoderncpp.com/2021/01/22/migrating-towards-from_chars/)


6. Although my goal for current implementation was just correctness, but using find -> O(n) lookup for id exists was very slow for 1.2M vectors so bascially it was O(n^2) and i would take me hours to even insert GLoVE in my VectorStore. So i shifted to using hashing. And it was able to load dataset in 18 secs.