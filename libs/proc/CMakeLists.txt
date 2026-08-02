gbox_library(proc NO_MODULES)

gbox_import(llvm PACKAGE LLVM)

gbox_import(clang
  PACKAGE Clang
  LINK clangAST clangTooling clangBasic clangFrontend
)

gbox_dependencies(core)
