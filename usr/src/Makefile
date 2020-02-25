SUBDIRS = libc \
          libmp \
          cmd

all:
	@for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} all; done

clean:
	@for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} clean; done
