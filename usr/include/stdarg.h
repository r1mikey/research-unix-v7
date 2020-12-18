#ifndef __V7_STDARG_H_20201218_H
#define __V7_STDARG_H_20201218_H

#define va_start(v,l)        __builtin_va_start(v,l)
#define va_end(v)        __builtin_va_end(v)
#define va_arg(v,l)        __builtin_va_arg(v,l)
#define va_copy(d,s)        __builtin_va_copy(d,s)
#define __va_copy(d,s)        __builtin_va_copy(d,s)

typedef __builtin_va_list va_list;
#if 0
# define va_dcl int va_alist;
# define va_start(list) list = (char *) &va_alist
# define va_end(list)
# define va_arg(list,mode) ((mode *)(list += sizeof(mode)))[-1]
#endif

#endif
