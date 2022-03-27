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
