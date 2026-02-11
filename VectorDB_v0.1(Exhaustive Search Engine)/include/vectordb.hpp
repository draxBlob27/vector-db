#include <cstdint>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <variant>
#include <vector>

struct Vector {
    std::vector<float> data;
    bool normalized;
    float norm() const {
        ;
    }
    void normalize();
};

enum class Metric {
    L2, 
    Cosine, 
    DotProduct
};

struct okTag {};
struct errTag {};

template <typename T, typename E>
struct Result {
private:
    std::variant<T, E> res;

public:
    Result(okTag, const T& t) :res{t} 
    {}

    Result (errTag, const E& e) :res{e}
    {}

    bool is_ok() const {
        return res.index() == 0;
    }

    const T& value() const {
        if (!is_ok()) {
            throw std::runtime_error("Value() called on error result.");
        }

        return std::get<0>(res);
    }

    const E& error() const {
        if (is_ok()) {
            throw std::runtime_error("Error() called on valid result.");
        }

        return std::get<1>(res);
    }
};

class VectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;

public:
    Result<void, std::runtime_error> insert(std::uint64_t id, const Vector& vector) {
        auto dims_valid{[&]() {
            return m_vectors[0].second.data.size() == vector.data.size();
        }};

        if (m_vectors.empty() || dims_valid()) {
            m_vectors.push_back({id, vector});
            return;
        }

        throw std::runtime_error("Dimensions mismatch.");
    }

    Result<void, std::runtime_error> remove(std::uint64_t id) {
        auto find_id{[&id](std::pair<std::uint64_t, Vector>& a) {
            return a.first == id;
        }};
        auto new_logical_end = std::remove_if(m_vectors.begin(), m_vectors.end(), find_id);
        // auto new_logical_end = std::ranges::remove_if(m_vectors, find_id);
        m_vectors.erase(new_logical_end, m_vectors.end());
    }   

    Result<std::vector<std::pair<std::uint64_t, float>>, std::runtime_error> query(const std::vector<float>& vector, int k = 10, Metric metric){
        
    }

    void save(const std::string& filename) {

    }

    void load(const std::string& filename) { //can use std::string_view -> improves perf, but does it matter that much. We'll seee
        
    }

    std::uint64_t size() {

    }

    std::uint64_t dimensions() {

    }

};

template <Metric M> //can use auto as well
float distance(const Vector& a, const Vector& b);

template<> float distance<Metric::L2>(const Vector& a, const Vector& b) {
    float res = 0;

    for (int i{0}; i < a.data.size(); i++) {
        res += (a.data[i] - b.data[i]) * (a.data[i] - b.data[i]);
    }

    return std::sqrt(res);
}

template<> float distance<Metric::Cosine>(const Vector& a, const Vector& b) {
    if (a.normalized && b.normalized) {
        return distance<Metric::DotProduct>(a, b);
    }

    float res = distance<Metric::DotProduct>(a, b);

    return res / (a.norm() * b.norm());
}

template<> float distance<Metric::DotProduct>(const Vector& a, const Vector& b) {
    float res = 0;

    for (int i{0}; i < a.data.size(); i++) {
        res += (a.data[i] - b.data[i]);
    }

    return res;
}