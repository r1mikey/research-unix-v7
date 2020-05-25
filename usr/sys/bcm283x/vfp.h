#ifndef BCM283X_VFP_H
#define BCM283X_VFP_H

#include "kstdint.h"

extern uint32_t initial_fpscr;

extern void vfp_init(void);
extern int vfp_bounce(void);
extern void vfp_discard(void);
extern void vfp_atexec(void);

extern uint32_t read_fpsid(void);
extern uint32_t read_fpexc(void);
extern uint32_t read_mvfr0(void);
extern uint32_t read_mvfr1(void);
extern uint32_t read_fpscr(void);
extern uint32_t read_fpinst(void);
extern uint32_t read_fpinst2(void);

extern void write_fpscr(uint32_t v);
extern void write_fpexc(uint32_t v);
extern void write_fpinst(uint32_t v);
extern void write_fpinst2(uint32_t v);

extern void vfp_loadregs(uint64_t *v, int is_d32);
extern void vfp_saveregs(uint64_t *v, int is_d32);

#endif
