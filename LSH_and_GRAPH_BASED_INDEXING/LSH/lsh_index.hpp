#include <vector>
#include <unordered_map>
#include "Vector.hpp"
#include "Random_engine.hpp"

class LSHIndex {
    private:
        std::vector<std::pair<std::uint64_t, Vector>> m_vectors;
        std::vector<std::unordered_map<std::uint64_t, std::vector<std::size_t>>> m_hash_tables;
        std::vector<std::vector<Vector>> m_hyperplanes;
        std::uint32_t m_num_tables, m_num_projections;

    public:
        LSHIndex(const std::uint32_t& num_tables, const std::uint32_t& num_projections)
            :m_num_tables{num_tables}, m_num_projections{num_projections}
        {
            m_hash_tables.resize(m_num_tables);
            m_hyperplanes.resize(m_num_tables, std::vector<Vector>(m_num_projections));
        }

        void build(const std::vector<Vector>& vectors) {
            for (std::uint32_t i{0}; i < m_num_tables; i++) {
                for (std::uint32_t j{0}; j < m_num_projections; j++) {
                    
                }
            }
        }

        std::vector<const std::pair<std::uint64_t, float>>& query(std::vector<float> query, const std::uint32_t& k) {

        }

        void save(const std::string& filename) {

        }

        void load(const std::string& filename) {
            
        }
};