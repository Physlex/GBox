# Builds a downstream project against gbox. Takes the arguments of `stdenv.mkDerivation`, plus
# an optional `target` naming which toolchain to build with, defaulting to the host.
#
#   mkGbDerivation { pname = "firmware"; src = ./.; target = "arm-none-eabi"; }
#
# The gbox parts for that target are built and put on the cmake prefix path, so the project
# finds them through `find_package(gbox)`.
{ pkgs }:
let
  toolchains = import ./toolchains.nix { inherit pkgs; };
  parts = import ./parts.nix { inherit pkgs; };
in
args:
let
  toolchain = toolchains.${args.target or pkgs.system};
  targetMono = parts { inherit toolchain; };
  prefixPath = "-DCMAKE_PREFIX_PATH=${targetMono}";
in
pkgs.stdenv.mkDerivation ({
  nativeBuildInputs = toolchain.nativeBuildInputs
    ++ (args.nativeBuildInputs or []);

  buildInputs = [ targetMono ]
    ++ toolchain.buildInputs
    ++ (args.buildInputs or []);

  configurePhase = toolchain.mkConfigurePhase {
    inherit prefixPath;
    extraFlags = args.cmakeFlags or [];
  };

  buildPhase = "ninja -C build";
} // (builtins.removeAttrs args [
  "target" "nativeBuildInputs" "buildInputs" "cmakeFlags"
]))
