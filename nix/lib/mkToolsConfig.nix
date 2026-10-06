{ toolchain }:
let
  clangd = ''
    cat > ./.clangd <<EOF
    CompileFlags:
      CompilationDatabase: build
      Add:
        - -I${toolchain.llvm.llvm.dev}/include
        - -I${toolchain.llvm.libclang.dev}/include
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
      readability-*,
      cppcoreguidelines-*,
      clang-analyzer-*,
      -bugprone-easily-swappable-parameters,
      -bugprone-crtp-constructor-accessibility,
      -readability-identifier-length,
      -readability-else-after-return,
      -cppcoreguidelines-avoid-const-or-ref-data-members,
      -cppcoreguidelines-pro-type-vararg,
      -cppcoreguidelines-pro-bounds-pointer-arithmetic,
      -cppcoreguidelines-pro-bounds-constant-array-index,
      -cppcoreguidelines-pro-type-union-access,
      -cppcoreguidelines-special-member-functions,
      -readability-redundant-declaration,
      -cppcoreguidelines-non-private-member-variables-in-classes,

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
