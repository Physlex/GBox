{ pkgs }:
{ toolchain }:
pkgs.stdenv.mkDerivation {
  pname = "gbox-mono";
  version = "0.1.0";
  src = ../.;

  nativeBuildInputs = toolchain.nativeBuildInputs;
  buildInputs = toolchain.buildInputs ++ [ pkgs.sdl3 ];

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
