struct	exec {	/* a.out header */
	unsigned int	a_magic;	/* magic number */
	unsigned int	a_text; 	/* size of text segment */
	unsigned int	a_data; 	/* size of initialized data */
	unsigned int	a_bss;  	/* size of unitialized data */
	unsigned int	a_syms; 	/* size of symbol table */
	unsigned int	a_entry; 	/* entry point */
	unsigned int	a_unused;	/* not used */
	unsigned int	a_flag; 	/* relocation info stripped */
};

#define	A_MAGIC1	0407       	/* normal */
#define	A_MAGIC2	0410       	/* read-only text */
#define	A_MAGIC3	0411       	/* separated I&D */
#define	A_MAGIC4	0405       	/* overlay */

struct	nlist {	/* symbol table entry */
	char		n_name[8];	/* symbol name */
	unsigned short	n_type;    	/* type flag */
	unsigned short	n_pad;    	/* ... */
	unsigned int	n_value;	/* value */
};

		/* values for type flag */
#define	N_UNDF	0	/* undefined */
#define	N_ABS	01	/* absolute */
#define	N_TEXT	02	/* text symbol */
#define	N_DATA	03	/* data symbol */
#define	N_BSS	04	/* bss symbol */
#define	N_TYPE	037
#define	N_REG	024	/* register name */
#define	N_FN	037	/* file name symbol */
#define	N_EXT	040	/* external bit, or'ed in */
#define	FORMAT	"%06o"	/* to print a value */
