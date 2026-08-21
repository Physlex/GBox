{
  description = "Nix package management for the gbox framework";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
  {
    lib.gbpkgs = args:
      import ./nix/gbpkgs.nix args;
  }
  //
  flake-utils.lib.eachDefaultSystem (system:
    let
      gbpkgs = import ./nix/gbpkgs.nix { inherit system nixpkgs; };
      gbox = gbpkgs.gbox;

      mkToolsConfig = gbox.lib.mkToolsConfig { inherit (gbox) toolchain; };

      duck = gbpkgs.callPackage ./nix/packages/duck.nix {
        libclang = gbox.toolchain.llvm.libclang;
      };
    in {
      packages.default = gbox.modules;
      devShells.default = gbox.lib.mkShell {
        toolchain = gbox.toolchain;

        buildInputs = gbox.lib.deps.buildInputs;
        nativeBuildInputs = gbox.lib.deps.nativeBuildInputs
          ++ [ gbpkgs.pre-commit gbpkgs.gdb gbpkgs.nixd duck ];

        shellHook = ''
          ${mkToolsConfig.config}
          echo "Nix gbox-mono development environment initialized."
        '';
      };
    });
}
