#include <cstdint>
#include <ios>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <vector>

//TODO -> handle diff k values.
//TODO -> code is still redundant, can use lamdas to fix, Will se later on.
//TODO -> handle unique id, also to handle remove_if by find_if, using the fact that id is unique.
//TODO -> apply buffer in writing, using struct types to handle {id, vector of float}, could also use byte buffer which cares only take bytes into acounnt.
//TODO -> Add reserve() for bulk loading
enum class DBError : std::int32_t {
    MetricError = (-1),
    DimensionError = (-2),
    IdNotFoundError = (-3),
    ZeroNormError = (-4),
    DataBaseEmptyError = (-5),
    FileCorrupted = (-6),
    IdAlreadyPresent = (-7)
};

std::ostream& operator<<(std::ostream& out, const DBError& err) {
    switch (err) {
        case DBError::MetricError:
            out << "MetricError";
            break;
        case DBError::DimensionError:
            out << "DimensionError";
            break;
        case DBError::IdNotFoundError:
            out << "IdNotFoundError";
            break;
        case DBError::ZeroNormError:
            out << "ZeroNormError";
            break;
        case DBError::DataBaseEmptyError:
            out << "DataBaseEmptyError";
            break;
        case DBError::FileCorrupted:
            out << "FileCorrupted";
            break;
        case DBError::IdAlreadyPresent:
            out << "IdAlreadyPresent";
            break;
        default:
            out << "Unknown DBError";
            break;
    }
    return out;
}


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

struct Vector {
    std::vector<float> data;
    float norm_data{0.0f};
    bool normalized{false};

    float norm() const {
        return norm_data;
    }

    void compute_norm() {
        norm_data = 0.0f;

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
    static const inline std::uint32_t s_magic_bytes{0x56454344};
    static const inline std::uint32_t s_version{1};

public:
    Result<Unit, DBError> insert(std::uint64_t id, Vector&& i_vector);

    Result<Unit, DBError> remove(std::uint64_t id);

    Result<std::vector<std::pair<std::uint64_t, float>>, DBError> query(const Vector& q_vector, std::uint64_t k = 10, Metric metric = Metric::Cosine);

    Result<Unit, DBError> save(const std::string& filename);

    Result<Unit, DBError> load(const std::string& filename);

    std::uint64_t size() {
        return (m_vectors.size() * (dimensions() + 3) * sizeof(float)) + sizeof(s_magic_bytes) + sizeof(s_version);
    }

    std::uint64_t dimensions() {
        if (m_vectors.empty()) {
            return 0;
        }

        return m_vectors[0].second.data.size();
    }
};