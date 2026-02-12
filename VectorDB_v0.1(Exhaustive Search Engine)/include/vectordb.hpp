#include <cstdint>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <queue>
#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

//TODO -> handle diff k values.

enum class Metric {
    L2, //smaller is better
    Cosine, //larger is better
    DotProduct //larger is better
}; 

// typedef int32_t status_t;
// #define S_BAD_METRIC (-3)

template <typename T>
class Ok {
    T value;

public:

    explicit Ok(T value) //disallows implicit conversion. To not get surprised by compiler implicit conversions
        :value{std::move(value)}
    {}

    T copy_value() const {
        return value;
    }

    T&& take_value() {
        return std::move(value);
    }
};

template <typename T>
class Err {
    T value;
    
    public:
    explicit Err(T value)
    :value{std::move(value)}
    {}
    
    T copy_value() const {
        return value;
    }
    
    T&& take_value() {
        return std::move(value);
    }
};

template <typename OkT, typename ErrT>
class Result {
    std::variant<Ok<OkT>, Err<ErrT>> variant;
    
    public:
    Result(Ok<OkT> value)
    :variant(std::move(value))
    {}
    
    Result(Err<ErrT> value)
    :variant(std::move(value))
    {}
    
    bool is_ok() const {
        return std::holds_alternative<Ok<OkT>>(variant);
    }
    bool is_err() const {
        return std::holds_alternative<Err<ErrT>>(variant);
    }
    OkT ok_value() const {
        return std::get<Ok<OkT>>(variant).copy_value(); //returns a copy, throws upon wrong call.
    }
    ErrT err_value() const{
        return std::get<Err<ErrT>>(variant).copy_value();
    }
    
    OkT&& take_ok_value() {
        return std::get<Ok<OkT>>(variant).take_value(); //returns ownership, throws upon wrong call, after operation class value is invalid/empty;
    }
    ErrT&& take_err_value() {
        return std::get<Err<ErrT>>(variant).take_value();
    }
};

struct Unit{}; //replaces void return type with empty class type, inspried from article linked.
template<typename ErrT>
class Result<Unit, ErrT> {
    std::variant<Ok<Unit>, Err<ErrT>> variant;
    
    public:
    Result(Ok<Unit> value)
    :variant(std::move(value))
    {}
    
    Result(Err<ErrT> value)
    :variant(std::move(value))
    {}
    
    bool is_ok() const {
        return std::holds_alternative<Ok<Unit>>(variant);
    }
    bool is_err() const {
        return std::holds_alternative<Err<ErrT>>(variant);
    }

    Unit ok_value() const {
        return Unit{}; 
    }
    ErrT err_value() const{
        return std::get<Err<ErrT>>(variant).copy_value();
    }
    
    Unit&& take_ok_value() {
        return Unit{};
    }
    ErrT&& take_err_value() {
        return std::get<Err<ErrT>>(variant).take_value();
    }
};

enum class DBError : std::int32_t {
    MetricError = (-1),
    DimensionError = (-2),
    IdNotFoundError = (-3),
    ZeroNormError = (-4),
    DataBaseEmptyError = (-5)
};

struct Vector {
    std::vector<float> data;
    float norm_data{0.0f};
    bool normalized{false};

    float norm() const {
        return norm_data;
    }

    void computed_norm() {
        for (const auto& it : data) {
            norm_data += (it * it);
        }

        norm_data = std::sqrt(norm_data);
        normalized = true;
    };
};

class VectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;

public:
    Result<Unit, DBError> insert(std::uint64_t id, Vector& i_vector) { //need to handle Id already exists
        auto dims_valid{[&]() {
            return m_vectors[0].second.data.size() == i_vector.data.size();
        }};

        if (m_vectors.empty() || dims_valid()) {
            i_vector.computed_norm(); //normalise at insgestion

            m_vectors.push_back({id, std::move(i_vector)}); //used move because it could be(i think..) wil see later on).
            return Ok<Unit>(Unit{});
        }

        return Err<DBError>(DBError::DimensionError);
    }

    Result<Unit, DBError> remove(std::uint64_t id) {
        auto find_id{[&id](std::pair<std::uint64_t, Vector>& a) {
            return a.first == id;
        }};
        auto new_logical_end = std::remove_if(m_vectors.begin(), m_vectors.end(), find_id);
        // auto new_logical_end = std::ranges::remove_if(m_vectors, find_id);

        if (new_logical_end == m_vectors.end()) { //incase of id not found
            return Err<DBError>(DBError::IdNotFoundError);
        }

        m_vectors.erase(new_logical_end, m_vectors.end());
        return Ok<Unit>(Unit{});
    }   

    Result<std::vector<std::pair<std::uint64_t, float>>, DBError> query(Vector& q_vector, int k = 10, Metric metric) {//very large object is getting created, can think of move semantics
        //very high chances of using quick select, just saw LC soln today(12 feb, 2026) regarding this
        //saying quick select is best in terms of TC ~ O(n) for best k kinda things
        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        std::vector<std::pair<std::uint64_t, float>> res(std::min(static_cast<uint64_t>(k), size())); //handles if DB size is less than k.
        int i{res.size() - 1};

        switch (metric)
        {
        case(Metric::L2): {
            std::priority_queue<std::pair<float, std::uint64_t>> pq;
            
            if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
                return Err<DBError>{DBError::DimensionError};
            }

            for (const auto& it : m_vectors) {
                float distance{0.0f};
                /* code */
                //it.first -> id
                //it.second -> Vector
                
                for (int i{0}; i < it.second.data.size(); i++) {
                    distance += (it.second.data[i] - q_vector.data[i]) * (it.second.data[i] - q_vector.data[i]);
                }

                if (pq.size() < res.size()) { //smaller is better
                    pq.push({distance, it.first}); //holds the id
                } else if (pq.top().first > distance) {
                    pq.pop();
                    pq.push({distance, it.first});
                }
            }

            while (!pq.empty()) {
                res[i--] = {pq.top().second, pq.top().first};
                pq.pop();
            }
            break;
        }
        case(Metric::DotProduct): {
            std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> pq;
            
            if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
                return Err<DBError>{DBError::DimensionError};
            }

            for (const auto& it : m_vectors) {
                float distance{0.0f};
                /* code */
                //it.first -> id
                //it.second -> Vector

                for (int i{0}; i < it.second.data.size(); i++) {
                    distance += (it.second.data[i] * q_vector.data[i]);
                }

                if (pq.size() < res.size()) { //larger is better due to similarity -> vector more aligned
                    pq.push({distance, it.first}); //holds the id
                } else if (pq.top().first < distance) {
                    pq.pop();
                    pq.push({distance, it.first});
                }
            }

            while (!pq.empty()) {
                res[i--] = {pq.top().second, pq.top().first};
                pq.pop();
            }
            break;
        }
        case (Metric::Cosine): {
            std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> pq;
            
            q_vector.computed_norm(); //perform normailzation of query vector
            float query_norm{q_vector.norm()};
            //if query norm is 0 then outright riject it.
            if (query_norm == 0.0f) {
                return Err<DBError>{DBError::ZeroNormError};
            }

            if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
                return Err<DBError>{DBError::DimensionError};
            }
            
            for (const auto& it : m_vectors) {
                float distance{0.0f};
                /* code */
                //it.first -> id
                //it.second -> Vector
                if (it.second.norm() == 0.0f) {
                    continue;
                }

                for (int i{0}; i < it.second.data.size(); i++) {
                    distance += (it.second.data[i] * q_vector.data[i]);
                }

                distance /= (it.second.norm() * query_norm);

                if (pq.size() < res.size()) {
                    pq.push({distance, it.first}); //holds the id
                } else if (pq.top().first < distance) {
                    pq.pop();
                    pq.push({distance, it.first});
                }
            }

            while (!pq.empty()) {
                res[i--] = {pq.top().second, pq.top().first};
                pq.pop();
            }
            break;
        }
        default:
            return Err<DBError>{DBError::MetricError};
        }

        return Ok(res);
    }

    void save(const std::string& filename) {

    }

    void load(const std::string& filename) { //can use std::string_view -> improves perf, but does it matter that much. We'll seee
        
    }

    std::uint64_t size() {
        return m_vectors.size();
    }

    std::uint64_t dimensions() {
        return m_vectors[0].second.data.size();
    }

};

