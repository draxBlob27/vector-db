#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include "vectorDB/utils/Importer.hpp"
#include "vectorDB/HNSW_Index.hpp"
#include "vectorDB/utils/Timer.hpp"

int main() {
    //NSW_Index(std::uint32_t M, std::uint32_t efConstruction = 120, std::uint32_t efSearch = 50)
    std::vector<std::uint32_t> efSearches{10, 15, 20, 25, 50, 100, 200, 500};
    std::vector<std::uint32_t> M{8, 16, 32, 48}, efConstruction{50, 100, 200, 400};

    std::ofstream outf{"/home/rohitfeb641/vector-db/benchmarks/benchmark_hnsw.txt", std::ios::app};

    for (auto m : M) {
        for (auto efC : efConstruction) {

            HNSW_Index hnsw_10K{m, efC};
            Timer t{};
        
            std::string dir = "/home/rohitfeb641/vector-db/sift1M/";
                
            auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs");
        
            const std::vector<std::uint64_t>& ids{sift_res.ok_value().ids};
            const std::vector<Vector>& vectors{sift_res.ok_value().vectors};
        
            std::vector<std::uint32_t> order(vectors.size());
            std::random_device rd;
            std::mt19937 g{rd()};
            std::iota(order.begin(), order.end(), 0);
            // std::shuffle(order.begin(), order.end(), g);
        
            outf << "Parameters(M = " << m << ", efConst = " << efC << ")\n";
        
            t.reset();
            for (std::size_t i{0}; i < ids.size(); i++) {
                hnsw_10K.insert(ids[order[i]], vectors[order[i]]);
            }
        
            outf << "Build time: " << t.elapsed() / 1000 <<  " secs \n";
            t.reset();
        
            outf.flush();
        
            for (const auto& efSearch: efSearches) {
                outf << "efSearch = " << efSearch << "\n";
                const std::vector<std::vector<float>>& queries{sift_res.ok_value().queries};
                const std::vector<std::vector<std::uint32_t>>& truths{sift_res.ok_value().truths};
                const std::vector<std::uint32_t>& truth_k{sift_res.ok_value().truth_k};
            
                std::size_t num_queries = 100;
                auto calc_qps{[&](const HNSW_Index& hnsw) {
                    int intersection{0};
                    std::uint32_t my_k{10};
                    double dur = 0;
                    for (std::size_t i{0}; i < num_queries; i++) {
                        t.reset();
                        auto res = hnsw.query(queries[i], my_k, efSearch); //impilcit conversion fo float query to Vector query
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
                    outf << "MsPQ for " << hnsw.getSize() << " vectors : " << dur << " Milliseconds\n";
                    outf << "Recall@10 for " << hnsw.getSize() << " vectors : " << intersection * 100 / (num_queries * my_k) << "%\n";
            
                    outf << "QPS for " << hnsw.getSize() << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";
            
                    outf.flush();
                }};
            
                calc_qps(hnsw_10K);
            }
        }
    }
    outf << '\n';
}
