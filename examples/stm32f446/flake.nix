{
  description = "gbox monorepo full platform usage example";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
    gbox.url = "../.."; # A real consumer would use: github:user/gbox
  };

  outputs = {
    nixpkgs,
    flake-utils,
    gbox,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        gbpkgs = gbox.lib.gbpkgs {
          inherit system nixpkgs;
          config.allowUnfreePredicate = pkg: builtins.elem (gbpkgs.lib.getName pkg) [
            "stm32cubemx"
          ];
        };

        stm32cubef4 = gbpkgs.fetchFromGitHub {
          owner = "STMicroelectronics";
          repo = "STM32CubeF4";
          rev = "v1.28.2";
          sha256 = "sha256-deweMHeu0bSv2b6L+KCec/ld8GTQJWtsk773PjPwrso=";
          fetchSubmodules = true;
        };

        target = "arm-none-eabi";
        toolchain = gbpkgs.gbox.lib.toolchains.${target};

        cpu = "cortex-m4";
        fpu = "fpv4-sp-d16";
        floatAbi = "hard";
        mcuFlags = "-mthumb -mcpu=${cpu} -mfpu=${fpu} -mfloat-abi=${floatAbi} -fshort-enums";

        gccVersion = builtins.readFile (gbpkgs.runCommand "gcc-ver" {} ''
          ${gbpkgs.gcc-arm-embedded}/bin/arm-none-eabi-gcc -dumpversion | tr -d '\n' > $out
        '');

        multilibDir = builtins.readFile (gbpkgs.runCommand "multilib-dir" {} ''
          ${gbpkgs.gcc-arm-embedded}/bin/arm-none-eabi-gcc ${mcuFlags} -print-multi-directory | tr -d '\n' > $out
        '');

        gccLibDir = "${gbpkgs.gcc-arm-embedded}/lib/gcc/arm-none-eabi/${gccVersion}/${multilibDir}";
        armLibDir = "${gbpkgs.gcc-arm-embedded}/arm-none-eabi/lib/${multilibDir}";
      in {
        packages.default = gbpkgs.stdenv.mkDerivation {
          pname = "firmware";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = toolchain.nativeBuildInputs ++ (with gbpkgs; [
            stm32cubemx
            xvfb-run
            openocd
          ]);

          buildInputs = toolchain.buildInputs;

          configurePhase = toolchain.mkConfigurePhase {
            extraFlags = [
              "-DCMAKE_C_FLAGS=\"${mcuFlags}\""
              "-DCMAKE_CXX_FLAGS=\"${mcuFlags} -fno-rtti -fno-exceptions -fno-threadsafe-statics\""
              "-DGCC_MULTILIB_DIR_ARM=${armLibDir}"
              "-DGCC_MULTILIB_DIR_GCC=${gccLibDir}"
            ];
          };

          buildPhase = "ninja -C build";

          preConfigure = ''
            export HOME=$TMPDIR
            mkdir -p $HOME/.config/java
            mkdir -p $HOME/STM32Cube/Repository

            cp -r ${stm32cubef4} $HOME/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.2

            export JAVA_TOOL_OPTIONS="-Djava.util.prefs.userRoot=$HOME/.config/java -Djava.util.prefs.systemRoot=$HOME/.config/java"

            mkdir -p $HOME/.stm32cubemx/plugins/updater

            cat > cubemx.txt <<EOF
            config load gen/gen.ioc
            project generate
            exit
            EOF

            xvfb-run stm32cubemx -s cubemx.txt
          '';

          installPhase = ''
            mkdir -p $out/bin
            cp build/bin/firmware $out/bin/
          '';
        };

        devShells.default = gbpkgs.gbox.lib.mkShell {
          inherit toolchain;
          nativeBuildInputs = with gbpkgs; [ stm32cubemx xvfb-run ];
        };
      });
}
