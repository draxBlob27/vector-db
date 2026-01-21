#include <gtest/gtest.h>
#include "safeVector.h"

//in first type of test i will be initializing a safe vector of size 10;
//looks like {0, 0, 0, 0, 0, 0, 0, 0, 0, 0} for int
template <typename T>
class SafeVector_test : public ::testing::Test {
protected:
    SafeVector<T> arr{};
};

using MyTypes = ::testing::Types<int>;

TYPED_TEST_SUITE(SafeVector_test, MyTypes);

TYPED_TEST(SafeVector_test, defaultTest) {
    ASSERT_EQ(this->arr.size(), 0);
    ASSERT_EQ(this->arr.capacity(), 0);
    ASSERT_TRUE(this->arr.empty());
}

TYPED_TEST(SafeVector_test, resizeTest) {
    this->arr.resize(10);
    ASSERT_EQ(this->arr.size(), 10);
    ASSERT_EQ(this->arr.capacity(), 10);
    EXPECT_EQ(this->arr.back(), 0);
    EXPECT_EQ(this->arr.front(), 0);
    EXPECT_EQ(this->arr[5], 0);
    EXPECT_EQ(this->arr.at(9), 0);
}

TYPED_TEST(SafeVector_test, reserveTest) {
    this->arr.reserve(10);
    ASSERT_EQ(this->arr.capacity(), 10);
    ASSERT_EQ(this->arr.size(), 0);
    EXPECT_THROW(this->arr.back(), std::out_of_range);
    EXPECT_THROW(this->arr.front(), std::out_of_range);
    EXPECT_THROW(this->arr.pop_back(), std::out_of_range);
    EXPECT_THROW(this->arr[0], std::out_of_range);
    EXPECT_THROW(this->arr.at(0), std::out_of_range);
}

TYPED_TEST(SafeVector_test, insertData) {
    this->arr.reserve(4);
    this->arr.push_back(10);
    int val = 20;
    this->arr.push_back(val);
    EXPECT_EQ(this->arr.size(), 2);
    EXPECT_EQ(this->arr.back(), 20);
    EXPECT_EQ(this->arr.front(), 10);

    this->arr.push_back(10);
    this->arr.push_back(10);
    this->arr.push_back(10);
    EXPECT_GT(this->arr.capacity(), 4);
    
    this->arr[3] = 45;
    EXPECT_EQ(this->arr.at(3), 45);

    this->arr.pop_back();
    EXPECT_EQ(this->arr.size(), 4);
}

TYPED_TEST(SafeVector_test, invalidInput) {
    this->arr.resize(10);
    EXPECT_THROW(this->arr.at(-2), std::out_of_range);
    EXPECT_THROW(this->arr[-2], std::out_of_range);

    std::size_t sz = this->arr.size();
    EXPECT_THROW(this->arr[sz], std::out_of_range);
}

TYPED_TEST(SafeVector_test, constructor_test) {
    this->arr.resize(30);
    SafeVector copy_arr{this->arr}; //copy construtor
    EXPECT_EQ(copy_arr.size(), 30);
    EXPECT_EQ(this->arr.size(), 30);

    SafeVector move_arr{std::move(this->arr)}; //move costructor
    EXPECT_EQ(move_arr.size(), 30);
    EXPECT_TRUE(this->arr.empty());
}

TYPED_TEST(SafeVector_test, assignment_test) {
    this->arr.resize(30);
    SafeVector copy_arr = this->arr; //copy assignment
    EXPECT_EQ(copy_arr.size(), 30);
    EXPECT_EQ(this->arr.size(), 30);
    
    //self assignment
    this->arr = std::move(this->arr);
    EXPECT_FALSE(this->arr.empty());
    this->arr = this->arr;
    EXPECT_FALSE(this->arr.empty());

    SafeVector move_arr = std::move(this->arr); //move assignemt
    EXPECT_EQ(move_arr.size(), 30);
    EXPECT_TRUE(this->arr.empty());


}

TYPED_TEST(SafeVector_test, shrinking) {
    this->arr.resize(30);
    this->arr.resize(5);

    EXPECT_EQ(this->arr.size(), 5);
    EXPECT_GE(this->arr.capacity(), 30);

    this->arr.resize(30);
    EXPECT_NO_THROW(this->arr.resize(5));

    this->arr.clear();
    this->arr.push_back(20);
    int val = 30;
    this->arr.push_back(val);

    EXPECT_EQ(this->arr.size(), 2);
    EXPECT_GE(this->arr.size(), 2);
}

TYPED_TEST(SafeVector_test, iterators) {
    EXPECT_EQ(this->arr.begin(), this->arr.end());

    this->arr.resize(30);
    EXPECT_EQ(*(this->arr.begin()), this->arr.front());

    EXPECT_EQ(this->arr.end() - this->arr.begin(), this->arr.size());
}

TYPED_TEST(SafeVector_test, constructor_with_size) {
    SafeVector<TypeParam> vec(15);
    EXPECT_EQ(vec.size(), 15);
    EXPECT_EQ(vec.capacity(), 15);
    EXPECT_EQ(vec[0], TypeParam{}); // default initialized
}

TYPED_TEST(SafeVector_test, multiple_reallocations) {
    for (int i = 0; i < 100; ++i) {
        this->arr.push_back(i);
    }
    EXPECT_EQ(this->arr.size(), 100);
    EXPECT_EQ(this->arr[0], 0);
    EXPECT_EQ(this->arr[99], 99);
}

TYPED_TEST(SafeVector_test, range_based_for) {
    for (int i = 0; i < 10; ++i) {
        this->arr.push_back(i);
    }
    
    int count = 0;
    for (auto& val : this->arr) {
        EXPECT_EQ(val, count++);
    }
    EXPECT_EQ(count, 10);
}

TYPED_TEST(SafeVector_test, usable_after_exception) {
    EXPECT_THROW(this->arr.pop_back(), std::out_of_range);
    EXPECT_NO_THROW(this->arr.push_back(42));
    EXPECT_EQ(this->arr.back(), 42);
}

TYPED_TEST(SafeVector_test, illegal) {
    EXPECT_DEATH(this->arr.resize(-1), "");
    EXPECT_DEATH(this->arr.reserve(-1), "");
}