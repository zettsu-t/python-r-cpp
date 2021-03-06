#include "popcount.h"

PYBIND11_MODULE(py_cpp_sample_cpp_impl, mod) {
    mod.doc() = "C++ implementation of the py_cpp_sample package";
    mod.def("popcount_cpp_uint8", &py_cpp_sample::popcount_cpp_uint8);
    mod.def("popcount_cpp_uint64", &py_cpp_sample::popcount_cpp_uint64);
}
