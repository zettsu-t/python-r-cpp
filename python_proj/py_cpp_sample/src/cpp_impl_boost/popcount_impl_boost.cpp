#include "popcount_boost.hpp"

namespace py_cpp_sample {
    /**
     * @tparam SourceType The type of xs elements
     * @param[in] xs An integer array
     * @return The number of 1's of each element in xs
     */
    template<typename SourceType>
    boost::python::numpy::ndarray popcount_cpp_impl_boost(const boost::python::numpy::ndarray& xs) {
        // Assuming NumPy arrays have C-like dense memory layout
        if (xs.strides(0) != sizeof(SourceType)) {
            throw std::runtime_error("Unexpected array layout");
        }

        auto size = xs.shape(0);
        const boost::python::tuple shape = boost::python::make_tuple(size);
        const boost::python::numpy::dtype data_type = boost::python::numpy::dtype::get_builtin<Count>();
        auto counts = boost::python::numpy::zeros(shape, data_type);
        if (counts.strides(0) != sizeof(Count)) {
            throw std::runtime_error("Unexpected array layout");
        }

        const SourceType* src = reinterpret_cast<const SourceType*>(xs.get_data());
        Count* dst = reinterpret_cast<Count*>(counts.get_data());
        for(decltype(size) i {0}; i < size; ++i) {
            const auto value = src[i];
#ifdef __GNUC__
            const auto count = static_cast<Count>(__builtin_popcountll(value));
#else
#error Use an alternative of __builtin_popcount
#endif
            dst[i] = count;
        }
        return counts;
    }

    boost::python::numpy::ndarray popcount_cpp_boost(const boost::python::numpy::ndarray& xs) {
        if (xs.get_nd() != 1) {
            throw std::runtime_error("xs must be a 1-D uint array");
        }

        if (xs.get_dtype() == boost::python::numpy::dtype::get_builtin<uint8_t>()) {
            return popcount_cpp_impl_boost<uint8_t>(xs);
        } else if (xs.get_dtype() == boost::python::numpy::dtype::get_builtin<uint64_t>()) {
            return popcount_cpp_impl_boost<uint64_t>(xs);
        }

        throw std::runtime_error("Unsupported array element types");
    }
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/