#include <vector>
#include <limits>
#include <gtest/gtest.h>
#include "popcount.hpp"

class TestPopcount : public ::testing::Test {};

namespace {
    using ArrayShape = const boost::python::tuple;
    using DataType = const boost::python::numpy::dtype;
    using Count = py_cpp_sample::Count;
}

namespace {
    template<typename T>
    DataType create_numpy_data_type() {
        return boost::python::numpy::dtype::get_builtin<T>();
    }

    DataType create_count_data_type() {
        return create_numpy_data_type<Count>();
    }

    template<typename ArrayElementType, typename SizeType, typename ArrayType>
    bool check_numpy_array_type(SizeType expected_size, const ArrayType& actual) {
        // 1-D array
        if (actual.get_nd() != 1) {
            return false;
        }

        if (actual.strides(0) != sizeof(ArrayElementType)) {
            return false;
        }

        // Exact element size
        if (actual.shape(0) != expected_size) {
            return false;
        }

        if (actual.get_dtype() != create_numpy_data_type<ArrayElementType>()) {
            return false;
        }

        return true;
    }

    template<typename SizeType, typename ArrayType>
    bool check_count_type(SizeType expected_size, const ArrayType& actual) {
        return check_numpy_array_type<Count>(expected_size, actual);
    }
}

TEST_F(TestPopcount, InvalidDimension) {
    ArrayShape shape = boost::python::make_tuple(2, 3);
    DataType data_type = create_numpy_data_type<uint8_t>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_THROW(py_cpp_sample::popcount_cpp(arg), std::runtime_error);
}

TEST_F(TestPopcount, InvalidElemenyType) {
    constexpr size_t size = 1;
    ArrayShape shape = boost::python::make_tuple(size);
    DataType data_type = create_numpy_data_type<uint64_t>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_THROW(py_cpp_sample::popcount_cpp(arg), std::runtime_error);
}

TEST_F(TestPopcount, ArraySize) {
    using Element = uint8_t;

    // Empty arrays are acceptable
    for(size_t array_size=0; array_size<3; ++array_size) {
        ArrayShape shape = boost::python::make_tuple(array_size);
        DataType data_type = create_numpy_data_type<Element>();
        auto arg = boost::python::numpy::zeros(shape, data_type);
        ASSERT_EQ(sizeof(Element), arg.strides(0));
        ASSERT_TRUE(check_numpy_array_type<Element>(array_size, arg));

        const auto actual = py_cpp_sample::popcount_cpp(arg);
        ASSERT_EQ(sizeof(Count), actual.strides(0));

        const auto n_dim = actual.get_nd();
        ASSERT_EQ(1, n_dim);
        auto actual_size = actual.shape(0);
        ASSERT_EQ(array_size, actual_size);
        ASSERT_EQ(actual.get_dtype(), create_count_data_type());
        ASSERT_TRUE(check_count_type(array_size, actual));
    }
}

TEST_F(TestPopcount, ValuesUint8) {
    using Element = uint8_t;

    std::vector<Element> target {0, 1, 2, 3, 6, 7, 254, 255};
    std::vector<Count> expected {0, 1, 1, 2, 2, 3, 7, 8};
    auto array_size = expected.size();
    ASSERT_EQ(array_size, target.size());

    ArrayShape shape = boost::python::make_tuple(array_size);
    DataType data_type = create_numpy_data_type<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type<Element>(array_size, arg));

    Element* arg_values = reinterpret_cast<Element*>(arg.get_data());
    for(decltype(array_size) index {0}; index<array_size; ++index) {
        arg_values[index] = target.at(index);
    }

    const auto actual = py_cpp_sample::popcount_cpp(arg);
    ASSERT_TRUE(check_count_type(array_size, actual));

    Count* actual_counts = reinterpret_cast<Count*>(actual.get_data());
    for(decltype(array_size) index {0}; index<array_size; ++index) {
        ASSERT_EQ(expected.at(index), actual_counts[index]);
    }
}

TEST_F(TestPopcount, ValuesUint32) {
    using Element = uint32_t;

    std::vector<Element> target {0, 1, 0xfe, 0xff, 0x100, 0xffff,
        0x10000, 0xfffffffe, 0xffffffff,
        0xf0f0f0f0, 0xf0f0f0f};
    std::vector<Count> expected {0, 1, 7, 8, 1, 16, 1, 31, 32, 16, 16};
    auto array_size = expected.size();
    ASSERT_EQ(array_size, target.size());

    ArrayShape shape = boost::python::make_tuple(array_size);
    DataType data_type = create_numpy_data_type<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type<Element>(array_size, arg));

    Element* arg_values = reinterpret_cast<Element*>(arg.get_data());
    for(decltype(array_size) index {0}; index<array_size; ++index) {
        arg_values[index] = target.at(index);
    }

    const auto actual = py_cpp_sample::popcount_cpp(arg);
    ASSERT_TRUE(check_count_type(array_size, actual));

    Count* actual_counts = reinterpret_cast<Count*>(actual.get_data());
    for(decltype(array_size) index {0}; index<array_size; ++index) {
        ASSERT_EQ(expected.at(index), actual_counts[index]);
    }
}

TEST_F(TestPopcount, FullValuesUint8) {
    using Element = uint8_t;

    size_t element_size = 256;
    size_t set_size = 1024;
    size_t array_size = element_size * set_size;
    std::vector<Count> expected(array_size);
    ASSERT_EQ(array_size, expected.size());

    ArrayShape shape = boost::python::make_tuple(array_size);
    DataType data_type = create_numpy_data_type<Element>();
    auto arg = boost::python::numpy::zeros(shape, data_type);
    ASSERT_TRUE(check_numpy_array_type<Element>(array_size, arg));

    Element* arg_values = reinterpret_cast<Element*>(arg.get_data());
    constexpr auto max_value = std::numeric_limits<Element>::max();
    for(decltype(array_size) index {0}; index<array_size; ++index) {
        auto low_value = index;
        constexpr auto mask_value = static_cast<decltype(low_value)>(max_value);
        low_value &= mask_value;
        auto element = static_cast<Element>(low_value);
        arg_values[index] = element;
        for (size_t bit_index = 0; bit_index < (sizeof(Element) * 8); ++bit_index) {
            decltype(low_value) mask = 1;
            mask <<= bit_index;
            expected.at(index) += ((low_value & mask) != 0);
        }
    }

    const auto actual = py_cpp_sample::popcount_cpp(arg);
    ASSERT_TRUE(check_count_type(array_size, actual));

    Count* actual_counts = reinterpret_cast<Count*>(actual.get_data());
    for(decltype(array_size) index {0}; index<array_size; ++index) {
        EXPECT_EQ(expected.at(index), actual_counts[index]);
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
