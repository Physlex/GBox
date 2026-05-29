{ pkgs }:
  let
    llvm = pkgs.llvmPackages_latest;
  in {
    buildInputs = [
      llvm.llvm
      llvm.llvm.dev
      llvm.libclang.dev
      llvm.libclang.lib
      pkgs.gtest # TODO: Move out of the toolchain
    ];

    nativeBuildInputs = with pkgs; [
      cmake
      ninja
      llvm.clang
      llvm.lld
      llvm.bintools
    ];

    cmake = rec {
      flags = [
        "-DCMAKE_C_COMPILER=${llvm.clang}/bin/clang"
        "-DCMAKE_CXX_COMPILER=${llvm.clang}/bin/clang++"
        "-DCMAKE_BUILD_TYPE=Debug"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"
        "-DCMAKE_AR=${llvm.llvm}/bin/llvm-ar"
        "-DCMAKE_RANLIB=${llvm.llvm}/bin/llvm-ranlib"
        "-DCMAKE_LINKER=${llvm.lld}/bin/ld.lld"
        "-DCMAKE_BUILD_TYPE=Debug"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"
      ];

      armFlags = [
      ];

      exports = [
        "export LLVM_DIR=${pkgs.llvmPackages_latest.llvm.dev}/lib/cmake/llvm"
        "export Clang_DIR=${pkgs.llvmPackages_latest.libclang.dev}/lib/cmake/clang"
        "export LD_LIBRARY_PATH=${pkgs.stdenv.cc.cc.lib}/lib/"
        "export LIBCLANG_PATH=${pkgs.llvmPackages_latest.libclang.lib}/lib"
      ];

      configurePhase = ''
        ${builtins.concatStringsSep "\n" exports}
        cmake -B build -S . -G Ninja ${builtins.concatStringsSep " " flags}
      '';
    };

    inherit llvm;
}
