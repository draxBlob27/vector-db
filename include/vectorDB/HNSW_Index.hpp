#include <cmath>
#include <random>
#include <unordered_set>
#include <queue>
#include <ranges>
#include <algorithm>
#include "vectorDB/utils/Metric.hpp"
#include "vectorDB/utils/distances.hpp"
#include "vectorDB/utils/HNSW/Node.hpp"
#include "vectorDB/utils/Vector.hpp"

class HNSW_Index {
private:
    std::uint64_t m_ep; //entry point in graph
    std::vector<Node> m_nodes; //hnsw
    int m_max_layer; //top layer of hnsw
    std::uint32_t m_M; //max edges per node per layer
    std::uint32_t m_Mmax; //max edges at leayer != 0
    std::uint32_t m_Mmax0; //max edges at layer 0
    std::uint32_t m_efConstruction; //search width during build
    std::uint32_t m_efSearch; //search width during query
    double m_ml; //normalization factor for level generation mL

    int select_layer() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        std::random_device gen;
        return static_cast<int>(std::floor(-std::log(dist(gen)) * m_ml));
    }

public:
    HNSW_Index(std::uint32_t M, std::uint32_t efConstruction = 120, std::uint32_t efSearch = 50) 
        :m_M{M}, m_efConstruction{efConstruction}, m_efSearch{efSearch} 
    {
        m_ml = 1 / std::log(M); 
        m_Mmax0 = 2 * m_M;
        m_Mmax = m_M;
    }

    void insert(std::uint64_t id, const Vector& v) {
        int l = select_layer();
        Node new_node = Node(id, v, l);
        if (m_nodes.empty()) {
            m_ep = 0;
            m_max_layer = 1;
            m_nodes.push_back(new_node);

            return;
        }
        
        std::vector<std::pair<float, std::uint64_t>> entry_points{{0.0, m_ep}};

        for (int lc = m_max_layer; lc > l; lc--) {
            entry_points = search_layer(v, entry_points, 1, lc);
        }

        std::size_t ind{m_nodes.size()};
        m_nodes.push_back(new_node);

        for (int lc = std::min(l, m_max_layer); lc >= 0; lc--) {
            auto candidates{search_layer(v, entry_points, m_efConstruction, lc)};
            int M_use = lc == 0 ? m_Mmax0 : m_Mmax;
            auto neighbors{select_neighbors(v, candidates, M_use, lc)};
            
            for (auto [dist, nei_id] : neighbors) {
                if (lc > m_nodes[nei_id].layer) 
                    continue;

                m_nodes[ind].neighbors[lc].push_back({dist, nei_id});
                m_nodes[nei_id].neighbors[lc].push_back({dist, ind});
            }
            
            for (auto [dist, nei_id] : neighbors) {
                if (lc > m_nodes[nei_id].layer) 
                    continue;

                auto& econn{m_nodes[nei_id].neighbors[lc]};
    
                if (econn.size() > static_cast<std::size_t>(M_use)) {
                    auto new_conn{select_neighbors(m_nodes[nei_id].data(), econn, M_use, lc)};
                    econn = new_conn;
                }
            }

            entry_points = candidates;
        }

        if (l > m_max_layer) {
            m_ep = ind;
            m_max_layer = l;
        }
    }

    std::vector<std::pair<float, std::uint64_t>> search_layer(const Vector& query, const std::vector<std::pair<float, std::uint64_t>>& entry_points, std::uint32_t ef, int lc) const {
        std::unordered_set<std::uint64_t> vis; //set of visited elements 
        std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> candidates; //set of candidates
        std::priority_queue<std::pair<float, std::uint64_t>> result; // dynamic list of found nearest neighbors

        for (auto [_, ep] : entry_points) {
            float dist = calc_distance<Metric::L2>(m_nodes[ep].data(), query);
            candidates.emplace(dist, ep);
            result.emplace(dist, ep);
            vis.insert(ep);
        }

        while (!candidates.empty()) {
            auto [curr_dist, curr_idx] = candidates.top();
            candidates.pop();

            if (curr_dist > result.top().first) {
                break;
            }

            if (lc > m_nodes[curr_idx].layer) {
                continue;
            }
            
            for (auto [_, nei_id] : m_nodes[curr_idx].neighbors[lc]) {
                auto [__, inserted] = vis.insert(nei_id);
                if (!inserted) {
                    continue;
                }
                
                auto dist = calc_distance<Metric::L2>(query, m_nodes[nei_id].data());
                if (result.size() < ef || dist < result.top().first) {
                    candidates.emplace(dist, nei_id);
                    result.emplace(dist, nei_id);
                    if (result.size() > ef) {
                        result.pop();
                    }
                }
            }
        }

        std::vector<std::pair<float, std::uint64_t>> output;
        while (result.size()) {
            output.push_back(result.top());
            result.pop();
        }

        std::ranges::reverse(output);
        return output; //returns output of size ef sorted in order (closest -> farthest).
    }

    std::vector<std::pair<float, std::uint64_t>> select_neighbors(const Vector& query, const std::vector<std::pair<float, std::uint64_t>>& dist_cand, int M, int lc) {
        //dist_cand is sorted in order neares to farthest from query q.
        std::vector<std::pair<float, std::uint64_t>> result;
        std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> w_discarded;

        for (auto [dist, idx] : dist_cand) {
            if (result.size() >= static_cast<std::size_t>(M)) {
                break;
            }

            bool is_discarded = false;
            for (auto [_, r_idx] : result) {
                if (calc_distance<Metric::L2>(m_nodes[r_idx].data(), m_nodes[idx].data()) < dist) {
                    //cand is rejected if it breaks heuristic i.e if dist(r, e) < dist(q, e)
                    //meaning new neighbor is not shadowed by existing nodes.
                    is_discarded = true;
                    break;
                }
            }

            if (!is_discarded) {
                result.push_back({dist, idx}); 
            } else {
                w_discarded.push({dist, idx});
            }
        }

        while (w_discarded.size() && result.size() < static_cast<std::size_t>(M)) {
            result.push_back(w_discarded.top());
            //doing this might make result unsorted.
            w_discarded.pop();
        }

        //performs a sort to keep result sorted;
        std::ranges::sort(result);
        return result;
    }

    std::vector<std::pair<std::uint64_t, float>> query(const Vector& query, std::uint32_t k, std::uint32_t efSearch) const {
        std::vector<std::pair<float, std::uint64_t>> entry_points{{0.0, m_ep}};
        for (int lc = m_max_layer; lc > 0; lc--) {
            entry_points = search_layer(query, entry_points, 1, lc);
        }

        auto candidates = search_layer(query, entry_points, efSearch, 0);
        std::vector<std::pair<std::uint64_t, float>> result;

        for (auto [scr, idx] : candidates) {
            result.push_back({idx, scr});
            if (result.size() >= k) {
                break;
            }
        }

        for (auto& [lg_id, score] : result) {
            lg_id = m_nodes[lg_id].id;
        }

        return result;
    }

    std::uint64_t getSize() const {
        return m_nodes.size();
    }
};