#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "Vector.hpp"
#include "distances.hpp"
#include "Random_engine.hpp"

class LSHIndex {
    private:
        std::vector<std::pair<std::uint64_t, Vector>> m_vectors;
        std::vector<std::unordered_map<std::uint64_t, std::vector<std::size_t>>> m_hash_tables;
        std::vector<std::vector<Vector>> m_hyperplanes;
        std::uint32_t m_num_tables, m_num_projections;

        //function for finding hash val for each table with random projections --- 101010100011 like this
        std::uint64_t hash_val(const Vector& a, const std::vector<Vector>& planes) {
                
            std::uint64_t hash{0};

            for (std::size_t j{0}; j < planes.size(); j++) {
                double dot{0};
                for (std::size_t i{0}; i < a.data.size(); i++) {
                    dot += (a.data[i] * planes[j].data[i]);
                }

                if (dot > 0) {
                    hash |= (1ULL << j);
                }
            }

            return hash;
        }

    public:
        LSHIndex(const std::uint32_t& num_tables, const std::uint32_t& num_projections)
            :m_num_tables{num_tables}, m_num_projections{num_projections}
        {
            m_hash_tables.resize(m_num_tables);
            m_hyperplanes.resize(m_num_tables, std::vector<Vector>(m_num_projections));
        }

        void build(std::vector<Vector> vectors) {
            std::size_t dims{vectors.front().data.size()};//will implement error handling later on

            for (std::uint32_t i{0}; i < m_num_tables; i++) { 
                std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
                for (std::uint32_t j{0}; j < m_num_projections; j++) { //for each table(independent) we have this many projectstions
                    std::vector<float> normal(dims); //create hyperplances in higher dim space
                    for (std::size_t k{0}; k < dims; k++) {
                        normal[k] = Random::get(-0.05f, 0.05f); //hyperplanes wrt origin
                    }

                    t_hyperplanes[j] = Vector(normal);
                }
            }
            
            m_vectors.resize(vectors.size());
            for (std::size_t emb_cnt{0}; emb_cnt < vectors.size(); emb_cnt++) {
                m_vectors[emb_cnt] = {emb_cnt, std::move(vectors[emb_cnt])};
                const Vector& emb = m_vectors[emb_cnt].second;

                for (std::uint32_t i{0}; i < m_num_tables; i++) {
                    std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
                    

                    auto hash = hash_val(emb, t_hyperplanes);

                    m_hash_tables[i][hash].push_back(emb_cnt); //in this table, we got hash_val for curr emb, we map this table->hash_val->id for retrieval later on
                }
            }
        }

        std::vector<std::pair<std::uint64_t, float>> query(const Vector& query, const std::uint32_t& k) {
            std::unordered_set<std::size_t> st;

            Vector copied_query{query};
            copied_query.compute_norm(); //perform normailzation of query vector

            std::vector<std::uint64_t> hash_vals(m_num_tables);

            for (std::size_t i{0}; i < m_num_tables; i++) {
                std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
                std::uint64_t hash = hash_val(copied_query, t_hyperplanes);
                hash_vals[i] =  hash;//hash val of query 
                
                auto got = m_hash_tables[i].find(hash);
                if (got != m_hash_tables[i].end()) {
                    for (const auto& it : got->second) { //extracting collided vectors with same hash val
                        st.insert(it); //de-duplicating
                    }
                }
            }

            int j{0};
            while (st.size() < k && j < m_num_projections) {
                for (std::uint32_t i{0}; i < m_num_tables; i++) {
                    std::uint64_t hash = hash_vals[i];

                    hash ^= (1ULL << j);

                    auto got = m_hash_tables[i].find(hash);
                    if (got != m_hash_tables[i].end()) {
                        for (const auto& it : got->second) { //extracting collided vectors with same hash val
                            st.insert(it); //de-duplicating
                        }
                    } 
                }

                j++;
            }

            std::priority_queue<std::pair<float, uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> pq; //min heap

            for (const auto& it: st) {
                m_vectors[it].second.compute_norm();
                float dist = calc_distance<Metric::Cosine>(m_vectors[it].second, copied_query);
                //cosine metirc, higher val is better;
                if (pq.size() >= k) {
                    if (pq.top().first < dist) {
                        pq.pop();
                        pq.push({dist, m_vectors[it].first});
                    }
                } else {
                    pq.push({dist, m_vectors[it].first});
                }
            }

            std::vector<std::pair<std::uint64_t, float>> res;

            while (!pq.empty()) {
                res.push_back({pq.top().second, pq.top().first});
                pq.pop();
            }
            
            std::reverse(res.begin(), res.end());

            return res;
        }

        void save(const std::string& filename) {

        }

        void load(const std::string& filename) {
            
        }
};