.syntax unified
.arm
.section .text

.extern errno


@ int nice(int niceness)
.global nice
.type nice,%function
nice:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    swi     #34                                        @ 34 is nice
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       2f
1:  mov     r0, #0                                     @ the kernel does not return a success value
2:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size nice, . - nice
