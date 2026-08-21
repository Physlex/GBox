# nixpkgs for `system`, with gbox attached. Downstream flakes get this from `gbox.lib.gbpkgs`
# and use it wherever they would have used nixpkgs.
{ system, nixpkgs, config ? { }, }:

let
  pkgs = import nixpkgs { inherit system config; };

  gblib = import ./lib { inherit pkgs; };

  toolchain = gblib.mkToolchain { target = system; };
in
pkgs // {
  gbox = {
    inherit toolchain;

    modules = gblib.parts { inherit toolchain; };
    lib = gblib;
  };

  inherit (gblib) mkGbDerivation;
}
