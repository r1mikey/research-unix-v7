# Research UNIX v7 for Raspberry Pi

This repository contains a port of AT&T Research UNIX v7 to the Raspberry Pi 1 single-board computer.

[![r1mikey](https://circleci.com/gh/r1mikey/research-unix-v7/tree/modernisation.svg?style=svg)](https://circleci.com/gh/r1mikey/research-unix-v7/?branch=modernisation)

Most development happens on [Qemu](https://www.qemu.org/) (version 6.0.0 or better is required).  The code is expected to gracefully degrade to a single-CPU experience on later Raspberry Pi models, up the the Pi 3.

UNIX Code Copyrights and Ownership:
* The UNIX v7 codebase is covered under the [Caldera License Grant](https://github.com/r1mikey/research-unix-v7/blob/rpi1-development/Caldera-license.pdf).
* The userland malloc implementation is taken from Research v10 UNIX. Use of this code for non-commercial purposes is allowed per the [Alcatel Lucent statement](https://github.com/r1mikey/research-unix-v7/blob/rpi1-development/statement_regarding_Unix_3-7-17.pdf).

While much of the port code is original work, portions originate in from other open-source projects:
* Userland printf is a port of [A printf / sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf) by [Marco Paland](https://github.com/mpaland), used the [MIT license](https://github.com/mpaland/printf/blob/master/LICENSE)
* Userland and kernel C runtime library support originates in [Compiler-RT](https://github.com/llvm/llvm-project/tree/master/compiler-rt/lib/builtins), a part of [LLVM Project](https://llvm.org/) and under the [Apache 2.0 with LLVM exception](https://llvm.org/LICENSE.txt) license.
* The MBR reading code in the SD card driver is derived from [SDCard.c](https://github.com/LdB-ECM/Raspberry-Pi/blob/master/SD_FAT32/SDCard.c) by Leon de Boer, auspiciously freeware under CC Attribution.
* The VFP code is based on, or inspired by, [sys/arm/arm/vfp.c](https://github.com/freebsd/freebsd/blob/master/sys/arm/arm/vfp.c) from [FreeBSD](https://www.freebsd.org/).

## Recommended Reading

* [Operating Systems: Three Easy Pieces](http://pages.cs.wisc.edu/~remzi/OSTEP/) - An outstanding resource, giving a good introduction to key operating system concepts.
* [A Commentary on the Sixth Edition UNIX Operating System](http://warsus.github.io/lions-/) - The famous "Lions book".  While this describes the previous version of Research UNIX, working through this book will give you an understanding of how Research UNIX v7 works.
* [The Design of the UNIX Operating System](https://www.amazon.co.uk/Design-UNIX-Operating-System-Prentice-Hall/dp/0132017997) - this book describes System 3 UNIX, which is quite a lot newer than Research UNIX v7, but much of the material is directly relevant.
* [The Design and Implementation of the 4.4BSD Operating System](https://download.freebsd.org/ftp/doc/en/books/design-44bsd/book.pdf) - while this book describes a much later UNIX derivative, much of the information presented here applies directly to Research UNIX v7.

## Preparing an SD Card and Image

### macOS

Insert an SD card (even a 512MiB card is sufficient).  Assuming `diskutil list` shows this at `disk2`, use the following command to partition the card:
```shell
diskutil partitionDisk disk2 4 MBR FAT32 BOOT 48M FAT32 UNIX 252M "MS-DOS FAT16" SWAP 16M "MS-DOS FAT32" SENTINEL 48M
```

Note that due to the way an MBR partition table works, the SENTINEL partition will show up as all of the remaining space on the disk.  This oddity is, in fact, the reason the partition exists at all: to bound the size of the swap partition.

Now tweak the UNIX partition type using fdisk:
```shell
sudo fdisk -e /dev/disk2
  setpid 2
  7f
  setpid 3
  7f
  quit
```
After changing the partition type, you will need to eject and reinsert the disk (ignore the dire warnings about rebooting):
```
diskutil eject disk2
```

After reinserting, the boot partition should show up at `/Volumes/BOOT`.

Copy the required firmware files over to the disk so that your Pi can boot:
```shell
curl -L -o /Volumes/BOOT/bootcode.bin \
    'https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin'
curl -L -o /Volumes/BOOT/fixup.dat \
    'https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat'
curl -L -o /Volumes/BOOT/start.elf \
    'https://github.com/raspberrypi/firmware/raw/master/boot/start.elf'
```

To aid in development, grab an image of the SD card and place it into the tools subdirectory of your source tree - this can take a long time if you have a large SD card:
```shell
sudo dd if=/dev/rdisk2 of=tools/sd.img bs=16m
```

Alternately, you can download an SD card image from the releases page of this GitHub repo.

## Build Environment Setup

### macOS

* Start by installing [Homebrew](https://brew.sh/).  This is a package manager for macOS, and simplifies installing open source software.
* Install qemu (`brew install qemu`)
* Install the *GNU Arm Embedded Toolchain* [from Arm](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).  You should install the `.pkg` version of the toolchain, which has been signed and notarised to work on modern macOS.  Once installed, you should add `/Applications/ARM/bin` to your `PATH`.

The development compiler version is currenty:
```
arm-none-eabi-gcc (GNU Arm Embedded Toolchain 10-2020-q4-major) 10.2.1 20201103 (release)
```

## Build Instructions

After installing all build dependencies, you should use the `rehash` shell builtin to be able to run the new commands.

Now that you have all the pieces in place, and assuming you placed an image of your SD card into the tools subdirectory (as described above), you're ready to build and run:

```shell
make clean all
```

Assuming you have put a properly prepared SD card image into the `tools` subdirectory, you are ready to build a runnable image:

```shell
make sdimg
```

And, finally, run the system under qemu:
```shell
make -C usr/sys run
```

If everything looks good under qemu, insert your SD card and, once again assuming that `diskutil list` shows the SD card as `disk2`, install the build to the SD card:
```shell
sudo dd if=tools/abc.fs of=/dev/rdisk2s2 bs=512k
cp usr/sys/pi1unix.bin /Volumes/BOOT/kernel.img
sync && sync && sync
diskutil eject disk2
```

Put the SD card into your Pi and connect a serial console.  Fire up `screen` before you power the Pi on (adjust the device path below as needed):
```shell
screen -L /dev/cu.usbserial-FTHAV4JP 115200,cs8,-parenb,-cstopb,-hupcl,-crtscts
```

You can use the following key sequence to leave `screen`:
```
ctrl+a
crtl+\
```

Now power your Pi on - the system will print the available memory (capped to a supported maximum value), and you'll land in a single-user shell.  Once you get into single-user mode, have a look around and, when ready, use `ctrl+D` to enter multi-user mode.  Entering multi-user mode will present a login prompt, where you can log in as `root` with a password of `root`.

There's no `shutdown` command in ancient UNIX, so you'll need to put the OS into a safe state before powering off.  To do so, leave multi-user mode, ensure all disk buffers are flushed, then exit (the same applies to qemu):
```
kill -1 1
sync && sync && sync && sync && echo safe
# remove power now
```

## Contributing

Please feel free to hack away on this code.  I'm trying to keep things reasonably close to the original codebase where there's no compelling reason to change things.  There's a lot of code that needs to be fleshed out, made robust or made less conservative, so if something catches your eye, please open up a pull request, and I'll be happy to merge any fixes and improvements.

I might, in the future, branch this port to modernise the codebase and trim out unused code - all with an eye on using this code for teaching.  If this effort interests you, please let me know.
