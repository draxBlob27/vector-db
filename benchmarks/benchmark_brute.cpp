#include <chrono>
#include <fstream>
#include "vectorDB/VectorStore.hpp"
#include "vectorDB/utils/Importer.hpp"
#include "vectorDB/utils/Timer.hpp"

int main() {
    VectorStore vdb_10K{};
    // VectorStore vdb_100K{};
    // VectorStore vdb_1M{};
    Timer t{};

    auto sift_res{Importer::import_sift1m("/home/sp27022003/vector-db/sift10K/siftsmall_base.fvecs", "/home/sp27022003/vector-db/sift10K/siftsmall_query.fvecs", "/home/sp27022003/vector-db/sift10K/siftsmall_groundtruth.ivecs", 10000)};

    auto fill_db{[&](VectorStore& vdb) {
        std::vector<Vector> vectors{sift_res.ok_value().vectors};
        std::vector<std::uint64_t> ids{sift_res.ok_value().ids};

        for (std::size_t i{0}; i < vectors.size(); i++) {
            vdb.insert(ids[i], std::move(vectors[i]));
        }
    }};

    fill_db(vdb_10K);

    // sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", 1'00'000);
    // fill_db(vdb_100K);

    // sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", 1'000'000);
    // fill_db(vdb_1M);

    std::vector<std::vector<float>> queries{sift_res.ok_value().queries};
    std::vector<std::vector<std::uint32_t>> truths{sift_res.ok_value().truths};
    std::vector<std::uint32_t> truth_k{sift_res.ok_value().truth_k};

    std::size_t num_queries = queries.size();
    std::ofstream outf{"/home/sp27022003/vector-db/VectorDB_v0.1(Exhaustive Search Engine)/bench/benchmark_brute.txt"};

    auto calc_qps{[&](const VectorStore& vdb) {
        t.reset();
        int intersection{0};
        int k_sz{0};
        for (std::size_t i{0}; i < num_queries; i++) {
            int my_k = truth_k[i];
            auto res = vdb.query(queries[i], my_k, Metric::L2).ok_value();
            k_sz += res.size();
            for (const auto& [id, _] : res) {
                // std::cout << id << "\n";
                for (int j{0}; j < res.size(); j++) {
                    // std::cout << truths[i][j] << " ";
                    if (truths[i][j] == id) {
                        intersection++;
                        break;
                    }
                }
                // std::cout << '\n';
            }
        }

        double dur = t.elapsed() / num_queries;
        int sz = vdb.size().ok_value();

        std::cout << "Intersection: " << intersection << "\n";
        std::cout << "Total: " << k_sz << "\n";

        outf << "MsPQ for " << sz << " vectors : " << dur << " Milliseconds\n";

        double recall = static_cast<double>(intersection) / k_sz;

        outf << "Recall@10 for " << sz << " vectors : " << recall * 100 << "%\n";

        outf << "QPS for " << sz << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";

        outf.flush();
    }};

    calc_qps(vdb_10K);
    // calc_qps(vdb_100K);
    // calc_qps(vdb_1M);
}