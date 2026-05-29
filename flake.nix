{
  description = "Nix package management for the gbox framework";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
  {
    lib.gbpkgs = { system, nixpkgs }:
      import ./nix/gbpkgs.nix { inherit system nixpkgs; };
  }
  //
  flake-utils.lib.eachDefaultSystem (system:
    let
      gbpkgs = import ./nix/gbpkgs.nix { inherit system nixpkgs; };
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
      packages.default = gbpkgs.gbox.mono;
      devShells.default = gbpkgs.mkShell {
        packages = gbpkgs.gbox.mono.buildInputs
          ++ gbpkgs.gbox.mono.nativeBuildInputs
          ++ [ gbpkgs.pre-commit gbpkgs.uv gbpkgs.gdb gbpkgs.nixd duck ];

        shellHook = ''
          cat > ./.clangd <<EOF
          CompileFlags:
            CompilationDatabase: build
            Add:
              - -I${gbpkgs.gbox.toolchain.llvm.llvm.dev}/include
              - -I${gbpkgs.gbox.toolchain.llvm.libclang.dev}/include
          Index:
            Background: Build
          EOF

          echo "Nix gbox-mono development environment initialized."
        '';
      };
    });
}
