#include <stdio.h>
#include <varargs.h>

extern void _doprnt(struct _iobuf *buf, const char* format, va_list va);

int printf(const char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  _doprnt(stdout, fmt, va);
  va_end(va);
  return(ferror(stdout)? EOF: 0);
}
