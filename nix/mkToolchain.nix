{ pkgs }:
{ target }:
  let
    llvm = pkgs.llvmPackages_latest;
    arm-none-eabi = "arm-none-eabi";
    x86_64-linux = "x86_64-linux";
 
    # This maps a nix system definition to the related cmake flags for clang
    cmakeSystemFlagMap = rec {
      compilerBase = "${llvm.clang}/bin/clang";
      compilerBaseUnwrapped = "${llvm.clang-unwrapped}/bin/clang";
      flags = {
        shared = [
          "-DCMAKE_AR=${llvm.llvm}/bin/llvm-ar"
          "-DCMAKE_RANLIB=${llvm.llvm}/bin/llvm-ranlib"
          "-DCMAKE_LINKER=${llvm.lld}/bin/ld.lld"

          "-DCMAKE_BUILD_TYPE=Debug"
          "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"
        ];

        "${x86_64-linux}" = [
          "-DCMAKE_C_COMPILER=${compilerBase}"
          "-DCMAKE_CXX_COMPILER=${compilerBase}++"
          "-DCMAKE_ASM_COMPILER=${compilerBase}"

          "-DCMAKE_C_COMPILER_TARGET=${x86_64-linux}-gnu"
          "-DCMAKE_CXX_COMPILER_TARGET=${x86_64-linux}-gnu"
        ];

        # Note: More of a hybrid architecture for now, expecting to make our own compiler_rt and libc++ a'la clang specifics later.
        "${arm-none-eabi}" = let
          linkerFlags = "-fuse-ld=${pkgs.gcc-arm-embedded}/bin/${arm-none-eabi}-ld --gcc-toolchain=${pkgs.gcc-arm-embedded} -nostdlib";
        in [
          "-DCMAKE_SYSTEM_NAME=Generic"
          "-DCMAKE_SYSTEM_PROCESSOR=arm"
          "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"

          "-DCMAKE_C_COMPILER=${compilerBaseUnwrapped}"
          "-DCMAKE_CXX_COMPILER=${compilerBaseUnwrapped}++"
          "-DCMAKE_ASM_COMPILER=${compilerBaseUnwrapped}"

          "-DCMAKE_C_COMPILER_TARGET=${arm-none-eabi}"
          "-DCMAKE_CXX_COMPILER_TARGET=${arm-none-eabi}"
          "-DCMAKE_ASM_COMPILER_TARGET=${arm-none-eabi}"
          "-DCMAKE_SYSROOT=${pkgs.gcc-arm-embedded}/${arm-none-eabi}"

          "-DCMAKE_EXE_LINKER_FLAGS_INIT=\"${linkerFlags}\""
        ];
      };
    };

    exports = "${builtins.concatStringsSep "\n" [
      "export LLVM_DIR=${llvm.llvm.dev}/lib/cmake/llvm"
      "export Clang_DIR=${llvm.libclang.dev}/lib/cmake/clang"
      "export LD_LIBRARY_PATH=${pkgs.stdenv.cc.cc.lib}/lib/"
      "export LIBCLANG_PATH=${llvm.libclang.lib}/lib"
    ]}";

    targetFlags = let
      failureMsg = "gbpkgs.toolchain: no gb toolchain exists for ${target}";
      flags = if cmakeSystemFlagMap.flags ? ${target}
              then cmakeSystemFlagMap.flags
              else abort failureMsg;
    in
      flags.shared ++ flags.${target};

    mkConfigurePhase = { prefixPath ? "", extraFlags ? [] }: let
      cmakeFlags = "${builtins.concatStringsSep " " targetFlags} ${prefixPath} ${builtins.concatStringsSep " " extraFlags}";
    in ''
      runHook preConfigure
      ${exports}
      cmake -B build -S . -G Ninja ${cmakeFlags}
      runHook postConfigure
    '';
  in {
    buildInputs = [
      llvm.llvm
      llvm.llvm.dev
      llvm.libclang.dev
      llvm.libclang.lib
      pkgs.gtest # TODO: Move out of the toolchain
    ];

    nativeBuildInputs = with pkgs; [
      cmake ninja llvm.clang llvm.lld llvm.bintools
    ] ++ (if target == "arm-none-eabi" then [
      gcc-arm-embedded
    ] else []);

    configurePhase = mkConfigurePhase {};
    inherit mkConfigurePhase targetFlags target llvm exports;
}
