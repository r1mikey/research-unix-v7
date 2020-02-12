OBJECTS = args.o \
          blok.o \
          builtin.o \
          cmd.o \
          ctype.o \
          error.o \
          expand.o \
          fault.o \
          io.o \
          macro.o \
          main.o \
          msg.o \
          name.o \
          print.o \
          service.o \
          setbrk.o \
          stak.o \
          string.o \
          word.o \
          xec.o

all: sh.aout

sh.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

SUBDIR_LEVEL := ../../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
