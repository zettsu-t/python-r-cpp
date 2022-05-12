#ifndef SRC_POPCOUNT_IMPL_H
#define SRC_POPCOUNT_IMPL_H

#include "popcount.h"
#include <type_traits>

namespace {
#ifdef UNIT_TEST_CPP
template <typename T> inline constexpr bool is_na_integer(T) = delete;

inline constexpr bool is_na_integer(uint8_t x) {
    return false;
}

inline constexpr bool is_na_integer(int x) {
    return (x == rCppSample::NaInteger);
}

inline constexpr int get_na_int_value() {
    return rCppSample::NaInteger;
}

#else  // UNIT_TEST_CPP
template <typename T, typename U>
inline bool is_na_integer(const U& x) {
    return T::is_na(x);
}

// Cannot use constexpr for NA_INTEGER
inline int get_na_int_value() {
    return NA_INTEGER;
}
#endif // UNIT_TEST_CPP
} // namespace

#endif // SRC_POPCOUNT_IMPL_H
