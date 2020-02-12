OBJECTS = sed0.o \
          sed1.o

all: sed.aout

sed.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

SUBDIR_LEVEL := ../../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
