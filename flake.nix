{
  inputs = {
    flake-utils.url = github:numtide/flake-utils;
  };

  outputs = { self, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
    let
      thisDrv = import ./default.nix { system = system; };
    in
    {
      system = builtins.currentSystem;
      devShell = thisDrv;
    }
  );
}
