#include <cmath>
#include <queue>
#include "vectorDB/VectorStore.hpp"
#include "vectorDB/utils/distances.hpp"

std::ostream& operator<<(std::ostream& out, const DBError& err) {
    switch (err) {
        case DBError::MetricError:
            out << "MetricError";
            break;
        case DBError::DimensionError:
            out << "DimensionError";
            break;
        case DBError::IdNotFoundError:
            out << "IdNotFoundError";
            break;
        case DBError::ZeroNormError:
            out << "ZeroNormError";
            break;
        case DBError::DataBaseEmptyError:
            out << "DataBaseEmptyError";
            break;
        case DBError::FileCorrupted:
            out << "FileCorrupted";
            break;
        case DBError::IdAlreadyPresent:
            out << "IdAlreadyPresent";
            break;
        case DBError::FileNotFound:
            out << "FileNotFound\n";
            break;
        default:
            out << "Unknown DBError";
            break;
    }
    return out;
}

Result<Unit, DBError> VectorStore::insert(std::uint64_t id, Vector i_vector) { //need to handle Id already exists, here my DB is going to own input_vector, hence rref makes sense, but pass by value is equally good. Because, internal calls are passed by rvalue hence, move semantics(2 moves), and explicit call by user, does 1 copy + 1 move, and user calls are limited to very less. If want to keep rref, user has to pass rvalue, otherwise compile error(no defn of func).
    auto [it, inserted] = m_id_set.insert(id);
    if (!inserted) {
        return Err<DBError>(DBError::IdAlreadyPresent);
    }

    auto dims_valid{[&]() {
        return m_vectors[0].second.data.size() == i_vector.data.size();
    }};

    if (m_vectors.empty() || dims_valid()) {
        i_vector.compute_norm(); //normalise at insgestion

        m_vectors.push_back({id, std::move(i_vector)}); //used move because it could be(i think..) wil see later on).
        return Ok<Unit>(Unit{});
    } else {
        m_id_set.erase(id);

        return Err<DBError>(DBError::DimensionError);
    }
}

Result<Unit, DBError> VectorStore::remove(std::uint64_t id) {
    auto find_id{[&id](std::pair<std::uint64_t, Vector>& a) {
        return a.first == id;
    }};
    auto new_logical_end = std::remove_if(m_vectors.begin(), m_vectors.end(), find_id);
    // auto new_logical_end = std::ranges::remove_if(m_vectors, find_id);

    if (new_logical_end == m_vectors.end()) { //incase of id not found
        return Err<DBError>(DBError::IdNotFoundError);
    }

    m_vectors.erase(new_logical_end, m_vectors.end());
    m_id_set.erase(id);
    return Ok<Unit>(Unit{});
}   

Result<std::vector<float>, DBError> VectorStore::get(std::uint64_t id) const {
    if (m_id_set.count(id)) {
        for (const auto& it : m_vectors) {
            if (it.first == id) {
                return Ok<std::vector<float>>{it.second.data};
            }
        }
    }

    return Err<DBError>{DBError::IdNotFoundError};
}

Result<std::vector<std::pair<std::uint64_t, float>>, DBError> VectorStore::query(const Vector& q_vector, std::uint64_t k, Metric metric) const {//very large object is getting created, can think of move semantics
    //very high chances of using quick select, just saw LC soln today(12 feb, 2026) regarding this
    //saying quick select is best in terms of TC ~ O(n) for best k kinda things
    if (m_vectors.empty()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
    }

    std::vector<std::pair<std::uint64_t, float>> res; //handles if DB size is less than k.

    switch (metric)
    {
    case(Metric::L2): {
        std::priority_queue<std::pair<float, std::uint64_t>> pq; //max head, becuase closer is better, and we keep the farthest best .ie. kth closest vector.
        
        if (m_vectors[0].second.size() != q_vector.size()) { //no need for each data point check because these are already verified at insertion.
            return Err<DBError>{DBError::DimensionError};
        }

        for (const auto& it : m_vectors) {
            /* code */
            //it.first -> id
            //it.second -> Vector
            float distance{calc_distance<Metric::L2>(it.second, q_vector)};

            if (pq.size() < k) { //smaller is better
                pq.push({distance, it.first}); //holds the id
            } else if (pq.top().first > distance) {
                pq.pop();
                pq.push({distance, it.first});
            }
        }

        while (!pq.empty()) {
            res.push_back({pq.top().second, pq.top().first});
            pq.pop();
        }

        std::ranges::reverse(res);
        break;
    }
    case(Metric::DotProduct): {
        std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> pq;

        
        if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
            return Err<DBError>{DBError::DimensionError};
        }

        for (const auto& it : m_vectors) {
            /* code */
            //it.first -> id
            //it.second -> Vector
            float distance{calc_distance<Metric::DotProduct>(it.second, q_vector)};
            
            if (pq.size() < k) { //larger is better due to similarity -> vector more aligned
                pq.push({distance, it.first}); //holds the id
            } else if (pq.top().first < distance) {
                pq.pop();
                pq.push({distance, it.first});
            }
        }

        while (!pq.empty()) {
            res.push_back({pq.top().second, pq.top().first});
            pq.pop();
        }

        std::ranges::reverse(res);
        break;
    }
    case (Metric::Cosine): {
        std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>> pq; //min heap, because closer vectos have higher score and hence we keep at top the kth closest vector.
        
        Vector copied_q_vector{q_vector};
        copied_q_vector.compute_norm(); //perform normailzation of query vector
        float query_norm{copied_q_vector.norm()};
        //if query norm is 0 then outright riject it.
        if (query_norm == 0.0f) {
            return Err<DBError>{DBError::ZeroNormError};
        }

        if (m_vectors[0].second.data.size() != q_vector.data.size()) { //no need for each data point check because these are already verified at insertion.
            return Err<DBError>{DBError::DimensionError};
        }
        
        for (const auto& it : m_vectors) {
            /* code */
            if (it.second.norm() == 0.0f) {
                continue;
            }
            
            float distance{calc_distance<Metric::Cosine>(it.second, copied_q_vector)};

            if (pq.size() < k) {
                pq.push({distance, it.first}); //holds the id
            } else if (pq.top().first < distance) {
                pq.pop();
                pq.push({distance, it.first});
            }
        }

        while (!pq.empty()) {
            res.push_back({pq.top().second, pq.top().first});
            pq.pop();
        }

        std::ranges::reverse(res);
        break;
    }
    default:
        return Err<DBError>{DBError::MetricError};
    }

    return Ok(res);
}

Result<std::vector<std::pair<std::uint64_t, float>>, DBError> VectorStore::query_parallel(Vector q_vector, std::uint64_t k, Metric metric) const {//very large object is getting created, can think of move semantics

    if (m_vectors.empty()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
    }

    if (m_vectors[0].second.size() != q_vector.size()) {//check for dimension mismatch with
        return Err<DBError>{DBError::DimensionError};
    }

    std::size_t threads{std::thread::hardware_concurrency()};
    std::size_t chunks = m_vectors.size() / threads;

    std::vector<std::pair<std::uint64_t, float>> res; //handles if DB size is less than k.
    std::vector<std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>, std::greater<>>> mn_pqs(threads);
    std::vector<std::priority_queue<std::pair<float, std::uint64_t>>> mx_pqs(threads);//max heap, becuase closer is better, and we keep the farthest best .ie. kth closest vector.
    q_vector.compute_norm();
    // if query norm is 0 then outright riject it.
    if (q_vector.norm() == 0.0f) {
        return Err<DBError>{DBError::ZeroNormError};
    }

    auto l2_lambda = [&](const auto& item, std::size_t tid = -1) {
        /* code */
        //it.first -> id
        //it.second -> Vector
        std::size_t cmp = -1;
        if (tid == cmp) {
            thread_local int tbb_tid = global_tcnt.fetch_add(1);
            tid = tbb_tid;
        }
        float distance{calc_distance<Metric::L2>(item.second, q_vector)};

        if (mx_pqs[tid].size() < k) { //smaller is better
            mx_pqs[tid].push({distance, item.first}); //holds the id
        } else if (mx_pqs[tid].top().first > distance) {
            mx_pqs[tid].pop();
            mx_pqs[tid].push({distance, item.first});
        }
    };

    auto cosine_lambda = [&](const auto& item, std::size_t tid = -1) {
        if (item.second.norm() == 0.0f) {
            return;
        }

        thread_local int tbb_tid = global_tcnt.fetch_add(1);
        std::size_t cmp = -1;
        if (tid == cmp) {
            tid = tbb_tid;
        }
        
        float distance{calc_distance<Metric::Cosine>(item.second, q_vector)};

        if (mn_pqs[tid].size() < k) {
            mn_pqs[tid].push({distance, item.first}); //holds the id
        } else if (mn_pqs[tid].top().first < distance) {
            mn_pqs[tid].pop();
            mn_pqs[tid].push({distance, item.first});
        }
    };

    auto dot_lambda = [&](const auto& item, std::size_t tid = -1) {
        /* code */
        //it.first -> id
        //it.second -> Vector

        thread_local int tbb_tid = global_tcnt.fetch_add(1);
        std::size_t cmp = -1;
        if (tid == cmp) {
            tid = tbb_tid;
        }

        float distance{calc_distance<Metric::DotProduct>(item.second, q_vector)};
        
        if (mn_pqs[tid].size() < k) { //larger is better due to similarity -> vector more aligned
            mn_pqs[tid].push({distance, item.first}); //holds the id
        } else if (mn_pqs[tid].top().first < distance) {
            mn_pqs[tid].pop();
            mn_pqs[tid].push({distance, item.first});
        }
    };

    auto run_parallel = [&](auto lambda) {
        //MANNUAL PARTITIONING - QPS = 65+
        std::vector<std::thread> workers;
        std::size_t chunk = m_vectors.size() / threads;

        for (std::size_t t{0}; t < threads; t++) {
            std::size_t start = t * chunk;
            std::size_t end = (t == threads-1) ? m_vectors.size() : start + chunk;

            workers.emplace_back([&, t, start, end]() {
                for (std::size_t i{start}; i < end; i++)
                    lambda(m_vectors[i], t);  // no atomic, no contention
            });
        }

        //ATOMIC COUNTER APPROACH - QPS = 19
        // std::atomic<std::size_t> ctr{0};
        // std::vector<std::thread> workers;

        // auto worker = [&](std::size_t tid) {
        //     for (std::size_t n{ctr++}; n < m_vectors.size(); n = ctr++) {
        //         lambda(m_vectors.at(n), tid);
        //     }
        // };

        // for (std::size_t n{0}; n < threads; n++) {
        //     workers.emplace_back(worker, n);
        // }


        //UNCOMMENT FOR ABOVE METHODS
        for(auto& w : workers)
            w.join();

        //TBB CONCURRENCY APPROACH - QPS = 70+
        //Gets race condition
        // std::for_each(std::execution::par, m_vectors.cbegin(), m_vectors.cend(), lambda);
    };

    switch(metric) {
        case(Metric::Cosine): {
            run_parallel(cosine_lambda);
            break;
        }
        case(Metric::L2): {
            run_parallel(l2_lambda);
            break;
        }
        case(Metric::DotProduct): {
            run_parallel(dot_lambda);
            break;
        }
    }
    

    std::priority_queue<std::pair<float, std::uint64_t>, std::vector<std::pair<float, std::uint64_t>>> mn_pq;
    std::priority_queue<std::pair<float, std::uint64_t>> mx_pq;

    for (auto& pq : mx_pqs) {
        //for n threads, we got n pqs, each with k nearest vectors locally.
        //now we will merge them to best k globally.
        
        while (!pq.empty()) {
            mx_pq.push(pq.top());
            pq.pop();
            if (mx_pq.size() > k) {
                mx_pq.pop();
            }
        }
    }

    for (auto& pq : mn_pqs) {
        while (!pq.empty()) {
            mn_pq.push(pq.top());
            pq.pop();
            if (mn_pq.size() > k) {
                mn_pq.pop();
            }
        }
    }

    while (!mx_pq.empty()) {
        res.push_back({mx_pq.top().second, mx_pq.top().first});
        mx_pq.pop();
    }

    while (!mn_pq.empty()) {
        res.push_back({mn_pq.top().second, mn_pq.top().first});
        mn_pq.pop();
    }

    std::ranges::reverse(res);

    return Ok(res);
}


Result<Unit, DBError> VectorStore::save(const std::string& filename) const {
    // std::uint32_t crc_32_header{0xFFFFFFFF};
    const std::uint64_t count{m_vectors.size()};
    if (!count) {
        //throw InvalidOperationError("Empty vectors.");
        return Err<DBError>{DBError::DataBaseEmptyError};
    }

    const std::uint32_t dimension{static_cast<std::uint32_t>(m_vectors[0].second.data.size())};
    if (!dimension) {
        return Err<DBError>{DBError::DimensionError};
        // throw InvalidOperationError("Empty vectors.");
    }
    
    std::ofstream outf{filename, std::ios::binary};
    if (!outf) {
        return Err<DBError>{DBError::FileNotFound};
        // throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for writing!\n");
    }
    
    outf.write(reinterpret_cast<const char*>(&VectorStore::s_magic_bytes), sizeof(VectorStore::s_magic_bytes));
    if (outf.bad() || outf.fail()) {
        return Err<DBError>{DBError::FileCorrupted};
        // throw InsufficientSpaceError("Insufficient space on disk.");
    }
    // update_crc(crc_32_header, &VectorStore::s_magic_bytes, sizeof(VectorStore::s_magic_bytes));
    
    outf.write(reinterpret_cast<const char *>(&VectorStore::s_version), sizeof(VectorStore::s_version));
    if (outf.bad() || outf.fail()) {
        return Err<DBError>{DBError::FileCorrupted};
        // throw InsufficientSpaceError("Insufficient space on disk.");
    }
    // update_crc(crc_32_header, &VectorStore::s_version, sizeof(VectorStore::s_version));

    outf.write(reinterpret_cast<const char *>(&dimension), sizeof(dimension));
    if (outf.bad() || outf.fail()) {
        return Err<DBError>{DBError::FileCorrupted};
        // throw InsufficientSpaceError("Insufficient space on disk.");
    }
    // update_crc(crc_32_header, &dimension, sizeof(dimension));
    
    outf.write(reinterpret_cast<const char *>(&count), sizeof(count));
    if (outf.bad() || outf.fail()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw InsufficientSpaceError("Insufficient space on disk.");
    }
    // update_crc(crc_32_header, &count, sizeof(count));
    
    // crc_32_header ^= 0xFFFFFFFF;
    // outf.write(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
    // if (outf.bad() || outf.fail()) {
    //     throw InsufficientSpaceError("Insufficient space on disk.");
    // }

    // std::uint32_t crc_32_data{0xFFFFFFFF};
    for (std::uint64_t i{0}; i < count; i++)
    {
        if (dimension != m_vectors[i].second.data.size()) {
            return Err<DBError>{DBError::DimensionError};
            // throw InvalidOperationError("Dimension of data mismatch.");
        }
        
        //write id to disk.
        outf.write(reinterpret_cast<const char *>(&m_vectors[i].first), sizeof(std::uint64_t));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::FileCorrupted};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        // update_crc

        unsigned const char* d_ptr = reinterpret_cast<unsigned const char*>(&m_vectors[i].second.data[0]);
        outf.write(reinterpret_cast<const char*>(d_ptr), dimension * sizeof(float));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        
        //write norm val to disk
        outf.write(reinterpret_cast<const char*>(&m_vectors[i].second.norm_data), sizeof(m_vectors[i].second.norm_data));
        if (outf.bad() || outf.fail()) {
            return Err<DBError>{DBError::FileCorrupted};
            // throw InsufficientSpaceError("Insufficient space on disk.");
        }
        // update_crc
    }

    // crc_32_data ^= 0xFFFFFFFF;
    // outf.write(reinterpret_cast<char *>(&crc_32_data), sizeof(std::uint32_t));
    // if (outf.bad() || outf.fail()) {
    //     throw InsufficientSpaceError("Insufficient space on disk.");
    // }

    return Ok<Unit>{Unit{}};
}

Result<Unit, DBError> VectorStore::load(const std::string& filename) { //can use std::string_view -> improves perf, but does it matter that much. We'll seee
    std::ifstream inf{filename, std::ios::binary};
    if (!inf) {
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw FileNotFoundError("Uh oh, file: " + file_path + " could not be opened for reading!\n");
    }

    // std::uint32_t calc_crc_32_header{0xFFFFFFFF};
    std::uint32_t magic_bytes;
    inf.read(reinterpret_cast<char *>(&magic_bytes), sizeof(std::uint32_t));
    if (inf.fail() || inf.bad()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw ArchiveError("Could not read file.");
    }
    if (magic_bytes != VectorStore::s_magic_bytes)
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw CorruptedDataError("Magic bytes mismatch."); // placeholder;
    // update_crc(calc_crc_32_header, &magic_bytes, sizeof(magic_bytes));

    std::uint32_t version;
    inf.read(reinterpret_cast<char *>(&version), sizeof(std::uint32_t));
    if (inf.fail() || inf.bad()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw ArchiveError("Could not read file.");
    }
    if (version != VectorStore::s_version)
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw CorruptedDataError("Version mismatch.");
    // update_crc(calc_crc_32_header, &version, sizeof(version));

    std::uint32_t dimension;
    inf.read(reinterpret_cast<char *>(&dimension), sizeof(std::uint32_t));
    if (inf.fail() || inf.bad()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw ArchiveError("Could not read file.");
    }
    // update_crc(calc_crc_32_header, &dimension, sizeof(dimension));

    std::uint64_t count;
    inf.read(reinterpret_cast<char *>(&count), sizeof(std::uint64_t));
    if (inf.fail() || inf.bad()) {
        return Err<DBError>{DBError::DataBaseEmptyError};
        // throw ArchiveError("Could not read file.");
    }
    // update_crc(calc_crc_32_header, &count, sizeof(count));

    // std::uint32_t crc_32_header;
    // inf.read(reinterpret_cast<char*>(&crc_32_header), sizeof(crc_32_header));
    // if (inf.fail() || inf.bad()) {
    //     return Err<DBError>{DBError::DataBaseEmptyError};
    //     // throw ArchiveError("Could not read file.");
    // }

    // calc_crc_32_header ^= 0xFFFFFFFF;
    // if (crc_32_header != calc_crc_32_header) {
        // throw CorruptedDataError("Header CRC mismatch.");
    // }

    // std::uint32_t calc_crc_32_data{0xFFFFFFFF};
    // std::vector<std::vector<double>> data(count, std::vector<double>(dimension));
    //to implement check for file size here

    m_vectors.resize(count);

    for (std::uint64_t i = 0; i < count; i++)
    {
        m_vectors[i].second.data.resize(dimension);

        inf.read(reinterpret_cast<char*>(&m_vectors[i].first), sizeof(std::uint64_t));//reads and loads id.

        unsigned char *d_ptr = reinterpret_cast<unsigned char *>(&m_vectors[i].second.data[0]);
        inf.read(reinterpret_cast<char*>(d_ptr), dimension * sizeof(float));
        if (inf.fail() || inf.bad()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw ArchiveError("Could not read file.");
        }

        //reads norm data and load it
        inf.read(reinterpret_cast<char *>(&m_vectors[i].second.norm_data), sizeof(m_vectors[i].second.norm_data));
        if (inf.fail() || inf.bad()) {
            return Err<DBError>{DBError::DataBaseEmptyError};
            // throw ArchiveError("Could not read file.");
        }

        // if (!already_verified) {
        //     update_crc(calc_crc_32_data, d_ptr, dimension * sizeof(double));
        // }
    }

    // if (!already_verified)
    //     calc_crc_32_data ^= 0xFFFFFFFF;

    // std::uint32_t crc_32_data;
    // inf.read(reinterpret_cast<char *>(&crc_32_data), sizeof(std::uint32_t));
    // if (inf.fail() || inf.bad()) {
    //     throw ArchiveError("Could not read file.");
    // }

    // if (!already_verified && calc_crc_32_data != crc_32_data)
    //     throw CorruptedDataError("Data CRC mismatch.");

    return Ok<Unit>{Unit{}};
}