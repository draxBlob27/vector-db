#include <gtest/gtest.h>
#include <random>
#include <print>
#include "archive.h"
#include <iostream>

std::vector<std::vector<double>> generateData(const int dim, const int count, bool isZero)
{
    std::vector<std::vector<double>> res(count, std::vector<double>(dim, 0));
    if (isZero)
        return res;

    std::mt19937 mt{ std::random_device{}()};
    std::uniform_real_distribution<float> unf;

    
    for (int i{0}; i < count; i++) {
        for (int j{0}; j < dim; j++) {
            res[i][j] = unf(mt);
        }
    }

    return res;
}

class VectorArchive_test : public ::testing::Test{
protected:
    VectorArchive vecd{};
    std::vector<std::vector<double>> test_vector;
    std::vector<std::vector<double>> loaded_vector;
    
};
//correct loading
TEST_F(VectorArchive_test, correct_save) {
    test_vector = generateData(128, 10, 0);
    std::string filepath{"../dump/correct_save.bin"};
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, empty_data) {
    test_vector = {{}};
    std::string filepath{"../dump/empty_data.bin"};
    EXPECT_THROW(vecd.save(filepath, test_vector), InvalidOperationError);
}

TEST_F(VectorArchive_test, dimension_mismatch) {
    test_vector = {{1, 2, 4}, {4, 5}};
    std::string filepath{"../dump/dimension_mismatch.bin"};
    EXPECT_THROW(vecd.save(filepath, test_vector), InvalidOperationError);
}

TEST_F(VectorArchive_test, overwrite_with_shrink) {
    std::string filepath = "../dump/overwrite_with_shrink.bin";
    test_vector = generateData(128, 10, 0);
    vecd.save(filepath, test_vector);
    VectorArchive::FileInfo f1 = vecd.info(filepath);
    // std::cout << f1;

    test_vector = generateData(128, 1, 0);
    vecd.save(filepath, test_vector);
    VectorArchive::FileInfo f2 = vecd.info(filepath);

    EXPECT_NE((std::vector<uint64_t>{f1.bytes, f1.count, f1.dim}), (std::vector<uint64_t>{f2.bytes, f2.count, f2.dim}));
}

TEST_F(VectorArchive_test, test_corruption) {
    std::string filepath = "../dump/corrupted.bin";
    EXPECT_FALSE(vecd.verify(filepath));
}

// TEST_F(VectorArchive_test, large_data) {
//     test_vector = generateData(128, 1000000, 0);
//     std::string filepath = "../dump/large_data.bin";
//     EXPECT_NO_THROW(vecd.save(filepath, test_vector));
// }

// TEST_F(VectorArchive_test, 

