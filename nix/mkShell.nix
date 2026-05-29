{ pkgs, lib, packages }: args:
pkgs.mkShell (args // {
  packages = packages ++ (args.packages or []);

  shellHook = ''
    ${builtins.concatStringsSep "\n" lib.toolchain.cmake.exports}

    configure() {
      cmake -B build -S . -G Ninja \
        ${builtins.concatStringsSep " " lib.toolchain.cmake.flags}
    }

    ${args.shellHook or ""}
  '';
})
