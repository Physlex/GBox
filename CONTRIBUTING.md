# Contributing to GBox

This document describes the necessary information required to contribute to the gbox
application development kit.

Before reading this doc, it's recommended you read the README.md avaliable in the root
directory.

## Nix

GBox uses Nix in an attempt to simplify cross-platform building and environmental setup,
as well as simplify C++ package management. It also provides a wrapper to bash, linux
only, which HEAVILY simplifies development.

Skipping use of the development shell is not only *not reccomended*, __it's impossible__.
Gbox was built with the intention of heavily relying on nix for hermetic builds.

### Devshell

Enter the development shell using nix, with the following command from root:

```bash
nix develop
```

This installs all of the necessary system libraries, hooks up the linking paths, and sets
your std env to gbox's.

## Code style

C and C++ formatting is enforced by `.clang-format`; the pre-commit hook runs
`clang-format -i` on staged C/C++ files. IDE support comes from the nix-generated
`.clangd`, and the nix devshell.

## CMake Diagnostics

Every `message()` raised by the gbox build system follows one format, so that build output
is greppable and reads the same regardless of which macro produced it:

```
gbox [<operation>] <subject>: <description>, <remedy/reason>.
```

### Operations

The subject names what the message is about. The operation, when present, names what the
build system was doing to it as in `gbox import <library>`. No parentheses on either.

| Operation | Meaning |
| --- | --- |
| `module` | Module directory discovery |
| `import` | Third-party package resolution |
| `link` | Dependency name resolution |

### Description

Lowercase fact, backticks around non-path identifiers.

### Remedies and Reasons

Remedy after a comma, required on `FATAL_ERROR`. On `STATUS` it may state the consequence
instead. Every message ends in a period.

`DEBUG` messages are a local debugging aid and must never reach `main`. Strip them before
you commit.

### Multi-Line Messages

When the text exceeds the line width, split it across multiple string arguments to
`message()`. CMake concatenates them with no separator, so keep the trailing space at the
end of each fragment:

```cmake
message(
  FATAL_ERROR
  "gbox import ${IMPORT_NAME}: COMPONENTS given without PACKAGE, components can only "
  "be requested from a package."
)
```

### Examples

This message breaks every rule. The prefix is missing, the operation wears parentheses, the
description is capitalized and leaves `<target>` un-backticked, and no remedy follows:

```
Import(<library>) No target named <target>
```

The same message, breaking none:

```
gbox import <library>: no target named `<target>`, add it to the LINK list.
```
