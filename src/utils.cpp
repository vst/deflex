#include <vector>
#include <Rcpp.h>
#include "utils.h"

std::vector<int> createSequence (int from, int to) {
  // Initialize the return value:
  std::vector<int> retval (to - from);

  // Iterate over the vector and populate:
  for (int i = 0; i < retval.size(); i++) {
    retval[i] = from + i;
  }

  // Done, return:
  return retval;
}


std::vector<int> chooseRandom (std::vector<int> input, int count) {
  // Get a copy of the input:
  std::vector<int> copied(input);

  // Shuffle copied:
  std::random_shuffle(copied.begin(), copied.end(), _runifIntWrapper);

  // Slice the first `count` elements:
  copied.resize(count);

  // Done, return the copied, shuffled, sliced vector:
  return copied;
}


bool _terminate(Rcpp::Function predicate, int generation) {
  // Get the value:
  const Rcpp::NumericVector pred(predicate(generation));

  // Check the return value:
  return pred.size() == 0 || pred(0);
}


double vectorsDifference (Rcpp::NumericMatrix population, std::vector<int> indices, int element) {
  // Declare the return value:
  double retval = 0;

  // Define the flag for minus op:
  bool minus = false;

  // Iterate over the indices and perform difference operation:
  for (int i = 0; i < indices.size(); i++) {
    // Get the row index:
    const int row = indices[i];

    // Act accorting to the flag:
    if (minus) {
      retval -= population(row, element);
    }
    else {
      retval += population(row, element);
    }

    // Flip the flag:
    minus = !minus;
  }

  // Done, return:
  return retval;
}
