.syntax unified
.arm
.section .text

.extern errno

@
@ Userland view:
@  result = ptrace(req, pid, addr, data);
@ Kernel view:
@  int     data;
@  int     pid;
@  int     *addr;
@  int     req;
@
@  -1 on error - what on success?
.global ptrace
.type ptrace,%function
ptrace:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}                                       @ req
    push    {r1}                                       @ pid
    push    {r2}                                       @ addr
    mov     r0, r3                                     @ data is passed to the kernel in r0 - was r3 to the function
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #26                                        @ 26 is ptrace
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size ptrace, . - ptrace
