# This file implements a set of build alias' for the build system
# 
# TODO: Remove the makefile for our nix flake stuffs

format:
	cmake --build build --target=format	

clean-examples:
	rm -rf build/examples

.PHONY: format clean-examples
