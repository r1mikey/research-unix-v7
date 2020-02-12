#ifndef BCM2835_ARM1176JZFS_H
#define BCM2835_ARM1176JZFS_H

#include "kstdint.h"

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

extern uint32_t read_cpuid(void);
extern uint32_t read_ttbr0(void);

/* returns 0xffffffff on error */
extern uint32_t translate_va_to_pa(uint32_t a);

extern void pre_page_table_modification(void);
extern void post_page_table_modification(void);

extern int spl0(void);
extern int spl1(void);
extern int spl2(void);
extern int spl3(void);
extern int spl4(void);
extern int spl5(void);
extern int spl6(void);
extern int spl7(void);
extern void splx(int s);

extern int fuword(uint32_t a);
extern int fuiword(uint32_t a);

extern int subyte(uint32_t a, uint8_t v);
extern int suibyte(uint32_t a, uint8_t v);

extern int suword(uint32_t a, uint32_t v);
extern int suiword(uint32_t a, uint32_t v);

#endif
