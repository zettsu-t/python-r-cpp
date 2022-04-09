#' Count 1's in each element
#'
#' @param xs A raw or integer vector to count populations
#' @return The populations of elements in the vector
#'
#' @export
#' @useDynLib rCppSample, .registration=TRUE
#' @importFrom Rcpp sourceCpp
popcount <- function(xs) {
  if (is.null(xs)) {
    return(NULL)
  }

  if (is.raw(xs)) {
    return(popcount_cpp_raw(xs))
  }

  ## Prevent crashing in calling rCppSample:::popcount_cpp_integer("str")
  return(popcount_cpp_integer(as.integer(xs)))
}
