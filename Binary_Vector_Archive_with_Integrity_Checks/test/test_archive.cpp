#include <gtest/gtest.h>
#include <random>
#include "archive.h"

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
    std::string filepath{"../dump/correct_save.bin"};
};
//correct loading
TEST_F(VectorArchive_test, correct_save) {
    test_vector = generateData(128, 10, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
}

TEST_F(VectorArchive_test, empty_data) {
    test_vector = {{}};
    EXPECT_THROW(vecd.save(filepath, test_vector), InvalidOperationError);
}

TEST_F(VectorArchive_test, dimension_mismatch) {
    test_vector = {{1, 2, 4}, {4, 5}};
    EXPECT_THROW(vecd.save(filepath, test_vector), InvalidOperationError);
}

