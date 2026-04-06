#ifndef THREADSAFE_VECTORDB_HPP
#define THREADSAFE_VECTORDB_HPP
#include <cstdint>
#include <ios>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <cmath>
#include <fstream>
#include <atomic>
#include <cmath>
#include <queue>
#include <atomic>
#include <execution>
#include <unordered_map>
#include <vector>
#include <condition_variable>
#include "utils/Vector.hpp"
#include "utils/Metric.hpp"
#include "utils/Result.hpp"
#include "vectorDB/utils/distances.hpp"

/*
    |-------------------------------------------------------------------------------------------------------|
    |   Because the system is bound to be, insert and query, majority of times.                             |
    |   I didnt focused expilictly on making use of hash based data structures for storing incoming vectors,|
    |   which would have made removal, extraction in O(1). But that is not the primary aim of this part.    |
    |   Primary aim is to do O(n) query search.                                                             |
    |   My target is to measure QPS metrics, rather than removal and finding a single vector.                |
    |-------------------------------------------------------------------------------------------------------|
*/


enum class DBError : std::int32_t {
    MetricError = (-1),
    DimensionError = (-2),
    IdNotFoundError = (-3),
    ZeroNormError = (-4),
    DataBaseEmptyError = (-5),
    FileCorrupted = (-6),
    IdAlreadyPresent = (-7),
    FileNotFound = (-8)
};

std::ostream& operator<<(std::ostream& out, const DBError& err);

struct Info {
    std::uint64_t size;
    std::uint64_t dims;
    std::uint64_t bytes;
};

class ThreadSafeVectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;
    std::unordered_set<std::uint64_t> m_id_set;
    static const inline std::uint32_t s_magic_bytes{0x56454344};
    static const inline std::uint32_t s_version{1};
    mutable std::atomic<int> global_tcnt{0};
    mutable std::shared_mutex entry_mutex;
    mutable std::condition_variable_any data_cond;

    std::uint64_t int_size() const {
        return m_vectors.size();
    }

    std::uint64_t int_dimensions() const {
        return m_vectors[0].second.size();
    }

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

        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        return Ok{int_size()};
    }

    Result<Info, DBError> info() const {
        std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        return Ok{Info{
            int_size(),
            int_dimensions(),
            (int_size() * int_dimensions() + 3) * sizeof(float) + sizeof(s_magic_bytes) + sizeof(s_version)
        }};
    }

    Result<std::uint64_t, DBError> dimensions() const {
        std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.

        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        return Ok{int_dimensions()};
    }

    void get_global_ctr() const {
        std::shared_lock<std::shared_mutex> lk{entry_mutex}; //Multiple readers allowed.
        std::cout << global_tcnt << '\n';
    }
};
#endif //THREADSAFE_VECTORDB_HPP