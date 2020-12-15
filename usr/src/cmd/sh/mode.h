#
/*
 *	UNIX shell
 */


#define BYTESPERWORD	(sizeof(char *))

typedef char	BOOL;
typedef char	*STKPTR;
typedef void	*BYTPTR;

typedef struct stat	STATBUF;	/* defined in /usr/sys/stat.h */
typedef struct blk	*BLKPTR;
typedef struct fileblk	FILEBLK;
typedef struct filehdr	FILEHDR;
typedef struct fileblk	*FILE;
typedef struct trenod	*TREPTR;
typedef struct forknod	*FORKPTR;
typedef struct comnod	*COMPTR;
typedef struct swnod	*SWPTR;
typedef struct regnod	*REGPTR;
typedef struct parnod	*PARPTR;
typedef struct ifnod	*IFPTR;
typedef struct whnod	*WHPTR;
typedef struct fornod	*FORPTR;
typedef struct lstnod	*LSTPTR;
typedef struct argnod	*ARGPTR;
typedef struct dolnod	*DOLPTR;
typedef struct ionod	*IOPTR;
typedef struct namnod	NAMNOD;
typedef struct namnod	*NAMPTR;
typedef struct sysnod	SYSNOD;
typedef struct sysnod	*SYSPTR;
#define NIL	((void*)0)


/* the following nonsense is required
 * because casts turn an Lvalue
 * into an Rvalue so two cheats
 * are necessary, one for each context.
 */
#define Lcheat(a)	(*(int *)&(a))
#define Rcheat(a)	((int)(a))


/* address puns for storage allocation */
typedef union {
	FORKPTR	_forkptr;
	COMPTR	_comptr;
	PARPTR	_parptr;
	IFPTR	_ifptr;
	WHPTR	_whptr;
	FORPTR	_forptr;
	LSTPTR	_lstptr;
	BLKPTR	_blkptr;
	NAMPTR	_namptr;
	BYTPTR	_bytptr;
	}	address;


/* heap storage */
struct blk {
	BLKPTR	word;
};

#define	BUFSIZ	64
struct fileblk {
	int	fdes;
	unsigned int	flin;
	BOOL	feof;
	char	fsiz;
	char *	fnxt;
	char *	fend;
	char *	*feval;
	FILE	fstak;
	char	fbuf[BUFSIZ];
};

/* for files not used with file descriptors */
struct filehdr {
	int	fdes;
	unsigned int	flin;
	BOOL	feof;
	char	fsiz;
	char *	fnxt;
	char *	fend;
	char *	*feval;
	FILE	fstak;
	char	_fbuf[1];
};

struct sysnod {
	char *	sysnam;
	int	sysval;
};

/* this node is a proforma for those that follow */
struct trenod {
	int	tretyp;
	IOPTR	treio;
};

/* dummy for access only */
struct argnod {
	ARGPTR	argnxt;
	char	argval[1];
};

struct dolnod {
	DOLPTR	dolnxt;
	int	doluse;
	char	dolarg[1];
};

struct forknod {
	int	forktyp;
	IOPTR	forkio;
	TREPTR	forktre;
};

struct comnod {
	int	comtyp;
	IOPTR	comio;
	ARGPTR	comarg;
	ARGPTR	comset;
};

struct ifnod {
	int	iftyp;
	TREPTR	iftre;
	TREPTR	thtre;
	TREPTR	eltre;
};

struct whnod {
	int	whtyp;
	TREPTR	whtre;
	TREPTR	dotre;
};

struct fornod {
	int	fortyp;
	TREPTR	fortre;
	char *	fornam;
	COMPTR	forlst;
};

struct swnod {
	int	swtyp;
	char *	swarg;
	REGPTR	swlst;
};

struct regnod {
	ARGPTR	regptr;
	TREPTR	regcom;
	REGPTR	regnxt;
};

struct parnod {
	int	partyp;
	TREPTR	partre;
};

struct lstnod {
	int	lsttyp;
	TREPTR	lstlef;
	TREPTR	lstrit;
};

struct ionod {
	int	iofile;
	char *	ioname;
	IOPTR	ionxt;
	IOPTR	iolst;
};

#define	FORKTYPE	(sizeof(struct forknod))
#define	COMTYPE		(sizeof(struct comnod))
#define	IFTYPE		(sizeof(struct ifnod))
#define	WHTYPE		(sizeof(struct whnod))
#define	FORTYPE		(sizeof(struct fornod))
#define	SWTYPE		(sizeof(struct swnod))
#define	REGTYPE		(sizeof(struct regnod))
#define	PARTYPE		(sizeof(struct parnod))
#define	LSTTYPE		(sizeof(struct lstnod))
#define	IOTYPE		(sizeof(struct ionod))
