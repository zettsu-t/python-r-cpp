"""
Testing Python code
"""

from collections import namedtuple
import re
import unittest
import numpy as np
from py_cpp_sample import popcount


EXPECTED_ERROR_STR = "^xs must be a 1\\-D np\\.ndarray\\(np\\.uint8\\) "\
                     "or np\\.ndarray\\(np\\.uint32\\)$"
EXPECTED_ERROR_MSG = re.compile(EXPECTED_ERROR_STR)

SIZE_OF_UNIT = 256
NUMBER_OF_UNIT = 10000
ArgSet = namedtuple(
    "ArgSet",
    ["popcount_table", "array_uint8", "array_uint32"]
)


def popcount_local(num):
    """Count 1's in an integer"""
    return bin(num).count("1")


def popcount_uint32(args):
    """Count 1's in uint32 integers"""
    count0 = args.popcount_table[
        args.array_uint32 & 0xff
    ]
    count1 = args.popcount_table[
        ((args.array_uint32 & 0xff00) >> 8) & 0xff
    ]
    count2 = args.popcount_table[
        ((args.array_uint32 & 0xff0000) >> 16) & 0xff
    ]
    count3 = args.popcount_table[
        ((args.array_uint32 & 0xff000000) >> 24) & 0xff
    ]
    return count0 + count1 + count2 + count3


def setup_table(size_set):
    """Fill the 256-entries look-up table and set up arguments"""
    popcount_table = np.array(
        [popcount_local(x) for x in range(256)], dtype=np.uint8
    )
    seq_uint8 = np.arange(SIZE_OF_UNIT, dtype=np.uint8)
    arg_uint8 = np.repeat(seq_uint8, size_set)
    arg_uint32 = np.arange(SIZE_OF_UNIT * size_set, dtype=np.uint32)
    return ArgSet(popcount_table, arg_uint8, arg_uint32)


def popcount_py_uint8(args):
    """Python implementation 8-bit"""
    return args.popcount_table[args.array_uint8] is not None


def popcount_py_uint32(args):
    """Python implementation 32-bit"""
    popcount_uint32(args)
    return True


def popcount_cpp_uint8(args):
    """C++ implementation 8-bit"""
    popcount(args.array_uint8)
    return True


def popcount_cpp_uint32(args):
    """C++ implementation 32-bit"""
    popcount(args.array_uint32)
    return True


def test_popcount_py_uint8(benchmark):
    """Test Python implementation 8-bit"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_py_uint8, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_py_uint32(benchmark):
    """Test Python implementation 32-bit"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_py_uint32, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_cpp_uint8(benchmark):
    """Test C++ implementation 8-bit"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_cpp_uint8, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


def test_popcount_cpp_uint32(benchmark):
    """Test C++ implementation 32-bit"""
    args = setup_table(NUMBER_OF_UNIT)
    ret_code = benchmark.pedantic(popcount_cpp_uint32, kwargs={"args": args},
                                  iterations=2, rounds=100)
    return ret_code


class TestPopcountMethods(unittest.TestCase):
    """Compare popcount methods"""

    def test_popcount_16(self):
        """16-bit"""
        args = setup_table(256)
        py_result = popcount_uint32(args)
        cpp_result = popcount(args.array_uint32)
        self.assertTrue(np.all(py_result == cpp_result))

    def test_popcount_32(self):
        """32-bit"""
        base_args = setup_table(256)
        array_uint32 = np.array([0x0113377f, 0xfeecc880], dtype=np.uint32)
        args = ArgSet(
            base_args.popcount_table, base_args.array_uint8, array_uint32
        )
        py_result = popcount_uint32(args)
        cpp_result = popcount(array_uint32)
        self.assertTrue(np.all(py_result == cpp_result))


class TestPopcount(unittest.TestCase):
    """Testing public interface(s)"""

    def test_invalid_type(self):
        """Not an np.ndarray"""
        with self.assertRaisesRegex(ValueError, EXPECTED_ERROR_MSG) as _:
            popcount(1)

    def test_invalid_dimension(self):
        """Not a 1-D array"""
        with self.assertRaisesRegex(ValueError, EXPECTED_ERROR_MSG) as _:
            arg = np.array([[1, 2], [3, 4]], dtype=np.uint8)
            popcount(arg)

    def test_invalid_element_type(self):
        """Not a uint8 or uint32 array"""
        with self.assertRaisesRegex(ValueError, EXPECTED_ERROR_MSG) as _:
            arg = np.array([1, 2], dtype=np.uint64)
            popcount(arg)

    def test_empty_array(self):
        """Empty"""
        arg8 = np.array([], dtype=np.uint8)
        actual = popcount(arg8)
        self.assertTrue(isinstance(actual, np.ndarray))
        self.assertEqual(actual.shape, (0,))

        arg32 = np.array([], dtype=np.uint32)
        actual = popcount(arg32)
        self.assertTrue(isinstance(actual, np.ndarray))
        self.assertEqual(actual.shape, (0,))

    def test_some_values_uint8(self):
        """Some uint8 values"""
        arg = np.array([0, 1, 2, 3, 6, 7, 254, 255], dtype=np.uint8)
        expected = np.array([0, 1, 1, 2, 2, 3, 7, 8], dtype=np.uint8)
        actual = popcount(arg)
        self.assertTrue(isinstance(actual, np.ndarray))
        self.assertTrue(isinstance(actual[0], np.uint8))
        self.assertTrue(np.all(actual == expected))

    def test_some_values_uint32(self):
        """Some uint32 values"""
        arg = np.array([0, 1, 0xfe, 0xff, 0x100, 0xffff,
                        0x10000, 0xfffffffe, 0xffffffff,
                        0xf0f0f0f0, 0xf0f0f0f], dtype=np.uint32)
        expected = np.array([0, 1, 7, 8, 1, 16, 1, 31, 32, 16, 16],
                            dtype=np.uint8)
        actual = popcount(arg)
        self.assertTrue(isinstance(actual, np.ndarray))
        self.assertTrue(isinstance(actual[0], np.uint8))
        self.assertTrue(np.all(actual == expected))

    def test_full_values_uint8(self):
        """Full values 0..255"""
        set_size = 1024
        arg_values = list(range(256))
        expected_values = [popcount_local(x) for x in arg_values]

        arg = np.repeat(np.array(arg_values, dtype=np.uint8), set_size)
        expected = np.repeat(
            np.array(expected_values, dtype=np.uint8), set_size
        )
        actual = popcount(arg)
        self.assertTrue(np.all(actual == expected))


if __name__ == "__main__":
    unittest.main()
