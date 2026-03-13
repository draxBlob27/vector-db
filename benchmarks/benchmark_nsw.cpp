#include <iostream>
#include <fstream>
#include "vectorDB/utils/Importer.hpp"
#include "vectorDB/NSW_Index.hpp"
#include "vectorDB/utils/Timer.hpp"

int main() {
    //NSW_Index(std::uint32_t M, std::uint32_t efConstruction = 120, std::uint32_t efSearch = 50)
    std::vector<std::uint32_t> efSearches{10, 25, 50, 100, 200, 500};
    std::uint32_t M{16}, efConstruction{200};
    NSW_Index nsw_10K{M, efConstruction};
        Timer t{};
    
        std::string dir = "/home/sp27022003/vector-db/sift10K/";
            
        auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs");
    
        const std::vector<std::uint64_t>& ids{sift_res.ok_value().ids};
        const std::vector<Vector>& vectors{sift_res.ok_value().vectors};
    
        std::ofstream outf{"/home/sp27022003/vector-db/benchmarks/benchmark_nsw.txt", std::ios::app};
    
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


/*
Parameters(M = 16, efConst = 200)
Build time: 4.14154 secs 
efSearch = 10
MsPQ for 10000 vectors : 0.0761788 Milliseconds
Recall@10 for 10000 vectors : 50%
QPS for 10000 vectors : 13127 queries
efSearch = 25
MsPQ for 10000 vectors : 0.1233 Milliseconds
Recall@10 for 10000 vectors : 58%
QPS for 10000 vectors : 8110 queries
efSearch = 50
MsPQ for 10000 vectors : 0.196022 Milliseconds
Recall@10 for 10000 vectors : 62%
QPS for 10000 vectors : 5101 queries
efSearch = 100
MsPQ for 10000 vectors : 0.34411 Milliseconds
Recall@10 for 10000 vectors : 65%
QPS for 10000 vectors : 2906 queries
efSearch = 200
MsPQ for 10000 vectors : 0.548516 Milliseconds
Recall@10 for 10000 vectors : 67%
QPS for 10000 vectors : 1823 queries
efSearch = 500
MsPQ for 10000 vectors : 1.10956 Milliseconds
Recall@10 for 10000 vectors : 70%
QPS for 10000 vectors : 901 queries

Parameters(M = 16, efConst = 200)
Build time: 3.81925 secs 
efSearch = 10
MsPQ for 10000 vectors : 0.0683027 Milliseconds
Recall@10 for 10000 vectors : 50%
QPS for 10000 vectors : 14640 queries
efSearch = 25
MsPQ for 10000 vectors : 0.10936 Milliseconds
Recall@10 for 10000 vectors : 58%
QPS for 10000 vectors : 9144 queries
efSearch = 50
MsPQ for 10000 vectors : 0.172735 Milliseconds
Recall@10 for 10000 vectors : 62%
QPS for 10000 vectors : 5789 queries
efSearch = 100
MsPQ for 10000 vectors : 0.286926 Milliseconds
Recall@10 for 10000 vectors : 65%
QPS for 10000 vectors : 3485 queries
efSearch = 200
MsPQ for 10000 vectors : 0.486213 Milliseconds
Recall@10 for 10000 vectors : 67%
QPS for 10000 vectors : 2056 queries
efSearch = 500
MsPQ for 10000 vectors : 0.959206 Milliseconds
Recall@10 for 10000 vectors : 70%
QPS for 10000 vectors : 1042 queries


Parameters(M = 16, efConst = 200)
Build time: 1433.71 secs 
efSearch = 10
MsPQ for 1000000 vectors : 0.23625 Milliseconds
Recall@10 for 1000000 vectors : 53%
QPS for 1000000 vectors : 4232 queries
efSearch = 25
MsPQ for 1000000 vectors : 0.373767 Milliseconds
Recall@10 for 1000000 vectors : 73%
QPS for 1000000 vectors : 2675 queries
efSearch = 50
MsPQ for 1000000 vectors : 0.591347 Milliseconds
Recall@10 for 1000000 vectors : 85%
QPS for 1000000 vectors : 1691 queries
efSearch = 100
MsPQ for 1000000 vectors : 0.923099 Milliseconds
Recall@10 for 1000000 vectors : 92%
QPS for 1000000 vectors : 1083 queries
efSearch = 200
MsPQ for 1000000 vectors : 1.57705 Milliseconds
Recall@10 for 1000000 vectors : 96%
QPS for 1000000 vectors : 634 queries
efSearch = 500
MsPQ for 1000000 vectors : 3.37052 Milliseconds
Recall@10 for 1000000 vectors : 98%
QPS for 1000000 vectors : 296 queries

---without heuristic
Parameters(M = 16, efConst = 200)
Build time: 4.58957 secs 
efSearch = 10
MsPQ for 10000 vectors : 0.0878133 Milliseconds
Recall@10 for 10000 vectors : 71%
QPS for 10000 vectors : 11387 queries
efSearch = 25
MsPQ for 10000 vectors : 0.150731 Milliseconds
Recall@10 for 10000 vectors : 89%
QPS for 10000 vectors : 6634 queries
efSearch = 50
MsPQ for 10000 vectors : 0.232463 Milliseconds
Recall@10 for 10000 vectors : 95%
QPS for 10000 vectors : 4301 queries
efSearch = 100
MsPQ for 10000 vectors : 0.367467 Milliseconds
Recall@10 for 10000 vectors : 98%
QPS for 10000 vectors : 2721 queries
efSearch = 200
MsPQ for 10000 vectors : 0.600013 Milliseconds
Recall@10 for 10000 vectors : 98%
QPS for 10000 vectors : 1666 queries
efSearch = 500
MsPQ for 10000 vectors : 1.17401 Milliseconds
Recall@10 for 10000 vectors : 98%
QPS for 10000 vectors : 851 queries


*/