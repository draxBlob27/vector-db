#ifndef NODE_HPP
#define NODE_HPP
#include <vector>
#include <set>
#include <algorithm>
#include "Vector.hpp"

struct Node {
    std::uint64_t id;
    Vector vector;
    std::vector<std::pair<float, std::uint64_t>> neighbors;

    Node(const std::uint64_t id, const Vector& v)
        :id{id}, vector{v}
    {}

    Node() = default;

    std::uint64_t data_size() const {
        return vector.size();
    }

    std::uint32_t edges() const {
        return neighbors.size();
    }

    const Vector& data() const {
        return vector;
    }

    std::uint64_t getId() const {
        return id;
    }

    void align(uint32_t max_M) {
        if (static_cast<double>(neighbors.size())<= static_cast<double>(1.5 * max_M)) return;
        
        auto nth{neighbors.begin() + max_M};
        std::ranges::nth_element(neighbors, nth);
        neighbors.erase(nth, neighbors.end());
    }

    friend bool operator==(const Node& a, const Node& b) {
        return a.id == b.id && a.vector == b.vector && a.neighbors == b.neighbors;
    }

    friend bool operator!=(const Node& a, const Node& b) {
        return !(a == b);
    }
};

#endif //NODE_HPP