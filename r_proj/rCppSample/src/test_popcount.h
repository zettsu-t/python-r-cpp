#ifndef SRC_TEST_POPCOUNT_H
#define SRC_TEST_POPCOUNT_H

#include "popcount_impl.h"
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

#endif // SRC_TEST_POPCOUNT_H
