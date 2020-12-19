#ifndef _V7_STDIO_H_20201218_H
#define _V7_STDIO_H_20201218_H

#define	BUFSIZ	512
#define	_NFILE	20
# ifndef FILE
extern	struct	_iobuf {
	char	*_ptr;
	short	_cnt;
	char	*_base;
	char	_flag;
	char	_file;
} _iob[_NFILE];
# endif

#define	_IOREAD	01
#define	_IOWRT	02
#define	_IONBF	04
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	_IORW	0200

#define	NULL	0
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])
#define	getc(p)		(--(p)->_cnt>=0? *(p)->_ptr++&0377:_filbuf(p))
#define	getchar()	getc(stdin)
#define putc(x,p) (--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(unsigned)(x))):_flsbuf((unsigned)(x),p))
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	p->_file

#include <stdarg.h>

extern FILE * fopen(const char *file, const char *mode);
extern FILE * freopen(const char *file, const char *mode, FILE *iop);
extern FILE * fdopen(int fd, const char *mode);
extern long ftell(FILE *iop);
extern char * fgets(char *s, int n, FILE *iop);
extern int fclose(FILE *iop);

extern int printf(const char *format, ...);
extern char *sprintf(char *str, char *fmt, ...);
extern int fprintf(FILE *iop, const char *fmt, ...);
extern int vfprintf(FILE *iop, const char *fmt, va_list va);

extern int ungetc(int c, FILE *iop);

/* implementation details */
extern int _filbuf(FILE *iop);
extern int _flsbuf(int c, FILE *iop);

#endif
