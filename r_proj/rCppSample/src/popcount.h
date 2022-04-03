#ifdef UNIT_TEST_CPP
#include <vector>
#include <cstdint>
#else // UNIT_TEST_CPP
#include <Rcpp.h>
#endif // UNIT_TEST_CPP

namespace rCppSample {
#ifdef UNIT_TEST_CPP
    // Types for testing
    using IntegerVector = std::vector<int>;
    using RawVector = std::vector<uint8_t>;
    using ArgIntegerVector = const std::vector<int>&;
    using ArgRawVector = const std::vector<uint8_t>&;
#else // UNIT_TEST_CPP
    using IntegerVector = Rcpp::IntegerVector;
    using RawVector = Rcpp::RawVector;
    using ArgIntegerVector = Rcpp::IntegerVector;
    using ArgRawVector = Rcpp::RawVector;
#endif // UNIT_TEST_CPP
}

#ifdef UNIT_TEST_CPP
extern rCppSample::IntegerVector popcount_cpp_raw(rCppSample::ArgRawVector xs);
extern rCppSample::IntegerVector popcount_cpp_integer(rCppSample::ArgIntegerVector xs);
#else // UNIT_TEST_CPP
// Call by value, not reference to check types!
//' Count 1's in each raw element
//'
//' @param xs A raw vector to count populations
//' @return The populations of elements in the vector
// [[Rcpp::export]]
extern Rcpp::IntegerVector popcount_cpp_raw(Rcpp::RawVector xs);

//' Count 1's in each integer element
//'
//' @param xs An integer vector to count populations
//' @return The populations of elements in the vector
// [[Rcpp::export]]
extern Rcpp::IntegerVector popcount_cpp_integer(Rcpp::IntegerVector xs);
#endif // UNIT_TEST_CPP

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
