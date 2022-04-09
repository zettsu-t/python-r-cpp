FROM rocker/tidyverse
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential cmake curl doxygen git git-core gnupg2 graphviz language-pack-ja lcov less make pandoc pandoc-citeproc patch python3-dev python3-numpy python3-sphinx software-properties-common unzip wget

RUN curl -O https://bootstrap.pypa.io/get-pip.py
RUN python3.8 get-pip.py
RUN pip install autopep8 check-manifest coverage find_libpython flake8 mypy numpy pep8 pipenv pybind11 pybind11-global pylint pytest pytest-benchmark pytest-cov py_pkg sphinx sphinx_rtd_theme types-requests

ENV BOOST_VERSION="boost_1_78_0"
RUN wget "https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/${BOOST_VERSION}.tar.gz"
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
RUN apt install -y libxt-dev
## R and required Linux packages if this Docker images is not based on rocker/tidyverse
# RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9
# RUN add-apt-repository "deb https://cloud.r-project.org/bin/linux/ubuntu focal-cran40/"
# RUN apt install -y r-base libcurl4-openssl-dev libgit2-dev libicu-dev libssl-dev libxml2-dev libxt-dev

## Set explicitly
ENV R_HOME="/usr/local/lib/R"

## R packages
RUN Rscript -e 'install.packages(c("remotes", "devtools"))'
RUN mkdir -p /usr/local/lib/R/etc
RUN echo "options(repos = c(CRAN = 'https://cran.rstudio.com/'), download.file.method = 'libcurl', Ncpus = 4)" >> /usr/local/lib/R/etc/Rprofile.site
RUN Rscript -e 'install.packages(c("Rcpp"))'
RUN Rscript -e 'install.packages(c("testthat", "spelling", "covr", "devtools", "DT", "microbenchmark", "dplyr", "ggplot2", "purrr", "lintr", "styler", "knitr", "markdown", "rmarkdown", "kableExtra"))'
RUN Rscript -e 'remotes::install_github("wch/extrafont")'
RUN Rscript -e 'remotes::install_github("hrbrmstr/cloc")'
RUN Rscript -e 'remotes::install_version("Rttf2pt1", version = "1.3.8")'

## Copy projects
ARG PROJECTS_TOP_DIR=/root/work
ARG PYTHON_PROJECT_DIR="${PROJECTS_TOP_DIR}/python_proj/py_cpp_sample"
ARG R_PROJECT_DIR="${PROJECTS_TOP_DIR}/r_proj/rCppSample"

RUN mkdir -p "${PYTHON_PROJECT_DIR}"
RUN mkdir -p "${R_PROJECT_DIR}"
COPY python_proj/ "${PROJECTS_TOP_DIR}/python_proj/"
COPY r_proj/ "${PROJECTS_TOP_DIR}/r_proj/"

## cloc
RUN Rscript -e "cloc::cloc('${PYTHON_PROJECT_DIR}')"
RUN Rscript -e "cloc::cloc('${R_PROJECT_DIR}')"

## Testing an R package
WORKDIR "${PROJECTS_TOP_DIR}/r_proj"
RUN R CMD build rCppSample
RUN R CMD INSTALL rCppSample_0.0.0.9000.tar.gz
WORKDIR "${R_PROJECT_DIR}"
RUN Rscript "${R_PROJECT_DIR}/../tests/r_tests.R"
RUN find src -name "*.so" | xargs objdump -d -M intel | grep popcnt

RUN mkdir -p "${R_PROJECT_DIR}/tests/build"
WORKDIR "${R_PROJECT_DIR}/tests/build"
RUN cmake ..
RUN make
RUN make test
WORKDIR "${R_PROJECT_DIR}/tests/build/CMakeFiles/test_popcount.dir"
RUN lcov -d . -c -o coverage.info
RUN lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info
RUN genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
WORKDIR "${R_PROJECT_DIR}"

## Testing a Python package
WORKDIR "${PYTHON_PROJECT_DIR}"
RUN python3.8 setup.py bdist_wheel
RUN pip3 install --force --user dist/py_cpp_sample-0.0.1-cp38-cp38-linux_x86_64.whl
RUN pytest tests
RUN find build -name "*.so" | xargs objdump -d -M intel | grep popcnt
RUN flake8 --exclude tests/build src tests
RUN pylint src tests
RUN mypy src

RUN pip3 install --force --user -e .
RUN export PYTHONPATH=src; pytest --cov=src --cov=tests --cov-report=html tests; unset PYTHONPATH
RUN coverage report -m
RUN coverage html

RUN mkdir -p "${PYTHON_PROJECT_DIR}/tests/build"
WORKDIR "${PYTHON_PROJECT_DIR}/tests/build"
RUN cmake ..
RUN make
RUN make test
WORKDIR "${PYTHON_PROJECT_DIR}/tests/build/CMakeFiles/test_popcount.dir"
RUN lcov -d . -c -o coverage.info
RUN lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info
RUN genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
WORKDIR "${PYTHON_PROJECT_DIR}"

RUN sphinx-quickstart -q -p py_cpp_sample -a "Author's name"
RUN patch < patch/conf.py.diff
RUN patch < patch/index.rst.diff
RUN export PYTHONPATH=src; make html; unset PYTHONPATH

RUN mkdir -p docs
WORKDIR "${PYTHON_PROJECT_DIR}/docs"
RUN doxygen -g

RUN patch < ../patch/Doxyfile.diff
RUN doxygen
WORKDIR  ..

## Wait to login for debugging
## CMD tail -f /dev/null
