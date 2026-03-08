#ifndef NSW_INDEX_HPP
#define NSW_INDEX_HPP
#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_set>
#include <string>
#include <fstream>
#include "Vector.hpp"
#include "distances.hpp"
#include "Serializer_De.hpp"
#include "Node.hpp"

class NSW_Index {
private:
    static const inline std::uint32_t s_magic_bytes{0x4e5357}; //NSW
    static const inline std::uint32_t s_version{1};

    std::vector<Node> m_nodes;
    std::uint64_t m_num_nodes = 0;
    std::uint32_t m_dimension = 0;
    std::uint64_t m_entry_point;
    std::uint32_t m_M, m_efConstruction, m_efSearch;

    std::vector<std::pair<std::uint64_t, float>> search_layer(const Vector& v, std::uint32_t ef = 0, std::uint32_t M = 0) const {
        if (ef == 0 && M == 0) {
            return search_layer(v, m_efConstruction, m_M);
        }
        //To keep candidtate vectors in heap, we should take min heap, so then closest is at top.
        std::priority_queue<std::pair<float, std::uint64_t>> candidates; //{score, id}
        //contains potential candiaties

        //contains efclose vectors, with top as farthest of close vector till now
        std::priority_queue<std::pair<float, std::uint64_t>> found_closest;

        std::unordered_set<std::uint64_t> vis;

        vis.insert(m_entry_point);
        candidates.push({-calc_distance<Metric::L2>(m_nodes[m_entry_point].vector, v), m_entry_point});
        found_closest.push({calc_distance<Metric::L2>(m_nodes[m_entry_point].vector, v), m_entry_point});

        while (!candidates.empty()) {//maintain the candidate size
            auto [score, nodeId] = candidates.top();
            candidates.pop();

            if (-score > found_closest.top().first) { //if dist is futher than the farthest best vector then break;
                break;
            }

            for (auto nei : m_nodes[nodeId].neighbors) {
                auto [_, inserted] = vis.insert(nei);

                if (!inserted) {
                    continue;
                }

                float dist{calc_distance<Metric::L2>(m_nodes[nei].vector, v)};
                if (found_closest.top().first > dist || static_cast<uint32_t>(found_closest.size()) < ef) {
                    //if neighbor closer then best farthest, we will pop farthest and push this.
                    candidates.push({dist, nei}); //push {score, neighbor id};
                    found_closest.push({dist, nei});

                    if (static_cast<uint32_t>(found_closest.size()) > ef) {
                        found_closest.pop();
                    } 
                }
            }
        }

        // std::uint32_t sz = std::min(static_cast<uint32_t>(found_closest.size()), m_M);
        
        std::vector<std::pair<std::uint64_t, float>> best;
        // best.reserve(sz);
        while (!found_closest.empty()) {
            if (static_cast<uint32_t>(found_closest.size()) <= M) {
                best.push_back({found_closest.top().second, found_closest.top().first});
            }

            found_closest.pop();
        }

        std::ranges::reverse(best);

        return best;
    }

public:
    NSW_Index(std::uint32_t M, std::uint32_t efConstruction = 120, std::uint32_t efSearch = 50) 
        :m_M{M}, m_efConstruction{efConstruction}, m_efSearch{efSearch}
    {}

    /*
        Problem statement: Insert a vector in index.
        Why: To build the index and allow new data points sequentially or iteratively
        Soln: 1.) If coming vector is first then assign it as entry point for incoming vectors.
            2.) For each incoming vector take distance with best/closest efConstruction vectors. ef-> expoloration factor during...
            3.) 
    */
    void insert(std::uint64_t id, const Vector& v) {
        // If this vector is first then assign it as entry point for incoming vectors.
        m_nodes.push_back({id, v});
        m_num_nodes++;
        
        if (m_num_nodes == 1) {
            m_entry_point = 0;
            m_dimension = static_cast<uint32_t>(v.size());
            return;
        }

        Node& inc(m_nodes.back()); //incoming vector node
        std::uint64_t inc_id{m_num_nodes - 1};

        std::vector<std::pair<std::uint64_t, float>> best{search_layer(v)}; //no issue of copoying, becase of mandatory copy elision in RVO
        std::uint32_t sz{static_cast<uint32_t>(best.size())};
        inc.neighbors.reserve(sz);

        for (std::uint32_t i{0}; i < sz; i++) {
            std::uint64_t closest_id{best[i].first};

            inc.neighbors.push_back(closest_id); //inesrt id of closest nei -- creating graph edges

            m_nodes[closest_id].neighbors.push_back(inc_id);

            //if we connect this new inc vector to its closest nodes, then we need to recheck if there degree becomes more than M.
            //TODO -> in later stages
        }
    }

    std::vector<std::pair<std::uint64_t, float>> query(const Vector& v, std::uint32_t k, std::uint32_t efSearch) const {
        std::vector<std::pair<std::uint64_t, float>> best{search_layer(v, efSearch, k)};

        for (auto& [lg_id, score] : best) {
            lg_id = m_nodes[lg_id].id;
        }

        return best;
    }

    void save(const std::string& filename) {
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

        std::ofstream outf{filename, std::ios::binary};
        Serializer_De::file_exists(filename, outf);

        using namespace std::string_literals;
        Serializer_De::stream_write(s_magic_bytes, "Insufficient space on disk"s, outf);
        Serializer_De::stream_write(s_version, "Insufficient space on disk"s, outf);

        Serializer_De::stream_write(m_efConstruction, "Insufficient space on disk"s, outf);
        Serializer_De::stream_write(m_M, "Insufficient space on disk"s, outf);
        Serializer_De::stream_write(m_num_nodes, "Insufficient space on disk"s, outf);
        Serializer_De::stream_write(m_dimension, "Insufficient space on disk"s, outf);

        Serializer_De::stream_write(m_nodes, "Insufficient space on disk"s, outf);
        Serializer_De::stream_write(m_entry_point, "Insufficient space on disk"s, outf);
    }  
    
    // void load (const std::string& filename) {

    // }

    std::uint64_t getSize() const {
        return m_num_nodes;
    }
};

#endif //NSW_INDEX_HPP