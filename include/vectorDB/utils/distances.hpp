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

template<>
inline float calc_distance<Metric::L2>(const Vector& a, const Vector& b) {
    float distance{0.0f};
    for (std::size_t i{0}; i < a.data.size(); i++) {
        distance += ((a.data[i] - b.data[i]) * (a.data[i] - b.data[i]));
    }

    return distance;
}

#endif //DISTANCES_HPP