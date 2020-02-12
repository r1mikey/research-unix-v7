OUTPUT_FORMAT("elf32-littlearm")
ENTRY(_start)

SECTIONS
{
  .text . : AT (0) {
    _stext = .;
    crt0.o(.text)
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    _etext = .;
  }

  .data : {
    _sdata = .;
    *(.rodata)
    *(.rodata*)
    *(.data)
    *(.data*)
    _edata = .;
  }

  .bss : {
    __bss_start = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    __bss_end = .;
    _ebss = .;
  }

  _end = .;
}
