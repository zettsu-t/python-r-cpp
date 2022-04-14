#ifndef CPP_IMPL_BOOST_POPCOUNT_BOOST_H
#define CPP_IMPL_BOOST_POPCOUNT_BOOST_H

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <cstdint>

/**
 C++ implementation
 */
namespace py_cpp_sample {
using Count = uint8_t;

/**
 * @param[in] xs An integer array
 * @return The number of 1's of each element in xs
 */
extern boost::python::numpy::ndarray
popcount_cpp_boost(const boost::python::numpy::ndarray &xs);
} // namespace py_cpp_sample

#endif // CPP_IMPL_BOOST_POPCOUNT_BOOST_H
