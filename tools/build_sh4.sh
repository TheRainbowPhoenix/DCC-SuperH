#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build-sh4
flags='-m4-nofpu -mb -std=c99 -O2 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror'
sh-elf-gcc $flags -Iinclude -c src/dcc.c -o build-sh4/dcc.o
sh-elf-ar rcs build-sh4/libdcc.a build-sh4/dcc.o
sh-elf-gcc $flags -Iinclude -Itests -c tests/selftest.c -o build-sh4/selftest.o
sh-elf-gcc -m4-nofpu -mb -c tests/start.S -o build-sh4/start.o
for base in ac140000 ac180000; do
    sh-elf-gcc -m4-nofpu -mb -nostdlib \
        -Wl,-T,tests/sh4.ld,--defsym,IMAGE_BASE=0x$base,-Map,build-sh4/dcc-$base.map \
        build-sh4/start.o build-sh4/selftest.o build-sh4/dcc.o -lgcc \
        -o build-sh4/dcc-$base.elf
    sh-elf-objcopy -O binary build-sh4/dcc-$base.elf build-sh4/dcc-$base.bin
    sh-elf-nm -n build-sh4/dcc-$base.elf > build-sh4/dcc-$base.sym
    test -z "$(sh-elf-nm -u build-sh4/dcc-$base.elf)"
done
