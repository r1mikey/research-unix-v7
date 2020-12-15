#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#define TRUE (-1)
#define FALSE 0
#define LOBYTE 0377
#define STRIP 0177
#define QUOTE 0200

#define EOF 0
#define NL '\n'
#define SP ' '
#define LQ '`'
#define RQ '\''
#define MINUS '-'
#define COLON ':'

#define MAX(a, b) ((a) > (b) ? (a) : (b))
