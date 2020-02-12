.syntax unified
.arm
.section .text

.extern errno
.extern environ

@ int execve(char *fname, char *argv[], char *envp[])
@ where argv is a vector argv[0] ... argv[x], 0
@ last vector element must be 0
@ environment passed automatically
@ same applies for envp
@  no return on success, -1 on error
.global execve
.type execve,%function
execve:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r2}                                       @ envp
    push    {r1}                                       @ argv
    push    {r0}                                       @ fname
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    mov     r0, #0                                     @ set up success return
    swi     #59                                        @ 59 is exece
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size execve, . - execve
