# A package with Python and C++ extensions

## Launch a development environment

```bash
docker-compose up -d
docker-compose ps
docker exec -it env-python_cpp-1 /bin/bash
```

## Build

```bash
## Change the working directory to /path/to/project
cd /home/pydev/work/py_cpp_sample
python3.8 setup.py bdist_wheel
pip3 install --force --user dist/py_cpp_sample-0.0.1-cp38-cp38-linux_x86_64.whl
```

## Install on a runtime environment and Run

```bash
docker run -v /path/to/wheel:/root/work -d python_cpp_runtime
docker ps
docker exec -it container-id /bin/bash
```

```bash
cd /root/work/
pip3 install py_cpp_sample-0.0.1-cp38-cp38-linux_x86_64.whl
python3.8
```

```python
from py_cpp_sample import popcount
import numpy as np
a = np.array([2,255], dtype=np.uint8)
popcount(a)
a = np.array([0xf0f0f0f0f0f0f0f0], dtype=np.uint64)
popcount(a)
```

## Testing

### Python code

```bash
pip3 uninstall -y py_cpp_sample
pip3 install --force --user dist/py_cpp_sample-0.0.1-cp38-cp38-linux_x86_64.whl
pytest tests
flake8 --exclude tests/build src tests setup.py
pylint src tests setup.py
mypy src
```

Note that the editable mode requires write permission to /usr/lib/python3.8/site-packages.

```bash
python3.8 setup.py bdist_wheel
pip3 install --force --user -e .
export PYTHONPATH=src; pytest --cov=src --cov=tests --cov-report=html tests; unset PYTHONPATH
coverage report -m
coverage html
```

Using pipenv is a good practice and it does not require write permission to /usr/lib/python3.8/site-packages.

```bash
pipenv install
pipenv shell
python3.8 setup.py bdist_wheel
pip3 install --force -e .
export PYTHONPATH=src; pytest --cov=src --cov=tests --cov-report=html tests; unset PYTHONPATH
pipenv lock
exit
```

We can format Python code pretty with autopep8.

```
find . -maxdepth 3 -name "*.py" -print0 | xargs --null -I '{}' sh -c 'autopep8 "$1" > "$1".new' -- '{}'
find . -maxdepth 3 -name "*.py" -print0 | xargs --null -I '{}' sh -c 'diff --unified=0 "$1" "$1".new' -- '{}'
```

### C++ code

```bash
rm -rf tests/build
mkdir -p tests/build
cd tests/build
cmake ..
make VERBOSE=1
make test
cd CMakeFiles/test_popcount.dir
lcov -d . -c -o coverage.info
lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info
genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
cd ../../../..
```

We can use clang++ and AddressSanitizer in debugging.

```
mkdir -p tests/build
cp tests/ClangOverrides.txt tests/build/
cd tests/build
CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_USER_MAKE_RULES_OVERRIDE=ClangOverrides.txt ..
make VERBOSE=1
make test
```

We can format C++ code pretty with clang-format.

```
find . -maxdepth 3 \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs --null -I '{}' sh -c 'clang-format -style="{IndentWidth: 4}" "$1" > "$1".new' -- '{}'
find . -maxdepth 3 \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs --null -I '{}' sh -c 'diff --unified=0 "$1" "$1".new' -- '{}'
```

We can use clang-tidy to improve C++ code. Note that we have to run the command below after installing Google Test.

```
clang-tidy src/cpp_impl/*.cpp src/cpp_impl_boost/*.cpp tests/*.cpp -checks=perf\* -- -I src/cpp_impl -I src/cpp_impl_boost -I /opt/boost/include -I $(python -m sysconfig | egrep "\\bINCLUDEPY" | awk '{print $3}' | sed -e 's/"//g') -I tests/build/googletest-src/googletest/include
```

## Make documents

### Python code

```bash
sphinx-quickstart -q -p py_cpp_sample -a "Zettsu Tatsuya"
```

Apply patches

```bash
patch < patch/conf.py.diff
patch < patch/index.rst.diff
```

and run `make`.

```bash
make html
```

### C++ code

```bash
mkdir -p docs
cd docs
doxygen -g
```

Apply a patch

```bash
patch < ../patch/Doxyfile.diff
```

and run doxygen.

```bash
doxygen
cd ..
```

## Benchmarking

In this package, pytest measures performance of Python and C++ code.

```python
pytest tests
```

If the target is x86-64, make sure `extra_compile_args` in setup.py includes `-msse4.2` that lets compilers use the popcnt instruction.

|Name (time in us)|Median|
|:------------------------|:-------------------------------|
|test_popcount_cpp_uint8|817.8250 (1.0)|
|test_popcount_cpp_uint8_boost|1,026.9750 (1.26)|
|test_popcount_cpp_uint64|1,307.8000 (1.60)|
|test_popcount_cpp_uint64_boost|1,474.6000 (1.80)|
|test_popcount_py_uint8|4,625.5250 (5.66)|
|test_popcount_py_uint64|96,573.4250 (118.09)|
