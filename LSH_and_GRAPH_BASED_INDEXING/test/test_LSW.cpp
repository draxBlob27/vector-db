#include <gtest/gtest.h>
#include "LSH_Index.hpp"
#include "Importer.hpp"

class LSH_test : public ::testing::Test{
protected:
    inline static LSHIndex lsh{10, 12};
    inline static const std::string sift_data{"/home/sp27022003/vector-db/sift/sift_base.fvecs"};
    inline static const std::string sift_query{"/home/sp27022003/vector-db/sift/sift_query.fvecs"};
    inline static const std::string sift_truth{"/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs"};

    inline static SiftRes sift;

    static void SetUpTestSuite() {
        auto res{Importer::import_sift1m(sift_data, sift_query, sift_truth)};

        ASSERT_TRUE(res.is_ok());
        sift = res.ok_value();

        std::vector<std::pair<std::uint64_t, Vector>> data;
        const std::vector<std::uint64_t>& ids{sift.ids};
        const std::vector<Vector>& vectors{sift.vectors};

        for (std::size_t i{0}; i < ids.size(); i++) {
            // std::cout << i << '\n';
            data.push_back({ids[i], vectors[i]});
        }

        lsh.build(std::move(data));
    }
};

TEST_F(LSH_test, save_test) {
    lsh.save("../persist/index.bin");
}

TEST_F(LSH_test, load_test) {
    LSHIndex t_LSH;
    t_LSH.load("../persist/index.bin");
}
