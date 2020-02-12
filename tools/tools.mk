CURRENT_OS := $(strip $(shell uname -s))
HAS_CARLSON_MINOT_USR_LOCAL := $(shell test -d /usr/local/carlson-minot/crosscompilers/bin && echo "yes" || echo "no")
HAS_CARLSON_MINOT_IN_HOME := $(shell test -d $(HOME)/Downloads/usr/local/carlson-minot/crosscompilers/bin && echo "yes" || echo "no")

ifeq ($(CURRENT_OS),Darwin)
ifeq ($(HAS_CARLSON_MINOT_USR_LOCAL),yes)
COMPILERS_DIR ?= /usr/local/carlson-minot/crosscompilers/bin
else
ifeq ($(HAS_CARLSON_MINOT_IN_HOME),yes)
COMPILERS_DIR ?= $(HOME)/Downloads/usr/local/carlson-minot/crosscompilers/bin
else
$(error Could not find a Carlson Minot cross compiler in either /usr/local/carlson-minot/crosscompilers/bin or $(HOME)/Downloads/usr/local/carlson-minot/crosscompilers/bin)
endif
endif
COMPILERS_PREFIX ?= arm-none-eabi-
else ifeq ($(CURRENT_OS),Linux)
ifeq ($(COMPILERS_DIR),)
LINARO_DIR := $(shell (cd /opt && ls -1d gcc-linaro-*arm-eabi | tail -1))
ifeq ($(LINARO_DIR),)
$(error Could not find a Linaro gcc cross compiler for arm-eabi in /opt)
endif
COMPILERS_DIR ?= /opt/$(LINARO_DIR)/bin
endif
COMPILERS_PREFIX ?= arm-eabi-
else
$(error $(CURRENT_OS) is not a supported OS)
endif

CROSS_COMPILE ?= $(COMPILERS_DIR)/$(COMPILERS_PREFIX)

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
YACC = $(SUBDIR_LEVEL)/tools/yacc/yacc

CPPFLAGS ?= -I$(SUBDIR_LEVEL)/usr/include
# -fstack-usage option and -fcallgraph-info
# -Wall -pedantic
CFLAGS ?= -std=c89 -ffreestanding -nostdlib -nostdinc -nostartfiles -marm -march=armv6k -mabi=aapcs -mfloat-abi=hard -ggdb
ASFLAGS ?= -ggdb -march=armv6k
LDFLAGS ?= $(CPPFLAGS) $(CFLAGS) -Wl,--build-id=none -T$(SUBDIR_LEVEL)/tools/cinit.lds
LOADLIBES ?= -L$(SUBDIR_LEVEL)/usr/src/libc
LDLIBS ?= -lc

CPP11SPECIALFLAGS =
C11SPECIALFLAGS = -std=c11 -ffreestanding -nostdlib -nostartfiles -marm -march=armv6k -mabi=aapcs -mfloat-abi=hard -ggdb

KERNEL_LOADLIBES =
KERNEL_CPPFLAGS =
KERNEL_LDLIBS =
KERNEL_CFLAGS = -std=c89 -ffreestanding -nostdlib -nostartfiles -marm -march=armv6k -mabi=aapcs -ggdb
KERNEL_C11SPECIALFLAGS = -std=c11 -ffreestanding -nostdlib -nostartfiles -marm -march=armv6k -mabi=aapcs -ggdb
KERNEL_LDFLAGS = $(KERNEL_CPPFLAGS) $(KERNEL_CFLAGS) -Wl,--build-id=none -Wl,--defsym,PROGRAM_ENTRY_OFFSET=$(PROGRAM_ENTRY_OFFSET) -Wl,--defsym,KERNVIRTADDR=$(KERNVIRTADDR)
