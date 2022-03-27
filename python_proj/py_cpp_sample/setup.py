"""
A Python and C++ sample project
"""

from setuptools import setup, Extension

setup(
    ext_modules=[Extension(
        'py_cpp_sample.cpp_impl',
        define_macros=[('BOOST_PYTHON_STATIC_LIB', None)],
        sources=['src/cpp_impl/popcount.cpp',
                 'src/cpp_impl/popcount_impl.cpp'],
        include_dirs=['/opt/boost/include'],
        library_dirs=['/opt/boost/lib'],
        runtime_library_dirs=[],
        libraries=['boost_python', 'boost_numpy'],
    )]
)
