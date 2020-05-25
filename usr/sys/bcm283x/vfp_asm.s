.syntax unified
.arm
.section .text
.fpu vfpv2
.fpu vfpv3

.global read_fpsid
read_fpsid:
    vmrs    r0, fpsid
    mov     pc, lr

.global read_fpexc
read_fpexc:
    vmrs    r0, fpexc
    mov     pc, lr

.global write_fpexc
write_fpexc:
    vmsr    fpexc, r0
    mov     pc, lr

.global read_mvfr0
read_mvfr0:
    vmrs    r0, mvfr0
    mov     pc, lr

.global read_mvfr1
read_mvfr1:
    vmrs    r0, mvfr1
    mov     pc, lr

.global read_fpscr
read_fpscr:
    vmrs    r0, fpscr
    mov     pc, lr

.global write_fpscr
write_fpscr:
    vmsr    fpscr, r0
    mov     pc, lr

.global read_fpinst
read_fpinst:
    vmrs    r0, fpinst
    mov     pc, lr

.global write_fpinst
write_fpinst:
    vmsr    fpinst, r0
    mov     pc, lr

.global read_fpinst2
read_fpinst2:
    vmrs    r0, fpinst2
    mov     pc, lr

.global write_fpinst2
write_fpinst2:
    vmsr    fpinst2, r0
    mov     pc, lr

.global vfp_loadregs
vfp_loadregs:
    vldmia   r0!, {d0-d15}
    cmp      r1, #0
    vldmiane r0!,  {d16-d31}
    addeq    r0, r0, #128
    mov      pc, lr

.global vfp_saveregs
vfp_saveregs:
    vstmia   r0!, {d0-d15}
    cmp      r1, #0
    vstmiane r0!, {d16-d31}
    addeq    r0, r0, #128
    mov      pc, lr
