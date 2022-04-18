#include "popcount.h"
#include <algorithm>
#include <testthat.h>

#define ASSERT_IS_EQUAL(x, y)                                                  \
    expect_true((x) == (y));                                                   \
    if ((x) != (y)) {                                                          \
        return;                                                                \
    };

namespace {
template <typename T, typename U>
bool are_equal(const T &actual, const U &expected) {
    return ((actual.size() == expected.size()) &&
            std::equal(actual.begin(), actual.end(), expected.begin(), expected.end()));
}
} // namespace

// Initialize a unit test context. This is similar to how you
// might begin an R test file with 'context()', expect the
// associated context should be wrapped in braced.
context("Popcount") {

    // The format for specifying tests is similar to that of
    // testthat's R functions. Use 'test_that()' to define a
    // unit test, and use 'expect_true()' and 'expect_false()'
    // to test the desired conditions.
    test_that("Size") {
        // Empty arrays are acceptable
        for (size_t array_size{0}; array_size < 3; ++array_size) {
            const rCppSample::RawVector arg(array_size, 0);
            const auto actual = popcount_cpp_raw(arg);
            ASSERT_IS_EQUAL(actual.size(), arg.size());
        }
    }

    test_that("RawValues") {
        // Check 0..255 uint8 values
        const rCppSample::RawVector arg{0, 1, 2, 3, 6, 7, 254, 255};
        const rCppSample::IntegerVector expected{0, 1, 1, 2, 2, 3, 7, 8};
        const auto actual = popcount_cpp_raw(arg);
        are_equal(actual, expected);
    }

    test_that("IntegerValues") {
        // Check some non-negative int32 values
        const rCppSample::IntegerVector arg{
            0,       1,          0xfe,       0xff,       0x100,    0xffff,
            0x10000, 0x7ffffffe, 0x7fffffff, 0x70f0f0f0, 0xf0f0f0f};
        const rCppSample::IntegerVector expected{0, 1,  7,  8,  1, 16,
                                                 1, 30, 31, 15, 16};
        const auto actual = popcount_cpp_integer(arg);
        are_equal(actual, expected);
    }

    test_that("NAs") {
        // Check NA values
        const rCppSample::IntegerVector arg{2, rCppSample::NaInteger, 14,
                                            rCppSample::NaInteger, 62};
        const rCppSample::IntegerVector expected{1, rCppSample::NaInteger, 3,
                                                 rCppSample::NaInteger, 5};
        const auto actual = popcount_cpp_integer(arg);
        are_equal(actual, expected);
    }

    test_that("NegativeIntegerValues") {
        // Check negative int32 values
        // 0xffffffff, 0xfffffffe, 0xff0f0f0f, 0xf0f0f0f0, 0x80000001
        // Note that 0x80000000 is treated as NA in R
        const rCppSample::IntegerVector arg{-1, -2, -15790321, -252645136,
                                            -2147483647};
        const rCppSample::IntegerVector expected{32, 31, 20, 16, 2};
        const auto actual = popcount_cpp_integer(arg);
        are_equal(actual, expected);
    }

    test_that("FullRawValues") {
        // Check all uint8 values
        size_t element_size = 256;
        size_t set_size = 2;
        size_t array_size = element_size * set_size;
        rCppSample::RawVector arg(array_size);
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
        are_equal(actual, expected);
    }
}
