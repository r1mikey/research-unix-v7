.syntax unified
.arm
.section .text

.extern errno


@ int profil(int *bufbase, unsigned int bufsize, unsigned int pcoffset, unsigned ing pcscale)
.global profil
.type profil,%function
profil:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    push    {r3}                                       @ pcscale
    push    {r2}                                       @ pcoffset
    push    {r1}                                       @ bufsize
    push    {r0}                                       @ bufbase
    mov     lr, sp                                     @ pointer to indirect args is kept in lr
    swi     #44                                        @ 44 is profil
    movcc   r0, #0                                     @ set up the success return
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size profil, . - profil
