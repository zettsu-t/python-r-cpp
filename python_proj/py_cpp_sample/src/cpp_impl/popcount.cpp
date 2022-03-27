#include "popcount.hpp"

BOOST_PYTHON_MODULE(cpp_impl) {
    Py_Initialize();
    boost::python::numpy::initialize();
    boost::python::def("popcount_cpp", py_cpp_sample::popcount_cpp);
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
