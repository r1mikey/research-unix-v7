.syntax unified
.arm
.section .text

.extern errno

@ int access(char *fname, int fmode)
@  R_OK = 0x04
@  W_OK = 0x02
@  X_OK = 0x01
@  F_OK = 0x00
@  0 on success, -1 on error
.global access
.type access,%function
access:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r1}                                       @ fmode
    push    {r0}                                       @ fname
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #33                                        @ 33 is access
    movcc   r0, #0                                     @ set up success return
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size access, . - access
