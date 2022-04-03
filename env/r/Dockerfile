FROM rocker/tidyverse
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential cmake curl git git-core language-pack-ja lcov less make pandoc pandoc-citeproc wget libxt-dev

RUN echo "options(repos = c(CRAN = 'https://cran.rstudio.com/'), download.file.method = 'libcurl', Ncpus = 4)" >> /usr/local/lib/R/etc/Rprofile.site
RUN R -e 'install.packages(c("Rcpp"))'
RUN R -e 'install.packages(c("testthat", "spelling", "covr", "devtools", "DT", "microbenchmark", "dplyr", "ggplot2", "purrr", "lintr", "styler", "knitr", "markdown", "rmarkdown", "kableExtra"))'

EXPOSE 8787
