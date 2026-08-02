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
        packages.default = gbpkgs.mkGbDerivation {
          pname = "ex-app-loop";
          version = "0.1.0";
          src = ./.;
          target = system;

          installPhase = ''
            mkdir -p $out/examples/bin
            cp build/bin/app_loop $out/examples/bin/
          '';
        };

        devShells.default = gbpkgs.mkShell {
          toolchain = gbpkgs.gbox.toolchain;
          buildInputs = [ gbpkgs.gbox.modules ];
        };
      });
}
