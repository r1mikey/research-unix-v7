#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"dup.h"

int              ioset;

/* ========	input output and file copying ======== */

initf(fd)
	int		fd;
{
	FILE	f=standin;

	f->fdes=fd; f->fsiz=((flags&(oneflg|ttyflg))==0 ? BUFSIZ : 1);
	f->fnxt=f->fend=f->fbuf; f->feval=0; f->flin=1;
	f->feof=FALSE;
}

estabf(s)
	char *	s;
{
	FILE	f;

	(f=standin)->fdes = -1;
	f->fend=length(s)+(f->fnxt=s);
	f->flin=1;
	return(f->feof=(s==0));
}

push(af)
	FILE		af;
{
	FILE	f;

	(f=af)->fstak=standin;
	f->feof=0; f->feval=0;
	standin=f;
}

pop()
{
	FILE	f;

	if( (f=standin)->fstak
	){	if( f->fdes>=0 ){ close(f->fdes) ;}
		standin=f->fstak;
		return(TRUE);
	} else {	return(FALSE);
	;}
}

chkpipe(pv)
	int		*pv;
{
	if( pipe(pv)<0 || pv[INPIPE]<0 || pv[OTPIPE]<0
	){	error(piperr);
	;}
}

chkopen(idf)
	char *		idf;
{
	int		rc;

	if( (rc=open(idf,0))<0
	){	failed(idf,badopen);
	} else {	return(rc);
	;}
}

rename(f1,f2)
	int		f1, f2;
{
	if( f1!=f2
	){	dup2(f1, f2);
		close(f1);
		if( f2==0 ){ ioset|=1 ;}
	;}
}

create(s)
	char *		s;
{
	int		rc;

	if( (rc=creat(s,0666))<0
	){	failed(s,badcreate);
	} else {	return(rc);
	;}
}

tmpfil()
{
	itos(serial++); movstr(numbuf,tmpnam);
	return(create(tmpout));
}

/* set by trim */
BOOL		nosubst;

copy(ioparg)
	IOPTR		ioparg;
{
	char		c, *ends;
	char	*cline, *clinep;
	int		fd;
	IOPTR	iop;

	if( iop=ioparg
	){	copy(iop->iolst);
		ends=mactrim(iop->ioname); if( nosubst ){ iop->iofile &= ~IODOC ;}
		fd=tmpfil();
		iop->ioname=cpystak(tmpout);
		iop->iolst=iotemp; iotemp=iop;
		cline=locstak();

		for(;;){	clinep=cline; chkpr(NL);
			while( (c = (nosubst ? readc() :  nextc(*ends)),  !eolchar(c)) ){ *clinep++ = c ;}
			*clinep=0;
			if( eof || eq(cline,ends) ){ break ;}
			*clinep++=NL;
			write(fd,cline,clinep-cline);
		}
		close(fd);
	;}
}
