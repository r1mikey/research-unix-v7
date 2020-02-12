.syntax unified
.arm
.section .text

.extern main
.extern _cleanup
.extern errno
.extern _end

.global _start
.type _start,%function
_start:
    mov     r0, sp          @ just up from the initial sp is the argc (int *)
    add     r1, sp, #4      @ up from there is the argv (char ***)
    ldr     r0, [r0]        @ deref argc
    mov     r3, r0, lsl #2  @ get number of bytes to move to the end of argv
    add     r3, r3, #4      @ one past the end of argv is envp
    add     r2, r1, r3      @ r2 now points to envp
    ldr     r3, =environ    @ load up the address of the environ pointer...
    str     r2, [r3]        @ set the value of the environ pointer
    ldr     r3, =_end       @ load up the address of _end (this is the value we want)
    str     r3, [r3]        @ save the current value to the address
    mov     r3, #0          @ reset r3 to keep things tidy...
    bl      main            @ call main, return is in r0
    push    {r0}            @ save the return from main
    bl      _cleanup        @ flush and clean up I/O...
    pop     {r0}            @ restore the return code from main to pass to exit
    swi     #1              @ 1 is exit - exit code is in r0 - no return on success
    b       .               @ should never happen
.size _start, . - _start

.bss
.balign 4
.global environ
environ:
    .word   0
