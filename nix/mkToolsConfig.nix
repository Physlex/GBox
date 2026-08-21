{ gbpkgs }:
let
  clangd = ''
    cat > ./.clangd <<EOF
    CompileFlags:
      CompilationDatabase: build
      Add:
        - -I${gbpkgs.gbox.toolchain.llvm.llvm.dev}/include
        - -I${gbpkgs.gbox.toolchain.llvm.libclang.dev}/include
    Index:
      Background: Build
    EOF
  '';
  clangTidy = ''
    cat > ./.clang-tidy <<EOF
    Checks: >
      -*,
      bugprone-*,
      performance-*,
      modernize-*,
      readability-*,
      cppcoreguidelines-*,
      clang-analyzer-*,
      -modernize-use-trailing-return-type,
      -readability-redundant-declaration

    CheckOptions:
      readability-identifier-length.MinimumVariableNameLength: 2

    HeaderFilterRegex: '(apps|examples|libs)/.*'

    WarningsAsErrors: '*'
    EOF
  '';
in {
  config = ''
    ${clangd}
    ${clangTidy}
  '';
}
