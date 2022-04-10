#ifndef CPP_IMPL_POPCOUNT_H
#define CPP_IMPL_POPCOUNT_H

#include <cstdint>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

/**
 C++ implementation
 */
namespace py_cpp_sample {
using Count = uint8_t;

/**
 * @param[in] xs A uint8_t array
 * @return The number of 1's of each element in xs
 */
extern pybind11::array_t<uint8_t>
popcount_cpp_uint8(pybind11::array_t<uint8_t, pybind11::array::c_style |
                                                  pybind11::array::forcecast>
                       xs);

/**
 * @param[in] xs A uint64_t array
 * @return The number of 1's of each element in xs
 */
extern pybind11::array_t<uint8_t>
popcount_cpp_uint64(pybind11::array_t<uint64_t, pybind11::array::c_style |
                                                    pybind11::array::forcecast>
                        xs);
} // namespace py_cpp_sample

#endif // CPP_IMPL_POPCOUNT_H

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
