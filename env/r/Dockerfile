FROM rocker/tidyverse
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential cmake curl git git-core language-pack-ja lcov less make pandoc pandoc-citeproc unifdef wget libxt-dev
RUN apt-get install -y clang clang-format clang-tidy gdb

RUN echo "options(repos = c(CRAN = 'https://cran.rstudio.com/'), download.file.method = 'libcurl', Ncpus = 4)" >> /usr/local/lib/R/etc/Rprofile.site
RUN Rscript -e 'install.packages(c("remotes", "devtools", "rlang", "stringr"))'
RUN Rscript -e 'install.packages(c("Rcpp", "testthat", "spelling", "xml2", "covr", "devtools", "DT", "microbenchmark", "dplyr", "ggplot2", "purrr", "lintr", "styler", "knitr", "markdown", "rmarkdown", "kableExtra"))'
RUN Rscript -e 'remotes::install_github("wch/extrafont");remotes::install_github("hrbrmstr/cloc");remotes::install_version("Rttf2pt1", version = "1.3.8")'

EXPOSE 8787
