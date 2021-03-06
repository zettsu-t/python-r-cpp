are_equal_with_nas <- function(x, y) {
  (NROW(x) == NROW(y)) && all(purrr::map2_lgl(x, y, `%in%`))
}

test_that("Empty", {
  arg_raw <- raw()
  arg_integer <- integer()
  expect_true(is.raw(arg_raw))
  expect_true(is.integer(arg_integer))

  expect_equal(NROW(rCppSample::popcount(arg_raw)), 0)
  expect_equal(NROW(rCppSample::popcount(arg_integer)), 0)
  expect_equal(NROW(rCppSample::popcount(c())), 0)
})

test_that("No arguments", {
  expect_error(rCppSample::popcount())
})

test_that("Named arguments", {
  expected <- 1
  actual <- rCppSample::popcount(xs = c(2))
  expect_equal(NROW(actual), NROW(expected))
  expect_true(all(actual == expected))
})

test_that("NULL and other non-integers", {
  expect_true(is.null(rCppSample::popcount(NULL)))
  expect_true(is.na(rCppSample::popcount(NaN)))
  expect_true(is.na(suppressWarnings(rCppSample::popcount(Inf))))
  expect_true(is.na(suppressWarnings(rCppSample::popcount(-Inf))))
})

test_that("NAs", {
  integer_na_set <- as.integer(c(2, NA, 14, NA, 62))
  expected_na_set <- as.integer(c(1, NA, 3, NA, 5))
  expect_true(are_equal_with_nas(
    rCppSample::popcount(integer_na_set), expected_na_set
  ))

  integer_na_set <- c(NaN, 3, Inf, 15)
  expected_na_set <- c(NA, 2, NA, 4)
  actual <- suppressWarnings(rCppSample::popcount(integer_na_set))
  expect_true(are_equal_with_nas(actual, expected_na_set))
})

test_that("popcount_full_raw_value", {
  popcount_local <- function(xs) {
    purrr::map_dbl(xs, function(x) {
      sum(as.numeric(intToBits(as.raw(x))))
    })
  }

  arg_values <- as.raw(0:255)
  expected_values <- popcount_local(arg_values)
  set_size <- 1000
  arg <- rep(arg_values, set_size)
  expected <- rep(expected_values, set_size)

  actual <- rCppSample::popcount(arg)
  expect_equal(NROW(actual), NROW(expected))
  expect_true(all(actual == expected))
})

test_that("popcount_non_negative_integer_value", {
  arg_values <- as.integer(c(
    0, 1, 0xfe, 0xff, 0x100, 0xffff, 0x10000,
    0x7ffffffe, 0x7fffffff, 0x70f0f0f0, 0xf0f0f0f
  ))
  expected_values <- as.integer(c(0, 1, 7, 8, 1, 16, 1, 30, 31, 15, 16))
  set_size <- 1000
  arg <- rep(arg_values, set_size)
  expected <- rep(expected_values, set_size)

  actual <- rCppSample::popcount(arg)
  expect_equal(NROW(actual), NROW(expected))
  expect_true(all(actual == expected))
})

test_that("popcount_negative_integer_value", {
  arg <- as.integer(c(-1, -2, -15790321, -252645136, -2147483647))
  expected <- as.integer(c(32, 31, 20, 16, 2))
  actual <- rCppSample::popcount(arg)
  expect_equal(NROW(actual), NROW(expected))
  expect_true(all(actual == expected))
})

test_that("Floating numbers", {
  arg <- c(7.1, 7.9, 8.0, 1e+50, -7.1, -7.9, -8.0, -1e+50)
  expected <- c(3, 3, 1, NA, 30, 30, 29, NA)
  actual <- suppressWarnings(rCppSample::popcount(arg))
  expect_true(are_equal_with_nas(actual, expected))
})
