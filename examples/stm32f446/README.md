# Overview

Simple example of using GBox to scaffold an stm32f4 project.

## Description

TODO: DOCS

## Requirements

All requirements are provided by the nix flake. Just run nix build .# to get access to the
binary.

### Note
While running the nix build .# command, it's possible to fail to invoke the stm32cubemx
tool due to a conflict between nix's bubblewrap and the host environment.

To solve this problem, run the script in `scripts/apparmor_fix.sh`. Then re-run nix build .#

## Flashing to the stm32

TODO: DOCS
