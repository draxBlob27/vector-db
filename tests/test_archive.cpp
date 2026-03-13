#include <gtest/gtest.h>
#include <random>
#include <cstdint>
#include <iostream>
#include "vectorDB/archive.hpp"

std::vector<std::vector<double>> generateData(const int dim, const int count, bool isZero=0)
{
    std::vector<std::vector<double>> res(count, std::vector<double>(dim, 0));
    if (isZero)
        return res;

    std::mt19937 mt{ std::random_device{}()};
    std::uniform_real_distribution<double> unf;

    
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
    std::string filepath{"/home/sp27022003/vector-db/db/archive/correct_save.bin"};
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, empty_data) {
    test_vector = {{}};
    std::string filepath{"/home/sp27022003/vector-db/db/archive/empty_data.bin"};
    EXPECT_THROW(vecd.save(filepath, test_vector), InvalidOperationError);
}

TEST_F(VectorArchive_test, dimension_mismatch) {
    test_vector = {{1, 2, 4}, {4, 5}};
    std::string filepath{"/home/sp27022003/vector-db/db/archive/dimension_mismatch.bin"};
    EXPECT_THROW(vecd.save(filepath, test_vector), InvalidOperationError);
}

TEST_F(VectorArchive_test, overwrite_with_shrink) {
    std::string filepath = "/home/sp27022003/vector-db/db/archive/overwrite_with_shrink.bin";
    test_vector = generateData(128, 10, 0);
    vecd.save(filepath, test_vector);
    VectorArchive::FileInfo f1 = vecd.info(filepath);
    // std::cout << f1;

    test_vector = generateData(128, 1, 0);
    vecd.save(filepath, test_vector);
    VectorArchive::FileInfo f2 = vecd.info(filepath);

    EXPECT_NE((std::vector<std::uint64_t>{f1.bytes, f1.count, f1.dim}), (std::vector<std::uint64_t>{f2.bytes, f2.count, f2.dim}));
}

TEST_F(VectorArchive_test, test_corruption) {
    std::string filepath = "/home/sp27022003/vector-db/db/archive/corrupted.bin";
    EXPECT_FALSE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, buffer_int_1) { //buff_cnt - 1 -> int means intergrtiy check
    std::string filepath = "/home/sp27022003/vector-db/db/archive/buffer_int_1.bin";
    test_vector = generateData(128, 31, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, buffer_int_2) { //buff_cnt + 1
    std::string filepath = "/home/sp27022003/vector-db/db/archive/buffer_int_2.bin";
    test_vector = generateData(127, 33, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, buffer_int_3) { //count = 1
    std::string filepath = "/home/sp27022003/vector-db/db/archive/buffer_int_3.bin";
    test_vector = generateData(127, 1, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, buffer_int_4) { //large prime
    std::string filepath = "/home/sp27022003/vector-db/db/archive/buffer_int_4.bin";
    test_vector = generateData(127, 9973, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, dim_int_1) {//dim = 1
std::string filepath = "/home/sp27022003/vector-db/db/archive/dim_int_1.bin";
    test_vector = generateData(1, 5, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}


TEST_F(VectorArchive_test, dim_int_2) {//dim = odd
std::string filepath = "/home/sp27022003/vector-db/db/archive/dim_int_2.bin";
    test_vector = generateData(9, 5, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

TEST_F(VectorArchive_test, dim_int_3) {//dim = large
std::string filepath = "/home/sp27022003/vector-db/db/archive/dim_int_3.bin";
    test_vector = generateData(9999, 5, 0);
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
    ASSERT_EQ(loaded_vector, test_vector);
    EXPECT_TRUE(vecd.verify(filepath));
}

// TEST_F(VectorArchive_test, large_data) { //takes 31 secs, but passssing
//     test_vector = generateData(128, 1000000, 0);
//     std::string filepath = "/home/sp27022003/vector-db/db/archive/large_data.bin";
//     EXPECT_NO_THROW(vecd.save(filepath, test_vector));
//     EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));
//     ASSERT_EQ(loaded_vector, test_vector);
//     EXPECT_TRUE(vecd.verify(filepath));
// }

// TEST_F(VectorArchive_test, destructive_int_1) {//file truncated mannuly
//     EXPECT_FALSE(vecd.verify("/home/sp27022003/vector-db/db/archive/dest_int_1.bin"));
// }

// TEST_F(VectorArchive_test, destructive_int_2) {//dataf flip
//     EXPECT_FALSE(vecd.verify("/home/sp27022003/vector-db/db/archive/dest_int_2.bin"));
// }

// TEST_F(VectorArchive_test, destructive_int_3) {//crc flip
//     EXPECT_FALSE(vecd.verify("/home/sp27022003/vector-db/db/archive/dest_int_3.bin"));
// }

// TEST_F(VectorArchive_test, destructive_int_4) {//header flip
//     EXPECT_FALSE(vecd.verify("/home/sp27022003/vector-db/db/archive/dest_int_4.bin"));
// }


// TEST_F(VectorArchive_test, similar_data_int) {//if same data in diff files
//     std::string fpath1{"/home/sp27022003/vector-db/db/archive/same_data_int.bin"};
//     std::string fpath2{"/home/sp27022003/vector-db/db/archive/same_data_int_copy.bin"};

//     std::vector<std::vector<double>> ld1{vecd.load(fpath1, false)};
//     std::vector<std::vector<double>> ld2{vecd.load(fpath2, false)};

//     EXPECT_EQ(ld1, ld2);

//     VectorArchive::FileInfo f1 = vecd.info(fpath1);
//     VectorArchive::FileInfo f2 = vecd.info(fpath2);

//     EXPECT_EQ((std::vector<std::uint64_t>{f1.bytes, f1.count, f1.dim}), (std::vector<std::uint64_t>{f2.bytes, f2.count, f2.dim}));
// }

TEST_F(VectorArchive_test, correct_append) {//checks correct append
    test_vector = generateData(128, 10, 0);
    std::string filepath{"/home/sp27022003/vector-db/db/archive/correct_append.bin"};
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    
    std::vector<std::vector<double>> append_vector{generateData(128, 12, 0)};
    EXPECT_NO_THROW(vecd.append(filepath, append_vector));
    
    std::vector<std::vector<double>> combined_vector{test_vector};
    combined_vector.insert(combined_vector.end(), append_vector.begin(), append_vector.end());
    EXPECT_NO_THROW(loaded_vector = vecd.load(filepath, false));

    EXPECT_EQ(loaded_vector, combined_vector);
}

TEST_F(VectorArchive_test, incorrect_append) {//checks incorrect append
    test_vector = generateData(127, 10, 0);
    std::string filepath{"/home/sp27022003/vector-db/db/archive/correct_append.bin"};
    EXPECT_NO_THROW(vecd.save(filepath, test_vector));
    
    std::vector<std::vector<double>> append_vector{generateData(128, 12, 0)};
    EXPECT_THROW(vecd.append(filepath, append_vector), InvalidOperationError);
}
