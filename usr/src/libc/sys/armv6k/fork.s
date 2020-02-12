.syntax unified
.arm
.section .text

.extern errno


@ int fork()
@
@ The implementation looks weird because the kernel increments
@ the pc in the parent case, so the unconditional branch after
@ the swi is skipped in the parent case.  This lets the code
@ easily differentiate between the parent and child cases,
@ where the parent case does not copy to the parent id variable
@ and the child does, but without complicated logic.
@
@ Non-obvious on casual inspection (kernel point of view):
@  if child process:
@    increment lr
@    return parent pid
@  else parent process:
@    return child pid
@
.global fork
.type fork,%function
fork:
    push    {fp, lr}                                   @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    swi     #2                                         @ 2 is fork
    b       1f                                         @ child code skips to copying parent id
    bcc     2f                                         @ no error, go to epilogue - r0 is the child pid
    ldr     r2, =errno                                 @ on error, set errno and jump to return
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
    b       2f                                         @ skip to the epilogue
1:  ldr     r1, =par_uid                               @ load up the address of the parent id variable
    str     r0, [r1]                                   @ store the returned integer to the par_uid variable
    mov     r0, #0                                     @ the child return value is 0 
2:  sub     sp, r11, #0                                @ restore our stack pointer to point to our stack frame
    pop     {fp, pc}                                   @ restore caller frame and return
.size fork, . - fork

.bss
par_uid:
    .word   0
