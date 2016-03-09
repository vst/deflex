#include <vector>
#include <Rcpp.h>
#include "utils.h"


/**
 * Provides a simple DE routine.
 *
 * @param objective    The objective function.
 * @param termination  Function which decides on terminating or proceeding with the evolution.
 * @param population   Initial population.
 * @param upper        Upper boundaries.
 * @param lower        Lower boundaries.
 * @param CR           Crossover probability.
 * @param F            Weighting factor of differential variation.
 * @param x            x of DE/x/y/z
 * @param y            y of DE/x/y/z
 * @param z            z of DE/x/y/z
 * @return             Optimization result.
 */
// [[Rcpp::export]]
SEXP deflex_urgent (Rcpp::Function objective,
                    Rcpp::Function termination,
                    Rcpp::NumericMatrix population,
                    Rcpp::NumericVector upper,
                    Rcpp::NumericVector lower,
                    double CR,
                    double F,
                    int x,
                    int y,
                    int z) {

  //////////////
  // PREAMBLE //
  //////////////

  // First, get the problem dimension:
  const int dimension = upper.size();

  // Now, get the population size:
  const int popsize = population.nrow();

  // Define the number of random vectors to be chosen:
  const int randVectorCount = (y * 2) + (x == 0);

  // Create a base vector of random vector indices to be used later:
  const std::vector<int> indexVector = createSequence(0, population.nrow());

  // Initialize the scores vector:
  Rcpp::NumericVector scores = evaluateObjectives(objective, population);

  // Declare the return value:
  Rcpp::List populationsList;
  Rcpp::List scoresList;

  // Initialize the generation counter and iterate over generations as
  // long as we don't hit the termination condition:
  int generation = 0;
  populationsList.push_back(population);
  scoresList.push_back(scores);
  while (!_terminate(termination, ++generation)) {
    // Initialize the new population:
    Rcpp::NumericMatrix newPopulation(popsize, dimension);

    // Initialize the new scoreboard:
    Rcpp::NumericVector newScores(popsize);

    // Iterate over the population:
    for (int c = 0; c < popsize; c++) {
      // Pick random indices:
      std::vector<int> vectors = chooseRandom(indexVector, randVectorCount);

      // Get the element to start with:
      int j = _runifIntWrapper(dimension);

      // Create a trial vector:
      Rcpp::NumericVector trial(dimension);

      // Get the base vector and remove the last index from vectors:
      Rcpp::NumericMatrix::Row baseVector = population.row(vectors.back());
      vectors.pop_back();

      // Iterate over elements:
      for (int k = 0; k < dimension; k++) {
        // Decide to crossover or not:
        if (unif_rand() < CR || k == dimension) {
          // Diffolve:
          trial[j] = baseVector[j] + F * vectorsDifference(population, vectors, j);
        }
        else {
          trial[j] = population(c, j);
        }

        // Move on to next element:
        j = (j + 1) % dimension;
      }

      // Trim:
      trial = trimVector(trial, upper, lower);

      // Compute the score for the candidate:
      const double score = evaluateObjective(objective, trial);

      // Check the score against the old one:
      if (score < scores[c]) {
        // OK, we like this trial. Save trial to the new population:
        newPopulation(c, Rcpp::_) = trial;

        // Save score to the new scores:
        newScores[c] = score;
      }
      else {
        // Nope, keep the old candidate:
        newPopulation(c, Rcpp::_) = population.row(c);

        // Keep the old score:
        newScores[c] = scores[c];
      }
    }

    //OK, set the new population as the new population:
    population = newPopulation;

    // Set the scores:
    scores = newScores;

    // Save if asked:
    populationsList.push_back(population);
    scoresList.push_back(scores);
  }

  // Done, return:
  return Rcpp::List::create(Rcpp::_["populations"] = populationsList,
                            Rcpp::_["scores"] = scoresList);
}
