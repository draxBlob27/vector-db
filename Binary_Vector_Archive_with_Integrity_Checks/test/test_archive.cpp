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
};
//correct loading
TEST_F(VectorArchive_test, correct_save) {
    EXPECT_NO_THROW(vecd.save("../dump/correct_save.bin", generateData(128, 10, 1)));
}

