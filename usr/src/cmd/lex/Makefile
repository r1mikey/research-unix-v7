OBJECTS = lmain.o \
          y.tab.o \
          sub1.o \
          sub2.o \
          header.o

all: lex.aout

y.tab.c: parser.y
	$(YACC) $^

lex.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

SUBDIR_LEVEL := ../../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk

CPPFLAGS += -Dunix
