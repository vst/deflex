# deflex

> **Note**
>
> This package is experimental. Therefore, expect significant API changes until
> it reaches a certain maturity.

*deflex* is et another *Differential Evolution* package.

There are 3 (and a half) Differential Evolution (DE) packages on CRAN as of the
moment this package has been initiated. This package, however, is designed to
provide functional control over the evolution process bringing greater
flexibility for experimenting with documented and custom DE strategies.

## Usage

A sample session looks like as follows:

```R
## Define the objective function:
Rosenbrock <- function(x) {
  x1 <- x[1]
  x2 <- x[2]
  100 * (x2 - x1 * x1)^2 + (1 - x1)^2
}

## Define lower and upper boundaries:
lower <- c(-10,-10)
upper <- -lower

## Define initial population:
initpop <- matrix(rnorm(1000), ncol=2)

## Define number of iterations:
iterations <- 500

## Define crossover probability from interval `[0, 1]`:
cr <- 0.5

## Define differential weighting factor from interval `[0, 2]`:
f <- 0.8

## Define crossover adaptation speed from interval `(0, 1]`:
c <- 0.5

## Define jitter factor:
jf <- 0.1

## Indicate whether we should bounce back at the boundaries or not:
bounce_back <- FALSE

## Define precision (`0` disables it):
precision <- 0

## Set the random seed:
set.seed(42)

## Call the optimisation routine:
system.time(result <- deflex:::deflex_strategy3(Rosenbrock, lower, upper, initpop, iterations, cr, f, c, jf, bounce_back, precision))
```

## Development

The development environment is powered by Nix. A Nix Shell is provided that
installs development dependencies such as `devtools` and `languageserver`.

Enter the Nix shell via:

```sh
nix-shell
```

... or using Nix Flakes:

```sh
nix develop
```

Then run R:

```sh
R
```

Check the package build:

```R
devtools::check()
```

Load the package into the R session:

```R
devtools::load_all()
```

Run a sample optimisation, same as above usage section but without namespace
qualifier:

```R
system.time(result <- deflex_strategy3(Rosenbrock, lower, upper, initpop, iterations, cr, f, c, jf, bounce_back, precision))
```

You can also launch a VS Code. If you have installed R language extension on VS
Code, it will use the `languageserver` package provided in the Nix shell
automatically.

This package is configured to be documented using (R)Markdown. The
auto-generated documentation can be updated at anytime via:

```R
devtools::document()
```

For further information about the (R)Markdown support for `roxygen2`, visit:

<https://roxygen2.r-lib.org/articles/rd-formatting.html>
