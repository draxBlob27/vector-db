#include <chrono>
#include <fstream>
#include "VectorStore.hpp"
#include "Importer.hpp"
#include "Timer.hpp"

int main() {
    VectorStore vdb_10K{};
    VectorStore vdb_100K{};
    VectorStore vdb_1M{};
    Timer t{};

    auto sift_res{Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", 10000)};

    auto fill_db{[&](VectorStore& vdb) {
        std::vector<Vector> vectors{sift_res.ok_value().vectors};
        std::vector<std::uint64_t> ids{sift_res.ok_value().ids};

        for (std::size_t i{0}; i < vectors.size(); i++) {
            vdb.insert(ids[i], std::move(vectors[i]));
        }
    }};

    fill_db(vdb_10K);

    sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", 1'00'000);
    fill_db(vdb_100K);

    sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", 1'000'000);
    fill_db(vdb_1M);

    std::vector<std::vector<float>> queries{sift_res.ok_value().queries};
    std::vector<std::vector<std::uint32_t>> truths{sift_res.ok_value().truths};
    std::vector<std::uint32_t> truth_k{sift_res.ok_value().truth_k};
    

    std::size_t num_queries = 100;
    std::ofstream outf{"/home/sp27022003/vector-db/VectorDB_v0.1(Exhaustive Search Engine)/bench/benchmark_brute.txt"};

    auto calc_qps{[&](const VectorStore& vdb) {
        t.reset();
        for (std::size_t i{0}; i < num_queries; i++) {
            auto res = vdb.query(queries[i], 10);
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