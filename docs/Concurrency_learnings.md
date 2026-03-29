# Resources
[Simple generic parallelism idiom & C++17 specifics](https://berthub.eu/articles/posts/simple-parallelism-idiom/#:~:text=For%20Linux%20and%20gcc%2C%20out,Summarising)
[OneTBB lib tutorial](https://uxlfoundation.github.io/oneTBB/index.html)

# Where is concurrency actually required!!!
* Concurrency should be first studied to think where it is actually needed.
* It is not required in:
1. Building the db, as pushing incomgin vector is a critical task, there is no benefit of spawning threads and then waiting for each thread to push to same shared data member.
2. Even though while inserting, norm is calculated, parallelizing this will require spawning threads overhead, and also it is premature optimization.
3. Serialization(Saving to disc), this is inhrerently serial task so no point of involving concurrency here.
4. loading from disc, this can be parallelised, but threads will be waiting for io operation most of the time, so bottleneck here isnt computation, but IO opr. So concurrency is not a correct tool to use here.

* This leaves only query searching to be parallelized. 
* We have effectively narrowed down our area of concern.