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
      mkToolsConfig = import ./nix/mkToolsConfig.nix { inherit gbpkgs; };
      gbox = gbpkgs.gbox;

      duck = gbpkgs.rustPlatform.buildRustPackage {
        pname = "duck";
        version = "unstable";

        src = gbpkgs.fetchFromGitHub {
          owner = "rdmsr";
          repo = "duck";
          rev = "ff95938795dfc9e55f891eb27de68a4bf63b122a";
          hash = "sha256-aBCm69V//ZtYmmE3GAvUqErSnVIp59te6bJOlibICSM==";
        };

        cargoHash = "sha256-XzqhofYePhrusi5FPx9qWh8gBCsNUb3QAvi78SKNoJs=";

        LIBCLANG_PATH = "${gbpkgs.gbox.toolchain.llvm.libclang.lib}/lib";
        buildInputs = [ gbpkgs.gbox.toolchain.llvm.libclang ];
        nativeBuildInputs = [ gbpkgs.pkg-config ];
      };
    in {
      packages.default = gbox.modules;
      devShells.default = gbpkgs.mkShell {
        toolchain = gbox.toolchain;

        buildInputs = gbox.modules.buildInputs;
        nativeBuildInputs = gbox.modules.nativeBuildInputs
          ++ [ gbpkgs.pre-commit gbpkgs.uv gbpkgs.gdb gbpkgs.nixd duck ];

        shellHook = ''
          ${mkToolsConfig.config}
          echo "Nix gbox-mono development environment initialized."
        '';
      };
    });
}
