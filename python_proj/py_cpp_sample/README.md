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
a = np.array([0xf0f0f0f0], dtype=np.uint32)
popcount(a)
```

## Testing

### Python code

```bash
pip3 uninstall -y py_cpp_sample
pip3 install --force --user dist/py_cpp_sample-0.0.1-cp38-cp38-linux_x86_64.whl
pytest tests
flake8 --exclude tests/build src tests
pylint src tests
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

### C++ code

```bash
mkdir -p tests/build
cd tests/build
cmake ..
make
make test
cd ~/work/py_cpp_sample/tests/build/CMakeFiles/test_popcount.dir
lcov -d . -c -o coverage.info
lcov -r coverage.info */googletest/* test/* */c++/* -o coverageFiltered.info
genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
cd ../../../..
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
export PYTHONPATH=src; make html; unset PYTHONPATH
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

|Name (time in ms)|Mean|Median|
|:------------------------|:----------------|:---------------|
|test_popcount_py_uint8|4.6992 (1.0)|4.6815 (1.0)|
|test_popcount_cpp_uint32|6.6576 (1.42)|6.6076 (1.41)|
|test_popcount_cpp_uint8|6.5899 (1.40)|6.5977 (1.41)|
|test_popcount_py_uint32|33.9425 (7.22)|33.8911 (7.24)|
