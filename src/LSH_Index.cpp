#include "vectorDB/LSH_Index.hpp"

std::ostream& operator<<(std::ostream& out, const index_info& inf) {
    // out << "Bitflips: " << inf.bitflips << '\n';
    // out << "Candidate" << inf.candidate_set_size << '\n';
    out << "Index size: " << inf.index_size;
    return out;
}

LSHIndex::LSHIndex(const std::uint32_t& num_tables, const std::uint32_t& num_projections)
            :m_num_tables{num_tables}, m_num_projections{num_projections}
        {
            m_hash_tables.resize(m_num_tables);
            m_hyperplanes.resize(m_num_tables, std::vector<Vector>(m_num_projections));
        }

std::uint64_t LSHIndex::hash_val(const Vector& a, const std::vector<Vector>& planes) {
                
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

void LSHIndex::build(std::vector<std::pair<std::uint64_t, Vector>> vectors)
{
    m_dimension = vectors.front().second.size();//will implement error handling later on
    m_count = vectors.size(); //total count of data points

    for (std::uint32_t i{0}; i < m_num_tables; i++) { 
        std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
        for (std::uint32_t j{0}; j < m_num_projections; j++) { //for each table(independent) we have this many projectstions
            std::vector<float> normal(m_dimension); //create hyperplances in higher dim space
            for (std::size_t k{0}; k < m_dimension; k++) {
                normal[k] = Random::get<float>(-0.0f, 1.0f); //hyperplanes wrt origin
            }

            t_hyperplanes[j] = Vector(normal);
        }
    }
    
    m_vectors.resize(vectors.size());
    for (std::size_t emb_cnt{0}; emb_cnt < vectors.size(); emb_cnt++) {
        m_vectors[emb_cnt] = std::move(vectors[emb_cnt]);
        m_vectors[emb_cnt].second.compute_norm();
        const Vector& emb = m_vectors[emb_cnt].second;

        for (std::uint32_t i{0}; i < m_num_tables; i++) {
            std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
            

            auto hash = hash_val(emb, t_hyperplanes);

            m_hash_tables[i][hash].push_back(emb_cnt); //in this table, we got hash_val for curr emb, we map this table->hash_val->id for retrieval later on
        }
    }
}

std::vector<std::pair<std::uint64_t, float>> LSHIndex::query(const Vector& query, const std::uint32_t& k) {
    info.reset();
    info.collided_ids.resize(m_num_tables);
    info.index_size = m_count;

    // std::unordered_set<std::size_t> st;
    std::vector<std::size_t> st;

    Vector copied_query{query};
    copied_query.compute_norm(); //perform normailzation of query vector

    std::vector<std::uint64_t> hash_vals(m_num_tables);

    for (std::size_t i{0}; i < m_num_tables; i++) {
        std::vector<Vector>& t_hyperplanes{m_hyperplanes[i]}; //this table hyplerplanes
        std::uint64_t hash = hash_val(copied_query, t_hyperplanes);
        hash_vals[i] =  hash;//hash val of query with this hyperplane
        
        auto got = m_hash_tables[i].find(hash);
        if (got != m_hash_tables[i].end()) {
            for (const auto& it : got->second) { //extracting collided vectors with same hash val
                // auto [_, inserted] = st.insert(it); //de-duplicating
                // if (inserted) {
                //     cnt++;
                // }
                st.push_back(it);
            }
        }

        // if (cnt) {
        //     info.collided_ids[i] = {hash, cnt};
        // } else {
        //     info.collided_ids[i] = {-1, -1};
        // }
    }

    std::uint32_t j{0};
    while (st.size() < k && j < m_num_projections) {
        for (std::uint32_t i{0}; i < m_num_tables; i++) {
            std::uint64_t hash = hash_vals[i];

            hash ^= (1ULL << j); //flipping 1 bit in case we dont have enogh matching vectos

            auto got = m_hash_tables[i].find(hash);
            if (got != m_hash_tables[i].end()) {
                for (const auto& it : got->second) { //extracting collided vectors with same hash val
                    st.push_back(it); //de-duplicating
                }
            } 
        }

        j++;
        info.bitflips++;
    }

    std::ranges::sort(st);
    auto it = std::ranges::unique(st);
    st.erase(it.begin(), it.end());

    info.candidate_set_size = st.size();

    // std::priority_queue<std::pair<float, uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> pq; //min heap
    std::priority_queue<std::pair<float, uint64_t>> pq; //max_heap

    for (const auto& it: st) {
        m_vectors[it].second.compute_norm();
        float dist = calc_distance<Metric::L2>(m_vectors[it].second, copied_query);
        //cosine metirc, higher val is better;
        // if (pq.size() >= k) {
        //     if (pq.top().first < dist) {
        //         pq.pop();
        //         pq.push({dist, m_vectors[it].first});
        //     }
        // } else {
        //     pq.push({dist, m_vectors[it].first});
        // }

        //For L2
        if (pq.size() >= k) {
            if (pq.top().first > dist) {
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
    
    std::ranges::reverse(res);
    
    return res;
}

void LSHIndex::save(const std::string& filename) {
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

    std::ofstream outf(filename, std::ios::binary);
    Serializer_De::file_exists(filename, outf);
    
    using namespace std::string_literals;
    Serializer_De::stream_write(LSHIndex::s_magic_bytes, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(LSHIndex::s_version, "Insufficient space on disk"s, outf);

    Serializer_De::stream_write(m_num_tables, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_num_projections, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_dimension, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_count, "Insufficient space on disk"s, outf);

    for (const auto& planes : m_hyperplanes) {
        Serializer_De::stream_write(planes, "Insufficient space on disk"s, outf); //template specialization
    }

    for (const auto& table : m_hash_tables) {
        Serializer_De::stream_write(table.size(), "Insufficient space on disk"s, outf); //how many active hash for this table
        for (const auto& [hash, ids] : table) {
            Serializer_De::stream_write(hash, "Insufficient space on disk"s, outf); //hash_val of current hash
            Serializer_De::stream_write(ids.size(), "Insufficient space on disk"s, outf); //no of vectors in this buccket
            Serializer_De::stream_write(ids, "Insufficient space on disk"s, outf); //there ids
        }
    }

    for (const auto& vec : m_vectors) {
        Serializer_De::stream_write(vec.first, "Insufficient space on disk"s, outf);
        Serializer_De::stream_write(vec.second.data, "Insufficient space on disk"s, outf);
    }
}

void LSHIndex::load(const std::string& filename) {
    std::ifstream inf(filename, std::ios::binary);
    Serializer_De::file_exists(filename, inf);
    
    using namespace std::string_literals;
    std::uint32_t magic_bytes, version;


    Serializer_De::stream_read(magic_bytes, "File corrupted\n"s, inf);
    Serializer_De::corruption_check(magic_bytes, s_magic_bytes);

    Serializer_De::stream_read(version, "File corrupted\n"s, inf);
    Serializer_De::corruption_check(version, s_version);

    Serializer_De::stream_read(m_num_tables, "File corrupted\n"s, inf);
    // std::cout << "tables: " << m_num_tables << "\n";
    Serializer_De::stream_read(m_num_projections, "File corrupted\n"s, inf);
    // std::cout << "projections: " << m_num_projections << "\n";
    Serializer_De::stream_read(m_dimension, "File corrupted\n"s, inf);
    // std::cout << "dimension: " << m_dimension << "\n";
    Serializer_De::stream_read(m_count, "File corrupted\n"s, inf);
    // std::cout << "count: " << m_count << "\n";

    m_hash_tables.resize(m_num_tables);
    m_hyperplanes.resize(m_num_tables);

    for (auto& plane: m_hyperplanes) {
        plane.resize(m_num_projections);
        for (auto& v : plane) {
            v.data.resize(m_dimension);
        }
    }

    for (std::size_t i{0}; i < m_num_tables; i++) {
        for (std::size_t j{0}; j < m_num_projections; j++) {
            Serializer_De::stream_read(m_hyperplanes[i][j], "File corrupted\n"s, inf);
        }
    }

    for (std::size_t i{0}; i < m_num_tables; i++) {
        std::size_t active_hashes;
        Serializer_De::stream_read(active_hashes, "File corrupted\n"s, inf);
        m_hash_tables[i].reserve(active_hashes);

        for (std::size_t j{0}; j < active_hashes; j++) {
            std::uint64_t hash;
            Serializer_De::stream_read(hash, "File corrupted\n"s, inf);

            std::size_t collisions;
            Serializer_De::stream_read(collisions, "File corrupted\n"s, inf);
            m_hash_tables[i][hash].resize(collisions);
            Serializer_De::stream_read(m_hash_tables[i][hash], "File corrupted\n"s, inf);
        }
    }

    m_vectors.resize(m_count);

    for (std::size_t i{0}; i < m_count; i++) {
        std::uint64_t id;
        Vector v;

        Serializer_De::stream_read(id, "File corrupted\n"s, inf);
        v.data.resize(m_dimension);
        Serializer_De::stream_read(v, "File corrupted\n"s, inf);

        m_vectors[i] = {id, std::move(v)};
    }
}

void LSHIndex::reset() {
    m_vectors.clear();
    m_dimension = 0;
    m_count = 0;
    m_hash_tables.clear();
    m_hyperplanes.clear();
    m_num_tables = 0, m_num_projections = 0;

    info.reset();
}

const index_info& LSHIndex::getInfo() const {
    return info;
}