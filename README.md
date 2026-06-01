# GraniteBox

## Overview

This project defines the GraniteBox micro-kernel linux-as-a-framework application development kit.

That's a lot of keywords, so let's break it down into some specifics.


### GraniteBox

GraniteBox (or GBox) is an application development toolkit used to implement cross-platform applications, using an *honest* open-source software license, and without the restrictions that come typical of other equivalent modern application development kits.

For justification, see: `docs/justification.md`


### Requirements

TODO: Docs on NIX, DIRENV, and CLANGD

Note: If running via linux, user namespaces must be enabled via:

```
echo 1 | sudo tee /proc/sys/kernel/unprivileged_userns_clone
echo "kernel.unprivileged_userns_clone=1" | sudo tee -a /etc/sysctl.d/99-nix-sandbox.conf
```

### Execution

run:

```bash
$ cmake -B build -S .
$ cmake --build build
$ ./build/bin/gbox-cli
```


### TODO: Testing
