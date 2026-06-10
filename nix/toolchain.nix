{ pkgs, system ? pkgs.stdenv.hostPlatform.config }:
  let
    llvm = pkgs.llvmPackages_latest;
    crossPkgs = pkgs.pkgsCross.arm-embedded;
    crossCc = crossPkgs.stdenv.cc;
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
      crossCc
    ];

    cmake = rec {
      flags = {
        shared = [
          "-DCMAKE_AR=${llvm.llvm}/bin/llvm-ar"
          "-DCMAKE_RANLIB=${llvm.llvm}/bin/llvm-ranlib"
          "-DCMAKE_LINKER=${llvm.lld}/bin/ld.lld"
          "-DCMAKE_BUILD_TYPE=Debug"
          "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"
        ];

        "x86_64-linux" = [
          "-DCMAKE_C_COMPILER=${llvm.clang}/bin/clang"
          "-DCMAKE_CXX_COMPILER=${llvm.clang}/bin/clang++"
        ];

        "arm-none-eabi" = [
          "-DCMAKE_SYSTEM_NAME=Generic"
          "-DCMAKE_SYSTEM_PROCESSOR=arm"
          "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
          "-DCMAKE_C_COMPILER=${crossCc}/bin/arm-none-eabi-clang"
          "-DCMAKE_ASM_COMPILER=${crossCc}/bin/arm-none-eabi-clang"
        ];
      };

      exports = [
        "export LLVM_DIR=${pkgs.llvmPackages_latest.llvm.dev}/lib/cmake/llvm"
        "export Clang_DIR=${pkgs.llvmPackages_latest.libclang.dev}/lib/cmake/clang"
        "export LD_LIBRARY_PATH=${pkgs.stdenv.cc.cc.lib}/lib/"
        "export LIBCLANG_PATH=${pkgs.llvmPackages_latest.libclang.lib}/lib"
      ];

      flagsFor = target:
        if flags ? ${target}
        then flags.shared ++ flags.${target}
        else abort "gbpkgs.toolchain: no gb toolchain exists for ${target}";

      configureFor = target: prefixPath: ''
        runHook preConfigure
        ${builtins.concatStringsSep "\n" exports}
        cmake -B build -S . -G Ninja ${builtins.concatStringsSep " " (flagsFor target)} ${prefixPath}
        runHook postConfigure
      '';
    };

    inherit llvm crossCc;
}
