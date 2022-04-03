#include "popcount_boost.hpp"

BOOST_PYTHON_MODULE(py_cpp_sample_cpp_impl_boost) {
    Py_Initialize();
    boost::python::numpy::initialize();
    boost::python::def("popcount_cpp_boost", py_cpp_sample::popcount_cpp_boost);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/