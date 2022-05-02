FROM ubuntu:focal
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential cmake curl doxygen git git-core graphviz language-pack-ja lcov less make pandoc pandoc-citeproc patch python3-dev python3-numpy python3-sphinx unzip wget
RUN apt-get install -y clang clang-format clang-tidy gdb

RUN curl -O https://bootstrap.pypa.io/get-pip.py
RUN python3.8 get-pip.py
RUN pip install autopep8 check-manifest coverage find_libpython flake8 mypy numpy pep8 pipenv pybind11 pybind11-global pylint pytest pytest-benchmark pytest-cov py_pkg sphinx sphinx_rtd_theme types-requests

ENV BOOST_VERSION="boost_1_79_0"
RUN wget "https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/${BOOST_VERSION}.tar.gz"
RUN tar xzf "${BOOST_VERSION}.tar.gz"
RUN cd "${BOOST_VERSION}" && ./bootstrap.sh --with-libraries=python --with-python=python3.8 && ./b2 install cxxflags=-fPIC address-model=64 threading=multi link=static,shared runtime-link=static variant=release -j4 "--prefix=/opt/${BOOST_VERSION}"
RUN test -d /opt/boost || ln -s "/opt/${BOOST_VERSION}" /opt/boost

RUN test -f "/opt/${BOOST_VERSION}/lib/libboost_python.a" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_python38.a" "/opt/${BOOST_VERSION}/lib/libboost_python.a"
RUN test -f "/opt/${BOOST_VERSION}/lib/libboost_numpy.a" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_numpy38.a" "/opt/${BOOST_VERSION}/lib/libboost_numpy.a"
RUN test -f "/opt/${BOOST_VERSION}/lib/libboost_python.so" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_python38.so" "/opt/${BOOST_VERSION}/lib/libboost_python.so"
RUN test -f "/opt/${BOOST_VERSION}/lib/libboost_numpy.so" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_numpy38.so" "/opt/${BOOST_VERSION}/lib/libboost_numpy.so"

## Editable
RUN mkdir -p /usr/lib/python3.8/site-packages
RUN chmod ugo+w /usr/lib/python3.8/site-packages

RUN adduser pydev
USER pydev
CMD tail -f /dev/null
