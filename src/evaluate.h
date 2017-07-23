#include <Rcpp.h>

#ifndef deflex_evaluate_h
#define deflex_evaluate_h

/**
 * Evaluates fcall on par within env.
 *
 * @param par Parameter vector to fcall.
 * @param fcall R function to evaluate.
 * @param env Environment to evaluate within.
 * @return Value the evaluation yields
 */
double evaluate (SEXP par, SEXP fcall, SEXP env) {
  SEXP sexp_fvec, fn;
  double f_result;

  PROTECT(fn = Rf_lang2(fcall, par));
  PROTECT(sexp_fvec = Rf_eval(fn, env));
  f_result = REAL(sexp_fvec)[0];

  if(ISNAN(f_result)) {
    Rf_error("NaN value of objective function! \nPerhaps adjust the bounds.");
  }

  UNPROTECT(2);
  return(f_result);
}

/**
 * An auxiliary function to evaluate the objective score over the
 * provided candidate.
 *
 * @param objective The objective function.
 * @param candidate The candidate.
 * @return The objective score.
 */
inline double evaluateObjective (const Rcpp::NumericVector candidate,
                                 const Rcpp::Function objective,
                                 const Rcpp::NumericVector lower,
                                 const Rcpp::NumericVector upper) {
  // Iterate over candidate elements and make sure that we are between
  // boundaries:
  for (int i = 0; i < candidate.size(); i++) {
    // Get the element:
    const double element = candidate[i];

    // Check the element against lower and upper boundaries:
    if (element < lower[i] || element > upper[i]) {
      return std::numeric_limits<double>::infinity();
    }
  }

  // Done, return the objective function score:
  return Rcpp::as<double>(objective(candidate));
}

/**
 * An auxiliary function to evaluate the objective score over
 * population.
 *
 * @param objective  The objective function.
 * @param population The population.
 * @return The objective scores.
 */
inline Rcpp::NumericVector evaluateObjectives (const Rcpp::NumericMatrix population,
                                               const Rcpp::Function objective,
                                               const Rcpp::NumericVector lower,
                                               const Rcpp::NumericVector upper) {
  // Create a vector of scores:
  Rcpp::NumericVector scores(population.nrow());

  // Iterate over the population and calculate scores:
  for (int i = 0; i < population.nrow(); i++) {
    scores[i] = evaluateObjective(population.row(i), objective, lower, upper);
  }

  // Done, return scores:
  return scores;
}

#endif
