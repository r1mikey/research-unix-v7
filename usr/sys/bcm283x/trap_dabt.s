.arm
.syntax unified
.section .text

.equ MODE_MASK, 0x0000001f
.equ USR_MODE, 0x00000010
.equ ABT_MODE, 0x00000017
.equ SVC_MODE, 0x00000013

.extern __eret_user
.extern __eret_kernel
.extern c_entry_dabt
.extern panic

.global _entry_dabt
_entry_dabt:
    sub     lr, lr, #8
    str     r0, [sp, #-4]
    str     lr, [sp, #-8]
    mrs     lr, spsr
    str     lr, [sp, #-12]

    mrs     r0, cpsr
    eor     r0, r0, #(ABT_MODE ^ SVC_MODE)
    msr     spsr_cxsf, r0

    mov     r0, sp

    and     lr, lr, #MODE_MASK

    cmp     lr, #SVC_MODE
    adreq   lr, __dabt_svc
    beq     1f

    cmp     lr, #USR_MODE
    adreq   lr, __dabt_usr
    beq     1f

    adr     lr, __dabt_invalid

1:  clrex
    movs    pc, lr

__dabt_usr:
    sub     sp, sp, #(4*17)
    stmia   sp, {r1 - r12}

    ldr     r3, [r0, #-4]
    str     r3, [sp, #-4]!

    add     sp, sp, #(4*13)
    stmia   sp, {sp, lr}^
    mov     r0, r0
    sub     sp, sp, #(4*13)

    ldr     r3, [r0, #-8]
    str     r3, [sp, #(4*15)]

    ldr     r3, [r0, #-12]
    str     r3, [sp, #(4*16)]

    mov     r0, sp
    bl      c_entry_dabt
    b       __eret_user

__dabt_svc:
    str     sp, [r0, #-16]
    bic     sp, sp, #7
    sub     sp, sp, #(4*17)
    stmia   sp, {r1 - r12}

    ldr     r3, [r0, #-4]
    str     r3, [sp, #-4]!

    ldr     r3, [r0, #-16]
    str     r3, [sp, #(4*13)]

    str     lr, [sp, #(4*14)]

    ldr     r3, [r0, #-8]
    str     r3, [sp, #(4*15)]

    ldr     r3, [r0, #-12]
    str     r3, [sp, #(4*16)]

    mov     r0, sp
    bl      c_entry_dabt
    b       __eret_kernel

__dabt_invalid:
    ldr     r0, addr_of_msg_dabt_invalid
    b       panic
addr_of_msg_dabt_invalid: .word msg_dabt_invalid

.data
.balign 4
msg_dabt_invalid: .asciz "The dabt spsr must be either usr or svc mode"
