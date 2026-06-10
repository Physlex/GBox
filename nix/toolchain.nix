{ pkgs, system ? pkgs.stdenv.hostPlatform.config }:
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
      gcc-arm-embedded
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
          "-DCMAKE_ASM_COMPILER=${llvm.clang}/bin/clang"

          "-DCMAKE_ASM_COMPILER=${llvm.clang}/bin/clang"
          "-DCMAKE_C_COMPILER_TARGET=x86_64-unknown-linux-gnu"
          "-DCMAKE_CXX_COMPILER_TARGET=x86_64-unknown-linux-gnu"
        ];

        "arm-none-eabi" = [
          "-DCMAKE_SYSTEM_NAME=Generic"
          "-DCMAKE_SYSTEM_PROCESSOR=arm"
          "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"

          "-DCMAKE_C_COMPILER=${llvm.clang-unwrapped}/bin/clang"
          "-DCMAKE_CXX_COMPILER=${llvm.clang-unwrapped}/bin/clang++"
          "-DCMAKE_ASM_COMPILER=${llvm.clang-unwrapped}/bin/clang"

          "-DCMAKE_C_COMPILER_TARGET=arm-none-eabi"
          "-DCMAKE_CXX_COMPILER_TARGET=arm-none-eabi"
          "-DCMAKE_ASM_COMPILER_TARGET=arm-none-eabi"
          "-DCMAKE_SYSROOT=${pkgs.gcc-arm-embedded}/arm-none-eabi"

          "-DCMAKE_C_FLAGS_INIT=--gcc-toolchain=${pkgs.gcc-arm-embedded}"
          "-DCMAKE_CXX_FLAGS_INIT=\"--gcc-toolchain=${pkgs.gcc-arm-embedded} -stdlib=libstdc++\""
          "-DCMAKE_EXE_LINKER_FLAGS_INIT=-fuse-ld=\"${llvm.lld}/bin/ld.lld -nodefaultlibs -lc -lm -lstdc++ -lnosys ${pkgs.gcc-arm-embedded}/lib/gcc/arm-none-eabi/14.3.1/thumb/v7e-m+fp/hard/libgcc.a\""
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

      configureFor = target: prefixPath: extraFlags: ''
        runHook preConfigure
        ${builtins.concatStringsSep "\n" exports}
        cmake -B build -S . -G Ninja ${builtins.concatStringsSep " " (flagsFor target)} ${prefixPath} ${builtins.concatStringsSep " " extraFlags}
        runHook postConfigure
      '';
    };

    inherit llvm;
}
