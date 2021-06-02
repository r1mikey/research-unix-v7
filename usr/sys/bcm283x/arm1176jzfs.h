#ifndef BCM2835_ARM1176JZFS_H
#define BCM2835_ARM1176JZFS_H

#include "../h/types.h"

#define FULL_SYNC DMB; DSB; ISB

#define DMB do_arm1176jzfs_dmb()
#define DSB do_arm1176jzfs_dsb()
#define ISB do_arm1176jzfs_isb()

extern void do_arm1176jzfs_dmb(void);
extern void do_arm1176jzfs_dsb(void);
extern void do_arm1176jzfs_isb(void);

extern void do_invalidate_icache(void);

extern void do_clean_dcache(void);
extern void do_invalidate_dcache(void);
extern void do_clean_and_invalidate_dcache(void);

extern u32 read_cpuid(void);
extern u32 read_ttbr0(void);

extern u32 read_cpsr(void);

/* returns 0xffffffff on error */
extern u32 translate_va_to_pa(u32 a);

extern void pre_page_table_modification(void);
extern void post_page_table_modification(void);

extern u32 read_cpacr(void);
extern void write_cpacr(u32 v);

extern int spl0(void);
extern int spl1(void);
extern int spl2(void);
extern int spl3(void);
extern int spl4(void);
extern int spl5(void);
extern int spl6(void);
extern int spl7(void);
extern void splx(int s);

extern int fuword(caddr_t a);
extern int fuiword(caddr_t a);

extern int subyte(caddr_t a, u8 v);
extern int suibyte(caddr_t a, u8 v);

extern int suword(caddr_t a, u32 v);
extern int suiword(caddr_t a, u32 v);

#endif
