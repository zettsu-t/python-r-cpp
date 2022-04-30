#ifndef TESTS_TEST_POPCOUNT_H
#define TESTS_TEST_POPCOUNT_H

#include "popcount.h"
#include <type_traits>

namespace {
template <typename T> struct VectorSize {
#if __cplusplus < 201703L
    using size_type = typename std::result_of<decltype (&T::size)(T)>::type;
#else
    using size_type = std::invoke_result_t<decltype(&T::size), T>;
#endif
};
} // namespace

#endif // TESTS_TEST_POPCOUNT_H
