.syntax unified
.arm
.section .text


.text
.align 2
.global _bad_exception
.type _bad_exception,#function
.code 32
_bad_exception:
@.fnstart
    ldr     r0, addr_of_msg_exception_bad
    bl      panic
1:  cpsid   iaf                                        @ disable interrupts
    wfi                                                @ wait for an interrupt
    b       1b                                         @ and, if we ever get out of that, go again
addr_of_msg_exception_bad:
    .word   msg_exception_bad
.ltorg
.size _bad_exception, . - _bad_exception
@.fnend


.text
.align 2
.global _entry_fiq
.type _entry_fiq,#function
.code 32
_entry_fiq:
@.fnstart
    ldr     r0, addr_of_msg_fiq_bad
    bl      panic
1:  cpsid   iaf                                        @ disable interrupts
    wfi                                                @ wait for an interrupt
    b       1b                                         @ and, if we ever get out of that, go again
addr_of_msg_fiq_bad:
    .word   msg_fiq_bad
.ltorg
.size _entry_fiq, . - _entry_fiq
@.fnend

.data
.balign 4
msg_fiq_bad: .asciz "fiq unhandled"
.balign 4
msg_exception_bad: .asciz "bad exception"

.end
