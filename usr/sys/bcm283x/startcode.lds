OUTPUT_FORMAT("elf32-littlearm")
ENTRY(_start)

SECTIONS
{
  .text . : AT (0) {
    startcode.o(.text)
  }
}
