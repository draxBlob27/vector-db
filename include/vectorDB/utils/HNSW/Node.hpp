#ifndef NODE_HPP
#define NODE_HPP
#include <vector>
#include <set>
#include <algorithm>
#include "Vector.hpp"

struct Node {
    std::uint64_t id;
    Vector vector;
    int layer;
    std::vector<std::vector<std::pair<float, std::uint64_t>>> neighbors;

    Node(const std::uint64_t& id, const Vector& v)
        :id{id}, vector{v}
    {}

    Node() = default;

    std::uint64_t data_size() const {
        return vector.size();
    }

    const Vector& data() const {
        return vector;
    }

    std::uint64_t getId() const {
        return id;
    }
};

#endif //NODE_HPP