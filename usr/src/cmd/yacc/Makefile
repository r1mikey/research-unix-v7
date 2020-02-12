OBJECTS = y1.o \
          y2.o \
          y3.o \
          y4.o

all: yacc.aout

yacc.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

SUBDIR_LEVEL := ../../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
