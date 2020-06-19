#include "page_tables.h"

#include "../h/param.h"

l2pt_t __l2_page_table __attribute__((aligned(BCM283X_PT_ALIGN_BYTES), section(".l2pt")));
l1pt_t __l1_page_table __attribute__((aligned(BCM283X_PT_ALIGN_BYTES), section(".l1pt")));


extern void printf(const char *fmt, ...);                       /* sys/prf.c */
extern void panic(const char *s) __attribute__((noreturn));


void setup_one_page_mapping(unsigned int srcpg, unsigned int dstpg, unsigned int a)
{
  if (dstpg & 0xfff00000 || srcpg & 0xfff00000) {
    printf("dstpg: 0x%x, srcpg 0x%x\n");
    panic("setup_one_page_mapping: bad page");
  }

  __l2_page_table.entries[dstpg] = ((srcpg * PGSZ) | a);
}
