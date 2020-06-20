#ifndef BCM283X_VFP_H
#define BCM283X_VFP_H

#include "../h/types.h"

extern u32 initial_fpscr;

extern void vfp_init(void);
extern int vfp_bounce(void);
extern void vfp_discard(void);
extern void vfp_atexec(void);

extern u32 read_fpsid(void);
extern u32 read_fpexc(void);
extern u32 read_mvfr0(void);
extern u32 read_mvfr1(void);
extern u32 read_fpscr(void);
extern u32 read_fpinst(void);
extern u32 read_fpinst2(void);

extern void write_fpscr(u32 v);
extern void write_fpexc(u32 v);
extern void write_fpinst(u32 v);
extern void write_fpinst2(u32 v);

extern void vfp_loadregs(u64 *v, int is_d32);
extern void vfp_saveregs(u64 *v, int is_d32);

#endif
