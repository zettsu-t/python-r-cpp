"""
Exported function(s)
"""

import numpy as np
# Generated code
# pylint: disable=no-name-in-module, disable=import-error
from .py_cpp_sample_cpp_impl import popcount_cpp_uint8
# pylint: disable=no-name-in-module, disable=import-error
from .py_cpp_sample_cpp_impl import popcount_cpp_uint64
# pylint: disable=no-name-in-module, disable=import-error
from .py_cpp_sample_cpp_impl_boost import popcount_cpp_boost


TYPE_ERROR_MESSAGE = "xs must be a 1-D np.ndarray(np.uint8|np.uint64)"


def popcount(xs):
    """
    Count 1's of integers in a 1-D np.ndarray(np.uint8|np.uint64)

    :type xs: np.ndarray[np.uint]
    :rtype: np.ndarray[np.uint]
    :return: Returns the number of 1's of each element of xs
    """

    if isinstance(xs, np.ndarray):
        if len(xs.shape) != 1:
            raise ValueError(TYPE_ERROR_MESSAGE)

        if xs.shape[0] == 0:
            # Any element types are acceptable for empty 1-D arrays
            return np.array([], dtype=np.uint8)

        if isinstance(xs[0], (np.uint8)):
            return popcount_cpp_uint8(xs)

    # If xs is not convertible, C++ code throws an exception
    return popcount_cpp_uint64(xs)


def popcount_boost(xs):
    """
    Count 1's of integers in a 1-D np.ndarray(np.uint8|np.uint64)

    :type xs: np.ndarray[np.uint]
    :rtype: np.ndarray[np.uint]
    :return: Returns the number of 1's of each element of xs
    """

    if not isinstance(xs, np.ndarray):
        raise ValueError(TYPE_ERROR_MESSAGE)

    if len(xs.shape) != 1:
        raise ValueError(TYPE_ERROR_MESSAGE)

    if xs.shape[0] == 0:
        # Any element types are acceptable for empty 1-D arrays
        return np.array([], dtype=np.uint8)

    if not isinstance(xs[0], (np.uint8, np.uint64)):
        raise ValueError(TYPE_ERROR_MESSAGE)

    return popcount_cpp_boost(xs)
