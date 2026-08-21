# The gbox nix library. Knows nothing of gbpkgs or the gbox namespace it eventually hangs from,
# so every member takes plain nixpkgs and nothing here reaches back upwards.
#
# `gbpkgs.nix` attaches the result as `gbpkgs.gbox.lib`.
{ pkgs }:
{
  mkToolchain = import ./mkToolchain.nix { inherit pkgs; };
  mkShell = import ./mkShell.nix { inherit pkgs; };
  mkGbDerivation = import ./mkGbDerivation.nix { inherit pkgs; };
  mkToolsConfig = import ./mkToolsConfig.nix;

  toolchains = import ./toolchains.nix { inherit pkgs; };
  parts = import ./parts.nix { inherit pkgs; };
  deps = import ./deps.nix { inherit pkgs; };
}
