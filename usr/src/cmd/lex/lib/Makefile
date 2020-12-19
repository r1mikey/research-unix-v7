OBJECTS = allprint.o \
          main.o \
          reject.o \
          yyless.o \
          yywrap.o

TARGET = libln.a

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

SUBDIR_LEVEL := ../../../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk
