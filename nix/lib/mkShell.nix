{ pkgs }: args:
let
  nativeBuildInputs =
    args.nativeBuildInputs or []
    ++ args.toolchain.nativeBuildInputs or []
    ++ [pkgs.watchexec pkgs.python3 args.toolchain.llvm.clang-tools];
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

    # clang-tidy runs the frontend unwrapped, so BMIs built with the wrapper's hardening
    # flags fail to load against it.
    export NIX_HARDENING_ENABLE=""

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

    gbox-tidy() {
      local CONF_ARGS=() FILES=() FILTER=() MODULES=() TIDY_ARGS=() ROOT="" MODULE_PATH="" TESTS=0
      local REGEX_ERROR="" PATTERN="" ALTERNATION="" JOINED=""
      local REGEX_CHECK='import re, sys
try:
    re.compile(sys.argv[1])
except re.error as exc:
    sys.exit(str(exc))'

      while [ "$#" -gt 0 ]; do
        case "$1" in
          --tests)
            CONF_ARGS+=(-DGBOX_BUILD_TEST=ON)
            TESTS=1
            ;;
          -m)
            if [ "$#" -lt 2 ]; then
              echo "gbox tidy: no name given to \`-m\`, name a directory under libs/ or apps/." >&2
              return 1
            fi
            shift

            case "$1" in
              "" | "." | ".." | */*)
                echo "gbox module $1: not a module name, name a directory under libs/ or apps/ by its own name." >&2
                return 1
                ;;
            esac

            MODULE_PATH=""
            for ROOT in libs apps; do
              if [ -d "$ROOT/$1" ]; then
                MODULE_PATH="$ROOT/$1"
                break
              fi
            done

            if [ -z "$MODULE_PATH" ]; then
              echo "gbox module $1: no directory at libs/$1 or apps/$1, name a directory under libs/ or apps/." >&2
              return 1
            fi

            # The trailing slash keeps the module name from matching a longer sibling.
            MODULES+=("$MODULE_PATH/")
            ;;
          --)
            # Taken verbatim, so run-clang-tidy's own options stay reachable without this
            # function having to know them.
            shift
            TIDY_ARGS+=("$@")
            break
            ;;
          -*)
            echo "gbox tidy: unknown option \`$1\`, pass \`--tests\`, \`-m <module>\`, \`--\`, or a path." >&2
            return 1
            ;;
          *)
            # The positionals are compiled as a python regex downstream, where an unparsable
            # one raises a traceback -- and only once the build has finished.
            if ! REGEX_ERROR=$(python3 -c "$REGEX_CHECK" "$1" 2>&1); then
              echo "gbox tidy: \`$1\` is not a valid regex, $REGEX_ERROR." >&2
              return 1
            fi

            FILES+=("$1")
            ;;
        esac
        shift
      done

      if [ "''${#FILES[@]}" -gt 0 ]; then
        JOINED=$(printf '%s|' "''${FILES[@]}")

        if ! REGEX_ERROR=$(python3 -c "$REGEX_CHECK" "''${JOINED%|}" 2>&1); then
          echo "gbox tidy: the paths given are not a valid regex together, $REGEX_ERROR." >&2
          return 1
        fi
      fi

      gbox-build "''${CONF_ARGS[@]}" || return

      if [ "$TESTS" -eq 1 ]; then
        PATTERN='(?=.*/test/)'
      else
        PATTERN='(?!.*/test/)'
      fi

      if [ "''${#MODULES[@]}" -gt 0 ]; then
        ALTERNATION=$(printf '%s|' "''${MODULES[@]}")
        PATTERN="$PATTERN(?=.*(''${ALTERNATION%|}))"
      fi

      FILTER=(-source-filter "$PATTERN.*")

      # Each positional is matched against the source paths in the database, so a path
      # relative to the workspace root selects the unit it names. None means all of them.
      PYTHONUNBUFFERED=1 \
      run-clang-tidy -p build -quiet -j "$(nproc)" "''${FILTER[@]}" "''${TIDY_ARGS[@]}" "''${FILES[@]}" 2>&1 \
        | grep --line-buffered -vE '^[0-9]+ warnings? generated\.$'

      return "''${PIPESTATUS[0]}"
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
