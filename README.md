# Research UNIX v7 for Raspberry Pi

NOTE: This documentation is woefully incomplete.

This repository contains a port of AT&T Research UNIX v7 to the Raspberry Pi 1 single-board computer.

Most development happens on [Qemu](https://www.qemu.org/), so the code is expected to gracefully degrade to a single-CPU experience on later Raspberry Pi models.

UNIX Code Copyrights and Ownership:
* The UNIX v7 codebase is covered under the [Caldera License Grant](https://github.com/r1mikey/research-unix-v7/blob/rpi1-development/Caldera-license.pdf).
* The userland malloc implementation is taken from Research v10 UNIX. Use of this code for non-commercial purposes is allowed per the [Alcatel Lucent statement](https://github.com/r1mikey/research-unix-v7/blob/rpi1-development/statement_regarding_Unix_3-7-17.pdf).

While much of the port code is original work, portions originate in from other open-source projects:
* Userland printf is a port of [A printf / sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf) by [Marco Paland](https://github.com/mpaland), used the [MIT license](https://github.com/mpaland/printf/blob/master/LICENSE)
* Userland and kernel C runtime library support originates in [Compiler-RT](https://github.com/llvm/llvm-project/tree/master/compiler-rt/lib/builtins), a part of [LLVM Project](https://llvm.org/) and under the [Apache 2.0 with LLVM exception](https://llvm.org/LICENSE.txt) license.
* The SD card driver is derived from [SDCard.c](https://github.com/LdB-ECM/Raspberry-Pi/blob/master/SD_FAT32/SDCard.c) by Leon de Boer, auspiciously freeware under CC Attribution (see the file header for details)

## Recommended Reading

* [A Commentary on the Sixth Edition UNIX Operating System](http://warsus.github.io/lions-/) - The famous "Lions book".  While this describes the previous version of Research UNIX, working through this book will give you an understanding of how Research UNIX v7 works.
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

## Interim Build Instructions

I've been using the (now defunct) GCC cross toolchain from Carlson Minot to build on macOS.  Building on Linux should be a lot easier.  Obtain the Carlson Minot toolchain from [this GitHub mirror](https://github.com/mathworks/Carlson-Minot-G-Lite/releases) and install it to `/usr/local/carlson-minot` (so that `/usr/local/carlson-minot/crosscompilers/bin` exists).

Unfortunately, the GDB shipped with the Carlson Minot toolchain is a bit crashy, so I've been using the GDB from *GNU Tools for Arm Embedded Processors*.  This toolchain only targets M series CPUs, but the debugger is still usable for armv6k and armv7a.  You can install this using [Homebrew](https://brew.sh/) - the cask name is `gcc-arm-embedded`.

And, finally, you'll want qemu.  You should install qemu using Homebrew (`brew install qemu`).

After installing all build dependencies, you should use the `rehash` shell builtin to be able to run the new commands.

Now that you have all the pieces in place, and assuming you placed an image of your SD card into the tools subdirectory (as described above), you're ready to build and run.

Start by building libc:
```shell
make -C usr/src/libc clean all
```
Now build userland commands:
```shell
make -C usr/src/cmd clean all
```
Next up, the kernel:
```shell
make -C usr/sys clean all
```
You could run all of the preceding commands can be run with the parallel build (`-j N`) option to speed them up.

Built the UNIX partition image and copy it into the SD card image:
```shell
(cd tools && ./buildfs && sync)
dd if=tools/abc.fs of=tools/sd.img bs=512 seek=96256 conv=notrunc
sync
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
