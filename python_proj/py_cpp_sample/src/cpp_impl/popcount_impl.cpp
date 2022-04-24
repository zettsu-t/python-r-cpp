#include "popcount.h"
#include <stdexcept>

namespace py_cpp_sample {
/**
 * @tparam SourceType The type of xs elements
 * @param[in] xs An integer array
 * @return The number of 1's of each element in xs
 */
template <typename SourceType>
pybind11::array_t<Count> popcount_cpp_impl(
    pybind11::array_t<SourceType, pybind11::array::c_style |
                                      pybind11::array::forcecast> &xs) {
    if (!xs.dtype().is(pybind11::dtype::of<SourceType>())) {
        throw std::runtime_error("Unsupported array element types");
    }

    const auto buffer_xs = xs.request();
    if (buffer_xs.ndim == 0) {
        throw std::runtime_error(
            "xs must be a 1-D uint array (a scalar variable passed?)");
    }
    if (buffer_xs.ndim != 1) {
        throw std::runtime_error("xs must be a 1-D uint array");
    }

    // Assuming NumPy arrays have C-like dense memory layout
    if (buffer_xs.strides.at(0) != sizeof(SourceType)) {
        throw std::runtime_error("Unexpected array layout");
    }

    pybind11::array_t<Count, pybind11::array::c_style> counts{buffer_xs.shape};
    auto buffer_counts = counts.request();
    if (buffer_counts.strides.at(0) != sizeof(Count)) {
        throw std::runtime_error("Unexpected array layout");
    }

    auto size = buffer_xs.shape.at(0);
    const SourceType *src = static_cast<const SourceType *>(buffer_xs.ptr);
    Count *dst = static_cast<Count *>(buffer_counts.ptr);
    for (decltype(size) i{0}; i < size; ++i) {
        const auto value = src[i];
#ifdef __GNUC__
        const auto count = static_cast<Count>(__builtin_popcountll(value));
#else
#error Use an alternative of __builtin_popcountll
#endif
        dst[i] = count;
    }
    return counts;
}

pybind11::array_t<uint8_t>
popcount_cpp_uint8(pybind11::array_t<uint8_t, pybind11::array::c_style |
                                                  pybind11::array::forcecast>
                       xs) {
    return popcount_cpp_impl<uint8_t>(xs);
}

pybind11::array_t<uint8_t>
popcount_cpp_uint64(pybind11::array_t<uint64_t, pybind11::array::c_style |
                                                    pybind11::array::forcecast>
                        xs) {
    return popcount_cpp_impl<uint64_t>(xs);
}
} // namespace py_cpp_sample
