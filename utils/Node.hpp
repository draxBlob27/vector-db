#ifndef NODE_HPP
#define NODE_HPP
#include <vector>
#include "Vector.hpp"

struct Node {
    std::size_t id;
    Vector vector;
    std::vector<std::size_t> neighbors;

    Node(const std::uint64_t& id, const Vector& v)
        :id{id}, vector{v}
    {}

    std::size_t data_size() const {
        return vector.size();
    }

    std::size_t edges() const {
        return neighbors.size();
    }

    Vector data() const {
        return vector;
    }

    std::size_t getId() const {
        return id;
    }
};

#endif //NODE_HPP