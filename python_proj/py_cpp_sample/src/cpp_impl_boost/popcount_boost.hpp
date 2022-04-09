#include <cstdint>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

/**
 C++ implementation
 */
namespace py_cpp_sample {
    using Count = uint8_t;

    /**
     * @param[in] xs An integer array
     * @return The number of 1's of each element in xs
     */
    extern boost::python::numpy::ndarray popcount_cpp_boost(const boost::python::numpy::ndarray& xs);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
