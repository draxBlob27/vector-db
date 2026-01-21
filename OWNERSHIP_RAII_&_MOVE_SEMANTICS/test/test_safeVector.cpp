#include <gtest/gtest.h>
#include "safeVector.h"

//in first type of test i will be initializing a safe vector of size 10;
//looks like {_, _, _, _, _, _, _, _, _, _}
template <typename T>
class SafeVector_test : public ::testing::Test {
protected:
    SafeVector<T> arr{10};
};

using MyTypes = ::testing::Types<int>;

TYPED_TEST_SUITE(SafeVector_test, MyTypes);


TYPED_TEST(SafeVector_test, rValuePushBack) {
    this->arr.push_back(10);
    EXPECT_EQ(this->arr.size(), 11);
    EXPECT_EQ(this->arr[10], 10);
    EXPECT_EQ(this->arr.back(), 10);
    EXPECT_EQ(this->arr.capacity(), 20);
}

TYPED_TEST(SafeVector_test, lValuePushBack) {
    int val = 10;
    this->arr.push_back(val);
    EXPECT_EQ(this->arr.size(), 11);
    EXPECT_EQ(this->arr[10], 10);
    EXPECT_EQ(this->arr.back(), 10);
    EXPECT_EQ(this->arr.capacity(), 20);
}