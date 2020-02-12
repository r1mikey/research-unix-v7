.syntax unified
.arm
.section .text

.extern errno


@ int ioctl(int fdes, int cmd, void *arg)
.global ioctl
.type ioctl,%function
ioctl:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r2}                                       @ arg
    push    {r1}                                       @ cmd
    push    {r0}                                       @ fdes
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    mov     r0, #0                                     @ set up success return
    swi     #54                                        @ 54 is ioctl
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size ioctl, . - ioctl
