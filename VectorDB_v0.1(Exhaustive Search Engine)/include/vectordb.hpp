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

struct Unit{};
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


class VectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;

public:
    Result<Unit, std::runtime_error> insert(std::uint64_t id, const Vector& vector) {
        auto dims_valid{[&]() {
            return m_vectors[0].second.data.size() == vector.data.size();
        }};

        if (m_vectors.empty() || dims_valid()) {
            m_vectors.push_back({id, vector});
            return;
        }

        throw std::runtime_error("Dimensions mismatch.");
    }

    Result<Unit, std::runtime_error> remove(std::uint64_t id) {
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