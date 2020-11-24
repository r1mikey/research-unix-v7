#
/*
 * general TTY subroutines
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/proc.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/reg.h"
#include "../h/conf.h"
#include "../h/slp.h"
#include "../h/sig.h"
#include "../h/fio.h"
#include "../h/prim.h"
#include "../h/rdwri.h"
#include "../h/subr.h"
#include "../h/machdep.h"

extern char	partab[];


/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */

char	maptab[] ={
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};


/*
 * shorthand
 */
#define	q1	tp->t_rawq
#define	q2	tp->t_canq
#define	q3	tp->t_outq
#define	q4	tp->t_un.t_ctlq


/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
void ttyopen(dev_t dev, struct tty *tp)
{
	struct proc *pp;

	pp = u.u_procp;
	tp->t_dev = dev;
	if(pp->p_pgrp == 0) {
		u.u_ttyp = tp;
		u.u_ttyd = dev;
		if (tp->t_pgrp==0)
			tp->t_pgrp = pp->p_pid;
		pp->p_pgrp = tp->t_pgrp;
	}
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
}


/*
 * set default control characters.
 */
void ttychars(struct tty *tp)
{
	tun.tc.t_intrc = CINTR;
	tun.tc.t_quitc = CQUIT;
	tun.tc.t_startc = CSTART;
	tun.tc.t_stopc = CSTOP;
	tun.tc.t_eofc = CEOT;
	tun.tc.t_brkc = CBRK;
	tp->t_erase = CERASE;
	tp->t_kill = CKILL;
}

/*
 * flush all TTY queues
 */
static void flushtty(struct tty *tp)
{
	int s;

	while (getc(&tp->t_canq) >= 0)
		;
	wakeup((caddr_t)&tp->t_rawq);
	wakeup((caddr_t)&tp->t_outq);
	s = spl6();
	tp->t_state &= ~TTSTOP;
	(*cdevsw[major(tp->t_dev)].d_stop)(tp);
	while (getc(&tp->t_outq) >= 0)
		;
	while (getc(&tp->t_rawq) >= 0)
		;
	tp->t_delct = 0;
	splx(s);
}

/*
 * Wait for output to drain, then flush input waiting.
 */
static void wflushtty(struct tty *tp)
{
	spl5();
	while (tp->t_outq.c_cc && tp->t_state&CARR_ON) {
		(*tp->t_oproc)(tp);
		tp->t_state |= ASLEEP;
		sleep((caddr_t)&tp->t_outq, TTOPRI);
	}
	flushtty(tp);
	spl0();
}

/*
 * clean tp on last close
 */
void ttyclose(struct tty *tp)
{
	tp->t_pgrp = 0;
	wflushtty(tp);
	tp->t_state = 0;
}

/*
 * ioctl system call
 * Check legality, execute common code, and switch out to individual
 * device routine.
 */
void ioctl(void)
{
	struct file *fp;
	struct inode *ip;
	struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
	} *uap;
	dev_t dev;
	int fmt;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if (uap->cmd==FIOCLEX) {
		u.u_pofile[uap->fdes] |= EXCLOSE;
		return;
	}
	if (uap->cmd==FIONCLEX) {
		u.u_pofile[uap->fdes] &= ~EXCLOSE;
		return;
	}
	ip = fp->f_inode;
	fmt = ip->i_mode & IFMT;
	if (fmt != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	dev = (dev_t)ip->i_un.i_rdev;
	(*cdevsw[major(dev)].d_ioctl)(dev, uap->cmd, uap->cmarg, fp->f_flag);
}

/*
 * stty/gtty writearound
 */
void stty(void)
{
	u.u_arg[2] = u.u_arg[1];
	u.u_arg[1] = TIOCSETP;
	ioctl();
}

void gtty(void)
{
	u.u_arg[2] = u.u_arg[1];
	u.u_arg[1] = TIOCGETP;
	ioctl();
}

/*
 * Common code for several tty ioctl commands
 */
int ttioccomm(int com, struct tty *tp, caddr_t addr, dev_t dev)
{
	unsigned int t;
	struct ttiocb iocb;
	extern int nldisp;

	switch(com) {

	/*
	 * get discipline number
	 */
	case TIOCGETD:
		t = tp->t_line;
		if (copyout((caddr_t)&t, addr, sizeof(t)))
			u.u_error = EFAULT;
		break;

	/*
	 * set line discipline
	 */
	case TIOCSETD:
		if (copyin(addr, (caddr_t)&t, sizeof(t))) {
			u.u_error = EFAULT;
			break;
		}
		if (t >= nldisp) {
			u.u_error = ENXIO;
			break;
		}
		if (tp->t_line)
			(*linesw[tp->t_line].l_close)(tp);
		if (t)
			(*linesw[t].l_open)(dev, tp, addr);
		if (u.u_error==0)
			tp->t_line = t;
		break;

	/*
	 * prevent more opens on channel
	 */
	case TIOCEXCL:
		tp->t_state |= XCLUDE;
		break;
	case TIOCNXCL:
		tp->t_state &= ~XCLUDE;
		break;

	/*
	 * Set new parameters
	 */
	case TIOCSETP:
		wflushtty(tp);
	case TIOCSETN:
		if (copyin(addr, (caddr_t)&iocb, sizeof(iocb))) {
			u.u_error = EFAULT;
			return(1);
		}
		tp->t_ispeed = iocb.ioc_ispeed;
		tp->t_ospeed = iocb.ioc_ospeed;
		tp->t_erase = iocb.ioc_erase;
		tp->t_kill = iocb.ioc_kill;
		tp->t_flags = iocb.ioc_flags;
		break;

	/*
	 * send current parameters to user
	 */
	case TIOCGETP:
		iocb.ioc_ispeed = tp->t_ispeed;
		iocb.ioc_ospeed = tp->t_ospeed;
		iocb.ioc_erase = tp->t_erase;
		iocb.ioc_kill = tp->t_kill;
		iocb.ioc_flags = tp->t_flags;
		if (copyout((caddr_t)&iocb, addr, sizeof(iocb)))
			u.u_error = EFAULT;
		break;

	/*
	 * Hang up line on last close
	 */

	case TIOCHPCL:
		tp->t_state |= HUPCLS;
		break;

	case TIOCFLUSH:
		flushtty(tp);
		break;

	/*
	 * ioctl entries to line discipline
	 */
	case DIOCSETP:
	case DIOCGETP:
		(*linesw[tp->t_line].l_ioctl)(com, tp, addr);
		break;

	/*
	 * set and fetch special characters
	 */
	case TIOCSETC:
		if (copyin(addr, (caddr_t)&tun, sizeof(struct tc)))
			u.u_error = EFAULT;
		break;

	case TIOCGETC:
		if (copyout((caddr_t)&tun, addr, sizeof(struct tc)))
			u.u_error = EFAULT;
		break;

	default:
		return(0);
	}
	return(1);
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 */
void ttstart(struct tty *tp)
{
	int s;

	s = spl5();
	if((tp->t_state&(TIMEOUT|TTSTOP|BUSY)) == 0)
		(*tp->t_oproc)(tp);
	splx(s);
}

/*
 * Restart typewriter output following a delay
 * timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
void ttrstrt(struct tty *tp)
{
	tp->t_state &= ~TIMEOUT;
	ttstart(tp);
}

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
int canon(struct tty *tp)
{
	char *bp;
	char *bp1;
	int c;
	int mc;

	spl5();
	while ((tp->t_flags&(RAW|CBREAK))==0 && tp->t_delct==0
	    || (tp->t_flags&(RAW|CBREAK))!=0 && tp->t_rawq.c_cc==0) {
		if ((tp->t_state&CARR_ON)==0) {
			return(0);
		}
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	spl0();
loop:
	bp = &canonb[2];
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if ((tp->t_flags&(RAW|CBREAK))==0) {
			if (c==0377) {
				tp->t_delct--;
				break;
			}
			if (bp[-1]!='\\') {
				if (c==tp->t_erase) {
					if (bp > &canonb[2])
						bp--;
					continue;
				}
				if (c==tp->t_kill)
					goto loop;
				if (c==tun.tc.t_eofc)
					continue;
			} else {
				mc = maptab[c];
				if (c==tp->t_erase || c==tp->t_kill)
					mc = c;
				if (mc && (mc==c || (tp->t_flags&LCASE))) {
					if (bp[-2] != '\\')
						c = mc;
					bp--;
				}
			}
		}
		*bp++ = c;
		if (bp>=canonb+CANBSIZ)
			break;
	}
	bp1 = &canonb[2];
	b_to_q(bp1, bp-bp1, &tp->t_canq);

	if (tp->t_state&TBLOCK && tp->t_rawq.c_cc < TTYHOG/5) {
		if (putc(tun.tc.t_startc, &tp->t_outq)==0) {
			tp->t_state &= ~TBLOCK;
			ttstart(tp);
		}
		tp->t_char = 0;
	}

	return(bp-bp1);
}

/*
 * Send stop character on input overflow.
 */
static void ttyblock(struct tty *tp)
{
	int x;
	x = q1.c_cc + q2.c_cc;
	if (q1.c_cc > TTYHOG) {
		flushtty(tp);
		tp->t_state &= ~TBLOCK;
	}
	if (x >= TTYHOG/2) {
		if (putc(tun.tc.t_stopc, &tp->t_outq)==0) {
			tp->t_state |= TBLOCK;
			tp->t_char++;
			ttstart(tp);
		}
	}
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
static void ttyoutput(int c, struct tty *tp)
{
	char *colp;
	int ctype;

	tk_nout += 1;
	/*
	 * Ignore EOT in normal mode to avoid hanging up
	 * certain terminals.
	 * In raw mode dump the char unchanged.
	 */

	if ((tp->t_flags&RAW)==0) {
		c &= 0177;
		if (c==CEOT)
			return;
	} else {
		putc(c, &tp->t_outq);
		return;
	}

	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t' && (tp->t_flags&TBDELAY)==XTABS) {
		c = 8;
		do
			ttyoutput(' ', tp);
		while (--c >= 0 && tp->t_col&07);
		return;
	}
	/*
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	if (tp->t_flags&LCASE) {
		colp = "({)}!|^~'`";
		while(*colp++)
			if(c == *colp++) {
				ttyoutput('\\', tp);
				c = colp[-2];
				break;
			}
		if ('a'<=c && c<='z')
			c += 'A' - 'a';
	}
	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (c=='\n' && tp->t_flags&CRMOD)
		ttyoutput('\r', tp);
	putc(c, &tp->t_outq);
	/*
	 * Calculate delays.
	 * The numbers here represent clock ticks
	 * and are not necessarily optimal for all terminals.
	 * The delays are indicated by characters above 0200.
	 * In raw mode there are no delays and the
	 * transmission path is 8 bits wide.
	 */
	colp = &tp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {

	/* ordinary */
	case 0:
		(*colp)++;

	/* non-printing */
	case 1:
		break;

	/* backspace */
	case 2:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case 3:
		ctype = (tp->t_flags >> 8) & 03;
		if(ctype == 1) { /* tty 37 */
			if (*colp)
				c = max(((unsigned)*colp>>4) + 3, (unsigned)6);
		} else
		if(ctype == 2) { /* vt05 */
			c = 6;
		}
		*colp = 0;
		break;

	/* tab */
	case 4:
		ctype = (tp->t_flags >> 10) & 03;
		if(ctype == 1) { /* tty 37 */
			c = 1 - (*colp | ~07);
			if(c < 5)
				c = 0;
		}
		*colp |= 07;
		(*colp)++;
		break;

	/* vertical motion */
	case 5:
		if(tp->t_flags & VTDELAY) /* tty 37 */
			c = 0177;
		break;

	/* carriage return */
	case 6:
		ctype = (tp->t_flags >> 12) & 03;
		if(ctype == 1) { /* tn 300 */
			c = 5;
		} else if(ctype == 2) { /* ti 700 */
			c = 10;
		}
		*colp = 0;
	}
	if(c)
		putc(c|0200, &tp->t_outq);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
void ttyinput(int c, struct tty *tp)
{
	int t_flags;

	tk_nin += 1;
	c &= 0377;
	t_flags = tp->t_flags;
	if (t_flags&TANDEM)
		ttyblock(tp);
	if ((t_flags&RAW)==0) {
		c &= 0177;
		if (tp->t_state&TTSTOP) {
			if (c==tun.tc.t_startc) {
				tp->t_state &= ~TTSTOP;
				ttstart(tp);
				return;
			}
			if (c==tun.tc.t_stopc)
				return;
			tp->t_state &= ~TTSTOP;
			ttstart(tp);
		} else {
			if (c==tun.tc.t_stopc) {
				tp->t_state |= TTSTOP;
				(*cdevsw[major(tp->t_dev)].d_stop)(tp);
				return;
			}
			if (c==tun.tc.t_startc)
				return;
		}
		if (c==tun.tc.t_quitc || c==tun.tc.t_intrc) {
			flushtty(tp);
			c = (c==tun.tc.t_intrc) ? SIGINT:SIGQUIT;
			signal(tp->t_pgrp, c);
			return;
		}
		if (c=='\r' && t_flags&CRMOD)
			c = '\n';
	}
	if (tp->t_rawq.c_cc>TTYHOG) {
		flushtty(tp);
		return;
	}
	if (t_flags&LCASE && c>='A' && c<='Z')
		c += 'a'-'A';
	putc(c, &tp->t_rawq);
	if (t_flags&(RAW|CBREAK)||(c=='\n'||c==tun.tc.t_eofc||c==tun.tc.t_brkc)) {
		if ((t_flags&(RAW|CBREAK))==0 && putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
		wakeup((caddr_t)&tp->t_rawq);
	}
	if (t_flags&ECHO) {
		ttyoutput(c, tp);
		if (c==tp->t_kill && (t_flags&(RAW|CBREAK))==0)
			ttyoutput('\n', tp);
		ttstart(tp);
	}
}


/*
 * block transfer input handler.
 */
void ttyrend(struct tty *tp, char *pb, char *pe)
{
	int	tandem;

	tandem = tp->t_flags&TANDEM;
	if (tp->t_flags&RAW) {
		b_to_q(pb, pe-pb, &tp->t_rawq);
		wakeup((caddr_t)&tp->t_rawq);
	} else {
		tp->t_flags &= ~TANDEM;
		while (pb < pe)
			ttyinput(*pb++, tp);
		tp->t_flags |= tandem;
	}
	if (tandem)
		ttyblock(tp);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
int ttread(struct tty *tp)
{
	if ((tp->t_state&CARR_ON)==0)
		return(0);
	if (tp->t_canq.c_cc || canon(tp))
		while (tp->t_canq.c_cc && passc(getc(&tp->t_canq))>=0)
			;
	return(tp->t_rawq.c_cc + tp->t_canq.c_cc);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
caddr_t ttwrite(struct tty *tp)
{
	int c;

	if ((tp->t_state&CARR_ON)==0)
		return(NULL);
	while (u.u_count) {
		spl5();
		while (tp->t_outq.c_cc > TTHIWAT) {
			ttstart(tp);
			tp->t_state |= ASLEEP;
			sleep((caddr_t)&tp->t_outq, TTOPRI);
		}
		spl0();
		if ((c = cpass()) < 0)
			break;
		ttyoutput(c, tp);
	}
	ttstart(tp);
	return(NULL);
}
