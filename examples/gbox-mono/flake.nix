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
        gbpkgs = gbox.lib.gbpkgs { inherit system nixpkgs; };
      in {
        # gbpkgs.mkGbDerivation implies you are using the full monorepo
        packages.default = gbpkgs.mkGbDerivation {
          pname = "ex-app-loop";
          version = "0.1.0";
          src = ./.;

          installPhase = ''
            mkdir -p $out/examples/bin
            cp build/bin/app-loop $out/examples/bin/
          '';
        };

        # gpkgs.mkshell includes the `configure` utility and the hooking up of the llvm toolchain
        devShells.default = gbpkgs.mkShell {
          shellHook = ''
            echo "gbox-mono example dev environment ready."
            echo "  configure      – run cmake"
            echo "  ninja -C build – compile"
          '';
        };
      });
}
