#include "popcount.h"

namespace {
    //' Count 1's in each element
    //'
    //' @tparam T A type of integers
    //' @param xs An integer vector to count populations
    //' @return The populations of elements in the vector
    template<typename T>
    rCppSample::IntegerVector popcount_cpp_impl(const T& xs) {
        auto size = xs.size();
        // Initialize with 0s
        rCppSample::IntegerVector results(size);

        for(decltype(size) i = 0; i < size; ++i) {
            const auto x = xs[i];
#ifdef __GNUC__
            const auto v = __builtin_popcount(x);
#else
#error Use an alternative of __builtin_popcount
#endif
            results[i] = v;
        }
        return results;
    }
}

#ifdef UNIT_TEST_CPP
rCppSample::IntegerVector popcount_cpp_raw(rCppSample::ArgRawVector xs)
#else // UNIT_TEST_CPP
//' Count 1's in each raw element
//'
//' @param xs A raw vector to count populations
//' @return The populations of elements in the vector
// [[Rcpp::export]]
Rcpp::IntegerVector popcount_cpp_raw(Rcpp::RawVector xs)
#endif // UNIT_TEST_CPP
{
    return popcount_cpp_impl(xs);
}

#ifdef UNIT_TEST_CPP
rCppSample::IntegerVector popcount_cpp_integer(rCppSample::ArgIntegerVector xs)
#else // UNIT_TEST_CPP
//' Count 1's in each integer element
//'
//' @param xs An integer vector to count populations
//' @return The populations of elements in the vector
// [[Rcpp::export]]
Rcpp::IntegerVector popcount_cpp_integer(Rcpp::IntegerVector xs)
#endif // UNIT_TEST_CPP
{
    return popcount_cpp_impl(xs);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
