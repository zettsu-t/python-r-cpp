#include "test_popcount.h"
#include <algorithm>
#include <testthat.h>

#define ASSERT_IS_EQUAL(x, y)                                                  \
    do {                                                                       \
        expect_true((x) == (y));                                               \
        if ((x) != (y)) {                                                      \
            return;                                                            \
        }                                                                      \
    } while (0)

namespace {
template <typename T, typename U>
bool are_equal(const T &actual, const U &expected) {
    // Rcpp::Vector has no == operators.
    return ((actual.size() == expected.size()) &&
            std::equal(actual.begin(), actual.end(), expected.begin(),
                       expected.end()));
}
} // namespace

context("Helper") {
    test_that("AssertIsEqualValue") { ASSERT_IS_EQUAL(0, 0); }

    test_that("AssertIsEqualExpr") { ASSERT_IS_EQUAL(1 + 4, 2 + 3); }

    test_that("AreEqual") {
        using Numbers = rCppSample::IntegerVector;
        const Numbers empty1{};
        const Numbers empty2{};
        const Numbers one1{1};
        const Numbers one2{2};
        const Numbers many1a{2, 3, 5, 7};
        const Numbers many1b{2, 3, 5, 7};
        const Numbers many2{2, 3, 6, 18};
        expect_true(are_equal(empty1, empty2));
        expect_true(are_equal(one1, one1));
        expect_true(are_equal(many1a, many1b));
        expect_false(are_equal(one1, one2));
        expect_false(are_equal(one1, empty1));
        expect_false(are_equal(many1a, many2));
    }
}

context("NaInteger") {
    test_that("IsNaInteger") {
        constexpr uint8_t zero_uint8 = 0;
        constexpr auto max_uint8 = std::numeric_limits<uint8_t>::max();
        constexpr int zero_int = 0;
        constexpr auto max_int = std::numeric_limits<int>::max();
        constexpr auto min_int = std::numeric_limits<int>::min();
        expect_false(is_na_integer<rCppSample::RawVector>(zero_uint8));
        expect_false(is_na_integer<rCppSample::RawVector>(max_uint8));
        expect_false(is_na_integer<rCppSample::IntegerVector>(zero_int));
        expect_false(is_na_integer<rCppSample::IntegerVector>(max_int));
        expect_true(is_na_integer<rCppSample::IntegerVector>(min_int));
    }

    test_that("GetNaIntValue") {
        const auto expected = NA_INTEGER;
        const auto actual = get_na_int_value();
        expect_true(actual == expected);
    }
}

context("PopcountCpp") {
    test_that("Size") {
        using VectorType = rCppSample::RawVector;
        using SizeType = typename VectorSize<VectorType>::size_type;
        // Empty arrays are acceptable
        for (SizeType array_size{0}; array_size < 3; ++array_size) {
            const VectorType arg(array_size, 0);
            const auto actual = popcount_cpp_raw(arg);
            ASSERT_IS_EQUAL(actual.size(), arg.size());
        }
    }

    test_that("RawValues") {
        // Check 0..255 uint8 values
        const rCppSample::RawVector arg{0, 1, 2, 3, 6, 7, 254, 255};
        const rCppSample::IntegerVector expected{0, 1, 1, 2, 2, 3, 7, 8};
        const auto actual = popcount_cpp_raw(arg);
        expect_true(are_equal(actual, expected));
    }

    test_that("IntegerValues") {
        // Check some non-negative int32 values
        using VectorType = rCppSample::IntegerVector;
        const VectorType arg{0,          1,          0xfe,     0xff,
                             0x100,      0xffff,     0x10000,  0x7ffffffe,
                             0x7fffffff, 0x70f0f0f0, 0xf0f0f0f};
        const VectorType expected{0, 1, 7, 8, 1, 16, 1, 30, 31, 15, 16};
        const auto actual = popcount_cpp_integer(arg);
        expect_true(are_equal(actual, expected));
    }

    test_that("NAs") {
        // Check NA values
        using VectorType = rCppSample::IntegerVector;
        const VectorType arg{2, rCppSample::NaInteger, 14,
                             rCppSample::NaInteger, 62};
        const VectorType expected{1, rCppSample::NaInteger, 3,
                                  rCppSample::NaInteger, 5};
        const auto actual = popcount_cpp_integer(arg);
        expect_true(are_equal(actual, expected));
    }

    test_that("NegativeIntegerValues") {
        // Check negative int32 values
        // 0xffffffff, 0xfffffffe, 0xff0f0f0f, 0xf0f0f0f0, 0x80000001
        // Note that 0x80000000 is treated as NA in R
        using VectorType = rCppSample::IntegerVector;
        const VectorType arg{-1, -2, -15790321, -252645136, -2147483647};
        const VectorType expected{32, 31, 20, 16, 2};
        const auto actual = popcount_cpp_integer(arg);
        expect_true(are_equal(actual, expected));
    }

    test_that("FullRawValues") {
        // Check all uint8 values
        using ArgVectorType = rCppSample::RawVector;
        using SizeType = typename VectorSize<ArgVectorType>::size_type;
        SizeType element_size = 256;
        SizeType set_size = 2;
        SizeType array_size = element_size * set_size;
        ArgVectorType arg(array_size);
        rCppSample::IntegerVector expected(array_size);

        using Element = uint8_t;
        constexpr auto max_value = std::numeric_limits<Element>::max();
        for (decltype(array_size) index{0}; index < array_size; ++index) {
            auto low_value = index;
            constexpr auto mask_value =
                static_cast<decltype(low_value)>(max_value);
            low_value &= mask_value;
            auto element = static_cast<Element>(low_value);
            arg.at(index) = element;
            for (size_t bit_index{0}; bit_index < (sizeof(Element) * 8);
                 ++bit_index) {
                decltype(low_value) mask = 1;
                mask <<= bit_index;
                expected.at(index) += ((low_value & mask) != 0);
            }
        }

        const auto actual = popcount_cpp_raw(arg);
        const auto actual_size =
            static_cast<decltype(array_size)>(actual.size());
        ASSERT_IS_EQUAL(actual_size, array_size);
        expect_true(are_equal(actual, expected));
    }
}
