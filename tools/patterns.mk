%.elf: %.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

%.bin: %.elf
	$(OBJCOPY) -O binary --only-section=.text --only-section=.data $^ $@

%.aout: %.bin %.elf
	$(MKAOUT) $* $(shell $(SIZE) $(word 2, $^) | tail -1 | awk '{ printf "%d %d %d", $$1, $$2, $$3; }')

clean:
	$(RM) *.aout *.elf *.bin *.o *.a *.lst
	@for __SD in $(SUBDIRS); do $(MAKE) -C $${__SD} clean; done
