.arm
.syntax unified
.section .text

.equ MODE_MASK, 0x0000001f
.equ USR_MODE, 0x00000010
.equ SVC_MODE, 0x00000013

.extern __eret_user
.extern c_entry_swi
.extern panic

.global _entry_swi
_entry_swi:
    sub     sp, sp, #(4*18)
    stmia   sp, {r0 - r14}^
    mov     r0, r0
    str     lr, [sp, #(4*15)]
    mrs     r0, spsr
    str     r0, [sp, #(4*16)]

    and     r0, r0, #MODE_MASK
    cmp     r0, #USR_MODE
    ldrne   r0, addr_of_msg_swi_invalid
    bne     panic

    ldr     r0, [lr, #-4]
    bic     r0, r0, #0xff000000
    mov     r1, sp
    bl      c_entry_swi
    b       __eret_user

__swi_invalid:
    ldr     r0, addr_of_msg_swi_invalid
    b       panic
addr_of_msg_swi_invalid: .word msg_swi_invalid

.data
.balign 4
msg_swi_invalid: .asciz "The swi spsr must be usr mode"
