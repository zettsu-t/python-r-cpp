#include "popcount.hpp"
#include "popcount_boost.hpp"
#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace {
// Argument and return types
using PyUint8Array = pybind11::array_t<uint8_t, pybind11::array::c_style |
                                                    pybind11::array::forcecast>;

using PyUint64Array =
    pybind11::array_t<uint64_t,
                      pybind11::array::c_style | pybind11::array::forcecast>;
using Count = py_cpp_sample::Count;
using CountArray = pybind11::array_t<uint8_t>;

// pybind11 type(s)
using PyBindSize = pybind11::ssize_t;

// Boost.Numpy types
using BoostArrayShape = const boost::python::tuple;
using BoostDataType = const boost::python::numpy::dtype;

// Expected values
const std::vector<uint8_t> Input_Uint8{0, 1, 2, 3, 6, 7, 254, 255};
const std::vector<Count> Expected_Uint8{0, 1, 1, 2, 2, 3, 7, 8};
const std::vector<uint64_t> Input_Uint64{0,
                                         1,
                                         0xfe,
                                         0xff,
                                         0x100,
                                         0x101,
                                         0xfffe,
                                         0xffff,
                                         0x10000,
                                         0x10001,
                                         0xfffffffeull,
                                         0xffffffffull,
                                         0x100000000ull,
                                         0x100000001ull,
                                         0x3c3c3c3c3c3c3c3cull,
                                         0xc3c3c3c3c3c3c3c3ull,
                                         0xffffffffffffffffull};
const std::vector<Count> Expected_Uint64{0, 1,  7,  8, 1, 2,  15, 16, 1,
                                         2, 31, 32, 1, 2, 32, 32, 64};

template <typename T> auto create_numpy_data_type_pybind11() {
    return pybind11::dtype::of<T>();
}

auto create_count_data_type_pybind11() {
    return create_numpy_data_type_pybind11<Count>();
}

template <typename ArrayElementType, typename ArrayType, typename SizeType>
bool check_numpy_array_type_pybind11(const ArrayType &actual,
                                     SizeType expected_size) {
    const auto buffer = actual.request();

    // 1-D array
    if (buffer.ndim != 1) {
        return false;
    }

    // Exact element size
    if (buffer.shape.at(0) != expected_size) {
        return false;
    }

    // Exact element type
    if (!actual.dtype().is(
            create_numpy_data_type_pybind11<ArrayElementType>())) {
        return false;
    }

    // C-like dense array
    if (buffer.strides.at(0) != sizeof(ArrayElementType)) {
        return false;
    }

    return true;
}

template <typename ArrayType, typename SizeType>
bool check_count_type_pybind11(const ArrayType &actual,
                               SizeType expected_size) {
    return check_numpy_array_type_pybind11<Count>(actual, expected_size);
}

template <typename T> BoostDataType create_numpy_data_type_boost() {
    return boost::python::numpy::dtype::get_builtin<T>();
}

BoostDataType create_count_data_type_boost() {
    return create_numpy_data_type_boost<Count>();
}

template <typename ArrayElementType, typename ArrayType, typename SizeType>
bool check_numpy_array_type_boost(const ArrayType &actual,
                                  SizeType expected_size) {
    // 1-D array
    if (actual.get_nd() != 1) {
        return false;
    }

    // Exact element size
    if (actual.shape(0) != expected_size) {
        return false;
    }

    // Exact element type
    if (actual.get_dtype() !=
        create_numpy_data_type_boost<ArrayElementType>()) {
        return false;
    }

    // C-like dense array
    if (actual.strides(0) != sizeof(ArrayElementType)) {
        return false;
    }

    return true;
}

template <typename ArrayType, typename SizeType>
bool check_count_type_boost(const ArrayType &actual, SizeType expected_size) {
    return check_numpy_array_type_boost<Count>(actual, expected_size);
}

template <typename Element, typename SizeType>
std::vector<Count> setup_popcount(SizeType array_size, SizeType value_offset,
                                  Element *ptr) {
    std::vector<Count> expected(array_size);
    constexpr auto max_value = std::numeric_limits<Element>::max();

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        auto low_value = index + value_offset;
        constexpr auto mask_value = static_cast<decltype(low_value)>(max_value);
        low_value &= mask_value;
        auto element = static_cast<Element>(low_value);
        ptr[index] = element;

        for (size_t bit_index = 0; bit_index < (sizeof(low_value) * 8);
             ++bit_index) {
            decltype(low_value) mask = 1;
            mask <<= bit_index;
            expected.at(index) += ((low_value & mask) != 0);
        }
    }

    return expected;
}
} // namespace

class TestPopcountPybind11 : public ::testing::Test {};
class TestPopcountBoost : public ::testing::Test {};

template <typename T> class TestPopcountTyped : public ::testing::Test {};
using PopcountTypes = ::testing::Types<uint8_t, uint64_t>;
TYPED_TEST_SUITE(TestPopcountTyped, PopcountTypes);

TEST_F(TestPopcountPybind11, InvalidDimension) {
    PyUint8Array arg({2, 3});
    const auto shape0 = arg.shape(0);
    const auto shape1 = arg.shape(1);

    for (auto index0 = 0; index0 < shape0; ++index0) {
        for (auto index1 = 0; index1 < shape1; ++index1) {
            *arg.mutable_data(index0, index1) = 0;
        }
    }

    ASSERT_THROW(py_cpp_sample::popcount_cpp_uint8(arg), std::runtime_error);
}

TEST_F(TestPopcountBoost, InvalidDimension) {
    BoostArrayShape shape = boost::python::make_tuple(2, 3);
    BoostDataType data_type = create_numpy_data_type_boost<uint8_t>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_THROW(py_cpp_sample::popcount_cpp_boost(arg), std::runtime_error);
}

TEST_F(TestPopcountBoost, InvalidElemenyType) {
    constexpr size_t size = 1;
    BoostArrayShape shape = boost::python::make_tuple(size);
    BoostDataType data_type = create_numpy_data_type_boost<uint16_t>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_THROW(py_cpp_sample::popcount_cpp_boost(arg), std::runtime_error);
}

TYPED_TEST(TestPopcountTyped, ArraySizePybind11) {
    using Element = TypeParam;
    using PyArray = pybind11::array_t<Element, pybind11::array::c_style |
                                                   pybind11::array::forcecast>;

    // Empty arrays are acceptable
    for (PyBindSize array_size = 0; array_size < 3; ++array_size) {
        PyArray arg({array_size});
        const auto buffer_arg = arg.request();

        ASSERT_EQ(1, buffer_arg.ndim);
        ASSERT_EQ(array_size, buffer_arg.shape.at(0));
        ASSERT_TRUE(arg.dtype().is(create_numpy_data_type_pybind11<Element>()));
        ASSERT_EQ(sizeof(Element), buffer_arg.strides.at(0));
        ASSERT_TRUE(check_numpy_array_type_pybind11<Element>(arg, array_size));

        CountArray actual;
        if (std::is_same<Element, uint8_t>::value) {
            actual = py_cpp_sample::popcount_cpp_uint8(arg);
        } else if (std::is_same<Element, uint64_t>::value) {
            actual = py_cpp_sample::popcount_cpp_uint64(arg);
        } else {
            ASSERT_TRUE(false);
        }
        const auto buffer_actual = actual.request();
        ASSERT_EQ(1, buffer_actual.ndim);
        ASSERT_EQ(array_size, buffer_actual.shape.at(0));
        ASSERT_TRUE(actual.dtype().is(create_count_data_type_pybind11()));
        ASSERT_EQ(sizeof(Count), buffer_actual.strides.at(0));
        ASSERT_TRUE(check_count_type_pybind11(actual, array_size));
    }
}

TYPED_TEST(TestPopcountTyped, ArraySizeBoost) {
    using Element = TypeParam;

    // Empty arrays are acceptable
    for (size_t array_size = 0; array_size < 3; ++array_size) {
        BoostArrayShape shape = boost::python::make_tuple(array_size);
        BoostDataType data_type = create_numpy_data_type_boost<Element>();
        auto arg = boost::python::numpy::zeros(shape, data_type);

        ASSERT_EQ(1, arg.get_nd());
        ASSERT_EQ(array_size, arg.shape(0));
        ASSERT_EQ(create_numpy_data_type_boost<Element>(), arg.get_dtype());
        ASSERT_EQ(sizeof(Element), arg.strides(0));
        ASSERT_TRUE(check_numpy_array_type_boost<Element>(arg, array_size));

        const auto actual = py_cpp_sample::popcount_cpp_boost(arg);
        ASSERT_EQ(1, actual.get_nd());
        ASSERT_EQ(array_size, actual.shape(0));
        ASSERT_EQ(create_count_data_type_boost(), actual.get_dtype());
        ASSERT_EQ(sizeof(Count), actual.strides(0));
        ASSERT_EQ(create_count_data_type_boost(), actual.get_dtype());
    }
}

TEST_F(TestPopcountPybind11, ValuesUint8) {
    using Element = uint8_t;
    auto array_size = Expected_Uint8.size();
    ASSERT_EQ(array_size, Input_Uint8.size());

    const auto arg_array_size = static_cast<PyBindSize>(array_size);
    PyUint8Array arg({{arg_array_size}});
    ASSERT_TRUE(check_numpy_array_type_pybind11<Element>(arg, array_size));

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        *arg.mutable_data(index) = Input_Uint8.at(index);
    }

    const auto actual = py_cpp_sample::popcount_cpp_uint8(arg);
    ASSERT_TRUE(check_count_type_pybind11(actual, array_size));

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(Expected_Uint8.at(index), *actual.data(index));
    }
}

TEST_F(TestPopcountBoost, ValuesUint8) {
    using Element = uint8_t;
    auto array_size = Expected_Uint8.size();
    ASSERT_EQ(array_size, Input_Uint8.size());

    BoostArrayShape shape = boost::python::make_tuple(array_size);
    BoostDataType data_type = create_numpy_data_type_boost<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type_boost<Element>(arg, array_size));

    Element *arg_values = reinterpret_cast<Element *>(arg.get_data());
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        arg_values[index] = Input_Uint8.at(index);
    }

    const auto actual = py_cpp_sample::popcount_cpp_boost(arg);
    ASSERT_TRUE(check_count_type_boost(actual, array_size));

    Count *actual_counts = reinterpret_cast<Count *>(actual.get_data());
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(Expected_Uint8.at(index), actual_counts[index]);
    }
}

TEST_F(TestPopcountPybind11, ValuesUint64) {
    using Element = uint64_t;
    auto array_size = Expected_Uint64.size();
    ASSERT_EQ(array_size, Input_Uint64.size());

    const auto arg_array_size = static_cast<PyBindSize>(array_size);
    PyUint64Array arg({{arg_array_size}});
    ASSERT_TRUE(check_numpy_array_type_pybind11<Element>(arg, array_size));

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        *arg.mutable_data(index) = Input_Uint64.at(index);
    }

    const auto actual = py_cpp_sample::popcount_cpp_uint64(arg);
    ASSERT_TRUE(check_count_type_pybind11(actual, array_size));

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(Expected_Uint64.at(index), *actual.data(index));
    }
}

TEST_F(TestPopcountBoost, ValuesUint64) {
    using Element = uint64_t;
    auto array_size = Expected_Uint64.size();
    ASSERT_EQ(array_size, Input_Uint64.size());

    BoostArrayShape shape = boost::python::make_tuple(array_size);
    BoostDataType data_type = create_numpy_data_type_boost<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type_boost<Element>(arg, array_size));

    Element *arg_values = reinterpret_cast<Element *>(arg.get_data());
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        arg_values[index] = Input_Uint64.at(index);
    }

    const auto actual = py_cpp_sample::popcount_cpp_boost(arg);
    ASSERT_TRUE(check_count_type_boost(actual, array_size));

    Count *actual_counts = reinterpret_cast<Count *>(actual.get_data());
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(Expected_Uint64.at(index), actual_counts[index]);
    }
}

TEST_F(TestPopcountPybind11, FullValuesUint8) {
    using Element = uint8_t;
    size_t element_size = 256;
    size_t set_size = 1024;
    size_t array_size = element_size * set_size;
    const auto arg_array_size = static_cast<PyBindSize>(array_size);

    PyUint8Array arg({{arg_array_size}});
    ASSERT_TRUE(check_numpy_array_type_pybind11<Element>(arg, array_size));

    const auto buffer = arg.request();
    auto arg_values = static_cast<Element *>(buffer.ptr);
    decltype(array_size) value_offset{0};
    auto expected =
        setup_popcount<Element>(array_size, value_offset, arg_values);
    ASSERT_EQ(array_size, expected.size());

    const auto actual = py_cpp_sample::popcount_cpp_uint8(arg);
    ASSERT_TRUE(check_count_type_pybind11(actual, array_size));

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(expected.at(index), *actual.data(index));
    }
}

TEST_F(TestPopcountBoost, FullValuesUint8) {
    using Element = uint8_t;
    constexpr size_t element_size = 256;
    constexpr size_t set_size = 1024;
    size_t array_size = element_size * set_size;

    BoostArrayShape shape = boost::python::make_tuple(array_size);
    BoostDataType data_type = create_numpy_data_type_boost<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type_boost<Element>(arg, array_size));

    Element *arg_values = reinterpret_cast<Element *>(arg.get_data());
    decltype(array_size) value_offset{0};
    auto expected =
        setup_popcount<Element>(array_size, value_offset, arg_values);
    ASSERT_EQ(array_size, expected.size());

    const auto actual = py_cpp_sample::popcount_cpp_boost(arg);
    ASSERT_TRUE(check_count_type_boost(actual, array_size));

    Count *actual_counts = reinterpret_cast<Count *>(actual.get_data());
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(expected.at(index), actual_counts[index]);
    }
}

TEST_F(TestPopcountPybind11, LargeUint64) {
    using Element = uint64_t;
    size_t array_size = 0x100;
    constexpr size_t value_offset = 0x7fffffffffffff00;
    const auto arg_array_size = static_cast<PyBindSize>(array_size);

    PyUint64Array arg({{arg_array_size}});
    ASSERT_TRUE(check_numpy_array_type_pybind11<Element>(arg, array_size));

    const auto buffer = arg.request();
    auto arg_values = static_cast<Element *>(buffer.ptr);
    auto expected =
        setup_popcount<Element>(array_size, value_offset, arg_values);
    ASSERT_EQ(array_size, expected.size());

    const auto actual = py_cpp_sample::popcount_cpp_uint64(arg);
    ASSERT_TRUE(check_count_type_pybind11(actual, array_size));

    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(expected.at(index), *actual.data(index));
    }
}

TEST_F(TestPopcountBoost, LargeUint64) {
    using Element = uint64_t;
    size_t array_size = 0x100;
    size_t value_offset = 0x7ffffffffffffff0;

    BoostArrayShape shape = boost::python::make_tuple(array_size);
    BoostDataType data_type = create_numpy_data_type_boost<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type_boost<Element>(arg, array_size));

    Element *arg_values = reinterpret_cast<Element *>(arg.get_data());
    auto expected =
        setup_popcount<Element>(array_size, value_offset, arg_values);
    ASSERT_EQ(array_size, expected.size());

    const auto actual = py_cpp_sample::popcount_cpp_boost(arg);
    ASSERT_TRUE(check_count_type_boost(actual, array_size));

    Count *actual_counts = reinterpret_cast<Count *>(actual.get_data());
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        ASSERT_EQ(expected.at(index), actual_counts[index]);
    }
}

TEST_F(TestPopcountPybind11, BitsUint64) {
    using Element = uint64_t;
    Element value = 0;
    Element mask = 1;

    for (size_t count = 0; count < (sizeof(value) * 8 + 1); ++count) {
        constexpr PyBindSize array_size = 1;
        PyUint64Array arg({{array_size}});
        ASSERT_TRUE(check_numpy_array_type_pybind11<Element>(arg, array_size));

        *arg.mutable_data(0) = value;
        const auto actual = py_cpp_sample::popcount_cpp_uint64(arg);
        ASSERT_TRUE(check_count_type_pybind11(actual, array_size));

        ASSERT_EQ(count, *actual.data(0));
        value |= mask;
        mask <<= 1;
    }
}

TEST_F(TestPopcountBoost, BitsUint64) {
    using Element = uint64_t;
    Element value = 0;
    Element mask = 1;
    constexpr size_t array_size = 1;

    for (size_t count = 0; count < (sizeof(value) * 8 + 1); ++count) {
        BoostArrayShape shape = boost::python::make_tuple(array_size);
        BoostDataType data_type = create_numpy_data_type_boost<Element>();
        auto arg = boost::python::numpy::zeros(shape, data_type);
        ASSERT_TRUE(check_numpy_array_type_boost<Element>(arg, array_size));

        Element *arg_values = reinterpret_cast<Element *>(arg.get_data());
        *arg_values = value;

        const auto actual = py_cpp_sample::popcount_cpp_boost(arg);
        ASSERT_TRUE(check_count_type_boost(actual, array_size));

        Count *actual_counts = reinterpret_cast<Count *>(actual.get_data());
        ASSERT_EQ(count, *actual_counts);
        value |= mask;
        mask <<= 1;
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    Py_Initialize();
    boost::python::numpy::initialize();
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-unix
tab-width: nil
c-file-style: "stroustrup"
End:
*/
