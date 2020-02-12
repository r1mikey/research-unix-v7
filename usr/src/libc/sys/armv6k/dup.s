.syntax unified
.arm
.section .text

.extern errno


@ int dup(int fildes)
.global dup
.type dup,%function
dup:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    mov     r1, #0x1000                                @ something patently ridiculous...
    swi     #41                                        @ 41 is dup
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size dup, . - dup


@ int dup2(int fildes, int fildes2)
.global dup2
.type dup2,%function
dup2:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    mov     r2, #0x40                                  @ we'll OR in the dup2 indicator...
    orr     r0, r0, r2                                 @ r0 now has a magic bit set, indicating that this is a dup2 call
    swi     #41                                        @ 41 is dup
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size dup2, . - dup2
