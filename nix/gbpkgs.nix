{ system, nixpkgs, config ? { }, }:

let
  pkgs = import nixpkgs { inherit system config; };

  toolchain = import ./toolchain.nix { inherit pkgs; };
  lib = { inherit toolchain; };
  mono = import ./package.nix { inherit pkgs lib; };

  consumerCmake = rec {
    flags = toolchain.cmake.flags ++ [ "-DCMAKE_PREFIX_PATH=${mono}" ];
    exports = toolchain.cmake.exports;
    configurePhase = ''
      runHook preConfigure
      ${builtins.concatStringsSep "\n" exports}
      cmake -B build -S . -G Ninja ${builtins.concatStringsSep " " flags}
      runHook postConfigure
    '';
  };
in

pkgs // {
  gbox = { inherit toolchain mono; };

  mkGbDerivation = args: pkgs.stdenv.mkDerivation ({
    nativeBuildInputs = [ pkgs.cmake pkgs.ninja toolchain.llvm.clang ]
      ++ (args.nativeBuildInputs or []);
    buildInputs = [ mono ] ++ (args.buildInputs or []);
    configurePhase = consumerCmake.configurePhase;
    buildPhase = "ninja -C build";
  } // (builtins.removeAttrs args [ "nativeBuildInputs" "buildInputs" ]));

  mkShell = import ./mkShell.nix {
    inherit pkgs;
    lib = { toolchain = toolchain // { cmake = consumerCmake; }; };
    packages = [ pkgs.cmake pkgs.ninja toolchain.llvm.clang ];
  };
}
