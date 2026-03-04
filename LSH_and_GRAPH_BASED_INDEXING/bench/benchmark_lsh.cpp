#include "Importer.hpp"
#include "LSH_Index.hpp"
#include "Timer.hpp"

int main() {
    LSHIndex lsh(5, (1 << 10));
    Timer t{};

    auto sift_res{Importer::import_sift1m("/home/sp27022003/vector-db/sift/sift_base.fvecs", "/home/sp27022003/vector-db/sift/sift_query.fvecs", "/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs", lsh, 10000)};

    std::vector<std::vector<float>> queries{sift_res.ok_value().queries};
    std::vector<std::vector<std::uint32_t>> truths{sift_res.ok_value().truths};
    std::vector<std::uint32_t> truth_k{sift_res.ok_value().truth_k};

    std::size_t num_queries = 1000;
    // std::ofstream outf{"/home/sp27022003/vector-db/VectorDB_v0.1(Exhaustive Search Engine)/bench/benchmark_brute.txt"};

    auto calc_qps{[&](LSHIndex& lsh) {
        t.reset();
        for (std::size_t i{0}; i < num_queries; i++) {
            auto res = lsh.query(queries[i], truth_k[i]); //impilcit conversion fo float query to Vector query
        }

        double dur = t.elapsed() / num_queries;

        std::cout << "MsPQ for " << lsh.getInfo().candidate_set_size << " vectors : " << dur << " Milliseconds\n";

        std::cout << "QPS for " << lsh.getInfo().candidate_set_size << " vectors : " << static_cast<int>(1000 / dur) << " queries\n";

        // outf.flush();
    }};

    calc_qps(lsh);
}