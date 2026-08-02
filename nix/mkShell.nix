{ pkgs }: args:
let
  nativeBuildInputs =
    args.nativeBuildInputs or []
    ++ args.toolchain.nativeBuildInputs or []
    ++ [pkgs.watchexec args.toolchain.llvm.clang-tools];
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

    gbox-watch-clean() {
      gbox-watch-alive && kill "$(<"$WATCH_PIDFILE")" 2>/dev/null
      rm -f "$WATCH_PIDFILE"
    }

    WATCH_PIDFILE="$PWD/gbox.watch.pid"
    WATCH_LOG="$PWD/gbox.watch.log"
    WATCH_PID=""

    # True when the recorded watcher (from the pidfile) is still alive.
    gbox-watch-alive() {
      local PID=""
      [ -s "$WATCH_PIDFILE" ] && PID=$(<"$WATCH_PIDFILE")
      [ -n "$PID" ] && kill -0 "$PID" 2>/dev/null
    }

    if gbox-watch-alive; then
      echo "Watcher already running. Not spawning."
    elif [ -s "$WATCH_PIDFILE" ]; then
      echo "Stale watcher pidfile. Restarting the watcher."
    fi

    if ! gbox-watch-alive; then
      # watchexec runs the command in a child bash; export the build functions
      # so that child sees them, keeping gbox-build the single source of truth.
      export -f gbox-conf gbox-build
      watchexec \
        --on-busy-update queue \
        -e c,cc,cpp,cxx,cppm,ixx,h,hh,hpp,hxx \
        --filter CMakeLists.txt \
        -- gbox-build >> "$WATCH_LOG" 2>&1 &
      WATCH_PID=$!
      printf '%s\n' "$WATCH_PID" > "$WATCH_PIDFILE"
    fi

    trap '[ -n "$WATCH_PID" ] && gbox-watch-clean' EXIT

    ${shellHook}
  '';
})
