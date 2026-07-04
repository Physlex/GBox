{ system, nixpkgs, config ? { }, }:

let
  pkgs = import nixpkgs { inherit system config; };

  mkToolchain = import ./mkToolchain.nix { inherit pkgs; };
  mkShell = import ./mkShell.nix { inherit pkgs; };
  partsNoTarget = import ./parts.nix { inherit pkgs; };
  toolchains = import ./toolchains.nix { inherit pkgs; };

  gboxToolchain = mkToolchain { target = system; };
in
pkgs // {
  gbox = let
    toolchain = gboxToolchain;
    modules = partsNoTarget { inherit toolchain; };
  in {
    inherit toolchain modules;
  };

  inherit mkToolchain toolchains mkShell ;

  mkGbDerivation = args:
    let
      toolchain = toolchains.${args.target or system};
      targetMono = (partsNoTarget { inherit toolchain; });
      prefixPath = "-DCMAKE_PREFIX_PATH=${targetMono}";
    in
    pkgs.stdenv.mkDerivation ({
      nativeBuildInputs = toolchain.nativeBuildInputs
        ++ (args.nativeBuildInputs or []
      );

      buildInputs = [ targetMono ]
        ++ toolchain.buildInputs
        ++ (args.buildInputs or []
      );

      configurePhase = toolchain.mkConfigurePhase {
        inherit prefixPath;
        extraFlags = args.cmakeFlags or [];
      };

      buildPhase = "ninja -C build";
    } // (builtins.removeAttrs args [
      "target" "nativeBuildInputs" "buildInputs" "cmakeFlags"
    ]));
}
