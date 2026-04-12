#include "vectorDB/ThreadSafeVectorStore.hpp"

Result<Unit, DBError> ThreadSafeVectorStore::insert(std::uint64_t id, Vector i_vector) { //need to handle Id already exists, here my DB is going to own input_vector, hence rref makes sense, but pass by value is equally good. Because, internal calls are passed by rvalue hence, move semantics(2 moves), and explicit call by user, does 1 copy + 1 move, and user calls are limited to very less. If want to keep rref, user has to pass rvalue, otherwise compile error(no defn of func).
    Result<Unit, DBError> res = Ok<Unit>{Unit{}};
    {
        // std::cout << "Inserting: \n";
        std::unique_lock<std::shared_mutex> lk{entry_mutex}; //ONLY 1 writer allowed, no readers allowed.
    
        res = vdb.insert(id, std::move(i_vector));
    }
    // std::cout << "released writer lock\n";
    if (res.is_ok())
        data_cond.notify_all();

    return res;
}

Result<Unit, DBError> ThreadSafeVectorStore::remove(std::uint64_t id) {
    std::unique_lock<std::shared_mutex> lk{entry_mutex}; //ONLY 1 writer allowed, no readers allowed.
    return vdb.remove(id);
}   

Result<std::vector<float>, DBError> ThreadSafeVectorStore::get(std::uint64_t id) const {
    std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

    return vdb.get(id);
}

Result<std::vector<std::pair<std::uint64_t, float>>, DBError> ThreadSafeVectorStore::query (Vector q_vector, std::uint64_t k, Metric metric) const {//very large object is getting created, can think of move semantics
    //very high chances of using quick select, just saw LC soln today(12 feb, 2026) regarding this
    //saying quick select is best in terms of TC ~ O(n) for best k kinda things
    std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.
    // data_cond.wait_for(lk, std::chrono::seconds(1), [&]{return !(m_vectors.size() < 100000);});
    // std::cout << "Checking with current db size: " << m_vectors.size() << '\n';
    data_cond.wait(lk, [&]{return vdb.size().ok_value() >= 100000;});
    // std::cout << "Exit waiting with current db size: " << m_vectors.size() << '\n';

    return vdb.query(std::move(q_vector), k, metric);
}

Result<std::vector<std::pair<std::uint64_t, float>>, DBError> ThreadSafeVectorStore::query_parallel(Vector q_vector, std::uint64_t k, Metric metric) const {//very large object is getting created, can think of move semantics
    std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

    // std::cout << "Checking with current db size: " << m_vectors.size() << '\n';
    data_cond.wait(lk, [&]{return vdb.size().ok_value() >= 100000;});
    // std::cout << "Exit waiting with current db size: " << m_vectors.size() << '\n';

    return vdb.query_parallel(std::move(q_vector), k, metric);
}


Result<Unit, DBError> ThreadSafeVectorStore::save(const std::string& filename) const {
    std::unique_lock<std::shared_mutex> lk{entry_mutex}; //ONLY 1 writer allowed, no readers allowed.

    return vdb.save(filename);
}

Result<Unit, DBError> ThreadSafeVectorStore::load(const std::string& filename) { //can use std::string_view -> improves perf, but does it matter that much. We'll seee
    std::unique_lock<std::shared_mutex> lk{entry_mutex}; //ONLY 1 writer allowed, no readers allowed.

    return vdb.load(filename);
}