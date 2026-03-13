#ifndef LSH_INDEX_HPP
#define LSH_INDEX_HPP

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>
#include <algorithm>
#include <string>
#include "utils/Vector.hpp"
#include "utils/distances.hpp"
#include "utils/errors.hpp"
#include "utils/Random_engine.hpp"
#include "utils/Serializer_De.hpp"

struct index_info {
    std::size_t candidate_set_size;
    int bitflips;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> collided_ids;
    std::size_t index_size;

    void reset() {
        candidate_set_size = 0;
        bitflips = 0;
        collided_ids.clear();
        index_size = 0;
    }
};

std::ostream& operator<<(std::ostream& out, const index_info& inf);

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

        index_info info;

        //function for finding hash val for each table with random projections --- 101010100011 like this
        std::uint64_t hash_val(const Vector& a, const std::vector<Vector>& planes);

    public:
        LSHIndex(const std::uint32_t& num_tables = 0, const std::uint32_t& num_projections = 0);

        void build(std::vector<std::pair<std::uint64_t, Vector>> vectors);

        std::vector<std::pair<std::uint64_t, float>> query(const Vector& query, const std::uint32_t& k);
        //returns vector of k size, with {id, score} in descending order
        
        void save(const std::string& filename); 
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

        void load(const std::string& filename);

        void reset();

        const index_info& getInfo() const;
};

#endif //LSH_INDEX_HPP