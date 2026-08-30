gbox_library(proc)

gbox_import(llvm PACKAGE LLVM)

gbox_import(clang
  PACKAGE Clang
  LINK clangAST clangTooling clangBasic clangFrontend
)

gbox_dependencies(core func-ky)
