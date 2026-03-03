#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>
#include <string>
#include "Vector.hpp"
#include "distances.hpp"
#include "errors.hpp"
#include "Random_engine.hpp"

class LSHIndex {
    private:
        static const inline std::uint32_t s_magic_bytes{0x4c5348}; //LSH
        static const inline std::uint32_t s_version{1};

        std::vector<std::pair<std::uint64_t, Vector>> m_vectors;
        std::uint32_t m_dimension;
        std::uint64_t m_count;
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

        void stream_valid(const std::string& e_msg, std::ofstream& outf) { //cannot pass string view as exceptions need to own error message to keep them alive while stack unfolding
            if (outf.bad() || outf.fail()) {
                throw InsufficientSpaceError(e_msg);
            }
        };

        template <typename T>
        void stream_write(const T& data, const std::string& e_msg, std::ofstream& outf) {
            outf.write(reinterpret_cast<const char*>(&data), sizeof(data));
            stream_valid(e_msg, outf);
        }

        template <typename T>
        void stream_write(const std::vector<T>& data, const std::string& e_msg, std::ofstream& outf) {
            unsigned const char* d_ptr{reinterpret_cast<unsigned const char*>(&data[0])};
            outf.write(reinterpret_cast<const char*>(d_ptr), data.size() * sizeof(T));
            stream_valid(e_msg, outf);
        }

        void stream_write(const std::vector<Vector>& data, const std::string& e_msg, std::ofstream& outf) {
            for (const auto& emb : data) {
                using namespace std::string_literals;
                stream_write(emb.data, "Insufficient space on disk"s, outf);
            }
        }

    public:
        LSHIndex(const std::uint32_t& num_tables, const std::uint32_t& num_projections)
            :m_num_tables{num_tables}, m_num_projections{num_projections}
        {
            m_hash_tables.resize(m_num_tables);
            m_hyperplanes.resize(m_num_tables, std::vector<Vector>(m_num_projections));
        }

        void build(std::vector<Vector> vectors)
        {
            m_dimension = vectors.front().data.size();//will implement error handling later on
            m_count = vectors.size(); //total count of data points

            for (std::uint32_t i{0}; i < m_num_tables; i++) { 
                std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
                for (std::uint32_t j{0}; j < m_num_projections; j++) { //for each table(independent) we have this many projectstions
                    std::vector<float> normal(m_dimension); //create hyperplances in higher dim space
                    for (std::size_t k{0}; k < m_dimension; k++) {
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

                    hash ^= (1ULL << j); //flipping 1 bit in case we dont have enogh matching vectos

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
            /*
                |----------------------------------------------|
                | Magic number and version (LSH1)              |
                | Tuning parameters                            |
                | Data shapes - dimensionality and total count |
                | Table 0 -- projections                       |
                | Table 1 -- projections                       |
                | ...                                          |
                | Table 0 -- no of active hashes.              |
                | Table 0 -- hash -- no of ids -- ids          |
                | ...                                          |
                | m_vectors - id,Vector                        |
                |----------------------------------------------|
            */

            std::ofstream outf("LSH_and_GRAPH_BASED_INDEXING/LSH/persist" + filename, std::ios::binary);
            if (!outf) {
                throw FileNotFoundError("Uh oh, file: " + filename + " could not be opened for writing!\n");
            }
            
            using namespace std::string_literals;
            stream_write(LSHIndex::s_magic_bytes, "Insufficient space on disk"s, outf);
            stream_write(LSHIndex::s_version, "Insufficient space on disk"s, outf);

            stream_write(m_num_tables, "Insufficient space on disk"s, outf);
            stream_write(m_num_projections, "Insufficient space on disk"s, outf);
            stream_write(m_dimension, "Insufficient space on disk"s, outf);
            stream_write(m_count, "Insufficient space on disk"s, outf);

            for (const auto& planes : m_hyperplanes) {
                stream_write(planes, "Insufficient space on disk"s, outf); //template specialization
            }

            for (const auto& table : m_hash_tables) {
                stream_write(table.size(), "Insufficient space on disk"s, outf); //how many active hash for this table
                for (const auto& [hash, ids] : table) {
                    stream_write(hash, "Insufficient space on disk"s, outf); //hash_val of current hash
                    stream_write(ids.size(), "Insufficient space on disk"s, outf); //no of vectors in this buccket
                    stream_write(ids, "Insufficient space on disk"s, outf); //there ids
                }
            }

            for (const auto& vec : m_vectors) {
                stream_write(vec.first, "Insufficient space on disk"s, outf);
                stream_write(vec.second.data, "Insufficient space on disk"s, outf);
            }
        }

        void load(const std::string& filename) {
            
        }
};