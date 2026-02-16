#include <gtest/gtest.h>
#include <unordered_map>
#include <string>
#include <VectorStore.hpp>
#include <Importer.hpp>

class VectorStore_test : public ::testing::Test {
protected: 
    VectorStore vdb{};
    std::string filename{"/home/sp27022003/vector-db/glove_100d_2024.txt"};
    std::unordered_map<std::string, std::uint64_t> mpp;
    
    void SetUp() override {
        auto result = Importer::import_glove(filename, vdb);
        ASSERT_TRUE(result.is_ok()) << "Failed to import glove file";
        mpp = result.ok_value();
    }
};

TEST_F(VectorStore_test, Correct_Import) {
    auto dims = vdb.dimensions();
    ASSERT_TRUE(dims.is_ok());
    // ASSERT_EQ(dims.ok_value(), 100);
}