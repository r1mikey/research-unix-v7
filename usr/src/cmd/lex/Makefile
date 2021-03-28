SUBDIRS = lib
OBJECTS = lmain.o \
          y.tab.o \
          sub1.o \
          sub2.o \
          header.o

all: lex.aout
	@$(MAKE) -C lib all

y.tab.c: parser.y
	$(YACC) $^

lex.elf: $(OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

../yacc/yacc.native: ../yacc/y1.c ../yacc/y2.c ../yacc/y3.c ../yacc/y4.c ../yacc/dextern ../yacc/files ../yacc/Makefile-native
	$(MAKE) -C ../yacc -f Makefile-native

GENERATED_SOURCES = y.tab.c

SUBDIR_LEVEL := ../../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk

CPPFLAGS += -Dunix
