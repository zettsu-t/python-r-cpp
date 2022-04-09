"""
A Python and C++ sample project
"""

from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension

CPU_ARCH_FLAG = '-msse4.2'

setup(
    ext_modules=[Pybind11Extension(
        'py_cpp_sample.py_cpp_sample_cpp_impl',
        sources=['src/cpp_impl/popcount.cpp',
                 'src/cpp_impl/popcount_impl.cpp'],
        extra_compile_args=[CPU_ARCH_FLAG],
    ),
        Extension(
        'py_cpp_sample.py_cpp_sample_cpp_impl_boost',
        define_macros=[('BOOST_PYTHON_STATIC_LIB', None)],
        sources=['src/cpp_impl_boost/popcount_boost.cpp',
                 'src/cpp_impl_boost/popcount_impl_boost.cpp'],
        include_dirs=['/opt/boost/include'],
        library_dirs=['/opt/boost/lib'],
        runtime_library_dirs=[],
        libraries=['boost_python', 'boost_numpy'],
        extra_compile_args=['-isystem', '/opt/boost/include', CPU_ARCH_FLAG],
    )]
)
