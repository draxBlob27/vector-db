#include <gtest/gtest.h>
#include <vector>
#include <random>
#include "vectorDB/VectorStore.hpp"
#include "vectorDB/utils/distances.hpp"

class VectorStore_test : public ::testing::Test{
protected:
    VectorStore vdb;
    void fill_db (const std::int32_t& dims, const std::int32_t& count, Vector& a) {
        a.data.resize(dims);
        
        std::mt19937 mt{std::random_device{}()};
        std::uniform_real_distribution<float> unf;
        for (std::int32_t cnt{0}; cnt < count; cnt++) {
            for (std::int32_t i{0}; i < dims; i++) {
                a.data[i] = unf(mt);
            }
            auto res{vdb.insert(cnt, a)};

            ASSERT_TRUE(res.is_ok());
        }
    }
};

TEST(Math_verification, Orthogonal_vectors) {
    Vector a, b;
    a.data = {0, 1};
    b.data = {1, 0};
    a.compute_norm();
    b.compute_norm();

    ASSERT_EQ(calc_distance<Metric::Cosine>(a, b), 0);
    ASSERT_EQ(calc_distance<Metric::L2>(a, b), 2);
    ASSERT_EQ(calc_distance<Metric::DotProduct>(a, b), 0);
}

TEST(Math_verification, Self) {
    Vector a;
    a.data = {0, 1};
    a.compute_norm();

    ASSERT_EQ(calc_distance<Metric::Cosine>(a, a), 1);
}

TEST_F(VectorStore_test, empty_db) {
    Vector a;
    a.data = {0, 1};
    auto res{vdb.query(a)};

    ASSERT_TRUE(res.is_err());
    ASSERT_EQ(res.err_value(), DBError::DataBaseEmptyError);
}

TEST_F(VectorStore_test, wrong_k) {
    Vector a, b;
    a.data = {0, 1};
    b.data = {5,6};

    vdb.insert(0, std::move(a));
    auto res{vdb.query(b, 20)};

    ASSERT_TRUE(res.is_ok()) << res.err_value();
    ASSERT_EQ(res.ok_value().size(), vdb.size().ok_value());
}

TEST_F(VectorStore_test, wrong_dims_insert) {
    Vector a, b;
    a.data = {1,2,3,4};
    b.data = {5, 6};
    std::int32_t id{0};
    auto res_a{vdb.insert(id++, a)};
    ASSERT_TRUE(res_a.is_ok());

    auto res_b{vdb.insert(id++, b)};
    ASSERT_TRUE(res_b.is_err());
    ASSERT_EQ(res_b.err_value(), DBError::DimensionError);
}

TEST_F(VectorStore_test, save_load_trip) {
    Vector a;

    fill_db(128, 100, a);
    auto sz = vdb.size();
    ASSERT_TRUE(sz.is_ok());

    auto res_save{vdb.save("/home/sp27022003/vector-db/db/brute_search/save_load_trip.bin")};
    ASSERT_TRUE(res_save.is_ok()) << res_save.err_value();
    vdb = VectorStore{};
    
    auto res_load{vdb.load("/home/sp27022003/vector-db/db/brute_search/save_load_trip.bin")};
    ASSERT_TRUE(res_load.is_ok());

    auto query_res{vdb.query(a)};
    ASSERT_TRUE(query_res.is_ok());
}

TEST_F(VectorStore_test, removal) {
    Vector a;

    fill_db(128, 100, a);
    auto sz = vdb.size();
    ASSERT_TRUE(sz.is_ok());
    
    auto res_get{vdb.get(25)};
    ASSERT_TRUE(res_get.is_ok());

    auto dupli_insert{vdb.insert(25, a)};
    ASSERT_TRUE(dupli_insert.is_err());
    ASSERT_EQ(dupli_insert.err_value(), DBError::IdAlreadyPresent);


    auto res_remove{vdb.remove(25)};
    ASSERT_TRUE(res_remove.is_ok());

    res_get = vdb.get(25);
    ASSERT_TRUE(res_get.is_err());
    ASSERT_EQ(res_get.err_value(), DBError::IdNotFoundError);
}