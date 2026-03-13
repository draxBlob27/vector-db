#include <gtest/gtest.h>
#include "vectorDB/NSW_Index.hpp"
#include "vectorDB/utils/Importer.hpp"

class NSW_test :public ::testing::Test {
protected:
    inline static NSW_Index nsw{16};
    static void SetUpTestSuite() {
        std::string dir = "/home/sp27022003/vector-db/sift10K/";
                
        auto sift_res = Importer::import_sift1m(dir + "sift_base.fvecs", dir + "sift_query.fvecs", dir + "sift_groundtruth.ivecs");
    
        const std::vector<std::uint64_t>& ids{sift_res.ok_value().ids};
        const std::vector<Vector>& vectors{sift_res.ok_value().vectors};

        for (std::size_t i{0}; i < ids.size(); i++) {
            nsw.insert(ids[i], vectors[i]);
        }

        NSW_Index::successInsert();
    }
};

TEST_F(NSW_test, save_test) {
    ASSERT_NO_THROW(nsw.save("/home/sp27022003/vector-db/db/LSH/nsw_" + std::to_string(NSW_Index::getIndexNumber()) + ".bin"));
}

TEST_F(NSW_test, load_test) {
    NSW_Index check_load{};
    ASSERT_NO_THROW(check_load.load("/home/sp27022003/vector-db/db/LSH/nsw_" + std::to_string(NSW_Index::getIndexNumber()) + ".bin"));
    ASSERT_EQ(check_load, nsw);
}
