#ifndef NSW_INDEX_HPP
#define NSW_INDEX_HPP
#include <cstdint>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_set>
#include "Vector.hpp"
#include "distances.hpp"

struct Node {
    std::size_t id;
    Vector vector;
    std::vector<std::size_t> neighbors;

    Node(const std::uint64_t& id, const Vector& v)
        :id{id}, vector{v}
    {}

    std::size_t data_size() {
        return vector.size();
    }

    std::size_t edges() {
        return neighbors.size();
    }
};

class NSW_Index {
private:
    std::vector<Node> m_nodes;
    std::size_t m_entry_point;
    std::uint32_t m_M, m_efConstruction, m_efSearch;

public:
    NSW_Index(const std::uint32_t& M,const std::uint32_t& efConstruction,const std::uint32_t& efSearch) 
        :m_M{M}, m_efConstruction{efConstruction}, m_efSearch{efSearch}
    {}

    /*
        Problem statement: Insert a vector in index.
        Why: To build the index and allow new data points sequentially or iteratively
        Soln: 1.) If coming vector is first then assign it as entry point for incoming vectors.
            2.) For each incoming vector take distance with best/closest efConstruction vectors. ef-> expoloration factor during...
            3.) 
    */
    void insert(const std::uint64_t& id, const Vector& v) {
        // If this vector is first then assign it as entry point for incoming vectors.
        m_nodes.push_back({id, v});
        
        if (m_nodes.size() == 1) {
            m_entry_point = 0;
            return;
        }

        
        auto search_layer{
            [&](const std::size_t& entry_point, const Vector& v) {
                //To keep candidtate vectors in heap, we should take min heap, so then closest is at top.
                std::priority_queue<std::pair<float, std::size_t>> candidates; //{score, id}
        
                //contains ef_const close vectors, with top as farthest of close vector till now
                std::priority_queue<std::pair<float, std::size_t>> found_closest;
        
                std::unordered_set<std::size_t> vis;
        
                vis.insert(entry_point);
                candidates.push({-calc_distance<Metric::L2>(m_nodes[entry_point].vector, v), entry_point});
                found_closest.push({calc_distance<Metric::L2>(m_nodes[entry_point].vector, v), entry_point});
        
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
                        if (found_closest.top().first > dist || found_closest.size() < m_efConstruction) {
                            //if neighbor closer then best farthest, we will pop farthest and push this.
                            candidates.push({dist, nei}); //push {score, neighbor id};
                            found_closest.push({dist, nei});
        
                            if (found_closest.size() > m_efConstruction) {
                                found_closest.pop();
                            } 
                        }
                    }
                }

                // std::uint32_t sz = std::min(static_cast<uint32_t>(found_closest.size()), m_M);
                
                std::vector<std::size_t> best;
                // best.reserve(sz);
                while (!found_closest.empty()) {
                    if (found_closest.size() <= m_M) {
                        best.push_back(found_closest.top().second);
                    }

                    found_closest.pop();
                }

                std::ranges::reverse(best);

                return best;
            }
        };

        Node& inc(m_nodes.back()); //incoming vector node
        std::size_t inc_id{m_nodes.size() - 1};

        std::vector<std::size_t> best{search_layer(m_entry_point, v)}; //no issue of copoying, becase of mandatory copy elision in RVO
        std::uint32_t sz{static_cast<uint32_t>(best.size())};
        inc.neighbors.reserve(sz);

        for (std::uint32_t i{0}; i < sz; i++) {
            std::size_t closest_id{best[i]};

            inc.neighbors.push_back(closest_id); //inesrt id of closest nei -- creating graph edges

            m_nodes[closest_id].neighbors.push_back(inc_id);

            //if we connect this new inc vector to its closest nodes, then we need to recheck if there degree becomes more than M.
            //TODO -> in later stages
        }
    }
};

#endif //NSW_INDEX_HPP