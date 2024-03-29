#include <vector>
#include <Rcpp.h>
#include "utils.h"
#include "evaluate.h"


inline double roundDoubleDownTo(double value, double step) {
    if (step == 0) {
        return value;
    }
    else {
        return floor(value / step) * step;
    }
}

inline double roundDoubleUpTo(double value, double step) {
    if (step == 0) {
        return value;
    }
    else {
        return ceil(value / step) * step;
    }
}

inline double roundToClosest (double value, double step) {
    double down = roundDoubleDownTo(value, step);
    double up = roundDoubleUpTo(value, step);
    if (std::abs(value - down) < std::abs(value - up)) {
        return down;
    }
    return up;
}


/**
 * Provides precision adjustment.
 *
 * @param vector The vector to be adjusted.
 * @param step Precision step.
 * @return Adjusted vector.
 */
inline Rcpp::NumericVector precisionAdjustment (Rcpp::NumericVector vector, double step) {
    for (int i = 0; i < vector.size(); i++) {
        vector(i) = roundToClosest(vector(i), step);
    }
  return vector;
}

/**
 * Provides a simple DE routine.
 *
 * @param objective  Objective function.
 * @param lower      Lower-bounds for each parameter to be optimised.
 * @param upper      Upper-bounds for each parameter to be optimised.
 * @param initpop    Initial population.
 * @param iterations Number of iterations.
 * @param cr         Crossover probability from interval `[0, 1]`, for example `0.5`.
 * @param f          Differential weighting factor from interval `[0, 2]`, for example `0.8`.
 * @param c          Crossover adaptation speed from interval `(0, 1]`, for example `0.5`.
 * @param jf         Jitter factor, for example `0.10`.
 * @param bounceBack Whether to bounce back from boundaries or not.
 * @param precision  Precision as a positive number whereby 0 disables it.
 * @return Optimisation result.
 */
// [[Rcpp::export]]
SEXP deflex_strategy3 (Rcpp::Function objective,
                       Rcpp::NumericVector lower,
                       Rcpp::NumericVector upper,
                       Rcpp::NumericMatrix initpop,
                       int iterations,
                       double cr,
                       double f,
                       double c,
                       double jf,
                       bool bounceBack,
                       double precision) {
  //////////////
  // PREAMBLE //
  //////////////

  // Before we start, suspend RNG synchronisation:
  Rcpp::SuspendRNGSynchronizationScope rngScope;

  // First, get the problem dimension:
  const int dimension = upper.size();

  // Now, get the population size:
  const int popsize = initpop.nrow();

  // Create a base vector of random vector indices to be used later:
  Rcpp::IntegerVector indexVector = Rcpp::wrap(createSequence(0, initpop.nrow()));

  // Initialize the scores vector:
  Rcpp::NumericVector scores = evaluateObjectives(initpop, objective, lower, upper);

  // Get the best score and best candidate:
  int bestIndex = Rcpp::which_min(scores);
  double bestScore = scores[bestIndex];
  Rcpp::NumericMatrix::Row bestIndividual = initpop.row(bestIndex);

  // Declare the return value:
  Rcpp::List populationsList;
  Rcpp::List bestMembersList;
  Rcpp::List scoresList;
  Rcpp::List bestScoresList;
  Rcpp::List flagsList;
  Rcpp::Environment rhoenv = Rcpp::Environment::empty_env();

  // Keep the initial population and scores:
  populationsList.push_back(initpop);
  bestMembersList.push_back(bestIndividual);
  scoresList.push_back(scores);
  bestScoresList.push_back(bestScore);
  flagsList.push_back(Rcpp::rep(1, popsize));

  // Set the meanCR initially to the crossover:
  double meanCR = cr;

  // Set the meanF initially to the crossover:
  double meanF = f;

  // Set the goodNPCount:
  double goodCR = 0.0;
  double goodF = 0.0;
  double goodF2 = 0.0;
  int goodNPCount = 0;

  // Initialize the generation counter:
  int generation = 0;

  // Repeat as until maximum generation count, ie. iterations:
  while (++generation < iterations + 1) {
    // Procedure:
    //
    // 1. Mark the beginning of new generation
    // 2. Preamble: Initialize temporary generation data:
    //     1. Get the current best member from the last population.
    //     2. Initialize the new population matrix exactly the same as the last population.
    //     3. Initialize the new population scores vector the same as the last population scores.
    //     4. Initialize the new population validity flags vector.
    // 3. Iterate over each candidate in the last population.
    //     1. Adjust CR and F if adaptation speed is in use.
    //     2. Copy the trial from the current candidate.
    //     3. Pick 2 random candidates from the last population (ideally excluding the current candidate).
    //     4. Pick which element to start with for the trial.
    //     5. Iterate over each element in the trial and modify it (Actual work)
    //     6. Apply precision to each element in the trial, if "precision adjustment" is in use.
    //     7. Apply limits to each element in the trial, in case that we have violated.
    //     8. Assess the trial and take actions: Compute the score of the trial.
    //         1. Compute the trial score.
    //         2. If trial score is not better than the previous score, keep the new population candidate same as last
    //            population candidate, and mark the new population candidate index as "unchanged".
    //         3. Otherwise:
    //             1. Set the new population candidate to trial.
    //             2. Update the new population candidate score.
    //             2. Update goodCR, goodF and goodF2 if adaptation speed is in use.
    // 4. Epilogue:
    //     1. Update meanCR and meanF if adaptation speed is in use.
    //     2. Backup the old population (scores and flags, too)
    //     3. Set the new population to the canonical population.
    // 5. Mark the finishing of new generation.

    // 1. Mark the beginning of new generation
    // Rcpp::Rcout << "Generation Start: " << generation << std::endl;

    // 2. Preamble: Initialize temporary generation data:
    //     1. Get the current best member from the last population.
    Rcpp::NumericVector bestMember = bestMembersList[bestMembersList.size() - 1];
    Rcpp::NumericVector newBestMember = bestMember;
    double newBestMemberScore = bestScoresList[bestMembersList.size() - 1];

    //     2. Initialize the new population matrix exactly the same as the last population.
    Rcpp::NumericMatrix oldPopulation = populationsList[populationsList.size() - 1];
    Rcpp::NumericMatrix newPopulation(Rcpp::clone(oldPopulation));

    //     3. Initialize the new population scores vector the same as the last population scores.
    Rcpp::NumericVector oldScores = scoresList[scoresList.size() - 1];
    Rcpp::NumericVector newScores(Rcpp::clone(oldScores));

    // 3. Iterate over each candidate in the last population.
    for (int candidate = 0; candidate < newPopulation.rows(); candidate++) {
      //     1. Adjust CR and F if adaptation speed is in use.
      if (c > 0) {
        // Yes. We will now adjust the CR first:
        cr = Rf_rnorm(meanCR, 0.1);

        // Check and reset CR:
        cr = cr > 1 ? 1 : (cr < 0 ? 0 : cr);

        // OK, now we will adjust F:
        do {
          // Get the new F:
          f = Rf_rcauchy(meanF, 0.1);

          // Check and reset F if required:
          f = f > 1 ? 1 : f;
        } while (f <= 0.0);
      }

      //     2. Copy the trial from the current candidate.
      Rcpp::NumericVector oldCandidate = newPopulation(candidate, Rcpp::_);
      Rcpp::NumericVector trial(Rcpp::clone(oldCandidate));

      //     3. Pick 2 random candidates from the last population (ideally excluding the current candidate).
      // TODO: XXX
      Rcpp::IntegerVector tidx = indexVector[indexVector != candidate];
      Rcpp::IntegerVector ridx = Rcpp::sample(tidx, 2);

      //     4. Pick which element to start with for the trial.
      int j = Rcpp::sample(dimension, 1)[0] - 1;
      int k = 0;

      //     5. Iterate over each element in the trial and modify it (Actual work)
      do {
        // Get a random:
        const double rr = Rf_runif(0, 1);

        // Get the jitter:
        const double jitter = (rr * jf) + f;

        // Get the respective element of the best candidate:
        const double bestest = bestMember[j];

        // Get the random candidate elements:
        const double random1 = oldPopulation(ridx[0], j);
        const double random2 = oldPopulation(ridx[1], j);

        // Override trial:
        trial[j] = bestest + jitter * (random1 - random2);

        // Check for a weird case that we get NaN:
        if(ISNAN(trial[j])) {
          // TODO: Something funny here.
          //
          // It was due to the tidx statement and underlying
          // indexVector which was based on 1 previously. Shouldn't
          // repeat again, yet just on case for now.
          //
          // Rcpp::Rcout << bestest << " " << jitter << "(" << jf << ", " << f <<  ", " << rr << ")" << " " <<  random1 << " " << random2 << std::endl;
          trial[j] = (upper[j] + lower[j]) / 2.0;
        }

        // Move to the next element:
        j = (j + 1) % dimension;

        // Increment k:
        k++;
      } while (Rf_runif(0, 1) < cr && k < dimension);

      //     6. Apply precision to each element in the trial, if "precision adjustment" is in use.
      //     7. Apply limits to each element in the trial, in case that we have violated.
      for (int i = 0; i < trial.size(); i++) {
        // Adjust as per precision:
        if (precision != 0) {
            trial[i] = roundToClosest(trial[i], precision);
        }

        // Check lower limit:
        if (trial[i] < lower[i]) {
          if (bounceBack) {
            trial[i] = lower[i] + Rf_runif(0, 1) * (upper[i] - lower[i]);
          }
          else {
            trial[i] = lower[i];
          }
        }

        // Check upper limit:
        if (trial[i] > upper[i]) {
          if (bounceBack) {
            trial[i] = lower[i] - Rf_runif(0, 1) * (upper[i] - lower[i]);
          }
          else {
            trial[i] = upper[i];
          }
        }
      }

      //     8. Assess the trial and take actions: Compute the score of the trial.
      //         1. Compute the trial score.
      //double trialScore = Rcpp::as<double>(objective(trial));
      double trialScore = evaluate(trial, objective, rhoenv);

      //         2. If trial score is not better than the previous score, keep the new population candidate same as last
      //            population candidate, and mark the new population candidate index as "unchanged".
      //         3. Otherwise:
      //             1. Set the new population candidate to trial.
      //             2. Update the new population candidate score.
      //             2. Update goodCR, goodF and goodF2 if adaptation speed is in use.
      if (trialScore < oldScores[candidate]) {
        newPopulation(candidate, Rcpp::_) = trial;
        newScores[candidate] = trialScore;

        goodCR += cr / ++goodNPCount;
        goodF += f;
        goodF2 += std::pow(f, 2);

        if (trialScore < bestScore) {
          bestScore = trialScore;
          newBestMember = trial;
          newBestMemberScore = trialScore;
        }
      }
    }
    // 4. Epilogue:
    //     1. Update meanCR and meanF if adaptation speed is in use.
    //     2. Backup the old population (scores and flags, too)
    //     3. Set the new population to the canonical population.
    // Re-compute mean CR and F if required:
    if (c > 0 && goodF != 0) {
      meanCR = (1.0 - c) * meanCR + c * goodCR;
      meanF  = (1.0 - c) * meanF  + c * goodF2 / goodF;
    }

    // Add stuff:
    populationsList.push_back(newPopulation);
    scoresList.push_back(newScores);
    bestMembersList.push_back(newBestMember);
    bestScoresList.push_back(newBestMemberScore);

    // 5. Mark the finishing of new generation.
    // Rcpp::Rcout << "Generation End  : " << generation << std::endl;
  }


  // Done, return:
  return Rcpp::List::create(Rcpp::_["problem"] = NULL,
                            Rcpp::_["results"] = NULL,
                            Rcpp::_["populations"] = populationsList,
                            Rcpp::_["bestmembers"] = bestMembersList,
                            Rcpp::_["bestscores"] = bestScoresList,
                            Rcpp::_["popscores"] = scoresList,
                            Rcpp::_["popflags"] = flagsList,
                            Rcpp::_["bestmember"] = bestMembersList[bestMembersList.size() - 1],
                            Rcpp::_["bestscore"] = bestScoresList[bestScoresList.size() - 1]
                            );

  // return Rcpp::List::create(Rcpp::_["populations"] = populationsList,
  //                           Rcpp::_["scores"] = scoresList);
}
