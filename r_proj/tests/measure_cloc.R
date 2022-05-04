library(cloc)
library(dplyr)
library(purrr)
library(rlang)
library(stringr)

# On Windows + RTools
if (Sys.info()[["sysname"]] == "Windows") {
  rtools_path <- "C:\\rtools40\\usr\\bin"
  old_path <- Sys.getenv("PATH")
  if (dir.exists(rtools_path) &&
    !grepl(rtools_path, old_path, fixed = TRUE)) {
    new_path <- paste(old_path, rtools_path, sep = ";")
    Sys.setenv(PATH = new_path)
  }
}

measure_cloc <- function(top_dirname, filenames) {
  df_all <- purrr::map(file.path(top_dirname, filenames), ~ cloc::cloc(source = .x)) %>%
    dplyr::bind_rows() %>%
    dplyr::filter(language != "SUM") %>%
    dplyr::mutate(test = stringr::str_starts(.data$source, "test")) %>%
    dplyr::mutate(lang = ifelse(language == "C/C++ Header", "C++", language)) %>%
    dplyr::relocate(test)

  df_test <- df_all %>%
    dplyr::select(all_of(c("lang", "loc", "test"))) %>%
    dplyr::group_by(lang, test) %>%
    dplyr::summarize_all(sum) %>%
    dplyr::ungroup()

  list(df_all = df_all, df_test = df_test)
}

python_filename <- c(
  "src/py_cpp_sample/__init__.py",
  "src/py_cpp_sample/main.py",
  "tests/__init__.py",
  "tests/test_main.py",
  "src/cpp_impl/popcount.h",
  "src/cpp_impl/popcount.cpp",
  "src/cpp_impl/popcount_impl.cpp",
  "src/cpp_impl_boost/popcount_boost.h",
  "src/cpp_impl_boost/popcount_boost.cpp",
  "src/cpp_impl_boost/popcount_impl_boost.cpp",
  "tests/test_popcount.h",
  "tests/test_popcount.cpp"
)

r_filename <- c(
  "R/r_cpp_sample.R",
  "tests/testthat/test-popcount.R",
  "src/popcount.h",
  "src/test_popcount.h",
  "src/popcount.cpp",
  "src/test-popcount.cpp",
  "tests/test_popcount.cpp"
)

python_loc <- measure_cloc(top_dirname = "python_proj/py_cpp_sample", filenames = python_filename)
r_loc <- measure_cloc(top_dirname = "r_proj/rCppSample", filenames = r_filename)
print(python_loc$df_all)
print(r_loc$df_all)
print(python_loc$df_test)
print(r_loc$df_test)
