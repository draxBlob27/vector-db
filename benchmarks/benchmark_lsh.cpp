#include <iostream>
#include <fstream>
#include <string>
#include "vectorDB/utils/Importer.hpp"
#include "vectorDB/LSH_Index.hpp"
#include "vectorDB/utils/Timer.hpp"

int main() {
    int tab{4}, proj{10};
    // for (int tab{5}; tab < 10; tab++) {
        // for (int proj{10}; proj < 26; proj++) {
            // LSHIndex lsh_10k(12, 20);
            // LSHIndex lsh_100k(20, 20);
            // std::uint32_t tab{12}, proj{12};
            LSHIndex lsh_1M(tab, proj);
            // LSHIndex lsh_10K(10, 12);
            Timer t{};

            std::string dir = "/home/rohitfeb641/vector-db/sift1M/";
        
            auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs");
        
            std::vector<std::pair<std::uint64_t, Vector>> data;
            const std::vector<std::uint64_t>& ids{sift_res.ok_value().ids};
            const std::vector<Vector>& vectors{sift_res.ok_value().vectors};
            for (std::size_t i{0}; i < ids.size(); i++) {
                // std::cout << i << '\n';
                data.push_back({ids[i], vectors[i]});
            }
        
            // lsh_10k.build(std::vector<std::pair<std::uint64_t, Vector>>(data.begin(), data.begin() + 10000));
            // lsh_100k.build(std::vector<std::pair<std::uint64_t, Vector>>(data.begin(), data.begin() + 100000));
            std::ofstream outf{"/home/rohitfeb641/vector-db/benchmarks/benchmark_lsh.txt", std::ios::app};

            t.reset();
            lsh_1M.build(std::move(data));
            outf << "Build time: " << t.elapsed() / 1000 <<  " secs \n";
            t.reset();
        
            outf.flush();
        
            const std::vector<std::vector<float>>& queries{sift_res.ok_value().queries};
            const std::vector<std::vector<std::uint32_t>>& truths{sift_res.ok_value().truths};
            const std::vector<std::uint32_t>& truth_k{sift_res.ok_value().truth_k};
        
            std::size_t num_queries = 100;
            auto calc_qps{[&](LSHIndex& lsh) {
                int intersection{0};
                std::uint32_t my_k{10};
                double dur = 0;
                for (std::size_t i{0}; i < num_queries; i++) {
                    t.reset();
                    auto res = lsh.query(queries[i], my_k); //impilcit conversion fo float query to Vector query
                    // std::cout << lsh.getInfo().candidate_set_size << '\n';
                    dur += t.elapsed();
                    
                    for (std::uint32_t j = 0; j < my_k; j++) {
                        for (std::uint32_t k = 0; k < my_k; k++) {
                            if (res[j].first == truths[i][k]) {
                                intersection++;
                                break;
                            }
                        }
                    }
                }

                
                dur = dur / num_queries;
                outf << "Table: " << tab << " and Proj: " << proj << '\n';
                outf << "MsPQ for " << lsh.getInfo() << " vectors : " << dur << " Milliseconds\n";
                outf << "Recall@10 for " << lsh.getInfo() << " vectors : " << intersection * 100 / (num_queries * my_k) << "%\n";
        
                outf << "QPS for " << lsh.getInfo() << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";
        
                outf.flush();
            }};
        
            // calc_qps(lsh_10k);
            // calc_qps(lsh_100k);
            calc_qps(lsh_1M);
        // }
    // }
}