
# A package with R and C++ extensions

## Quickstart

1.  Launch RStudio Server and login /r-studio-server.example.com:8787/
2.  Open rCppSample.Rproj
3.  Select the Build tab and click the Install and Restart button
4.  Select the Check button

## Install and run manually

### Build and install on a shell

``` bash
cd ~/work
R CMD build rCppSample
R CMD INSTALL rCppSample_0.0.0.9000.tar.gz
```

### Install on R

``` r
remotes::install_local("rCppSample_0.0.0.9000.tar.gz", dependencies = "Imports")
```

### Run

``` r
library(rCppSample)
rCppSample::popcount(as.raw(c(2, 255)))
rCppSample::popcount(c(1023, 1024, 1025))
```

## Testing

### R code

``` r
library(rCppSample)
devtools::test()
coverage_report <- covr::package_coverage()
print(coverage_report)
covr::report(coverage_report, "./coverage_report.html")
lintr::lint_package()
```

### C++ code

``` bash
mkdir -p tests/build
cd tests/build
cmake ..
make
make test
cd ~/work/rCppSample/tests/build/CMakeFiles/test_popcount.dir
lcov -d . -c -o coverage.info
lcov -r coverage.info "/usr/*" "*/googletest/*" "/opt/boost*" -o coverageFiltered.info
genhtml -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
mkdir -p docs
cd docs
```

## Make documents

``` r
devtools::document()
rmarkdown::render("README.Rmd")
```

## Benchmarking

If the target is x86-64, make sure Makevars includes
`PKG_CXXFLAGS=-msse4.2` that lets compilers use the popcnt instruction.

<img src="man/figures/README-draw_benchmark-1.png" width="80%" />
<table>
<thead>
<tr>
<th style="text-align:left;">
method
</th>
<th style="text-align:left;">
median
</th>
<th style="text-align:left;">
ratio
</th>
</tr>
</thead>
<tbody>
<tr>
<td style="text-align:left;">
rCppSample::popcount(raw_set)
</td>
<td style="text-align:left;">
2.42805
</td>
<td style="text-align:left;">
1.00000
</td>
</tr>
<tr>
<td style="text-align:left;">
rCppSample::popcount(integer_set)
</td>
<td style="text-align:left;">
2.58995
</td>
<td style="text-align:left;">
1.06668
</td>
</tr>
<tr>
<td style="text-align:left;">
popcount_raw_r(raw_set)
</td>
<td style="text-align:left;">
16.28890
</td>
<td style="text-align:left;">
6.70863
</td>
</tr>
<tr>
<td style="text-align:left;">
popcount_integer_r(integer_set)
</td>
<td style="text-align:left;">
173.58930
</td>
<td style="text-align:left;">
71.49330
</td>
</tr>
</tbody>
</table>
