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
