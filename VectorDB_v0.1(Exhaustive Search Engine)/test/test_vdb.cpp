#include <gtest/gtest.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include "VectorStore.hpp"
#include "Importer.hpp"

class VectorStore_test : public ::testing::Test {
protected: 
    inline static VectorStore glove;
    inline static VectorStore sift;
    inline static const std::string glove_filename{"/home/sp27022003/vector-db/glove_100d_2024.txt"};
    inline static const std::string sift_data{"/home/sp27022003/vector-db/sift/sift_base.fvecs"};
    inline static const std::string sift_query{"/home/sp27022003/vector-db/sift/sift_query.fvecs"};
    inline static const std::string sift_truth{"/home/sp27022003/vector-db/sift/sift_groundtruth.ivecs"};
    inline static GloveRes glove_res;
    inline static SiftRes sift_res;

    static void SetUpTestSuite() {
        auto res_a = Importer::import_glove(glove_filename, glove);
        ASSERT_TRUE(res_a.is_ok());
        glove_res = res_a.ok_value();

        auto res_b = Importer::import_sift1m(sift_data, sift_query, sift_truth, sift);
        ASSERT_TRUE(res_b.is_ok());
        sift_res = res_b.ok_value();
    }
};

TEST_F(VectorStore_test, Correct_Import) {
    auto glove_dims = glove.dimensions();
    ASSERT_TRUE(glove_dims.is_ok());
    ASSERT_EQ(glove_dims.ok_value(), 100);

    auto sift_dims = sift.dimensions();
    ASSERT_TRUE(sift_dims.is_ok());
    ASSERT_EQ(sift_dims.ok_value(), 128);
}

TEST_F(VectorStore_test, Similarity_Test_1) {      
    std::vector<float> king = (glove.get(glove_res.word_to_id["king"])).ok_value();
    std::vector<float> man = (glove.get(glove_res.word_to_id["man"])).ok_value();
    std::vector<float> woman = (glove.get(glove_res.word_to_id["woman"])).ok_value();

    std::vector<float> query(100);
    for (std::size_t i{0}; i < king.size(); i++) {
        query[i] = king[i] - man[i] + woman[i];
    }

    auto res = glove.query(query, 10, Metric::L2);
    ASSERT_TRUE(res.is_ok());
    for (auto it : res.ok_value()) {
        std::cout << "Id: " << it.first << '\n';
        std::cout << "Word: " << glove_res.id_to_word[it.first] << '\n';
        std::cout << "Score: " << it.second << '\n';
    }
}

TEST_F(VectorStore_test, Result_match_1) {
    std::size_t num_queries = 100;

    std::cout << num_queries << '\n';

    for (std::size_t cnt = 0; cnt < num_queries; cnt++) {
        auto res = sift.query(sift_res.queries[cnt], sift_res.truth_k[cnt] , Metric::L2);
        EXPECT_TRUE(res.is_ok());

        std::unordered_set<std::uint32_t> truth_ids;
        for (auto it : sift_res.truths[cnt]) {
            truth_ids.insert(it);
        }
    
        std::uint32_t match{0};
        for (std::size_t i{0}; i < res.ok_value().size(); i++) {
            match += truth_ids.count(res.ok_value()[i].first);
        }

        EXPECT_GE(static_cast<double>(match) / sift_res.truth_k[cnt], 0.99) << "Query no: " << cnt << '\n';
    }
}
// TEST_F(VectorStore_test, Similarity_Test_2) {      
//     std::vector<float> king = (glove.get(glove_res.word_to_id["paris"])).ok_value();
//     std::vector<float> man = (glove.get(glove_res.word_to_id["france"])).ok_value();
//     std::vector<float> woman = (glove.get(glove_res.word_to_id["germany"])).ok_value();

//     std::vector<float> query(100);
//     for (std::size_t i{0}; i < king.size(); i++) {
//         query[i] = king[i] - man[i] + woman[i];
//     }

//     auto res = glove.query(query, 10, Metric::L2);
//     ASSERT_TRUE(res.is_ok());
//     for (auto it : res.ok_value()) {
//         std::cout << "Id: " << it.first << '\n';
//         std::cout << "Word: " << glove_res.id_to_word[it.first] << '\n';
//         std::cout << "Score: " << it.second << '\n';
//     }
// }
// TEST_F(VectorStore_test, Similarity_Test_3) {      
//     std::vector<float> king = (glove.get(glove_res.word_to_id["brother"])).ok_value();
//     std::vector<float> man = (glove.get(glove_res.word_to_id["man"])).ok_value();
//     std::vector<float> woman = (glove.get(glove_res.word_to_id["woman"])).ok_value();

//     std::vector<float> query(100);
//     for (std::size_t i{0}; i < king.size(); i++) {
//         query[i] = king[i] - man[i] + woman[i];
//     }

//     auto res = glove.query(query, 10, Metric::L2);
//     ASSERT_TRUE(res.is_ok());
//     for (auto it : res.ok_value()) {
//         std::cout << "Id: " << it.first << '\n';
//         std::cout << "Word: " << glove_res.id_to_word[it.first] << '\n';
//         std::cout << "Score: " << it.second << '\n';
//     }
// }
// TEST_F(VectorStore_test, Similarity_Test_4) {      
//     std::vector<float> king = (glove.get(glove_res.word_to_id["walking"])).ok_value();
//     std::vector<float> man = (glove.get(glove_res.word_to_id["walk"])).ok_value();
//     std::vector<float> woman = (glove.get(glove_res.word_to_id["swim"])).ok_value();

//     std::vector<float> query(100);
//     for (std::size_t i{0}; i < king.size(); i++) {
//         query[i] = king[i] - man[i] + woman[i];
//     }

//     auto res = glove.query(query, 10, Metric::L2);
//     ASSERT_TRUE(res.is_ok());
//     for (auto it : res.ok_value()) {
//         std::cout << "Id: " << it.first << '\n';
//         std::cout << "Word: " << glove_res.id_to_word[it.first] << '\n';
//         std::cout << "Score: " << it.second << '\n';
//     }
// }
// TEST_F(VectorStore_test, Similarity_Test_5) {      
//     std::vector<float> king = (glove.get(glove_res.word_to_id["biggest"])).ok_value();
//     std::vector<float> man = (glove.get(glove_res.word_to_id["big"])).ok_value();
//     std::vector<float> woman = (glove.get(glove_res.word_to_id["small"])).ok_value();

//     std::vector<float> query(100);
//     for (std::size_t i{0}; i < king.size(); i++) {
//         query[i] = king[i] - man[i] + woman[i];
//     }

//     auto res = glove.query(query, 10, Metric::L2);
//     ASSERT_TRUE(res.is_ok());
//     for (auto it : res.ok_value()) {
//         std::cout << "Id: " << it.first << '\n';
//         std::cout << "Word: " << glove_res.id_to_word[it.first] << '\n';
//         std::cout << "Score: " << it.second << '\n';
//     }
// }
