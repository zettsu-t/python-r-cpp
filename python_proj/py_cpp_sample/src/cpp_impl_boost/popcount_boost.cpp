#include "popcount_boost.h"

BOOST_PYTHON_MODULE(py_cpp_sample_cpp_impl_boost) {
    Py_Initialize();
    boost::python::numpy::initialize();
    boost::python::def("popcount_cpp_boost", py_cpp_sample::popcount_cpp_boost);
}
