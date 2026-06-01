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
      in {
        # gbpkgs.mkGbDerivation implies you are using the full monorepo
        packages.default = gbpkgs.mkGbDerivation {
          pname = "stm32f446";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = with gbpkgs; [
            stm32cubemx
            xvfb-run
          ];

          preConfigure = ''
            cat > cubemx.script <<EOF
load gen/gen.ioc
generate code
exit
EOF
            xvfb-run stm32cubemx -s cubemx.script
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
