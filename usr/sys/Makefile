#
# ~/clang10/clang+llvm-10.0.0-x86_64-apple-darwin/bin/scan-build --use-cc=arm-none-eabi-gcc --analyzer-target=armv6-none-eabi make
# ~/clang10/clang+llvm-10.0.0-x86_64-apple-darwin/bin/scan-view /var/folders/vl/9jhlz8815fv4kj_zvzkf6gjw0000gn/T/scan-build-2020-06-18-224732-59603-1
#
# For formatting, go file-by-file and:
#  ~/clang11/clang+llvm-11.0.0-x86_64-apple-darwin/bin/clang-format -style=file -i main.c
# Now check with 'git diff'
# To suppress formatting in a block, start with:
#  /* clang-format off */
#  ...
#  /* clang-format on */
#
#
SYSDIR := $(shell pwd)

VPATH = bcm283x/rt:bcm283x/dev:bcm283x/conf:bcm283x:dev:sys

PROGRAM_ENTRY_OFFSET ?= 0x8000
KERNVIRTADDR ?= 0x61609000
# GDB = $(CROSS_COMPILE)gdb --init-command=bcm283x/gdb-commands.txt --tui --quiet
GDB ?= arm-none-eabi-gdb-py --init-command=bcm283x/gdb-commands.txt --tui --quiet
QEMU ?= qemu-system-arm

V7_DEV_OBJECTS = \
                 mem.o \
                 sys.o

V7_SYS_OBJECTS = acct.o \
                 alloc.o \
                 bio.o \
                 clock.o \
                 fio.o \
                 iget.o \
                 main.o \
                 malloc.o \
                 nami.o \
                 partab.o \
                 pipe.o \
                 prf.o \
                 prim.o \
                 rdwri.o \
                 sig.o \
                 slp.o \
                 subr.o \
                 sys1.o \
                 sys2.o \
                 sys3.o \
                 sys4.o \
                 sysent.o \
                 text.o \
                 tty.o

V7_OBJECTS = $(V7_DEV_OBJECTS) $(V7_SYS_OBJECTS)

PORT_RT_NORMAL_OBJECTS = aeabi_idivmod.o \
                  aeabi_uidivmod.o \
                  aeabi_uldivmod.o \
                  divmodsi4.o \
                  divsi3.o \
                  udivmodsi4.o \
                  udivsi3.o
PORT_RT_SPECIAL_OBJECTS = udivmoddi4.o
PORT_RT_OBJECTS = $(PORT_RT_NORMAL_OBJECTS) $(PORT_RT_SPECIAL_OBJECTS)

PORT_DEV_NORMAL_OBJECTS = bcm283x_systimer_asm.o
PORT_DEV_SPECIAL_OBJECTS = bcm283x_gpio.o \
                           bcm283x_io.o \
                           bcm283x_irq.o \
                           bcm283x_mbox.o \
                           bcm283x_pl011.o \
                           arasan_sd.o \
                           sd_io.o \
                           sdx.o \
                           bcm283x_systimer.o
PORT_CONF_NORMAL_OBJECTS = 
PORT_CONF_SPECIAL_OBJECTS = bcm283x_conf.o

PORT_SYS_NORMAL_OBJECTS = startup.o \
                          exception.o \
                          trap_exit.o \
                          trap_irq.o \
                          trap_und.o \
                          trap_dabt.o \
                          trap_pabt.o \
                          trap_swi.o \
                          arm1176jzfs.o \
                          vfp_asm.o
PORT_SYS_SPECIAL_OBJECTS = bcm283x_machdep.o \
                           page_tables.o \
                           traps.o \
                           udot.o \
                           undefined.o \
                           vfp.o

PORT_NORMAL_OBJECTS = $(PORT_RT_NORMAL_OBJECTS) $(PORT_DEV_NORMAL_OBJECTS) $(PORT_CONF_NORMAL_OBJECTS) $(PORT_SYS_NORMAL_OBJECTS)
PORT_SPECIAL_OBJECTS = $(PORT_RT_SPECIAL_OBJECTS) $(PORT_DEV_SPECIAL_OBJECTS) $(PORT_CONF_SPECIAL_OBJECTS) $(PORT_SYS_SPECIAL_OBJECTS)

PORT_OBJECTS = $(PORT_NORMAL_OBJECTS) $(PORT_SPECIAL_OBJECTS)

OBJECTS = $(sort $(PORT_OBJECTS) $(V7_OBJECTS))

$(V7_OBJECTS): LOADLIBES = $(KERNEL_LOADLIBES)
$(V7_OBJECTS): CPPFLAGS = $(KERNEL_CPPFLAGS)
$(V7_OBJECTS): CFLAGS = $(KERNEL_CFLAGS)
$(V7_OBJECTS): LDLIBS = $(KERNEL_LDLIBS)
$(V7_OBJECTS): LDFLAGS = $(KERNEL_LDFLAGS)

$(PORT_OBJECTS): LOADLIBES = $(KERNEL_LOADLIBES)
$(PORT_OBJECTS): CPPFLAGS = $(KERNEL_CPPFLAGS)
$(PORT_NORMAL_OBJECTS): CFLAGS = $(KERNEL_CFLAGS)
$(PORT_SPECIAL_OBJECTS): CFLAGS = $(KERNEL_CFLAGS)
$(PORT_OBJECTS): LDLIBS = $(KERNEL_LDLIBS)
$(PORT_OBJECTS): LDFLAGS = $(KERNEL_LDFLAGS)

TARGET_NAME ?= pi1unix
LINKER_SCRIPT ?= bcm283x/$(TARGET_NAME).lds
TARGET_BIN ?= $(TARGET_NAME).bin
TARGET_ELF ?= $(TARGET_NAME).elf
TARGET_AOUT ?= $(TARGET_NAME).aout
TARGET_LST ?= $(TARGET_NAME).lst

SD_IMG ?= $(SUBDIR_LEVEL)/tools/sd.img

all: $(TARGET_BIN) $(TARGET_AOUT)

debug: $(TARGET_BIN) $(TARGET_ELF) bcm283x/gdb-commands.txt
	$(GDB)

rundebug: $(TARGET_ELF)
	@echo "Use 'ctrl+a, x' to leave qemu, 'ctrl+a, ?' for a menu"
	$(QEMU) -M raspi2 -m 1G -nographic -s -S -d guest_errors -drive file=$(SD_IMG),format=raw,if=sd -device loader,file=$(TARGET_ELF),cpu-num=0

run: $(TARGET_ELF)
	@echo "Use 'ctrl+a, x' to leave qemu, 'ctrl+a, ?' for a menu"
	$(QEMU) -M raspi2 -m 1G -d guest_errors -nographic -drive file=$(SD_IMG),format=raw,if=sd -device loader,file=$(TARGET_ELF),cpu-num=0

$(TARGET_ELF): $(LINKER_SCRIPT) $(OBJECTS) Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(OBJECTS) $(LOADLIBES) $(LDLIBS)
	$(SIZE) $(TARGET_ELF)

$(TARGET_LST): $(TARGET_ELF)
	$(OBJDUMP) -D -M reg-names-raw $(TARGET_ELF) > $(TARGET_LST)

$(TARGET_AOUT): $(TARGET_BIN) $(TARGET_ELF)
	$(MKAOUT) -s pi1unix $(shell $(SIZE) $(word 2, $^) | tail -1 | awk '{ printf "%d %d %d", $$1, $$2, $$3; }')

# --only-section=.text --only-section=.rodata --only-section=.data
$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary --only-section=.text --only-section=.data $(TARGET_ELF) $(TARGET_BIN)

$(TARGET_ELF): CFLAGS = $(KERNEL_CFLAGS)
$(TARGET_ELF): CPPFLAGS = $(KERNEL_CPPFLAGS)
$(TARGET_ELF): LDLIBS = $(KERNEL_LDLIBS)
$(TARGET_ELF): LDFLAGS = $(KERNEL_LDFLAGS)
$(TARGET_ELF): LOADLIBES = $(KERNEL_LOADLIBES)

SUBDIR_LEVEL := ../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
