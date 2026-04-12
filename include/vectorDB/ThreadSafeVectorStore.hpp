#ifndef THREADSAFE_VECTORDB_HPP
#define THREADSAFE_VECTORDB_HPP
#include <condition_variable>
#include <shared_mutex>
#include "VectorStore.hpp"

class ThreadSafeVectorStore {
private:
    VectorStore vdb;
    mutable std::shared_mutex entry_mutex;
    mutable std::condition_variable_any data_cond;

public:
    Result<Unit, DBError> insert(std::uint64_t id, Vector i_vector);

    Result<Unit, DBError> remove(std::uint64_t id);

    Result<std::vector<float>, DBError> get(std::uint64_t id) const;

    //can definitely have overloads for const lvalue ref, or a rvalue.
    //So we have segregation of tasks, one for owning and one for reading.
    Result<std::vector<std::pair<std::uint64_t, float>>, DBError> query_parallel (Vector q_vector, std::uint64_t k = 10, Metric metric = Metric::L2) const;

    Result<std::vector<std::pair<std::uint64_t, float>>, DBError> query (Vector q_vector, std::uint64_t k = 10, Metric metric = Metric::L2) const;

    Result<Unit, DBError> save(const std::string& filename) const;

    Result<Unit, DBError> load(const std::string& filename);

    Result<std::uint64_t, DBError> size() const {
        std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

        return vdb.size();
    }

    Result<Info, DBError> info() const {
        std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

        return vdb.info();
    }

    Result<std::uint64_t, DBError> dimensions() const {
        std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

        return vdb.dimensions();
    }
};
#endif //THREADSAFE_VECTORDB_HPP