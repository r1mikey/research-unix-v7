.syntax unified
.arm
.section .text

.extern errno


@ int times(struct tms *tp)
@  0 on success, -1 on error
@  results directly in times
@  Also:
@   struct tms {
@     time_t tms_utime;
@     time_t tms_stime;
@     time_t tms_cutime;
@     time_t tms_cstime;
@   };
.global times
.type times,%function
times:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r0}                                       @ tp
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #43                                        @ 43 is times
    movcc   r0, #0                                     @ set up success return
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the errno address
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size times, . - times