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
          rev = "v1.28.3";
          sha256 = "sha256-WPdfln4dzaejZUMGa27IYT62mB9IB8S/eGxXw7TPWuM=";
          fetchSubmodules = true;
        };
      in {
        # gbpkgs.mkGbDerivation implies you are using the full monorepo
        packages.default = gbpkgs.mkGbDerivation {
          pname = "stm32f446_hal";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = with gbpkgs; [
            stm32cubemx
            xvfb-run
            zip
          ];

          preConfigure = ''
            # 1. Isolate the sandbox's home directory structures
            export HOME=$TMPDIR
            mkdir -p $HOME/.config/java
            mkdir -p $HOME/STM32Cube/Repository

            # 2. Package the folder into the precise directory structure CubeMX expects
            echo "Structuring and compressing STM32CubeF4 repository..."
            mkdir -p $TMPDIR/zip_stage/STM32Cube_FW_F4_V1.28.3
            cp -r ${stm32cubef4}/* $TMPDIR/zip_stage/STM32Cube_FW_F4_V1.28.3/
            
            cd $TMPDIR/zip_stage
            zip -q -r $HOME/STM32Cube/Repository/en.stm32cubef4_v1-28-3.zip STM32Cube_FW_F4_V1.28.3
            cd $TMPDIR

            # 3. Suppress Java preference write exceptions completely
            export JAVA_TOOL_OPTIONS="-Djava.util.prefs.userRoot=$HOME/.config/java -Djava.util.prefs.systemRoot=$HOME/.config/java"

            # 4. Generate BOTH the workspace text configuration AND the user options profile
            mkdir -p $HOME/.stm32cubemx
            
            # Write fallback configuration file
            cat > $HOME/.stm32cubemx/RepositoryPath.txt <<EOF
$HOME/STM32Cube/Repository
EOF

            # Inject directly into the primary application runtime configuration map
            cat > $HOME/.stm32cubemx/mx.properties <<EOF
RepositoryPath=$HOME/STM32Cube/Repository
RecentProjects=
EOF

            # 5. Build up and execute the automated headless generation script
            cat > cubemx.txt <<EOF
config load gen/gen.ioc
project generate
exit
EOF
            xvfb-run stm32cubemx -s cubemx.txt
          '';

          installPhase = ''
            mkdir -p $out/examples/bin
            cp build/bin/stm32f446 $out/examples/bin/
          '';
        };

        # gpkgs.mkshell includes the `configure` utility and the hooking up of the llvm toolchain
        devShells.default = gbpkgs.mkShell {
          nativeBuildInputs = with gbpkgs; [
            stm32cubemx
          ];

          shellHook = ''
            export ARM_SYSROOT=${gbpkgs.gcc-arm-embedded}/arm-none-eabi
            export ARM_LLD=${gbpkgs.gbox.toolchain.llvm.lld}/bin/ld.lld
            export ARM_AR=${gbpkgs.gbox.toolchain.llvm.bintools}/bin/llvm-ar
            export ARM_RANLIB=${gbpkgs.gbox.toolchain.llvm.bintools}/bin/llvm-ranlib

            echo "gbox-mono example dev environment ready."
            echo "  configure      – run cmake"
            echo "  ninja -C build – compile"
          '';
        };
      });
}
