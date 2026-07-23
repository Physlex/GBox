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
        ${builtins.concatStringsSep " " args.toolchain.targetFlags} \
        -DGBOX_BUILD_TEST=OFF "$@"
    }
    
    gbox-build() {
      gbox-conf "$@" \
      && ninja -C build
    }

    gbox-test() {
      gbox-conf -DGBOX_BUILD_TEST=ON "$@" \
      && ninja -C build \
      && ctest --test-dir build --output-on-failure
    }

    gbox-clean() {
      rm -rf build/
    }

    ${shellHook}
    echo "Gbox mkShell is now active";
  '';
})
