#include "popcount_impl.h"

namespace {
//' Count 1's in each element
//'
//' @tparam T A type of integers
//' @param xs An integer vector to count populations
//' @return The populations of elements in the vector
template <typename T> rCppSample::IntegerVector popcount_cpp_impl(const T &xs) {
    auto size = xs.size();
    // Initialize with 0s
    rCppSample::IntegerVector results(size);

    for (decltype(size) i = 0; i < size; ++i) {
        const auto x = xs[i];
        static_assert(std::is_integral<decltype(x)>::value, "Must be integral");
#ifdef __GNUC__
#ifdef UNIT_TEST_CPP
        const auto v = is_na_integer(x)
                           ? get_na_int_value()
                           : __builtin_popcount(static_cast<unsigned int>(x));
#else  // UNIT_TEST_CPP
        const auto v = is_na_integer<T>(x)
                           ? get_na_int_value()
                           : __builtin_popcount(static_cast<unsigned int>(x));
#endif // UNIT_TEST_CPP
#else
#error Use an alternative of __builtin_popcount
#endif
        results[i] = v;
    }
    return results;
}
} // namespace

#ifdef UNIT_TEST_CPP
rCppSample::IntegerVector popcount_cpp_raw(rCppSample::ArgRawVector xs)
#else  // UNIT_TEST_CPP
Rcpp::IntegerVector popcount_cpp_raw(const Rcpp::RawVector &xs)
#endif // UNIT_TEST_CPP
{
    return popcount_cpp_impl(xs);
}

#ifdef UNIT_TEST_CPP
rCppSample::IntegerVector popcount_cpp_integer(rCppSample::ArgIntegerVector xs)
#else  // UNIT_TEST_CPP
Rcpp::IntegerVector popcount_cpp_integer(const Rcpp::IntegerVector &xs)
#endif // UNIT_TEST_CPP
{
    return popcount_cpp_impl(xs);
}
