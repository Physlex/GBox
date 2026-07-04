{
  description = "rendering example using gbox graphics utilities";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
    gbox.url = "../..";
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
          pname = "rendering";
          version = "0.1.0";
          src = ./.;
          target = system;

          nativeBuildInputs = with gbpkgs; [ sdl3 ];

          installPhase = ''
            mkdir -p $out/examples/bin
            cp build/bin/rendering $out/examples/bin/
          '';
        };

        devShells.default = gbpkgs.mkShell {
          toolchain = gbpkgs.gbox.toolchain;
        };
      });
}
