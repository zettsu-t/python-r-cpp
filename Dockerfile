FROM rocker/tidyverse
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential clang clang-format clang-tidy cmake curl doxygen git git-core gnupg2 graphviz language-pack-ja lcov less make pandoc pandoc-citeproc patch python3-dev python3-numpy python3-sphinx software-properties-common unifdef unzip wget

RUN curl -O https://bootstrap.pypa.io/get-pip.py && python3.8 get-pip.py && pip install autopep8 check-manifest coverage find_libpython flake8 mypy numpy pep8 pipenv pybind11 pybind11-global pylint pytest pytest-benchmark pytest-cov py_pkg sphinx sphinx_rtd_theme types-requests

ENV BOOST_VERSION="boost_1_79_0"
RUN wget "https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/${BOOST_VERSION}.tar.gz" && tar xzf "${BOOST_VERSION}.tar.gz"
RUN cd "${BOOST_VERSION}" && ./bootstrap.sh --with-libraries=python --with-python=python3.8 && ./b2 install cxxflags=-fPIC address-model=64 threading=multi link=static,shared runtime-link=static variant=release -j4 "--prefix=/opt/${BOOST_VERSION}" && { test -d /opt/boost || ln -s "/opt/${BOOST_VERSION}" /opt/boost ; }

RUN { test -f "/opt/${BOOST_VERSION}/lib/libboost_python.a" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_python38.a" "/opt/${BOOST_VERSION}/lib/libboost_python.a" ; } && { test -f "/opt/${BOOST_VERSION}/lib/libboost_numpy.a" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_numpy38.a" "/opt/${BOOST_VERSION}/lib/libboost_numpy.a" ; } && { test -f "/opt/${BOOST_VERSION}/lib/libboost_python.so" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_python38.so" "/opt/${BOOST_VERSION}/lib/libboost_python.so" ; } && { test -f "/opt/${BOOST_VERSION}/lib/libboost_numpy.so" || ln -s "/opt/${BOOST_VERSION}/lib/libboost_numpy38.so" "/opt/${BOOST_VERSION}/lib/libboost_numpy.so" ; }

## Make Python packages editable
RUN mkdir -p /usr/lib/python3.8/site-packages && chmod ugo+w /usr/lib/python3.8/site-packages && apt install -y libxt-dev

## Set explicitly
ENV R_HOME="/usr/local/lib/R"

## R packages
RUN Rscript -e 'install.packages(c("remotes", "devtools", "rlang", "stringr"))' && mkdir -p /usr/local/lib/R/etc
RUN echo "options(repos = c(CRAN = 'https://cran.rstudio.com/'), download.file.method = 'libcurl', Ncpus = 4)" >> /usr/local/lib/R/etc/Rprofile.site
RUN Rscript -e 'install.packages(c("Rcpp", "testthat", "spelling", "xml2", "covr", "devtools", "DT", "microbenchmark", "dplyr", "ggplot2", "purrr", "lintr", "styler", "knitr", "markdown", "rmarkdown", "kableExtra"))' && Rscript -e 'remotes::install_github("wch/extrafont");remotes::install_github("hrbrmstr/cloc");remotes::install_version("Rttf2pt1", version = "1.3.8")'

## Copy projects
ARG PROJECTS_TOP_DIR=/root/work
ARG PYTHON_PROJECT_DIR="${PROJECTS_TOP_DIR}/python_proj/py_cpp_sample"
ARG R_PROJECT_DIR="${PROJECTS_TOP_DIR}/r_proj/rCppSample"

RUN mkdir -p "${PYTHON_PROJECT_DIR}" && mkdir -p "${R_PROJECT_DIR}"
COPY python_proj/ "${PROJECTS_TOP_DIR}/python_proj/"
COPY r_proj/ "${PROJECTS_TOP_DIR}/r_proj/"

## Undef macros
RUN { find . -maxdepth 2 \( -name "*.cpp" -o -name "*.h" \) ! -name RcppExports.cpp -print0 | xargs --null -I '{}' sh -c 'unifdef -U UNIT_TEST_CPP "$1" > "$1".undef' -- '{}' ; } && { find . -maxdepth 2 \( -name "*.cpp" -o -name "*.h" \) ! -name RcppExports.cpp -print0 | xargs --null -I '{}' sh -c 'cp "$1".undef "$1"' -- '{}' ; }

## cloc
RUN rm -rf "${R_PROJECT_DIR}/tests/build" && rm -rf "${PYTHON_PROJECT_DIR}/tests/build" && Rscript -e "cloc::cloc('${PYTHON_PROJECT_DIR}')" && Rscript -e "cloc::cloc('${R_PROJECT_DIR}')"
WORKDIR "${PROJECTS_TOP_DIR}"
RUN Rscript "${PROJECTS_TOP_DIR}/r_proj/tests/measure_cloc.R"

## Testing a Python package
WORKDIR "${PYTHON_PROJECT_DIR}"
RUN python3.8 setup.py bdist_wheel && pip3 install --force --user dist/py_cpp_sample-0.0.1-cp38-cp38-linux_x86_64.whl && pytest tests && { find build -name "*.so" | xargs objdump -d -M intel | grep -i popcnt ; } && flake8 --exclude tests/build src tests setup.py && pylint src tests setup.py && mypy src

RUN pip3 install --force --user -e . && { PYTHONPATH=src; pytest --cov=src --cov=tests --cov-report=html tests; unset PYTHONPATH ; } && coverage report -m && coverage html

RUN rm -rf "${PYTHON_PROJECT_DIR}/tests/build" && mkdir -p "${PYTHON_PROJECT_DIR}/tests/build"
WORKDIR "${PYTHON_PROJECT_DIR}/tests/build"
RUN cmake .. && make && make test

WORKDIR "${PYTHON_PROJECT_DIR}/tests/build/CMakeFiles/test_popcount.dir"
RUN lcov -d . -c -o coverage.info && lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info && genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info

WORKDIR "${PYTHON_PROJECT_DIR}"
RUN rm -rf "${PYTHON_PROJECT_DIR}/tests/build" && mkdir -p "${PYTHON_PROJECT_DIR}/tests/build"
WORKDIR "${PYTHON_PROJECT_DIR}/tests/build"
RUN { cp "${PYTHON_PROJECT_DIR}/tests/ClangOverrides.txt" "${PYTHON_PROJECT_DIR}/tests/build/" ; } && { CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_USER_MAKE_RULES_OVERRIDE=ClangOverrides.txt .. ; } && make VERBOSE=1 && make test || ./test_popcount || exit 0

WORKDIR "${PYTHON_PROJECT_DIR}"
RUN rm -rf "${PYTHON_PROJECT_DIR}/tests/build" && mkdir -p "${PYTHON_PROJECT_DIR}/tests/build"
WORKDIR "${PYTHON_PROJECT_DIR}/tests/build"
RUN sed -i -e "s/sanitize=[a-z,]*/sanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls/" "${PYTHON_PROJECT_DIR}/tests/ClangOverrides.txt" && cp "${PYTHON_PROJECT_DIR}/tests/ClangOverrides.txt" "${PYTHON_PROJECT_DIR}/tests/build/" && { CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_USER_MAKE_RULES_OVERRIDE=ClangOverrides.txt .. ; } && make VERBOSE=1 && make test || ./test_popcount || exit 0

WORKDIR "${PYTHON_PROJECT_DIR}"
RUN clang-tidy src/cpp_impl/*.cpp src/cpp_impl_boost/*.cpp tests/*.cpp -checks=perf\* -- -I src/cpp_impl -I src/cpp_impl_boost -I /opt/boost/include -I $(python3.8 -m sysconfig | egrep "\\bINCLUDEPY" | awk '{print $3}' | sed -e 's/"//g') -I tests/build/googletest-src/googletest/include || echo "Non-zero exit code"
RUN rm -rf _build/ _static/ _templates/ conf.py index.rst make.bat Makefile; sphinx-quickstart -q -p py_cpp_sample -a "Author's name" && { patch < patch/conf.py.diff ; } && { patch < patch/index.rst.diff ; } && make html

RUN mkdir -p docs
WORKDIR "${PYTHON_PROJECT_DIR}/docs"
RUN doxygen -g

RUN patch < ../patch/Doxyfile.diff
RUN doxygen
WORKDIR  "${PROJECTS_TOP_DIR}"

## Testing an R package
WORKDIR "${PROJECTS_TOP_DIR}/r_proj"
RUN R CMD build rCppSample && R CMD INSTALL rCppSample_0.0.0.9000.tar.gz
WORKDIR "${R_PROJECT_DIR}"
RUN Rscript "${R_PROJECT_DIR}/../tests/r_tests.R" && find src -name "*.so" | xargs objdump -d -M intel | grep -i popcnt

RUN rm -rf "${R_PROJECT_DIR}/tests/build" && mkdir -p "${R_PROJECT_DIR}/tests/build"
WORKDIR "${R_PROJECT_DIR}/tests/build"
RUN cmake .. && make && make test

WORKDIR "${R_PROJECT_DIR}/tests/build/CMakeFiles/test_popcount.dir"
RUN lcov -d . -c -o coverage.info && lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info && genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info

WORKDIR "${R_PROJECT_DIR}/tests/build/CMakeFiles/test_popcount_std.dir"
RUN lcov -d . -c -o coverage.info && lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info && genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info

WORKDIR "${R_PROJECT_DIR}"
RUN rm -rf "${R_PROJECT_DIR}/tests/build" && mkdir -p "${R_PROJECT_DIR}/tests/build"
WORKDIR "${R_PROJECT_DIR}/tests/build"
RUN cp "${R_PROJECT_DIR}/tests/ClangOverrides.txt" "${R_PROJECT_DIR}/tests/build/" && { CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_USER_MAKE_RULES_OVERRIDE=ClangOverrides.txt .. ; } && make VERBOSE=1 && make test || ./test_popcount || exit 0

WORKDIR "${R_PROJECT_DIR}"
RUN rm -rf "${R_PROJECT_DIR}/tests/build" && mkdir -p "${R_PROJECT_DIR}/tests/build"
WORKDIR "${R_PROJECT_DIR}/tests/build"
RUN sed -i -e "s/sanitize=[a-z,]*/sanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls/" "${R_PROJECT_DIR}/tests/ClangOverrides.txt" && cp "${R_PROJECT_DIR}/tests/ClangOverrides.txt" "${R_PROJECT_DIR}/tests/build/" && { CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_USER_MAKE_RULES_OVERRIDE=ClangOverrides.txt .. ; } && make VERBOSE=1 && make test || ./test_popcount || exit 0

WORKDIR "${R_PROJECT_DIR}"
RUN { echo "-I $(find /usr -name R.h | head -1 | xargs dirname)" "$(Rscript -e 'cat(paste(paste0(" -I ", .libPaths(), "/Rcpp/include"), sep="", collapse=" "))')" "$(Rscript -e 'cat(paste(paste0(" -I ", .libPaths(), "/testthat/include"), sep="", collapse=" "))')" > _r_includes ; } && { clang-tidy src/*.cpp tests/*.cpp -checks=perf\* -- -I src $(cat _r_includes) -I tests/build/googletest-src/googletest/include || echo "Non-zero exit code" ; }
