#include "page_tables.h"
#include "arm1176jzfs.h"

#include "../h/param.h"
#include "../h/prf.h"

l2pt_t __l2_page_table __attribute__((aligned(BCM283X_PT_ALIGN_BYTES), section(".l2pt")));
l1pt_t __l1_page_table __attribute__((aligned(BCM283X_PT_ALIGN_BYTES), section(".l1pt")));


void setup_one_page_mapping(unsigned int srcpg, unsigned int dstpg, unsigned int a)
{
  if (dstpg & 0xfff00000 || srcpg & 0xfff00000) {
    printf("dstpg: 0x%x, srcpg 0x%x\n");
    panic("setup_one_page_mapping: bad page");
  }

  __l2_page_table.entries[dstpg] = ((srcpg * PGSZ) | a);
}


int page_is_mapped(unsigned int dstpg)
{
  if (dstpg & 0xfff00000) {
    panic("page_is_mapped: bad page");
  }

  return __l2_page_table.entries[dstpg] != 0;
}


void release_current_pagetable(u32 asid)
{
  int i;

  flush_current_pagetable(0, 1, asid);

  for (i = 0; i < (MAXMEM - 1); ++i) {
    setup_one_page_mapping(0, i, 0);
  }
}


void flush_current_pagetable(int clean, int invalidate, u32 asid)
{
  int i;
  u32 memstart;
  u32 memsize;
  u32 stkstart;
  u32 stksize;

  memstart = 0;
  memsize = 0;

  for (i = 0; i < (MAXMEM - 1); ++i) {
    if (!page_is_mapped(i))
      break;

    memsize += PGSZ;
  }

  stkstart = (USERTOP / PGSZ);
  stksize = 0;

  for (i = stkstart; i > ((memstart + memsize) / PGSZ); --i) {
    if (!page_is_mapped(i))
      break;

    stkstart -= 1;
    stksize += PGSZ;
  }

  if (stksize) {
    if (clean && invalidate)
      dcacheciva(stkstart * PGSZ, ((stkstart * PGSZ) + stksize) - 1);
    else if (clean)
      dcachecva(stkstart * PGSZ, ((stkstart * PGSZ) + stksize) - 1);
    else if (invalidate)
      dcacheiva(stkstart * PGSZ, ((stkstart * PGSZ) + stksize) - 1);
  }

  if (memsize) {
    if (clean && invalidate)
      dcacheciva(memstart * PGSZ, ((memstart * PGSZ) + memsize) - 1);
    else if (clean)
      dcachecva(memstart * PGSZ, ((memstart * PGSZ) + memsize) - 1);
    else if (invalidate)
      dcacheiva(memstart * PGSZ, ((memstart * PGSZ) + memsize) - 1);

    if (invalidate)
      icacheiva(memstart * PGSZ, ((memstart * PGSZ) + memsize) - 1);
  }

  if (invalidate)
    tlbiasid(asid);
}
