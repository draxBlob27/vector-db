#include <chrono>
#include <fstream>
#include "VectorStore.hpp"
#include "Importer.hpp"

class Timer {
    using millisec = std::chrono::duration<double, std::ratio<1, 1000>>;

    using Clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<Clock> m_beg{Clock::now()};

public:
    void reset() {
        m_beg = Clock::now();
    }

    double elapsed() const {
        return std::chrono::duration_cast<millisec>(Clock::now() - m_beg).count();
    }
};

int main() {
    VectorStore vdb_10K{};
    VectorStore vdb_100K{};
    VectorStore vdb_1M{};
    Timer t{};

    auto sift_res{Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", vdb_10K, 10000)};

    sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", vdb_100K, 1'00'000);

    sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", vdb_1M, 1'000'000);

    std::vector<std::vector<float>> queries{sift_res.ok_value().queries};
    std::vector<std::vector<std::uint32_t>> truths{sift_res.ok_value().truths};
    std::vector<std::uint32_t> truth_k{sift_res.ok_value().truth_k};

    std::size_t num_queries = 1000;
    std::ofstream outf{"/home/sp27022003/vector-db/VectorDB_v0.1(Exhaustive Search Engine)/bench/benchmark_brute.txt"};

    auto calc_qps{[&](const VectorStore& vdb) {
        t.reset();
        for (std::size_t i{0}; i < num_queries; i++) {
            auto res = vdb.query(queries[i], truth_k[i]);
        }

        double dur = t.elapsed() / num_queries;

        outf << "MsPQ for " << vdb.size().ok_value() << " vectors : " << dur << " Milliseconds\n";

        outf << "QPS for " << vdb.size().ok_value() << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";

        outf.flush();
    }};

    calc_qps(vdb_10K);
    calc_qps(vdb_100K);
    calc_qps(vdb_1M);
}