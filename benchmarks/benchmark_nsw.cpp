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
    
        std::string dir = "/home/rohitfeb641/vector-db/sift1M/";
            
        auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs");
    
        const std::vector<std::uint64_t>& ids{sift_res.ok_value().ids};
        const std::vector<Vector>& vectors{sift_res.ok_value().vectors};
    
        std::ofstream outf{"/home/rohitfeb641/vector-db/benchmarks/benchmark_nsw.txt", std::ios::app};
    
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
            int intersection{0};
            std::uint32_t my_k{10};
            double dur = 0;
            for (std::size_t i{0}; i < num_queries; i++) {
                t.reset();
                auto res = nsw.query(queries[i], my_k, efSearch); //impilcit conversion fo float query to Vector query
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
Build time: 1195.29 secs 
efSearch = 10
MsPQ for 1000000 vectors : 0.205985 Milliseconds
Recall@10 for 1000000 vectors : 53%
QPS for 1000000 vectors : 4854 queries
efSearch = 25
MsPQ for 1000000 vectors : 0.319483 Milliseconds
Recall@10 for 1000000 vectors : 72%
QPS for 1000000 vectors : 3130 queries
efSearch = 50
MsPQ for 1000000 vectors : 0.49865 Milliseconds
Recall@10 for 1000000 vectors : 84%
QPS for 1000000 vectors : 2005 queries
efSearch = 100
MsPQ for 1000000 vectors : 0.788994 Milliseconds
Recall@10 for 1000000 vectors : 93%
QPS for 1000000 vectors : 1267 queries
efSearch = 200
MsPQ for 1000000 vectors : 1.38755 Milliseconds
Recall@10 for 1000000 vectors : 97%
QPS for 1000000 vectors : 720 queries
efSearch = 500
MsPQ for 1000000 vectors : 2.97448 Milliseconds
Recall@10 for 1000000 vectors : 99%
QPS for 1000000 vectors : 336 queries


*/