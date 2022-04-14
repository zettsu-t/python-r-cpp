"""
Testing Python code
"""

from collections import namedtuple
import re
import numpy as np
import pytest
from py_cpp_sample import popcount
from py_cpp_sample import popcount_boost

# Tested functions
POPCOUNT_SET = [(popcount), (popcount_boost)]
POPCOUNT_ARG_SET = [(popcount, np.uint8), (popcount, np.uint64),
                    (popcount_boost, np.uint8), (popcount_boost, np.uint64)]
# Messages in exceptions
EXPECTED_ERROR_COMMON_STR = "^xs must be " \
    "a 1\\-D np\\.ndarray\\(np\\.uint8\\|np\\.uint64\\)$"
EXPECTED_ERROR_SCALAR_STR = "^xs must be " \
    "a 1\\-D uint array \\(a scalar variable passed\\?\\)$"
EXPECTED_ERROR_COMMON_MSG = re.compile(EXPECTED_ERROR_COMMON_STR)
EXPECTED_ERROR_SCALAR_MSG = re.compile(EXPECTED_ERROR_SCALAR_STR)

# Measure to time fo [All uint 8 values * NUMBER_OF_UNIT]
SIZE_OF_UNIT = 256
NUMBER_OF_UNIT = 10000

ArgSet = namedtuple(
    "ArgSet",
    ["popcount_table", "array_uint8", "array_uint64"]
)


def popcount_local(num):
    """Count 1's in an integer"""
    return bin(num).count("1")


def popcount_uint64(args):
    """Count 1's in uint64 integers"""
    count = 0
    # Up to 8 bytes
    for byte_index in range(8):
        shift = byte_index * 8
        mask = 0xff << shift
        count += args.popcount_table[
            ((args.array_uint64 & mask) >> shift) & 0xff
        ]

    return count


def setup_table(size_set):
    """Fill the 256-entries look-up table and set up arguments"""
    popcount_table = np.array(
        [popcount_local(x) for x in range(256)], dtype=np.uint8
    )
    seq_uint8 = np.arange(SIZE_OF_UNIT, dtype=np.uint8)
    arg_uint8 = np.repeat(seq_uint8, size_set)
    arg_uint64 = np.arange(SIZE_OF_UNIT * size_set, dtype=np.uint64)
    return ArgSet(popcount_table, arg_uint8, arg_uint64)


def popcount_py_uint8(args):
    """Python implementation 8-bit"""
    # Return a bool
    return args.popcount_table[args.array_uint8] is not None


def popcount_py_uint64(args):
    """Python implementation 64-bit"""
    popcount_uint64(args)
    return True


def popcount_cpp_uint8(args):
    """C++ implementation 8-bit using pybind11"""
    popcount(args.array_uint8)
    return True


def popcount_cpp_uint64(args):
    """C++ implementation 64-bit using pybind11"""
    popcount(args.array_uint64)
    return True


def popcount_cpp_uint8_boost(args):
    """C++ implementation 8-bit using Boost.Numpy"""
    popcount_boost(args.array_uint8)
    return True


def popcount_cpp_uint64_boost(args):
    """C++ implementation 64-bit using Boost.Numpy"""
    popcount_boost(args.array_uint64)
    return True


def test_popcount_py_uint8(benchmark):
    """Measure time of the Python implementation 8-bit"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_py_uint8, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_py_uint64(benchmark):
    """Measure time of the Python implementation 64-bit"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_py_uint64, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_cpp_uint8(benchmark):
    """Measure time of the C++ implementation 8-bit using pybind11"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_cpp_uint8, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_cpp_uint64(benchmark):
    """Measure time of the implementation 64-bit using pybind11"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_cpp_uint64, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_cpp_uint8_boost(benchmark):
    """Measure time of the C++ implementation 8-bit using Boost.Python"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_cpp_uint8_boost,
                                  kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_cpp_uint64_boost(benchmark):
    """Measure time of the C++ implementation 64-bit using Boost.Python"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_cpp_uint64_boost,
                                  kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_16():
    """16-bit integers"""
    args = setup_table(256)
    py_result = popcount_uint64(args)
    cpp_result = popcount(args.array_uint64)
    assert np.all(py_result == cpp_result)


def test_popcount_64():
    """64-bit integers"""
    base_args = setup_table(256)
    array_uint64 = np.array([0x3c3c3c3cc3c3c3c3, 0xc3c3c3c33c3c3c3c],
                            dtype=np.uint64)
    args = ArgSet(
        base_args.popcount_table, base_args.array_uint8, array_uint64
    )
    py_result = popcount_uint64(args)
    cpp_result = popcount(array_uint64)
    assert np.all(py_result == cpp_result)


@pytest.mark.parametrize("target_func, dtype", POPCOUNT_ARG_SET)
def test_invalid_dimension(target_func, dtype):
    """Not a 1-D array"""
    with pytest.raises(ValueError, match=EXPECTED_ERROR_COMMON_MSG):
        arg = np.array([[1, 2], [3, 4]], dtype=dtype)
        target_func(arg)


@pytest.mark.parametrize("target_func, dtype", POPCOUNT_ARG_SET)
def test_empty_array(target_func, dtype):
    """Empty"""
    arg = np.array([], dtype=dtype)
    actual = target_func(arg)
    assert isinstance(actual, np.ndarray)
    assert actual.shape == (0,)


def test_named_arguments():
    """Named Arguments"""
    expected = np.array([1], dtype=np.uint8)
    arg = np.array([2], dtype=np.uint8)
    assert np.all(popcount(xs=arg) == expected)
    assert np.all(popcount_boost(xs=arg) == expected)


def test_invalid_type():
    """Not an np.ndarray"""
    with pytest.raises(RuntimeError, match=EXPECTED_ERROR_SCALAR_MSG):
        popcount(1)

    with pytest.raises(ValueError, match=EXPECTED_ERROR_COMMON_MSG):
        popcount_boost(1)

    with pytest.raises(TypeError):
        # pylint: disable = no-value-for-parameter
        popcount()

    with pytest.raises(TypeError):
        # pylint: disable = no-value-for-parameter
        popcount_boost()


def test_convertible_element_type():
    """Not a uint8 or uint64 array"""
    # List
    values = [1, 3, 7]
    expected = np.array([1, 2, 3], dtype=np.uint8)
    assert np.all(popcount(values) == expected)

    # Promotion
    values = [0x7fff, 0x8000, 0xffff]
    expected = np.array([15, 1, 16], dtype=np.uint8)
    arg_np = np.array(values, dtype=np.uint16)
    assert np.all(popcount(arg_np) == expected)
    assert np.all(popcount(values) == expected)

    values = [0x7fffffff, 0x80000000, 0xffffffff]
    expected = np.array([31, 1, 32], dtype=np.uint8)
    arg_np = np.array(values, dtype=np.uint32)
    assert np.all(popcount(arg_np) == expected)
    assert np.all(popcount(values) == expected)

    # Lossy conversion before calling popcount
    values = [0x7ffffffc, 0x80000003, 0xfffffffe]
    expected8 = np.array([6, 2, 7], dtype=np.uint8)
    expected32 = np.array([29, 3, 31], dtype=np.uint8)
    arg_np = np.array(values, dtype=np.uint8)
    assert np.all(popcount(arg_np) == expected8)
    assert np.all(popcount(values) == expected32)


def test_not_convertible_element_type():
    """Not a uint8 or uint64 array"""
    with pytest.raises(TypeError):
        arg = [1, "str"]
        popcount(arg)


def test_invalid_element_type():
    """Not a uint8 or uint64 array"""
    with pytest.raises(ValueError, match=EXPECTED_ERROR_COMMON_MSG):
        arg = np.array([1, 2], dtype=np.uint16)
        popcount_boost(arg)


def test_negative_integer_type():
    """Negative integers"""
    arg8 = np.array([-1, -2], dtype=np.int8)
    arg16 = np.array([-4, -8], dtype=np.int16)
    arg32 = np.array([-16, -32], dtype=np.int32)
    expected8 = np.array([64, 63], dtype=np.uint8)
    expected16 = np.array([62, 61], dtype=np.uint8)
    expected32 = np.array([60, 59], dtype=np.uint8)
    assert np.all(popcount(arg8) == expected8)
    assert np.all(popcount(arg16) == expected16)
    assert np.all(popcount(arg32) == expected32)

    with pytest.raises(ValueError, match=EXPECTED_ERROR_COMMON_MSG):
        popcount_boost(arg8)

    with pytest.raises(ValueError, match=EXPECTED_ERROR_COMMON_MSG):
        popcount_boost(arg16)

    with pytest.raises(ValueError, match=EXPECTED_ERROR_COMMON_MSG):
        popcount_boost(arg32)


@pytest.mark.parametrize("target_func", POPCOUNT_SET)
def test_some_values_uint8(target_func):
    """Some uint8 values"""
    arg = np.array([0, 1, 2, 3, 6, 7, 254, 255], dtype=np.uint8)
    expected = np.array([0, 1, 1, 2, 2, 3, 7, 8], dtype=np.uint8)
    actual = target_func(arg)
    assert isinstance(actual, np.ndarray)
    assert isinstance(actual[0], np.uint8)
    assert np.all(actual == expected)


@pytest.mark.parametrize("target_func", POPCOUNT_SET)
def test_some_values_uint64(target_func):
    """Some uint64 values"""
    arg = np.array([0, 1, 0xfe, 0xff, 0x100, 0x101, 0xfffe, 0xffff,
                    0x10000, 0x10001, 0xfffffffe, 0xffffffff,
                    0x100000000, 0x100000001,
                    0x3c3c3c3c3c3c3c3c, 0xc3c3c3c3c3c3c3c3,
                    0xffffffffffffffff], dtype=np.uint64)
    expected = np.array(
        [0, 1, 7, 8, 1, 2, 15, 16, 1, 2, 31, 32, 1, 2, 32, 32, 64],
        dtype=np.uint8)
    actual = target_func(arg)
    assert isinstance(actual, np.ndarray)
    assert isinstance(actual[0], np.uint8)
    assert np.all(actual == expected)


@pytest.mark.parametrize("target_func", POPCOUNT_SET)
def test_full_values_uint8(target_func):
    """Full values 0..255"""
    set_size = 1024
    arg_values = list(range(256))
    expected_values = [popcount_local(x) for x in arg_values]

    arg = np.repeat(np.array(arg_values, dtype=np.uint8), set_size)
    expected = np.repeat(
        np.array(expected_values, dtype=np.uint8), set_size
    )
    actual = target_func(arg)
    assert np.all(actual == expected)


@pytest.mark.parametrize("target_func", POPCOUNT_SET)
def test_64bits_uint(target_func):
    """Full bits of uint64 values"""
    value = 0
    mask = 1
    for count in range(65):
        arg = np.array([value], dtype=np.uint64)
        expected = np.array([count], dtype=np.uint8)
        actual = target_func(arg)
        assert np.all(actual == expected)
        value |= mask
        if count < 64:
            mask <<= 1
