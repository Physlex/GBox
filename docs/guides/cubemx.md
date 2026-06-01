It is possible to invoke stm32cubemx from a devshell, but it's a little weird.

```
DISPLAY=:1 nix develop ./examples/stm32f446 --command stm32cubemx &
```

Which will invoke an instance of cubemx in non-headless mode.
