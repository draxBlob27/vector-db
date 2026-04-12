#ifndef NODE_HPP
#define NODE_HPP
#include <vector>
#include <set>
#include <algorithm>
#include <mutex>
#include "Vector.hpp"
#include "Spinlock.hpp"

struct Node {
    std::uint64_t id;
    Vector vector;
    std::vector<std::pair<float, std::uint64_t>> neighbors;
    Spinlock mutex;

    Node(const std::uint64_t& id, const Vector& v)
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
        std::lock_guard<Spinlock> lk{mutex};
        if (static_cast<double>(neighbors.size())<= static_cast<double>(1.5 * max_M)) return;
        
        auto nth{neighbors.begin() + max_M};
        std::ranges::nth_element(neighbors, nth);
        neighbors.resize(max_M);
    }

    friend bool operator==(const Node& a, const Node& b) {
        return a.id == b.id && a.vector == b.vector && a.neighbors == b.neighbors;
    }

    friend bool operator!=(const Node& a, const Node& b) {
        return !(a == b);
    }

    void add_edge_and_prune(float dist, std::uint64_t new_id, std::uint32_t max_M) {
        std::lock_guard<Spinlock> lk{mutex};

        // Read, modify, and write all under the same lock acquisition
        neighbors.emplace_back(dist, new_id);

        if (neighbors.size() > static_cast<size_t>(1.5 * max_M)) {
            auto nth = neighbors.begin() + max_M;
            std::ranges::nth_element(neighbors, nth);
            neighbors.resize(max_M);
        }
    }
};

#endif //NODE_HPP