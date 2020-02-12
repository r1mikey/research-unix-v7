# Research UNIX v7 for Raspberry Pi

This repository contains a port of AT&T Research UNIX v7 to the Raspberry Pi 1 single board computer.

Most development is performed on [Qemu](https://www.qemu.org/), so the code is expected to gracefully degrade to a single-CPU experience on later Raspberry Pi models.

While much of the port code is original work, portions are sourced from other open-source projects:
* Userland printf is a port of [mpaland](https://github.com/mpaland/printf) printf, user the [MIT license](https://github.com/mpaland/printf/blob/master/LICENSE)
* Userland and kernel C runtime library support is imported from the [Compiler-RT codebase](https://github.com/llvm/llvm-project/tree/master/compiler-rt/lib/builtins), a part of [LLVM Project](https://llvm.org/) and under the [Apache 2.0 with LLVM exception](https://llvm.org/LICENSE.txt) license.
* The SD card driver is derived from [SDCard.c](https://github.com/LdB-ECM/Raspberry-Pi/blob/master/SD_FAT32/SDCard.c) by Leon de Boer, auspiciously freeware under CC Attribution (see the file header for details)

There's a lot more to write about how to compile this and how to get it running...
