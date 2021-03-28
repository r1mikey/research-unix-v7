CURRENT_OS := $(strip $(shell uname -s))

ifeq ($(CURRENT_OS),Darwin)
CROSS_COMPILE ?= arm-none-eabi-
else ifeq ($(CURRENT_OS),Linux)
LINARO_DIR := $(shell (cd /opt && ls -1d gcc-linaro-*arm-eabi | tail -1))
ifeq ($(LINARO_DIR),)
CROSS_COMPILE ?= arm-eabi-
else
CROSS_COMPILE ?= /opt/$(LINARO_DIR)/bin/arm-eabi-
endif
else
$(error $(CURRENT_OS) is not a supported OS)
endif

CC = $(CROSS_COMPILE)gcc
CCLD = $(CROSS_COMPILE)gcc
#CC = clang -target armv6-none-eabi
AS = $(CROSS_COMPILE)as
CCAS = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB = $(CROSS_COMPILE)ranlib
SIZE = $(CROSS_COMPILE)size
NM = $(CROSS_COMPILE)nm
MKAOUT ?= $(SUBDIR_LEVEL)/tools/mkaout.py
YACC = $(SUBDIR_LEVEL)/usr/src/cmd/yacc/yacc.native

KERNEL_ARCH_FLAGS ?= -marm -march=armv6zk -mtune=arm1176jzf-s -mabi=aapcs
ARCH_FLAGS ?= -marm -march=armv6zk -mfpu=vfp -mtune=arm1176jzf-s -mabi=aapcs -mfloat-abi=hard
KERNEL_OPTIM_FLAGS ?= -ggdb
OPTIM_FLAGS ?= -ggdb
CPPFLAGS ?= -I$(SUBDIR_LEVEL)/usr/include
# -fstack-usage option and -fcallgraph-info
# -Wall -pedantic
CFLAGS ?= -std=c89 -ffreestanding -nostdlib -nostdinc -nostartfiles $(ARCH_FLAGS) $(OPTIM_FLAGS)
ASFLAGS ?= -ggdb -march=armv6zk
LDFLAGS ?= $(CPPFLAGS) $(CFLAGS) -Wl,--build-id=none -T$(SUBDIR_LEVEL)/tools/cinit.lds
LOADLIBES ?= -L$(SUBDIR_LEVEL)/usr/src/libc
LDLIBS ?= -lc

CPP11SPECIALFLAGS = -I$(SUBDIR_LEVEL)/usr/include
C11SPECIALFLAGS = -std=c11 -ffreestanding -nostdlib -nostartfiles $(ARCH_FLAGS) $(OPTIM_FLAGS)

KERNEL_LOADLIBES =
KERNEL_CPPFLAGS = -DKERNEL -DBCM283X
KERNEL_LDLIBS =
KERNEL_CFLAGS = -Wall -std=c11 -ffreestanding -nostdlib -nostartfiles $(KERNEL_ARCH_FLAGS) $(KERNEL_OPTIM_FLAGS)
KERNEL_LDFLAGS = $(KERNEL_CPPFLAGS) $(KERNEL_CFLAGS) -Wl,--build-id=none -Wl,--defsym,PROGRAM_ENTRY_OFFSET=$(PROGRAM_ENTRY_OFFSET) -Wl,--defsym,KERNVIRTADDR=$(KERNVIRTADDR) -Wl,-N
