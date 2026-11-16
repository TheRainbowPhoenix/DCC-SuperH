# DCC-lite

Portable C99 DCC library for SuperH 4 (and more !)

## SuperH

Inside the Gint Docker container:

```sh
fxsdk build-cp
```

This creates `DCC-SuperH.hh3`. For the freestanding SH4 library and test executables:

```sh
sh tools/build_sh4.sh
# run this on your emulator
```

## Library

Compile `src/dcc.c`, keep `src/dcc_tables.h` beside it, and add `include/`
to the include path. The SH4 archive is `build-sh4/libdcc.a`.

```c
uint8_t stream[16];
if (dcc_keystream(key, key_length, stream) == 0)
    dcc_xor(stream, input, output, length);
```

Keys contain 1..16 bytes. 

## Tables

```sh
python tools/generate_tables.py
```

Generate `src/dcc_tables.h` using binary32-rounded `sin(i/128)`, plus a special 0 value exception.
Tables are included in the repository, so building C does not require Python.

## Python and native builds

```sh
python python/dcc.py CFFCF4F4DB32E0525AF99698A4F00092
cmake -S windows -B build-native
cmake --build build-native
```

The native build works on Windows and Linux. The CLI prints a stream or
transforms a file with `dcc HEXKEY INPUT OUTPUT`.

Test key: `CFFCF4F4DB32E0525AF99698A4F00092`
Expected stream: `1B1D1B1D1E561E56218C218C24BD24BD`

Compare the native CLI and Python reference with:

```sh
python tests/run_native.py build-native/dcc.exe
```