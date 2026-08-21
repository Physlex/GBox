# The third-party packages the workspace builds against, beyond what a gbox toolchain carries.
# Add a dependency here to give it to the package and the development shell at once.
{ pkgs }:
{
  buildInputs = [ pkgs.sdl3 ];
  nativeBuildInputs = [ ];
}
