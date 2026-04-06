# Resources
[Simple generic parallelism idiom & C++17 specifics](https://berthub.eu/articles/posts/simple-parallelism-idiom/#:~:text=For%20Linux%20and%20gcc%2C%20out,Summarising)
[OneTBB lib tutorial](https://uxlfoundation.github.io/oneTBB/index.html)
Book:= c++ concurrency in action(chapter 1,2,3,4,5(5.1, - 5.3) at least)

# Where is concurrency actually required!!!
* Concurrency should be first studied to think where it is actually needed.
* It is not required in:
1. Building the db, as pushing incomgin vector is a critical task, there is no benefit of spawning threads and then waiting for each thread to push to same shared data member.
2. Even though while inserting, norm is calculated, parallelizing this will require spawning threads overhead, and also it is premature optimization.
3. Serialization(Saving to disc), this is inhrerently serial task so no point of involving concurrency here.
4. loading from disc, this can be parallelised, but threads will be waiting for io operation most of the time, so bottleneck here isnt computation, but IO opr. So concurrency is not a correct tool to use here.

* This leaves only query searching to be parallelized. 
* We have effectively narrowed down our area of concern.

# Learnings
* Correct place to implement concurrency is in building the database and in query retrieval.
* Mental model should be to make DS thread safe while improving performance. 
* Multiple threads should work to build DS, and multiple threads should be allowed to read it while querying lock free or with a shared mutex.
* Using mutex is not free lunch, it also comes with its own overhead, which should be less than the performance gains.
* Thing to beware while writing concurrent code, is to handle shared data access. Read chapter 3 from book.
