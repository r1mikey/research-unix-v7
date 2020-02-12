#include <stdio.h>
#include <varargs.h>

extern void _doprnt(struct _iobuf *buf, const char* format, va_list va);
extern int _flsbuf(int c, FILE *iop);

char *sprintf(char *str, char *fmt, ...)
{
  va_list va;
  struct _iobuf _strbuf;

  _strbuf._flag = _IOWRT+_IOSTRG;
  _strbuf._ptr = str;
  _strbuf._cnt = 32767;
  va_start(va, fmt);
  _doprnt(&_strbuf, fmt, va);
  va_end(va);
  putc('\0', &_strbuf);
  return(str);
}
