"""
Exported function(s)
"""

import numpy as np
# Generated code
# pylint: disable=no-name-in-module, disable=import-error
from .cpp_impl import popcount_cpp


TYPE_ERROR_MESSAGE = "xs must be a 1-D np.ndarray(np.uint8)"\
                     " or np.ndarray(np.uint32)"


def popcount(xs):
    """
    Count 1's of integers in a 1-D np.ndarray(np.uint)

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

    if not isinstance(xs[0], (np.uint8, np.uint32)):
        raise ValueError(TYPE_ERROR_MESSAGE)

    return popcount_cpp(xs)
