SUBDIRS = sh \
          dc \
          sed \
          yacc
#          lex
#SUBDIRS =
##x cpp
##x adb
##x as
##x awk << needs lex/yacc
##x bas << not actually used
##x bc.y << needs yacc
##x c
##x neqn << needs yacc
##x egrep.y << needs yacc
##x eqn
##x expr.y << needs yacc
##x f77
##x learn
##x lint
##x lpr
##x m4 << needs yacc
##x make << needs yacc
##x mip << part of pcc AFAIK
##x pcc << pcc - modernise?
##x plot
##x prep
##x prep.h
##x primes.s
##x factor.s
##x ratfor
##x refer
##x roff
##x spell
##x standalone
##x struct
##x tar
##x tbl
##x tp
##x troff
##x uucp
##x xsend

TARGETS = ac \
          accton \
          ar \
          arcv \
          at \
          atrun \
          basename \
          cal \
          calendar \
          cat \
          cb \
          cc \
          checkeq \
          chgrp \
          chmod \
          chown \
          clri \
          cmp \
          col \
          comm \
          cp \
          cron \
          crypt \
          cu \
          date \
          dcheck \
          dd \
          deroff \
          df \
          diff \
          diff3 \
          diffh \
          dmesg \
          du \
          dump \
          dumpdir \
          echo \
          ed \
          expr \
          fgrep \
          file \
          find \
          getty \
          grep \
          icheck \
          init \
          iostat \
          join \
          kill \
          ld \
          ln \
          login \
          look \
          ls \
          makekey \
          mail \
          mesg \
          mkdir \
          mkfs \
          mknod \
          mount \
          mv \
          ncheck \
          newgrp \
          nice \
          nm \
          od \
          osh \
          passwd \
          pr \
          prof \
          ps \
          pstat \
          ptx \
          pwd \
          quot \
          random \
          ranlib \
          restor \
          rev \
          rm \
          rmdir \
          sa \
          size \
          sleep \
          sort \
          sp \
          spline \
          split \
          strip \
          stty \
          su \
          sum \
          sync \
          tabs \
          tail \
          tc \
          tee \
          test \
          time \
          tk \
          touch \
          tr \
          tsort \
          tty \
          umount \
          uniq \
          units \
          update \
          vpr \
          wall \
          wc \
          who \
          write \
          yes

# graph needs plot, and plot needs libplot, and we don't have all the
# sources that allow us to build that!
# compile_simple_c graph - look in the 'plot' subdir

SIMPLE_AOUTS = $(addsuffix .aout,$(sort $(TARGETS)))

all: $(SIMPLE_AOUTS)
	@for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} all; done

GENERATED_SOURCES = bc.c egrep.c expr.c

SUBDIR_LEVEL := ../../..
include $(SUBDIR_LEVEL)/tools/tools.mk
include $(SUBDIR_LEVEL)/tools/patterns.mk

yacc/yacc.native: yacc/y1.c yacc/y2.c yacc/y3.c yacc/y4.c yacc/dextern yacc/files yacc/Makefile-native
	$(MAKE) -C yacc -f Makefile-native

bc.c egrep.c expr.c: bc.y egrep.y expr.y yacc/yacc.native
	yacc/yacc.native bc.y
	mv y.tab.c bc.c
	yacc/yacc.native egrep.y
	mv y.tab.c egrep.c
	yacc/yacc.native expr.y
	mv y.tab.c expr.c
