.syntax unified
.arm
.section .text

.extern errno


@ time_t time(time_t *t)
@ TODO: make time_t 64bit (much later)
@ TODO: make time_t a 32 bit value soonish
.global time
.type time,%function
time:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}                                       @ stash the out pointer
    swi     #13                                        @ 13 is gtime
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       2f
1:  pop     {r3}                                       @ restore the out pointer
    cmp     r3, #0                                     @ check if we have an out pointer
    beq     2f                                         @ no out pointer, skip to return
    str     r0, [r3]                                   @ write the return to the out pointer - useland will use two shorts for now
2:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size time, . - time



.global ftime
.type ftime,%function
ftime:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}                                       @ stash the out pointer
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    mov     r0, #0                                     @ set up the success return
    swi     #35                                        @ 35 is ftime
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       2f
1:  pop     {r3}                                       @ restore the out pointer
    cmp     r3, #0                                     @ check if we have an out pointer
    beq     2f                                         @ no out pointer, skip to return
    str     r0, [r3]                                   @ write the return to the out pointer
2:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size ftime, . - ftime
