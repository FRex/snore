# snore

A small C program that works like `sleep` but prints 1 dot a second, and a
newline after every 60th second/dot. Go to releases to download a 32-bit Windows exe or 64-bit statically linked Linux executable (made with musl).

Pass `--countdown` to use a single changing countdown number instead of dots.
If the stdout is not interactive (not a tty as checked by `isatty`) then `--countdown`
will print one line per second instead.
