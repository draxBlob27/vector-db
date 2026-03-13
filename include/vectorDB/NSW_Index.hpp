#ifndef NSW_INDEX_HPP
#define NSW_INDEX_HPP
#include <cstdint>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_set>
#include <string>
#include <fstream>
#include "utils/Vector.hpp"
#include "utils/distances.hpp"
#include "utils/Serializer_De.hpp"
#include "utils/Node.hpp"

class NSW_Index;
bool operator==(const NSW_Index& n1, const NSW_Index& n2);

class NSW_Index {
private:
    static const inline std::uint32_t s_magic_bytes{0x4e5357}; //NSW
    static const inline std::uint32_t s_version{1};
    static inline std::uint32_t indexNumber{0};

    std::vector<Node> m_nodes;
    std::uint64_t m_num_nodes = 0;
    std::uint32_t m_dimension = 0;
    std::uint64_t m_entry_point;
    std::uint32_t m_M, m_efConstruction, m_efSearch;

    std::vector<std::pair<std::uint64_t, float>> search_layer(const Vector& v, std::uint32_t ef = 0, std::uint32_t M = 0) const;

public:
    NSW_Index(std::uint32_t M, std::uint32_t efConstruction = 120, std::uint32_t efSearch = 50);

    NSW_Index() = default;

    /*
        Problem statement: Insert a vector in index.
        Why: To build the index and allow new data points sequentially or iteratively
        Soln: 1.) If coming vector is first then assign it as entry point for incoming vectors.
            2.) For each incoming vector take distance with best/closest efConstruction vectors. ef-> expoloration factor. 
            3.) Connect best k of these ef vectors to the incoming vector.
            4.) During this process if it happens to incerase the nodes connected of just now connected nodes, then find best of those, and prune remaining.
    */
    void insert(std::uint64_t id, const Vector& v);

    std::vector<std::pair<std::uint64_t, float>> query(const Vector& v, std::uint32_t k, std::uint32_t efSearch = 0) const;

    void save(const std::string& filename);
    /*
        |-------------------------------------------------------|
        | Magic number                                          |
        | Version                                               |
        | Parameters                                            |
        | Num nodes                                             |
        | Dimensions                                            |
        | For each node:                                        |
        |       id: uint64_t                                    |
        |       Vector: vector of float                          |
        |       Num neighbors: uint32_t                         |
        |       Neighbor indices                                |
        | Entry point index                                     |
        |-------------------------------------------------------|
    */
    
    void load (const std::string& filename);

    static inline void successInsert() {
        indexNumber++;
    }

    static inline std::uint32_t getIndexNumber() {
        return indexNumber;
    }

    std::uint64_t getSize() const {
        return m_num_nodes;
    }

    friend bool operator==(const NSW_Index& n1, const NSW_Index& n2);
};

#endif //NSW_INDEX_HPP