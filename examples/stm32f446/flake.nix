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
          ];

          preConfigure = ''
            # 1. Isolate the sandbox's home directory structures
            export HOME=$TMPDIR
            mkdir -p $HOME/.config/java
            mkdir -p $HOME/STM32Cube/Repository

            # 2. Pre-extract the firmware to the path CubeMX uses directly
            cp -r ${stm32cubef4} $HOME/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3

            # 3. Suppress Java preference write exceptions completely
            export JAVA_TOOL_OPTIONS="-Djava.util.prefs.userRoot=$HOME/.config/java -Djava.util.prefs.systemRoot=$HOME/.config/java"

            # 4. Point CubeMX at the repository and register the installed firmware package.
            #    CubeMX reads STMcheckcomputer.xml to resolve firmware InstallPath at LoadConfig time;
            #    without it the load hangs indefinitely waiting for user input.
            mkdir -p $HOME/.stm32cubemx/plugins/updater
            cat > $HOME/.stm32cubemx/mx.properties <<EOF
RepositoryPath=$HOME/STM32Cube/Repository
RecentProjects=
EOF
            cat > $HOME/.stm32cubemx/plugins/updater/STMcheckcomputer.xml <<EOF
<?xml version="1.0" encoding="ISO-8859-1" standalone="no"?>
<Computer DBVersion="2.0">
    <Software>
        <PackDescription Release="MX.6.15.0"/>
    </Software>
    <DataBase>
        <PackDescription Release="DB.6.0.150"/>
    </DataBase>
    <Firmware>
        <PackDescription InstallPath="$HOME/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3" Patch="FW.F4.1.28.3" Release="FW.F4.1.28.0">
            <Note Patch="ReleaseNotes_Patch.html" Release="ReleaseNotes.html"/>
        </PackDescription>
    </Firmware>
</Computer>
EOF

            # 5. Run headless code generation
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
            xvfb-run
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
