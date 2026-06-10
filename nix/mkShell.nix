{ pkgs, lib, packages, system }: args:
pkgs.mkShell (args // {
  packages = packages ++ (args.packages or []);

  shellHook = ''
    ${builtins.concatStringsSep "\n" lib.toolchain.cmake.exports}

    configure() {
      cmake -B build -S . -G Ninja \
        ${builtins.concatStringsSep " " (lib.toolchain.cmake.flagsFor system)}
    }

    ${args.shellHook or ""}
  '';
})
