#include <vector>
#include <Rcpp.h>


#ifndef deflex_utils_h
#define deflex_utils_h

/**
 * Creates a sequence of integer elements from/to provided values.
 *
 * @param  from The start of the sequence (inclusive).
 * @param  to   The end of the sequence (exclusive).
 * @return The sequence as a vector.
 */
std::vector<int> createSequence (int from, int to);


/**
 * Chooses a number of elements randomly from the given input vector.
 *
 * @param input The input vector from which elements will be chosen.
 * @param count The number of elements to be chosen.
 * @return      A vector of elements randomly chosen from the input vector.
 */
std::vector<int> chooseRandom (std::vector<int> input, int count);


/**
 * Indicates if we are going to terminate the evolution.
 *
 * @param  predicate  An R function which provides the termination
 *                    condition.
 * @param  generation The current generation count.
 * @return True if we need to terminate now, false otherwise.
 */
bool _terminate(Rcpp::Function predicate, int generation);


/**
 * Computes the difference of the vectors for the respective element.
 *
 * @param population The population.
 * @param indices    An even number of unique set of indices indicating the rows of the matrix.
 * @param element    The index of the element which the difference will be calculated for.
 * @return The difference.
 */
double vectorsDifference (Rcpp::NumericMatrix population, std::vector<int> indices, int element);


/**
 * Provides an auxiliary wrapper around R's random number generator to
 * get a uniform distribution over integers `[0, n)` to be supplied to
 * the STL algoritm for random shuffle.
 *
 * This method is taken from Rcpp gallery
 * (http://gallery.rcpp.org/articles/stl-random-shuffle/).
 *
 * @param  n The upper bound of the uniform distribution (exclusive).
 * @return An observation from Uniform[0, n)
 */
inline int _runifIntWrapper (const int n) {
  return floor(unif_rand() * n);
}


/**
 * Trims a vector for the upper and lower limits.
 *
 * @param vector The vector to be trimmed.
 * @param upper  Upper limits.
 * @param lower  Lower limits.
 * @return Trimmed vector.
 */
inline Rcpp::NumericVector trimVector (Rcpp::NumericVector vector, const Rcpp::NumericVector upper, const Rcpp::NumericVector lower) {
  // First, trim for the upper:
  vector = ifelse(vector > upper, upper, vector);

  // Now, trim for the lower:
  vector = ifelse(vector < lower, lower, vector);

  // Done, return:
  return vector;
}


/**
 * Trims a vector for the upper and lower limits by bouncing back.
 *
 * @param vector The vector to be trimmed.
 * @param upper  Upper limits.
 * @param lower  Lower limits.
 * @return Trimmed vector with bouncing back.
 */
inline Rcpp::NumericVector bounceBack (Rcpp::NumericVector vector, const Rcpp::NumericVector upper, const Rcpp::NumericVector lower) {
  // First, trim for the upper:
  vector = ifelse(vector > upper, upper - (unif_rand() * (upper - lower)), vector);

  // Now, trim for the lower:
  vector = ifelse(vector < lower, lower + (unif_rand() * (upper - lower)), vector);

  // Done, return:
  return vector;
}
#endif
