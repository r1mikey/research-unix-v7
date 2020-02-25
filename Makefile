SUBDIRS = usr/src \
          usr/sys

all:
	@for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} all; done

clean:
	@for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} clean; done

tools/abc.fs: all
	(sync && cd tools && ./buildfs && sync)

fsimg: tools/abc.fs

tools/sd.img: tools/abc.fs
	dd if=tools/abc.fs of=tools/sd.img bs=512 seek=96256 conv=notrunc
	sync

sdimg: tools/sd.img

run: tools/sd.img
	$(MAKE) -C usr/sys run
