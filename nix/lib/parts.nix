{ pkgs }:
let
  deps = import ./deps.nix { inherit pkgs; };
in
{ toolchain }:
pkgs.stdenv.mkDerivation {
  pname = "gbox-mono";
  version = "0.1.0";
  src = ../../.;

  nativeBuildInputs = toolchain.nativeBuildInputs ++ deps.nativeBuildInputs;
  buildInputs = toolchain.buildInputs ++ deps.buildInputs;

  configurePhase = toolchain.mkConfigurePhase {
    extraFlags = [
      "-DCMAKE_INSTALL_PREFIX=$out"
      "-DCMAKE_INSTALL_LIBDIR=lib"
    ];
  };

  buildPhase = "ninja -C build";

  # The package layout and its cmake config come from the install rules gbox_module() emits, so
  # there is nothing to assemble here.
  installPhase = "ninja -C build install";
}
