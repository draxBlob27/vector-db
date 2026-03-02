#ifndef VECTORDB_HPP
#define VECTORDB_HPP
#include <cstdint>
#include <ios>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <vector>
#include "Vector.hpp"

//TODO -> code is still redundant, can use lamdas to fix, Will se later on.
//TODO -> apply buffer in writing, using struct types to handle {id, vector of float}, could also use byte buffer which cares only take bytes into acounnt.
//TODO -> Add reserve() for bulk loading
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

enum class Metric {
    L2, //smaller is better
    Cosine, //larger is better
    DotProduct //larger is better
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

struct Info {
    std::uint64_t size;
    std::uint64_t dims;
    std::uint64_t bytes;
};

class VectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;
    std::unordered_set<std::uint64_t> m_id_set;
    static const inline std::uint32_t s_magic_bytes{0x56454344};
    static const inline std::uint32_t s_version{1};

public:
    Result<Unit, DBError> insert(std::uint64_t id, Vector i_vector);

    Result<Unit, DBError> remove(std::uint64_t id);

    Result<std::vector<float>, DBError> get(std::uint64_t id) const;

    Result<std::vector<std::pair<std::uint64_t, float>>, DBError> query (const Vector& q_vector, std::uint64_t k = 10, Metric metric = Metric::Cosine) const ;

    Result<Unit, DBError> save(const std::string& filename) const;

    Result<Unit, DBError> load(const std::string& filename);

    Result<std::uint64_t, DBError> size() const {
        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        return Ok{static_cast<std::uint64_t>(m_vectors.size())};
    }

    Result<Info, DBError> info() const {
        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        return Ok{Info{
            size().ok_value(),
            dimensions().ok_value(),
            (size().ok_value() * (dimensions().ok_value() + 3) * sizeof(float)) + sizeof(s_magic_bytes) + sizeof(s_version)
        }};
    }

    Result<std::uint64_t, DBError> dimensions() const {
        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        return Ok{static_cast<std::uint64_t>(m_vectors[0].second.data.size())};
    }
};
#endif