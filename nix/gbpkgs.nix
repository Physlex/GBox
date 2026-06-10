{ system, nixpkgs, config ? { }, }:

let
  pkgs = import nixpkgs { inherit system config; };

  mkToolchain = import ./toolchain.nix;
  toolchain = mkToolchain { inherit pkgs; };
  lib = { inherit toolchain; };
  mono = import ./package.nix { inherit pkgs lib system; };
in
pkgs // {
  gbox = { inherit mkToolchain toolchain mono; };

  mkGbDerivation = args:
  let
    target = args.toolchain or toolchain;
    targetSystem = args.system or system;
    prefixPath = "-DCMAKE_PREFIX_PATH=${mono}";
  in
  pkgs.stdenv.mkDerivation ({
    nativeBuildInputs = [
      pkgs.cmake
      pkgs.ninja
      target.llvm.clang
    ] ++ (args.nativeBuildInputs or []);

    buildInputs = [
      mono
    ] ++ (args.buildInputs or []);

    configurePhase = target.cmake.configureFor targetSystem prefixPath (args.cmakeFlags or []);
    buildPhase = "ninja -C build";
  } // (builtins.removeAttrs args [ "toolchain" "system" "nativeBuildInputs" "buildInputs" "cmakeFlags" ]));

  mkShell = import ./mkShell.nix {
    inherit pkgs system;
    lib = { inherit toolchain; };
    packages = [ pkgs.cmake pkgs.ninja toolchain.llvm.clang ];
  };
}
