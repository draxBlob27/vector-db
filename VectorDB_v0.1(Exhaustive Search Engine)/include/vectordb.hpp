#include <cstdint>
#include <ios>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <queue>
#include <functional>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <vector>

//TODO -> handle diff k values.
//TODO -> code is still redundant, can use lamdas to fix, Will se later on.
//TODO -> handle unique id, also to handle remove_if by find_if, using the fact that id is unique.
//TODO -> apply buffer in writing, using struct types to handle {id, vector of float}
//TODO -> Add reserve() for bulk loading

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

    void compute_norm() {
        norm_data = 0.0f;

        for (const auto& it : data) {
            norm_data += (it * it);
        }

        norm_data = std::sqrt(norm_data);
        normalized = true;
    };
};

template<Metric M>
float calc_distance(const Vector& a, const Vector& b);

template<>
float calc_distance<Metric::Cosine>(const Vector& a, const Vector& b) {
    float distance{0.0f};
    for (int i{0}; i < a.data.size(); i++) {
        distance += (a.data[i] * b.data[i]);
    }

    distance /= (a.norm() * b.norm());
    return distance;
}

template<>
float calc_distance<Metric::DotProduct>(const Vector& a, const Vector& b) {
    float distance{0.0f};
    for (int i{0}; i < a.data.size(); i++) {
        distance += (a.data[i] * b.data[i]);
    }

    return distance;
}

template<>
float calc_distance<Metric::L2>(const Vector& a, const Vector& b) {
    float distance{0.0f};
    for (int i{0}; i < a.data.size(); i++) {
        distance += ((a.data[i] - b.data[i]) * (a.data[i] - b.data[i]));
    }

    return distance;
}


class VectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;
    static const inline std::uint32_t s_magic_bytes{0x56454344};
    static const inline std::uint32_t s_version{1};

public:
    Result<Unit, DBError> insert(std::uint64_t id, Vector&& i_vector) { //need to handle Id already exists
        auto dims_valid{[&]() {
            return m_vectors[0].second.data.size() == i_vector.data.size();
        }};

        if (m_vectors.empty() || dims_valid()) {
            i_vector.compute_norm(); //normalise at insgestion

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

    Result<std::vector<std::pair<std::uint64_t, float>>, DBError> query(const Vector& q_vector, std::uint64_t k = 1, Metric metric = Metric::DotProduct) {//very large object is getting created, can think of move semantics
        //very high chances of using quick select, just saw LC soln today(12 feb, 2026) regarding this
        //saying quick select is best in terms of TC ~ O(n) for best k kinda things
        if (m_vectors.empty()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        std::vector<std::pair<std::uint64_t, float>> res(std::min(static_cast<uint64_t>(k), size())); //handles if DB size is less than k.

        switch (metric)
        {
        case(Metric::L2): {
            std::priority_queue<std::pair<float, std::uint64_t>> pq;
            int i{res.size() - 1};
            
            if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
                return Err<DBError>{DBError::DimensionError};
            }

            for (const auto& it : m_vectors) {
                /* code */
                //it.first -> id
                //it.second -> Vector
                float distance{calc_distance<Metric::L2>(it.second, q_vector)};

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
            int i{res.size() - 1};
            
            if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
                return Err<DBError>{DBError::DimensionError};
            }

            for (const auto& it : m_vectors) {
                /* code */
                //it.first -> id
                //it.second -> Vector
                float distance{calc_distance<Metric::DotProduct>(it.second, q_vector)};
                
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
            int i{res.size() - 1};
            
            Vector copied_q_vector{q_vector};
            copied_q_vector.compute_norm(); //perform normailzation of query vector
            float query_norm{copied_q_vector.norm()};
            //if query norm is 0 then outright riject it.
            if (query_norm == 0.0f) {
                return Err<DBError>{DBError::ZeroNormError};
            }

            if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
                return Err<DBError>{DBError::DimensionError};
            }
            
            for (const auto& it : m_vectors) {
                /* code */
                if (it.second.norm() == 0.0f) {
                    continue;
                }
                
                float distance{calc_distance<Metric::Cosine>(it.second, q_vector)};

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

    Result<Unit, DBError> save(const std::string& filename) {
        // std::uint32_t crc_32_header{0xFFFFFFFF};
        const std::uint64_t count{m_vectors.size()};
        if (!count) {
            //throw InvalidOperationError("Empty vectors.");
            return Err<DBError>{DBError::DataBaseEmptyError};
        }

        const std::uint32_t dimension{static_cast<std::uint32_t>(m_vectors[0].second.data.size())};
        if (!dimension) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw InvalidOperationError("Empty vectors.");
        }
        
        std::ofstream outf{filename, std::ios::binary};
        if (!outf) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for writing!\n");
        }
        
        outf.write(reinterpret_cast<const char*>(&VectorStore::s_magic_bytes), sizeof(VectorStore::s_magic_bytes));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        // update_crc(crc_32_header, &s_magic_bytes, sizeof(s_magic_bytes));
        
        outf.write(reinterpret_cast<const char *>(&VectorStore::s_version), sizeof(VectorStore::s_version));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        // update_crc(crc_32_header, &s_version, sizeof(s_version));

        outf.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        // update_crc(crc_32_header, &dimension, sizeof(dimension));
        
        outf.write(reinterpret_cast<const char *>(&count), sizeof(count));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        // update_crc(crc_32_header, &count, sizeof(count));
        
        // crc_32_header ^= 0xFFFFFFFF;
        // outf.write(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
        // if (outf.bad() || outf.fail()) {
        //     throw InsufficientSpaceError("Insufficient space on disk.");
        // }

        // std::uint32_t crc_32_data{0xFFFFFFFF};
        for (std::uint64_t i{0}; i < count; i++)
        {
            if (dimension != m_vectors[i].second.data.size()) {
                return Err<DBError>{DBError::DataBaseEmptyError};
                // throw InvalidOperationError("Dimension of data mismatch.");
            }
            
            //write id to disk.
            outf.write(reinterpret_cast<const char *>(&m_vectors[i].first), sizeof(std::uint64_t));
            if (outf.bad() || outf.fail()) {
                return Err<DBError>{DBError::DataBaseEmptyError};
                // throw InsufficientSpaceError("Insufficient space on disk.");
            }
            // update_crc

            unsigned const char* d_ptr = reinterpret_cast<unsigned const char*>(&m_vectors[i].second.data[0]);
            outf.write(reinterpret_cast<const char*>(d_ptr), dimension * sizeof(float));
            if (outf.bad() || outf.fail()) {
                return Err<DBError>{DBError::DataBaseEmptyError};
                // throw InsufficientSpaceError("Insufficient space on disk.");
            }
            
            //write norm val to disk
            outf.write(reinterpret_cast<const char*>(&m_vectors[i].second.norm_data), sizeof(m_vectors[i].second.norm_data));
            if (outf.bad() || outf.fail()) {
                return Err<DBError>{DBError::DataBaseEmptyError};
                // throw InsufficientSpaceError("Insufficient space on disk.");
            }
            // update_crc
        }

        // crc_32_data ^= 0xFFFFFFFF;
        // outf.write(reinterpret_cast<char *>(&crc_32_data), sizeof(std::uint32_t));
        // if (outf.bad() || outf.fail()) {
        //     throw InsufficientSpaceError("Insufficient space on disk.");
        // }

        return Ok<Unit>{Unit{}};
    }

    Result<Unit, DBError> load(const std::string& filename) { //can use std::string_view -> improves perf, but does it matter that much. We'll seee
        std::ifstream inf{filename, std::ios::binary};
        if (!inf) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
        }

        // std::uint32_t calc_crc_32_header{0xFFFFFFFF};
        std::uint32_t magic_bytes;
        inf.read(reinterpret_cast<char *>(&magic_bytes), sizeof(std::uint32_t));
        if (inf.fail() || inf.bad()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw ArchiveError("Could not read file.");
        }
        if (magic_bytes != s_magic_bytes)
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw CorruptedDataError("Magic bytes mismatch."); // placeholder;
        // update_crc(calc_crc_32_header, &magic_bytes, sizeof(magic_bytes));

        std::uint32_t version;
        inf.read(reinterpret_cast<char *>(&version), sizeof(std::uint32_t));
        if (inf.fail() || inf.bad()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw ArchiveError("Could not read file.");
        }
        if (version != s_version)
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw CorruptedDataError("Version mismatch.");
        // update_crc(calc_crc_32_header, &version, sizeof(version));

        std::uint32_t dimension;
        inf.read(reinterpret_cast<char *>(&dimension), sizeof(std::uint32_t));
        if (inf.fail() || inf.bad()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw ArchiveError("Could not read file.");
        }
        // update_crc(calc_crc_32_header, &dimension, sizeof(dimension));

        std::uint64_t count;
        inf.read(reinterpret_cast<char *>(&count), sizeof(std::uint64_t));
        if (inf.fail() || inf.bad()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw ArchiveError("Could not read file.");
        }
        // update_crc(calc_crc_32_header, &count, sizeof(count));

        // std::uint32_t crc_32_header;
        // inf.read(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
        // if (inf.fail() || inf.bad()) {
        //     return Err<DBError>{DBError::DataBaseEmptyError};
        //     // throw ArchiveError("Could not read file.");
        // }

        // calc_crc_32_header ^= 0xFFFFFFFF;
        // if (crc_32_header != calc_crc_32_header) {
            // throw CorruptedDataError("Header CRC mismatch.");
        // }

        // std::uint32_t calc_crc_32_data{0xFFFFFFFF};
        // std::vector<std::vector<double>> data(count, std::vector<double>(dimension));
        //to implement check for file size here

        m_vectors.resize(count);

        for (std::uint64_t i = 0; i < count; i++)
        {
            m_vectors[i].second.data.resize(dimension);

            inf.read(reinterpret_cast<char*>(&m_vectors[i].first), sizeof(std::uint64_t));//reads and loads id.

            unsigned char *d_ptr = reinterpret_cast<unsigned char *>(&m_vectors[i].second.data[0]);
            inf.read(reinterpret_cast<char*>(d_ptr), dimension * sizeof(float));
            if (inf.fail() || inf.bad()) {
                return Err<DBError>{DBError::DataBaseEmptyError};
                // throw ArchiveError("Could not read file.");
            }

            //reads norm data and load it
            inf.read(reinterpret_cast<char *>(&m_vectors[i].second.norm_data), sizeof(m_vectors[i].second.norm_data));
            if (inf.fail() || inf.bad()) {
                return Err<DBError>{DBError::DataBaseEmptyError};
                // throw ArchiveError("Could not read file.");
            }

            // if (!already_verified) {
            //     update_crc(calc_crc_32_data, d_ptr, dimension * sizeof(double));
            // }
        }

        // if (!already_verified)
        //     calc_crc_32_data ^= 0xFFFFFFFF;

        // std::uint32_t crc_32_data;
        // inf.read(reinterpret_cast<char *>(&crc_32_data), sizeof(std::uint32_t));
        // if (inf.fail() || inf.bad()) {
        //     throw ArchiveError("Could not read file.");
        // }

        // if (!already_verified && calc_crc_32_data != crc_32_data)
        //     throw CorruptedDataError("Data CRC mismatch.");

        return Ok<Unit>{Unit{}};
    }

    std::uint64_t size() {
        return m_vectors.size();
    }

    std::uint64_t dimensions() {
        if (m_vectors.empty()) {
            return 0;
        }

        return m_vectors[0].second.data.size();
    }
};