#include <chrono>
#include <fstream>
#include "vectorDB/ThreadSafeVectorStore.hpp"
#include "vectorDB/utils/Importer.hpp"
#include "vectorDB/utils/Timer.hpp"

int main() {
    ThreadSafeVectorStore vdb{};
    Timer t{};

    std::string dir = "/home/rohitfeb641/vector-db/sift1M/";
        
    auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs").take_ok_value();


    auto fill_db{[&](ThreadSafeVectorStore& vdb) {
        std::vector<Vector> vectors{std::move(sift_res.vectors)};
        std::vector<std::uint64_t> ids{std::move(sift_res.ids)};

        for (std::size_t i{0}; i < vectors.size(); i++) {
            vdb.insert(ids[i], std::move(vectors[i]));
        }
    }};

    fill_db(vdb);

    const std::vector<std::vector<float>>& queries{sift_res.queries};
    const std::vector<std::vector<std::uint32_t>>& truths{sift_res.truths};
    const std::vector<std::uint32_t>& truth_k{sift_res.truth_k};

    std::size_t num_queries = 100;
    std::ofstream outf{"/home/rohitfeb641/vector-db/benchmarks/benchmark_brute_threadsafe.txt", std::ios::app};

    auto calc_qps{[&](const ThreadSafeVectorStore& vdb) {
        t.reset();
        int intersection{0};
        int k_sz{0};
        for (std::size_t i{0}; i < num_queries; i++) {
            int my_k = truth_k[i];
            auto res = vdb.query(queries[i], my_k, Metric::L2).ok_value();
            k_sz += res.size();
            for (const auto& [id, _] : res) {
                // std::cout << id << "\n";
                for (std::size_t j{0}; j < res.size(); j++) {
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

        // std::cout << "Intersection: " << intersection << "\n";
        // std::cout << "Total: " << k_sz << "\n";

        outf << "MsPQ for " << sz << " vectors : " << dur << " Milliseconds\n";

        double recall = static_cast<double>(intersection) / k_sz;

        outf << "Recall@10 for " << sz << " vectors : " << recall * 100 << "%\n";

        outf << "QPS for " << sz << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";

        outf.flush();
    }};

    calc_qps(vdb);
    // calc_qps(vdb_100K);
    // calc_qps(vdb_1M);
}