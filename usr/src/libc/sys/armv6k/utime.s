.syntax unified
.arm
.section .text

.extern errno


@ int utime(char *fname, time_t t[2])
@  0 on success, -1 on error
.global utime
.type utime,%function
utime:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r1}                                       @ t
    push    {r0}                                       @ fname
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #30                                        @ 30 is utime
    movcc   r0, #0                                     @ set up success return
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size utime, . - utime
