
#include "vectorDB/NSW_Index.hpp"

std::vector<std::pair<std::uint64_t, float>> NSW_Index::find_neighbors(const Vector& v, std::uint32_t ef, std::uint32_t M) const {
    if (ef == 0 && M == 0) {
        return NSW_Index::find_neighbors(v, m_efConstruction, m_M);
    }

    if (m_num_nodes.load(std::memory_order_acquire) == 0) {
        return {};
    }
    //To keep candidtate vectors in heap, we should take min heap, so then closest is at top.
    std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> candidates; //{score, id}
    //contains potential candiaties
    
    //contains efclose vectors, with top as farthest of close vector till now
    std::priority_queue<std::pair<float, std::uint64_t>> found_closest;

    // std::unordered_set<std::uint64_t> vis;
    std::vector<int> vis(NSW_Index::m_num_nodes.load(std::memory_order_acquire));

    // vis.insert(m_entry_point.load(std::memory_order_acquire));
    vis[m_entry_point.load(std::memory_order_acquire)] = 1;
    candidates.push({calc_distance<Metric::L2>(m_nodes[m_entry_point.load(std::memory_order_acquire)].vector, v), m_entry_point.load(std::memory_order_acquire)});
    found_closest.push({calc_distance<Metric::L2>(m_nodes[m_entry_point.load(std::memory_order_acquire)].vector, v), m_entry_point.load(std::memory_order_acquire)});

    while (!candidates.empty()) {//maintain the candidate size
        auto [score, nodeId] = candidates.top();
        candidates.pop();

        if (score > found_closest.top().first) { //if dist is worse than the farthest best vector then break
            break;
        }

        for (const auto& [__, nei] : m_nodes[nodeId].neighbors) {
            // auto [_, inserted] = vis.insert(nei);

            // if (!inserted) {
            if (vis[nei]) {
                continue;
            }

            vis[nei] = 1;

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

    return best; //no issue of copoying, becase of mandatory copy elision in RVO
}

NSW_Index::NSW_Index(std::uint32_t M, std::uint32_t efConstruction, std::uint32_t efSearch) 
        :m_M{M}, m_efConstruction{efConstruction}, m_efSearch{efSearch}
    {}

void NSW_Index::link_node(std::uint64_t id, const Vector& v, const std::vector<std::pair<std::uint64_t, float>>& best) {
    // If this vector is first then assign it as entry point for incoming vectors.
    
    //is a read-modify-write operation
    //directly constructs node object at address allocated
    std::uint64_t index = m_num_reserved.fetch_add(1, std::memory_order_relaxed);
    new (&m_nodes[index]) Node(id, v);
    
    if (index == 0) {
        m_entry_point.store(0, std::memory_order_release);
        m_dimension = static_cast<uint32_t>(v.size());
        return;
    }

    Node& inc(m_nodes[index]); //incoming vector node

    for (const auto& [closest_id, closest_dist] : best) {
        float min_dist{std::numeric_limits<float>::max()};
        for (const auto& [_, nei_id]: inc.neighbors) {
            min_dist = std::min(min_dist, calc_distance<Metric::L2>(m_nodes[nei_id].vector, m_nodes[closest_id].vector));
        }

        //connect only if closest_dist is less than min of currently connected nodes of incoming vector/node
        if (closest_dist < min_dist) {
            inc.neighbors.push_back({closest_dist, closest_id}); //inesrt {score, id} of closest nei -- creating graph edges
        }
    }

    inc.align(m_M);

    //if the nodes with which new node connects, got degree issue, that is solved here.
    for (const auto& [_, nei_id] : inc.neighbors) {
        m_nodes[nei_id].add_edge_and_prune(
            calc_distance<Metric::L2>(m_nodes[nei_id].vector, inc.vector),
            index,
            m_M
        );
    }

    std::uint64_t expected = index;
    while (m_num_nodes.load(std::memory_order_acquire) != expected) {
        __builtin_ia32_pause();
    }

    // if (m_num_nodes == 1'000'000) {
    //     // graph_ready.count_down();
    // }
    m_num_nodes.fetch_add(1, std::memory_order_release);
}

void NSW_Index::insert(std::uint64_t id, const Vector& v) {
    NSW_Index::link_node(id, v, NSW_Index::find_neighbors(v));
    return;
}

std::vector<std::pair<std::uint64_t, float>> NSW_Index::query(const Vector& v, std::uint32_t k, std::uint32_t efSearch) const {
    // graph_ready.wait();
    //this guarantees, that the thread will see correct graph of db_size;
    if (!efSearch) {
        return query(v, k, m_efSearch);
    }

    std::vector<std::pair<std::uint64_t, float>> best{find_neighbors(v, efSearch, k)};

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
    Serializer_De::stream_write(m_entry_point.load(std::memory_order_acquire), "Insufficient space on disk"s, outf);
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

    // m_nodes.resize(m_num_nodes);
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