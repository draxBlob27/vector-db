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

7. Importance inline in static classes. Static members must still be defined somewhere. Inside the class you only declare, This does not create storage for them, You must define them in a .cpp file. inline makes it a definition.

8. Some delightfull expereince.
```
Running main() from ./googletest/src/gtest_main.cc
[==========] Running 3 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 3 tests from VectorStore_test
[ RUN      ] VectorStore_test.Correct_Import
[       OK ] VectorStore_test.Correct_Import (0 ms)
[ RUN      ] VectorStore_test.Similarity_Test_1
Id: 703
Word: king
Score: 14.1851
Id: 2112
Word: queen
Score: 17.2894
Id: 7055
Word: throne
Score: 24.8185
Id: 3116
Word: elizabeth
Score: 25.4592
Id: 1108
Word: daughter
Score: 26.0376
Id: 5244
Word: margaret
Score: 26.4619
Id: 13797
Word: niece
Score: 27.5969
Id: 780
Word: mother
Score: 27.7576
Id: 9117
Word: monarch
Score: 27.8991
Id: 12666
Word: granddaughter
Score: 28.3897
[       OK ] VectorStore_test.Similarity_Test_1 (155 ms)
[ RUN      ] VectorStore_test.Result_match_1
[       OK ] VectorStore_test.Result_match_1 (161 ms)
[----------] 3 tests from VectorStore_test (322 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 1 test suite ran. (23388 ms total)
[  PASSED  ] 3 tests.
sp27022003@sanil-cpp-2026:~/vector-db/VectorDB_v0.1(Exhaustive Search Engine)/build$ 
```

9. TODO -> QPS for glove
10. ``` cmake -DCMAKE_BUILD_TYPE=Release .. ```