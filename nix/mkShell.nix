{ pkgs }: args:
let
  nativeBuildInputs = args.nativeBuildInputs or [] ++ args.toolchain.nativeBuildInputs or [];
  buildInputs = args.buildInputs or [] ++ args.toolchain.buildInputs or [];
  shellHook = args.shellHook or "";
in
pkgs.mkShell ((builtins.removeAttrs args [
  "toolchain" "extraPackages" "shellHook" "nativeBuildInputs" "buildInputs"
]) // {
  inherit nativeBuildInputs buildInputs;

  shellHook = ''
    ${args.toolchain.exports}
    export PS1="(gbox:${args.toolchain.target}) $PS1"

    gbox-conf() {
      cmake -B build -S . -G Ninja \
        ${builtins.concatStringsSep " " args.toolchain.targetFlags} "$@"
    }

    gbox-build() {
      ninja -C build
    }

    gbox-clean() {
      rm -rf build/
    }

    ${shellHook}
    echo "Gbox mkShell is now active";
  '';
})
