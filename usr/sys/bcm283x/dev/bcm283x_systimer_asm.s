.syntax unified
.arm
.section .text

@ From DDI0406C_C_arm_architecture_reference_manual.pdf, page 1969 (B8-1969)
@            | CRn | opc1 | CRm, opc2 | width | type | description
@ CNTFRQ     | c14 | 0    |  c0  | 0  | 32    | RW   | Counter Frequency Register
@ CNTPCT     | -   | 0    |  c14 | -  | 64    | RO   | Physical Count Register
@ CNTKCTL    | c14 | 0    |  c1  | 0  | 32    | RW   | Timer PL1 Control Register
@ CNTP_TVAL  | ?   | ?    |  c2  | 0  | 32    | RW   | PL1 Physical TimerValue Register
@ CNTP_CTL   | ?   | ?    |  ?   | 1  | 32    | RW   | PL1 Physical Timer Control Register
@ CNTV_TVAL  | ?   | ?    |  c3  | 0  | 32    | RW   | Virtual TimerValue Register
@ CNTV_CTL   | ?   | ?    |  ?   | 1  | 32    | RW   | Virtual Timer Control Register
@ CNTVCT     | -   | 1    |  c14 | -  | 64    | RO   | Virtual Count Register
@ CNTP_CVAL  | ?   | 2    |  ?   | ?  | 64    | RW   | PL1 Physical Timer CompareValue Register
@ CNTV_CVAL  | ?   | 3    |  ?   | ?  | 64    | RW   | Virtual Timer CompareValue Register
@ CNTVOFF    | ?   | 4    |  ?   | ?  | 64    | RW   | Virtual Offset Register
@ CNTHCTL    | c14 | 4    |  c1  | 0  | 32    | RW   | Timer PL2 Control Register
@ CNTHP_TVAL | ?   | ?    |  c2  | 0  | 32    | RW   | PL2 Physical TimerValue Register
@ CNTHP_CTL  | ?   | ?    |  ?   | 1  | 32    | RW   | PL2 Physical Timer Control Register
@ CNTHP_CVAL | -   | 6    |  c14 | -  | 64    | RW   | PL2 Physical Timer CompareValue Register

.global _get_cntfrq
_get_cntfrq:
    mrc     p15, 0, r0, c14, c0, 0
    mov     pc, lr

.global _set_cntfrq
_set_cntfrq:
    mcr     p15, 0, r0, c14, c0, 0
    mov     pc, lr

.global _get_cntpct
_get_cntpct:
    mrrc    p15, 0, r0, r1, c14
    mov     pc, lr

.global _get_cntkctl
_get_cntkctl:
    mrc     p15, 0, r0, c14, c1, 0
    mov     pc, lr

.global _set_cntkctl
_set_cntkctl:
    mcr     p15, 0, r0, c14, c1, 0
    mov     pc, lr

.global _get_cntp_tval
_get_cntp_tval:
    mrc     p15, 0, r0, c14, c2, 0
    mov     pc, lr

.global _set_cntp_tval
_set_cntp_tval:
    mcr     p15, 0, r0, c14, c2, 0
    mov     pc, lr

.global _get_cntp_ctl
_get_cntp_ctl:
    mrc     p15, 0, r0, c14, c2, 1
    mov     pc, lr

.global _set_cntp_ctl
_set_cntp_ctl:
    mcr     p15, 0, r0, c14, c2, 1
    mov     pc, lr

.global _get_cntv_tval
_get_cntv_tval:
    mrc     p15, 0, r0, c14, c3, 0
    mov     pc, lr

.global _set_cntv_tval
_set_cntv_tval:
    mcr     p15, 0, r0, c14, c3, 0
    mov     pc, lr

.global _get_cntv_ctl
_get_cntv_ctl:
    mrc     p15, 0, r0, c14, c3, 1
    mov     pc, lr

.global _set_cntv_ctl
_set_cntv_ctl:
    mcr     p15, 0, r0, c14, c3, 1
    mov     pc, lr

.global _get_cntvct
_get_cntvct:
    mrrc    p15, 1, r0, r1, c14
    mov     pc, lr

.global _get_cntp_cval
_get_cntp_cval:
    mrrc    p15, 2, r0, r1, c14
    mov     pc, lr

.global _set_cntp_cval
_set_cntp_cval:
    mcrr    p15, 2, r0, r1, c14
    mov     pc, lr

.global _get_cntv_cval
_get_cntv_cval:
    mrrc    p15, 3, r0, r1, c14
    mov     pc, lr

.global _set_cntv_cval
_set_cntv_cval:
    mcrr    p15, 3, r0, r1, c14
    mov     pc, lr

.global _get_cntvoff
_get_cntvoff:
    mrrc    p15, 4, r0, r1, c14
    mov     pc, lr

.global _set_cntvoff
_set_cntvoff:
    mcrr    p15, 4, r0, r1, c14
    mov     pc, lr

.global _get_cnthctl
_get_cnthctl:
    mrc     p15, 4, r0, c14, c1, 0
    mov     pc, lr

.global _set_cnthctl
_set_cnthctl:
    mcr     p15, 4, r0, c14, c1, 0
    mov     pc, lr

.global _get_cnthp_tval
_get_cnthp_tval:
    mrc     p15, 4, r0, c14, c2, 0
    mov     pc, lr

.global _set_cnthp_tval
_set_cnthp_tval:
    mcr     p15, 4, r0, c14, c2, 0
    mov     pc, lr

.global _get_cnthp_ctl
_get_cnthp_ctl:
    mrc     p15, 4, r0, c14, c2, 1
    mov     pc, lr

.global _set_cnthp_ctl
_set_cnthp_ctl:
    mcr     p15, 4, r0, c14, c2, 1
    mov     pc, lr

.global _get_cnthp_cval
_get_cnthp_cval:
    mrrc    p15, 6, r0, r1, c14
    mov     pc, lr

.global _set_cnthp_cval
_set_cnthp_cval:
    mcrr    p15, 6, r0, r1, c14
    mov     pc, lr
