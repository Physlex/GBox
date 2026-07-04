{ pkgs }:
let
  mkToolchain = import ./mkToolchain.nix { inherit pkgs; };
in {
  "x86_64-linux" = mkToolchain { target = "x86_64-linux"; };
  "arm-none-eabi" = mkToolchain { target = "arm-none-eabi"; };
}
