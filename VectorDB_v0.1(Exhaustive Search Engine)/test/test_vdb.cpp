#include <gtest/gtest.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <VectorStore.hpp>
#include <Importer.hpp>

class VectorStore_test : public ::testing::Test {
protected: 
    VectorStore vdb{};
    std::string filename{"/home/sp27022003/vector-db/glove_100d_2024.txt"};
    Myres mr;
    
    void SetUp() override {
        auto result = Importer::import_glove(filename, vdb);
        ASSERT_TRUE(result.is_ok()) << "Failed to import glove file";
        mr = result.ok_value();
    }
};

TEST_F(VectorStore_test, Correct_Import) {
    auto dims = vdb.dimensions();
    ASSERT_TRUE(dims.is_ok());
    ASSERT_EQ(dims.ok_value(), 100);
}

TEST_F(VectorStore_test, Similarity_Test_1) {      
    std::vector<float> king = (vdb.get(mr.word_to_id["king"])).ok_value();
    std::vector<float> man = (vdb.get(mr.word_to_id["man"])).ok_value();
    std::vector<float> woman = (vdb.get(mr.word_to_id["woman"])).ok_value();

    std::vector<float> query(100);
    for (std::size_t i{0}; i < king.size(); i++) {
        query[i] = king[i] - man[i] + woman[i];
    }

    auto res = vdb.query(query);
    ASSERT_TRUE(res.is_ok());
    for (auto it : res.ok_value()) {
        std::cout << "Id: " << it.first << '\n';
        std::cout << "Word: " << mr.id_to_word[it.first] << '\n';
        std::cout << "Score: " << it.second << '\n';
    }
}
TEST_F(VectorStore_test, Similarity_Test_2) {      
    std::vector<float> king = (vdb.get(mr.word_to_id["paris"])).ok_value();
    std::vector<float> man = (vdb.get(mr.word_to_id["france"])).ok_value();
    std::vector<float> woman = (vdb.get(mr.word_to_id["germany"])).ok_value();

    std::vector<float> query(100);
    for (std::size_t i{0}; i < king.size(); i++) {
        query[i] = king[i] - man[i] + woman[i];
    }

    auto res = vdb.query(query);
    ASSERT_TRUE(res.is_ok());
    for (auto it : res.ok_value()) {
        std::cout << "Id: " << it.first << '\n';
        std::cout << "Word: " << mr.id_to_word[it.first] << '\n';
        std::cout << "Score: " << it.second << '\n';
    }
}
TEST_F(VectorStore_test, Similarity_Test_3) {      
    std::vector<float> king = (vdb.get(mr.word_to_id["brother"])).ok_value();
    std::vector<float> man = (vdb.get(mr.word_to_id["man"])).ok_value();
    std::vector<float> woman = (vdb.get(mr.word_to_id["woman"])).ok_value();

    std::vector<float> query(100);
    for (std::size_t i{0}; i < king.size(); i++) {
        query[i] = king[i] - man[i] + woman[i];
    }

    auto res = vdb.query(query);
    ASSERT_TRUE(res.is_ok());
    for (auto it : res.ok_value()) {
        std::cout << "Id: " << it.first << '\n';
        std::cout << "Word: " << mr.id_to_word[it.first] << '\n';
        std::cout << "Score: " << it.second << '\n';
    }
}
TEST_F(VectorStore_test, Similarity_Test_4) {      
    std::vector<float> king = (vdb.get(mr.word_to_id["walking"])).ok_value();
    std::vector<float> man = (vdb.get(mr.word_to_id["walk"])).ok_value();
    std::vector<float> woman = (vdb.get(mr.word_to_id["swim"])).ok_value();

    std::vector<float> query(100);
    for (std::size_t i{0}; i < king.size(); i++) {
        query[i] = king[i] - man[i] + woman[i];
    }

    auto res = vdb.query(query);
    ASSERT_TRUE(res.is_ok());
    for (auto it : res.ok_value()) {
        std::cout << "Id: " << it.first << '\n';
        std::cout << "Word: " << mr.id_to_word[it.first] << '\n';
        std::cout << "Score: " << it.second << '\n';
    }
}
TEST_F(VectorStore_test, Similarity_Test_5) {      
    std::vector<float> king = (vdb.get(mr.word_to_id["biggest"])).ok_value();
    std::vector<float> man = (vdb.get(mr.word_to_id["big"])).ok_value();
    std::vector<float> woman = (vdb.get(mr.word_to_id["small"])).ok_value();

    std::vector<float> query(100);
    for (std::size_t i{0}; i < king.size(); i++) {
        query[i] = king[i] - man[i] + woman[i];
    }

    auto res = vdb.query(query);
    ASSERT_TRUE(res.is_ok());
    for (auto it : res.ok_value()) {
        std::cout << "Id: " << it.first << '\n';
        std::cout << "Word: " << mr.id_to_word[it.first] << '\n';
        std::cout << "Score: " << it.second << '\n';
    }
}
