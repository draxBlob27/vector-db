#ifndef VECTOR_HPP
#define VECTOR_HPP
#include <vector>
#include <iostream>
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
        if (normalized) {
            // std::cout << "Not computing\n";
            return;
        }
            
        norm_data = 0.0f;
        // std::cout << "Computing\n";

        for (const auto& it : data) {
            norm_data += (it * it);
        }

        norm_data = std::sqrt(norm_data);
        normalized = true;
    };

    std::size_t size() const {
        return data.size();
    }

    friend bool operator==(const Vector& a, const Vector& b) {
        return a.data == b.data;
    }

    friend bool operator!=(const Vector& a, const Vector& b) {
        return !(a == b);
    }
};
#endif //VECTOR_HPP