.syntax unified
.arm
.section .text

.extern c_bad_exception
.extern c_entry_pabt
.extern c_entry_irq
.extern c_entry_fiq

.global _bad_exception
_bad_exception:
    bl      c_bad_exception
1:  cpsid   iaf                                        @ disable interrupts
    wfi                                                @ wait for an interrupt
    b       1b                                         @ and, if we ever get out of that, go again

.global _entry_fiq
_entry_fiq:
    bl      c_entry_fiq
1:  cpsid   iaf                                        @ disable interrupts
    wfi                                                @ wait for an interrupt
    b       1b                                         @ and, if we ever get out of that, go again
