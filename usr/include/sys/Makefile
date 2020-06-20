HFILES = acct.h \
         buf.h \
         conf.h \
         dir.h \
         fblk.h \
         file.h \
         filsys.h \
         ino.h \
         inode.h \
         param.h \
         proc.h \
         reg.h \
         stat.h \
         text.h \
         timeb.h \
         tty.h \
         types.h \
         user.h

all: $(HFILES)

../../sys/h/%.h: ;

%.h: ../../sys/h/%.h
	@echo "COPYSYS $@"
	@cp -pr "$<" "$@"

clean: ;
