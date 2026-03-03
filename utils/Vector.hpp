#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <vector>
#include <cmath>

struct Vector {
    std::vector<float> data;
    float norm_data{0.0f};
    bool normalized{false};

    Vector() = default;

    Vector(std::vector<float> vals) {
        data = std::move(vals);
    }

    float norm() const {
        return norm_data;
    }

    void compute_norm() {
        if (normalized)
            return;
            
        norm_data = 0.0f;

        for (const auto& it : data) {
            norm_data += (it * it);
        }

        norm_data = std::sqrt(norm_data);
        normalized = true;
    };

    std::size_t size() const {
        return data.size();
    }
};
#endif //VECTOR_HPP