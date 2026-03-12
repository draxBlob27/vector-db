#include <gtest/gtest.h>
#include "LSH_Index.hpp"
#include "Importer.hpp"

class LSH_test : public ::testing::Test{
protected:
    inline static LSHIndex lsh{10, 12};

    static void SetUpTestSuite() {
        std::string dir = "/home/sp27022003/vector-db/sift10K/";
        auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs").ok_value();

        const std::vector<std::uint64_t>& ids{sift_res.ids};
        const std::vector<Vector>& vectors{sift_res.vectors};


        std::vector<std::pair<std::uint64_t, Vector>> data;

        for (std::size_t i{0}; i < ids.size(); i++) {
            // std::cout << i << '\n';
            data.push_back({ids[i], vectors[i]});
        }

        lsh.build(std::move(data));
    }
};

TEST_F(LSH_test, save_test) {
    ASSERT_NO_THROW(lsh.save("/home/sp27022003/vector-db/LSH_and_GRAPH_BASED_INDEXING/persist/lsh_index.bin"));
}

TEST_F(LSH_test, load_test) {
    LSHIndex t_LSH;
    ASSERT_NO_THROW(t_LSH.load("/home/sp27022003/vector-db/LSH_and_GRAPH_BASED_INDEXING/persist/lsh_index.bin"));
}
