{
  pkgs,
  lib,
  system,
}:

pkgs.stdenv.mkDerivation {
  pname = "gbox-mono";
  version = "0.1.0";
  src = ../.;

  nativeBuildInputs = lib.toolchain.nativeBuildInputs;
  buildInputs       = lib.toolchain.buildInputs;

  configurePhase = lib.toolchain.cmake.configureFor system "";
  buildPhase = "ninja -C build";

  installPhase = ''
    mkdir -p $out/bin $out/lib $out/include $out/share/cmake/gbox
    cp -r build/bin/. $out/bin/
    cp -r build/lib/. $out/lib/
    cp -r libs/core/inc/. $out/include/
    cp -r libs/proc/inc/. $out/include/
    cp -r libs/runtime/inc/. $out/include/
    cp -r cmake/. $out/share/cmake/gbox

    sed "s|@out@|$out|g" > $out/share/cmake/gbox/gboxConfig.cmake << 'EOF'
set(GBOX_INCLUDE_DIR "@out@/include")
set(GBOX_LIB_DIR     "@out@/lib")
set(GBOX_BIN_DIR     "@out@/bin")
set(GBOX_CMAKE_DIR   "@out@/share/cmake/gbox")

include(''${GBOX_CMAKE_DIR}/AddTests.cmake)
include(''${GBOX_CMAKE_DIR}/TargetFormat.cmake)
include(''${GBOX_CMAKE_DIR}/AddModule.cmake)

add_library(gbox::core STATIC IMPORTED)
set_target_properties(gbox::core PROPERTIES
    IMPORTED_LOCATION             "@out@/lib/libgbox_core.a"
    INTERFACE_INCLUDE_DIRECTORIES "@out@/include"
)

add_library(gbox::runtime STATIC IMPORTED)
set_target_properties(gbox::runtime PROPERTIES
    IMPORTED_LOCATION             "@out@/lib/libgbox_runtime.a"
    INTERFACE_INCLUDE_DIRECTORIES "@out@/include"
)
EOF
  '';
}
