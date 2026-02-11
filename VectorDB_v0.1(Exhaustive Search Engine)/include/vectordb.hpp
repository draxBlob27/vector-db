#include <cstdint>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>

struct Vector {
    std::vector<float> data;
    bool normalized;
    float norm() const;
    void normalize();
};

enum class Metric {
    L2, 
    Cosine, 
    DotProduct
};

class VectorStore {
private:
    std::vector<std::pair<uint64_t, Vector>> m_vectors;
    // static inline std::uint64_t m_id{0};

public:
    void insert(std::uint64_t id, const Vector& vector) {
        auto dims_valid{[&]() {
            return m_vectors[0].second.data.size() == vector.data.size();
        }};

        if (m_vectors.empty() || dims_valid()) {
            m_vectors.push_back({id, vector});
            return;
        }

        throw std::runtime_error("Dimensions mismatch.");
    }

    void remove(std::uint64_t id) {
        
    }

    std::vector<std::pair<std::uint64_t, double>> query(const std::vector<float>& vector, int k, int metric){

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