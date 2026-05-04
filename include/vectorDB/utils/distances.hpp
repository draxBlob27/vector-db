#ifndef DISTANCES_HPP
#define DISTANCES_HPP
#include "Vector.hpp"
#include "Metric.hpp"

template<Metric M>
float calc_distance(const Vector& a, const Vector& b);

template<>
inline float calc_distance<Metric::Cosine>(const Vector& a, const Vector& b) {
    float distance{0.0f};
    for (std::size_t i{0}; i < a.data.size(); i++) {
        distance += (a.data[i] * b.data[i]);
    }

    distance /= (a.norm() * b.norm());
    return distance;
}

template<>
inline float calc_distance<Metric::DotProduct>(const Vector& a, const Vector& b) {
    float distance{0.0f};
    for (std::size_t i{0}; i < a.data.size(); i++) {
        distance += (a.data[i] * b.data[i]);
    }

    return distance;
}

#include <immintrin.h> // Required for AVX intrinsics

template<>
inline float calc_distance<Metric::L2>(const Vector& a, const Vector& b) {
    const float* ptr_a = a.data.data();
    const float* ptr_b = b.data.data();
    std::size_t size = a.data.size();
    
    __m256 sum = _mm256_setzero_ps(); // Initialize a vector of 8 zeros

    // Main loop: Process 8 floats at a time
    for (std::size_t i = 0; i < size; i += 8) {
        __m256 va = _mm256_loadu_ps(ptr_a + i); // Load 8 floats
        __m256 vb = _mm256_loadu_ps(ptr_b + i);
        __m256 diff = _mm256_sub_ps(va, vb);    // (va - vb)
        // Fused Multiply-Add: sum = (diff * diff) + sum
        sum = _mm256_fmadd_ps(diff, diff, sum); 
    }

    // Horizontal sum: Add the 8 individual sums inside the register together
    float res[8];
    _mm256_storeu_ps(res, sum);
    return res[0] + res[1] + res[2] + res[3] + res[4] + res[5] + res[6] + res[7];
}
#endif //DISTANCES_HPP