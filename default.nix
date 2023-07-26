{ sources ? import ./nix/sources.nix
, system ? builtins.currentSystem
, ...
}:

let
  ## Import nixpkgs pinned by niv:
  pkgs = import sources.nixpkgs { inherit system; };

  ## Get R package `deflex` (this) via Nix:
  deflex = pkgs.rPackages.buildRPackage {
    name = "deflex";
    src = ./.;
    propagatedBuildInputs = with pkgs.rPackages; [
      Rcpp
    ];
  };

  ## Declare R dependencies:
  rDeps = {
    dev = with pkgs.rPackages; [
      devtools
      languageserver
      roxygen2
      roxygen2md
    ];

    prd = [
      deflex
    ];
  };

  ## Get R for the shell:
  thisR = pkgs.rWrapper.override {
    packages = rDeps.dev ++ rDeps.prd;
  };
in
pkgs.mkShell {
  buildInputs = [
    pkgs.qpdf ## Needed by the R package build/check process.
    thisR
  ];

  shellHook = ''
    echo "Welcome to devshell"
  '';
}
