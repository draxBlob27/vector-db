#include "vectorDB/NSW_Index.hpp"

std::vector<std::pair<std::uint64_t, float>> NSW_Index::search_layer(const Vector& v, std::uint32_t ef, std::uint32_t M) const {
        if (ef == 0 && M == 0) {
            ef =  m_efConstruction; 
            M = m_M;
        }
        //To keep candidtate vectors in heap, we should take min heap, so then closest is at top.
        std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> candidates; //{score, id}
        //contains potential candiaties
        
        //contains efclose vectors, with top as farthest of close vector till now
        std::priority_queue<std::pair<float, std::uint64_t>> found_closest;

        // vis.insert(m_entry_point);
        m_visited[m_entry_point] = m_generation_counter;
        candidates.push({calc_distance<Metric::L2>(m_nodes[m_entry_point].vector, v), m_entry_point});
        found_closest.push({calc_distance<Metric::L2>(m_nodes[m_entry_point].vector, v), m_entry_point});

        while (!candidates.empty()) {//maintain the candidate size
            auto [score, nodeId] = candidates.top();
            candidates.pop();

            if (score > found_closest.top().first) { //if dist is worse than the farthest best vector then break
                break;
            }

            for (const auto& [__, nei] : m_nodes[nodeId].neighbors) {
                if (m_visited[nei] == m_generation_counter) {
                    continue;
                }

                m_visited[nei] = m_generation_counter;

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

        std::uint32_t sz = std::min(static_cast<uint32_t>(found_closest.size()), m_M);
        
        std::vector<std::pair<std::uint64_t, float>> best;
        best.reserve(sz);
        while (!found_closest.empty()) {
            if (static_cast<uint32_t>(found_closest.size()) <= M) {
                best.push_back({found_closest.top().second, found_closest.top().first});
            }

            found_closest.pop();
        }

        std::ranges::reverse(best);
        m_generation_counter++;
        return best;
    }

NSW_Index::NSW_Index(std::uint32_t M, std::uint32_t efConstruction, std::uint32_t efSearch) 
        :m_M{M}, m_efConstruction{efConstruction}, m_efSearch{efSearch}
    {}


void NSW_Index::insert(std::uint64_t id, const Vector& v) {
    // If this vector is first then assign it as entry point for incoming vectors.
    if (m_visited.size() < m_nodes.size() + 1) {
            m_visited.resize(m_nodes.size() + 5000);
    }

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

    for (const auto& [closest_id, closest_dist] : best) {
        bool is_discarded = false;
        for (const auto& [_, nei_id]: inc.neighbors) {
            if (calc_distance<Metric::L2>(m_nodes[nei_id].vector, m_nodes[closest_id].vector) < closest_dist) {
                is_discarded = true;
                break;
            }
        }

        if (!is_discarded) {
            inc.neighbors.push_back({closest_dist, closest_id}); //inesrt {score, id} of closest nei -- creating graph edges
    
            m_nodes[closest_id].neighbors.push_back({closest_dist, inc_id});
        }


        //if we connect this new inc vector to its closest nodes, then we need to recheck if there degree becomes more than M.
        //TODO -> in later stages
    }

    for (const auto& [closest_id, _] : best) {
        m_nodes[closest_id].align(m_M);
    }

    inc.align(m_M);
}

std::vector<std::pair<std::uint64_t, float>> NSW_Index::query(const Vector& v, std::uint32_t k, std::uint32_t efSearch) const {
    if (!efSearch) {
        efSearch = m_efSearch;
    }

    std::vector<std::pair<std::uint64_t, float>> best{search_layer(v, efSearch, k)};

    for (auto& [lg_id, score] : best) {
        lg_id = m_nodes[lg_id].id;
    }

    return best;
}

void NSW_Index::save(const std::string& filename) {
    std::ofstream outf{filename, std::ios::binary};
    Serializer_De::file_exists(filename, outf);

    using namespace std::string_literals;
    Serializer_De::stream_write(s_magic_bytes, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(s_version, "Insufficient space on disk"s, outf);

    Serializer_De::stream_write(m_efConstruction, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_efSearch, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_M, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_num_nodes, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_dimension, "Insufficient space on disk"s, outf);

    Serializer_De::stream_write(m_nodes, "Insufficient space on disk"s, outf);
    Serializer_De::stream_write(m_entry_point, "Insufficient space on disk"s, outf);
}

void NSW_Index::load (const std::string& filename) {
    std::ifstream inf{filename, std::ios::binary};
    Serializer_De::file_exists(filename, inf);
    
    using namespace std::string_literals;
    std::uint32_t magic_bytes, version;
    Serializer_De::stream_read(magic_bytes, "File corrupted on magic bytes\n"s, inf);
    Serializer_De::corruption_check(s_magic_bytes, magic_bytes);

    Serializer_De::stream_read(version, "File currupted on version\n", inf);
    Serializer_De::corruption_check(s_version, version);

    Serializer_De::stream_read(m_efConstruction,  "File currupted on efConstruction\n", inf);
    Serializer_De::stream_read(m_efSearch,  "File currupted on efConstruction\n", inf);
    Serializer_De::stream_read(m_M, "File currupted on efConstruction\n", inf);
    Serializer_De::stream_read(m_num_nodes,  "File currupted on num nodes\n", inf);
    Serializer_De::stream_read(m_dimension, "File currupted on dimension\n", inf);

    m_nodes.resize(m_num_nodes);
    Serializer_De::stream_read(m_nodes, "File currupted on nodes\n", inf);
    Serializer_De::stream_read(m_entry_point, "File currupted on entry point\n", inf);
}

bool operator==(const NSW_Index& n1, const NSW_Index& n2) {
    return (n1.m_num_nodes == n2.m_num_nodes &&
        n1.m_dimension == n2.m_dimension &&
        n1.m_entry_point == n2.m_entry_point &&
        n1.m_M == n2.m_M &&
        n1.m_efConstruction == n2.m_efConstruction &&
        n1.m_efSearch == n2.m_efSearch &&
        n1.m_nodes == n2.m_nodes);
}