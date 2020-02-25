OBJECTS = gcd.o \
          madd.o \
          mdiv.o \
          mout.o \
          msqrt.o \
          mult.o \
          pow.o \
          util.o

TARGET = libmp.a

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

SUBDIR_LEVEL := ../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
