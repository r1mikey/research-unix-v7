.syntax unified
.arm
.section .text

.extern errno

@
@ +-------------------------+--------------------------------+
@ | spilled4                | -- envp                        |
@ +-------------------------+--------------------------------+ fp + 52
@ | spilled3                | -- argv[5] - 0 terminator      |
@ +-------------------------+--------------------------------+ fp + 48
@ | spilled2                | -- argv[4]                     |
@ +-------------------------+--------------------------------+ fp + 44
@ | init sp, spilled1       | -- argv[3]                     |
@ +-------------------------+--------------------------------+ fp + 40
@ | direct3                 | -- argv[2]                     |
@ +-------------------------+--------------------------------+ fp + 36
@ | direct2                 | -- argv[1]                     |
@ +-------------------------+--------------------------------+ fp + 32
@ | direct1                 | -- argv[0]                     |
@ +-------------------------+--------------------------------+ fp + 28
@ | envp                    | -- copied over from the end    |
@ +-------------------------+--------------------------------+ fp + 24
@ | argv                    | -- points to direct1           |
@ +-------------------------+--------------------------------+ fp + 20
@ | fname                   | -- copied from r0              |
@ +-------------------------+--------------------------------+ fp + 16
@ | lr                      | -- preserved from lr           |
@ +-------------------------+--------------------------------+ fp + 12
@ | ip                      | -- preserved from ip           |
@ +-------------------------+--------------------------------+ fp + 8
@ | fp                      | -- preserved from fp           |
@ +-------------------------+--------------------------------+ fp + 4
@ | r10                     | -- preserved from r10          |
@ +-------------------------+--------------------------------+ fp
@

.global execle
.type execle,%function
execle:
    sub     sp, sp, #24                                @ 6 registers
    push    {r10, fp, ip, lr}                          @ set up our stack frame
    add     fp, sp, #0                                 @ adjust the frame pointer
    add     ip, fp, #28                                @ point to the direct1 slot
    stm     ip, {r1, r2, r3}                           @ store any direct arguments to their storage slots
    add     ip, fp, #16                                @ point to the fname slot
    str     r0, [ip], #4                               @ stash r0 into the fname slot, increment ip
    add     r0, fp, #28                                @ point r0 to the first argv value
    str     r0, [ip]                                   @ store the argv pointer and increment ip
    mov     r0, #0                                     @ set up success return (but should never be returned...)
    add     ip, fp, #28                                @ point to the direct1 slot
2:  ldr     r0, [ip], #4                               @ load up the argument, increment the load address
    cmp     r0, #0                                     @ check if the argument is the 0 terminator
    beq     3f                                         @ we've got the 0 terminator, ip contains the address of the envp arg
    b       2b                                         @ we haven't hit the terminator yet, try again
3:  ldr     r0, [ip]                                   @ load up the environment pointer
    add     ip, fp, #24                                @ point ip to the envp slot
    str     r0, [ip]                                   @ store the envp argument
    add     lr, fp, #16                                @ point lr to the syscall arguments
    swi     #59                                        @ 59 is exece
    bcc     1f                                         @ syscalls return carry-clear for success and carry-set for error
    ldr     r2, =errno                                 @ load up the address of errno
    str     r0, [r2]                                   @ stash the returned errno in the error case
    mov     r0, #-1                                    @ syscalls return -1 for error
1:  sub     sp, fp, #0                                 @ restore our stack pointer to point to our stack frame
    pop     {r10, fp, ip, lr}                          @ restore caller frame
    add     sp, sp, #24                                @ fix up the stack
    mov     pc, lr                                     @ return to the caller
.size execle, . - execle
