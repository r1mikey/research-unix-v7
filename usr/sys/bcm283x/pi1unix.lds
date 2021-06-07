OUTPUT_FORMAT("elf32-littlearm")
ENTRY(_start)

SECTIONS
{
  . = KERNVIRTADDR - 0x01609000;
  __kernelspace_start = .;

  /*
   * Basic peripherals mapping
   */
  .io1 BLOCK(0x1000) (NOLOAD): {
    __io1_start = .;
    . = . + 0x01000000;
    __io1_end = .;
  }

  /*
   * ARM control mapping for newer models
   */
  .io2 BLOCK(0x1000) (NOLOAD): {
    __io2_start = .;
    . = . + 0x00200000;
    __io2_end = .;
  }

  /*
   * L2 Page Tables
   */
  .l2pt BLOCK(0x1000) (NOLOAD): {
    __l2pt_start = .;
    *(.l2pt)
    __l2pt_end = ((. + (0xfff)) & 0xfffff000);
  }

  /*
   * L1 Page Table
   */
  .l1pt BLOCK(0x1000) (NOLOAD): {
    __l1pt_start = .;
    *(.l1pt)
    __l1pt_end = ((. + (0xfff)) & 0xfffff000);
  }

  /*
   * udot area, initially the process 0 stack
   *
   * Mapped as non-global
   */
  .udot BLOCK(0x1000) (NOLOAD): {
    __udot_start = .;
    *(.udot)
    __udot_end = ((. + (0xfff)) & 0xfffff000);
    . = __udot_end;
  }

  /*
   * Mode stacks, copypage and zeropage addresses
   */
  .misc BLOCK(0x1000) (NOLOAD): {
    __misc_start = .;
    __fiq_stack_start = .;
    . = . + 0x00000400;
    __fiq_stack_top = .;
    __irq_stack_start = .;
    . = . + 0x00000400;
    __irq_stack_top = .;
    __abt_stack_start = .;
    . = . + 0x00000400;
    __abt_stack_top = .;
    __und_stack_start = .;
    . = . + 0x00000400;
    __und_stack_top = .;
    __copypage_src = .;
    . = . + 0x00001000;
    __copypage_dst = .;
    . = . + 0x00001000;
    __clearpage_dst = .;
    . = . + 0x00001000;
    __misc_end = .;
  }

  /*
   * Kernel text
   *
   * . = KERNVIRTADDR at this point
   */
  .text BLOCK(0x1000): AT (PROGRAM_ENTRY_OFFSET) {
    _stext = .;
    startup.o(.text)
    *(.text) *(.text*)
    *(.glue_7)
    *(.glue_7t)
    _etext = .;
  }

  /*
   * Kernel read-only data
   */
  /*
  .rodata BLOCK(0x1000): {
    _srodata = .;
    *(.rodata) *(.rodata*)
    _erodata = .;
  }
  */

  /*
   * Kernel initialised data
   */
  .data BLOCK(0x1000): {
    _srodata = .;
    *(.rodata) *(.rodata*)
    _erodata = .;
    _sdata = .;
    *(.data) *(.data*)
    _edata = .;
  }

  /*
   * Kernel uninitialised data
   */
  .bss BLOCK(0x1000): {
    __bss_start = .;
    *(.bss) *(.bss*)
    *(COMMON)
    __bss_end = .;
    _ebss = .;
  }

  __kernelspace_end = .;
  _end = .;
}
