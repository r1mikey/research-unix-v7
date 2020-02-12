#include "../h/param.h"
#include "../h/systm.h"
#include "../h/seg.h"
#include "../h/buf.h"
#include "../h/conf.h"

/*
 * In case console is off,
 * panicstr contains argument to last
 * call to panic.
 */

char	*panicstr;

static void printn(unsigned long n, int b);

void __prf_vprintf(const char *fmt, __builtin_va_list va)
{
	int c;
	char *s;

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0')
			return;
		putchar(c);
	}

	c = *fmt++;
	if (c == 'd') {
		int a = __builtin_va_arg(va, int);
		printn((unsigned long)a, c == 'o' ? 8 : (c == 'x' ? 16 : 10));
	} else if (c == 'u' || c == 'o' || c == 'x') {
		unsigned int a = __builtin_va_arg(va, unsigned int);
		printn((unsigned long)a, c == 'o' ? 8 : (c == 'x' ? 16 : 10));
	} else if (c == 's') {
		s = __builtin_va_arg(va, char *);
		while (c = *s++)
			putchar(c);
	} else if (c == 'D') {
		unsigned int a = __builtin_va_arg(va, long);
		printn((unsigned long)a, 10);
	}
	goto loop;
}

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 * Used to print diagnostic information
 * directly on console tty.
 * Since it is not interrupt driven,
 * all system activities are pretty much
 * suspended.
 * Printf should not be used for chit-chat.
 */
void printf(const char *fmt, ...)
{
  __builtin_va_list va;
  __builtin_va_start(va, fmt);
  __prf_vprintf(fmt, va);
  __builtin_va_end(va);
}

#if 0
/* VARARGS 1 */
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register u16 *adx;
	char *s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}
#endif

/*
 * Print an unsigned integer in base b.
 */
static void printn(unsigned long n, int b)
{
	unsigned long a;

#if 0
	if (n<0) {	/* shouldn't happen */
		putchar('-');
		n = -n;
	}
#endif
	if(a = n/b)
		printn(a, b);
	putchar("0123456789abcdef"[n % b]);
}

/*
 * Panic is called on unresolvable
 * fatal errors.
 * It syncs, prints "panic: mesg" and
 * then loops.
 */
panic(s)
char *s;
{
	panicstr = s;
	update();
	printf("panic: %s\n", s);
	__endless_spin();
}

/*
 * prdev prints a warning message of the
 * form "mesg on dev x/y".
 * x and y are the major and minor parts of
 * the device argument.
 */
prdev(str, dev)
char *str;
dev_t dev;
{

	printf("%s on dev %u/%u\n", str, major(dev), minor(dev));
}

/*
 * deverr prints a diagnostic from
 * a device driver.
 * It prints the device, block number,
 * and an octal word (usually some error
 * status register) passed as argument.
 */
deverror(bp, o1, o2)
register struct buf *bp;
{

	prdev("err", bp->b_dev);
	printf("bn=%D er=%o,%o\n", bp->b_blkno, o1, o2);
}
