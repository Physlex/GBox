{ pkgs }:
  let
    llvm = pkgs.llvmPackages_latest;
  in {
    buildInputs = [
      llvm.llvm
      llvm.llvm.dev
      llvm.libclang.dev
      llvm.libclang.lib
    ];

    nativeBuildInputs = with pkgs; [
      cmake
      ninja
      gtest
      llvm.clang
    ];

    cmake = rec {
      flags = [
        "-DCMAKE_C_COMPILER=${llvm.clang}/bin/clang"
        "-DCMAKE_CXX_COMPILER=${llvm.clang}/bin/clang++"
        "-DCMAKE_BUILD_TYPE=Debug"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"
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
