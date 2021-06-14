#ifndef BCM2835_ARM1176JZFS_H
#define BCM2835_ARM1176JZFS_H

#include "../h/types.h"

#define DMB do_arm1176jzfs_dmb()
#define DSB do_arm1176jzfs_dsb()
#define ISB do_arm1176jzfs_isb()

extern void do_arm1176jzfs_dmb(void);
extern void do_arm1176jzfs_dsb(void);
extern void do_arm1176jzfs_isb(void);

extern void dcachecva(u32 start, u32 end);
extern void dcacheiva(u32 start, u32 end);
extern void dcacheciva(u32 start, u32 end);
extern void icacheiva(u32 start, u32 end);
extern void tlbimva(u32 addr, u32 asid);
extern void tlbiasid(u32 asid);
extern void flush_entire_btc(void);

extern u32 read_asid(void);
extern void write_asid(u32 asid);

extern int enable_interrupts(void);
extern int disable_interrupts(void);

extern u32 read_cpsr(void);
extern u32 read_dfar(void);
extern u32 read_dfsr(void);
extern u32 read_ifsr(void);

/* returns 0xffffffff on error */
extern u32 translate_va_to_pa(u32 a);

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

extern void __clearseg_helper(u32 pgaddr);
extern void __copyseg_helper(u32 srcaddr, u32 dstaddr);

#endif
