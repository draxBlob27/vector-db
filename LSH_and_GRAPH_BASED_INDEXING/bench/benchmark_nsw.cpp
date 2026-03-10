#include <iostream>
#include <fstream>
#include "Importer.hpp"
#include "NSW_Index.hpp"
#include "Timer.hpp"

int main() {
    //NSW_Index(std::uint32_t M, std::uint32_t efConstruction = 120, std::uint32_t efSearch = 50)
    std::vector<std::uint32_t> efSearches{10, 25, 50, 100, 200, 500};
    std::uint32_t M{16}, efConstruction{200};
    NSW_Index nsw_10K{M, efConstruction};
        Timer t{};
    
        std::string dir = "/home/sp27022003/vector-db/sift1M/";
            
        auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs");
    
        std::vector<std::pair<std::uint64_t, Vector>> data;
        const std::vector<std::uint64_t>& ids{sift_res.ok_value().ids};
        const std::vector<Vector>& vectors{sift_res.ok_value().vectors};
    
        std::ofstream outf{"/home/sp27022003/vector-db/LSH_and_GRAPH_BASED_INDEXING/bench/benchmark_nsw.txt", std::ios::app};
    
        outf << "Parameters(M = " << M << ", efConst = " << efConstruction << ")\n";
    
        t.reset();
        for (std::size_t i{0}; i < ids.size(); i++) {
            nsw_10K.insert(ids[i], vectors[i]);
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
        auto calc_qps{[&](const NSW_Index& nsw) {
            t.reset();
            int intersection{0};
            int my_k{10};
    
            for (std::size_t i{0}; i < num_queries; i++) {
                auto res = nsw.query(queries[i], my_k, efSearch); //impilcit conversion fo float query to Vector query
                
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
            outf << "MsPQ for " << nsw.getSize() << " vectors : " << dur << " Milliseconds\n";
            outf << "Recall@10 for " << nsw.getSize() << " vectors : " << intersection * 100 / (num_queries * my_k) << "%\n";
    
            outf << "QPS for " << nsw.getSize() << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";
    
            outf.flush();
        }};
    
        calc_qps(nsw_10K);
    }
    outf << '\n';
}