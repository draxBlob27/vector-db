#include <iostream>
#include "Importer.hpp"
#include "LSH_Index.hpp"
#include "Timer.hpp"

int main() {
    // LSHIndex lsh_10k(12, 20);
    // LSHIndex lsh_100k(20, 20);
    std::uint32_t tables{20}, proj{20};
    LSHIndex lsh_1M(tables, proj);
    Timer t{};

    auto sift_res = Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", 1'000'000);

    std::vector<std::pair<std::uint64_t, Vector>> data;
    std::vector<std::uint64_t> ids{sift_res.ok_value().ids};
    std::vector<Vector> vectors{sift_res.ok_value().vectors};
    for (std::size_t i{0}; i < ids.size(); i++) {
        // std::cout << i << '\n';
        data.push_back({ids[i], vectors[i]});
    }

    // lsh_10k.build(std::vector<std::pair<std::uint64_t, Vector>>(data.begin(), data.begin() + 10000));
    // lsh_100k.build(std::vector<std::pair<std::uint64_t, Vector>>(data.begin(), data.begin() + 100000));
    lsh_1M.build(std::move(data));


    std::vector<std::vector<float>> queries{sift_res.ok_value().queries};
    std::vector<std::vector<std::uint32_t>> truths{sift_res.ok_value().truths};
    std::vector<std::uint32_t> truth_k{sift_res.ok_value().truth_k};

    std::size_t num_queries = 100;
    // std::ofstream outf{"/home/sp27022003/vector-db/VectorDB_v0.1(Exhaustive Search Engine)/bench/benchmark_brute.txt"};
    auto calc_qps{[&](LSHIndex& lsh) {
        t.reset();
        int intersection{0};
        int my_k{10};

        for (std::size_t i{0}; i < num_queries; i++) {
            auto res = lsh.query(queries[i], my_k); //impilcit conversion fo float query to Vector query
            // std::cout << lsh.getInfo().candidate_set_size << '\n';
            
            for (size_t j = 0; j < my_k; j++) {
                for (size_t k = 0; k < my_k; k++) {
                    if (res[j].first == truths[i][k]) {
                        intersection++;
                        break;
                    }
                }
            }
        }


        double dur = t.elapsed() / num_queries;
        std::cout << "Table: " << tables << " and Proj: " << proj << '\n';
        std::cout << "MsPQ for " << lsh.getInfo() << " vectors : " << dur << " Milliseconds\n";
        std::cout << "Recall@10 for " << lsh.getInfo() << " vectors : " << intersection * 100 / (num_queries * my_k) << "%\n";

        std::cout << "QPS for " << lsh.getInfo() << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";

        // outf.flush();
    }};

    // calc_qps(lsh_10k);
    // calc_qps(lsh_100k);
    calc_qps(lsh_1M);
}