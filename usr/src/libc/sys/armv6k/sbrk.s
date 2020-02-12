.syntax unified
.arm
.section .text

.extern errno


@ void *sbrk(int incr)
@ returns the start of the new area, or -1 on exit
@ i.e. old = sbrk(incr);
@
@ We add the incr arg to the value of _end, then call the kernel break syscall.
@ On success return, we stash the old _end, store our new value to _end and return the old.
@
@ When incr is 0, we do the same thing, just skip the kernel part.
@
@ brk seems worse - just push the value to the kernel, then set it in _end on success.
@
.global sbrk
.type sbrk,%function
sbrk:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    sub     sp, sp, #8                                 @ make a bit of space for locals
    str     r0, [fp, #-4]                              @ store the incr value as a local
    ldr     r1, =_end                                  @ load up the address of _end (this is the value we want)
    ldr     r1, [r1]                                   @ deref _end to get the current value
    str     r1, [fp, #-8]                              @ store _end as a local
    cmp     r0, #0                                     @ is incr 0?
    beq     2f                                         @ yep, incr is 0, skip to the out codepath
    add     r0, r1, r0                                 @ add the new size to the value of _end to get what we pass to the kernel
    push    {r0}                                       @ ugh, stack based args
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #17                                        @ 17 is break, call the kernel
    bcc     2f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       1f                                         @ jump to the epilogue to return the error to the caller
2:  ldr     r1, [fp, #-4]                              @ load up the original incr
    ldr     r2, [fp, #-8]                              @ load up the original _end
    add     r0, r2, r1                                 @ we'll save the new break end
    ldr     r1, =_end                                  @ load up the address of _end
    str     r0, [r1]                                   @ save the new value of _end
    mov     r0, r2                                     @ we'll return the original break end
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size sbrk, . - sbrk


.global brk
.type brk,%function
brk:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}                                       @ set arg0 as the kernel arg
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #17                                        @ 17 is break, call the kernel
    bcc     2f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       1f                                         @ jump to the epilogue to return the error to the caller
2:  ldr     r1, =_end                                  @ load up the address of _end
    str     r0, [r1]                                   @ store what we passed to the kernel to _end
    mov     r0, #0                                     @ our success return is 0
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size brk, . - brk
