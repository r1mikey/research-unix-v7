.arm
.syntax unified
.section .text

.global __eret_user
__eret_user:
    ldr     r0, [sp, #(4*16)]
    msr     spsr_cxsf, r0

    ldmia   sp, {r0-r14}^
    add     sp, sp, #(4*18)
    ldr     lr, [sp, #-12]
    clrex
    movs    pc, lr

.global __eret_kernel
__eret_kernel:
    ldr     r0, [sp, #(4*16)]
    msr     spsr_cxsf, r0
    clrex
    ldmia   sp, {r0-r15}^
