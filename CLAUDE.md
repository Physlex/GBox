# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

GBox (GraniteBox) is a general embedded systems development toolkit built in C/C++. It provides:
- A Rust-style `Result<T, E>` error propagation system
- A lightweight async task scheduler and runtime (written in C)
- A procedural macro system for C++ via a clang wrapper tool (`gbclang`)

The long-term direction is toward an embedded systems framework, but in practice it is a personal monorepo of reusable libraries for C/C++ projects.

## Architecture

The repo is split into `libs/` (hardware-agnostic libraries) and `apps/` (tools and compiler driver wrappers run by a developer). Hardware-specific libraries do not yet have a designated location in the tree.

The CMake macros in `cmake/AddModule.cmake` auto-discover `src/`, `inc/`, and `test/` subdirectories for each module,
enabling a monorepository architecture similar to existing build systems. (Cargo, pyproject, etc...)

### `gbox-core`

This library implements and defines various hardware agnostic stack-or-static type definitions.

Some examples of provided utilities includes:
  - A rust-style result type for error handling in C++.
  - Redefinitions of type specifiers to always be explicit about memory size, such as float32_t, and float64_t

### `gbox-runtime`

Asynchronous runtime system and some primitives for single-threaded embedded systems.

Some examples of provided utilities includes:
  - A fifo function queue
  - timers and a core application loop
  - spawners

### `gbox-proc`

Clang AST tooling that implements a cpp procedural macro system. Two main areas:
  - Parsing to a custom concrete syntax tree (CST)
  - Replacing code defined in existing files with virtual procedural files

See `examples` for usage demonstrations of all of the above.

### `apps/gbclang`

A drop-in replacement for `clang`/`clang++` that intercepts compilation, applies `gbox-proc` transformations to source files, then delegates to the real compiler.
It strips its own `gb` prefix to determine which real compiler to invoke and extracts `-emit-obj` jobs to apply AST rewrites.

## Code Style

Enforced by `.clang-format`. The pre-commit hook runs `clang-format -i` automatically on staged C/C++ files.
IDE support via `.clangd`.
