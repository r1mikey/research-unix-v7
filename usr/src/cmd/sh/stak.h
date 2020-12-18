#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

/* To use stack as temporary workspace across
 * possible storage allocation (eg name lookup)
 * a) get ptr from `relstak'
 * b) can now use `pushstak'
 * c) then reset with `setstak'
 * d) `absstak' gives real address if needed
 */
#define relstak()   ((unsigned char *)(staktop - stakbot))
#define absstak(x)  ((unsigned char *)(((unsigned int)stakbot) + Rcheat(x)))
#define setstak(x)  (staktop = absstak(x))
#define pushstak(c) (*staktop++ = (c))
#define zerostak()  (*staktop = 0)

/* Used to address an item left on the top of
 * the stack (very temporary)
 */
#define curstak() (staktop)

/* `usestak' before `pushstak' then `fixstak'
 * These routines are safe against heap
 * being allocated.
 */
#define usestak()                                                              \
	{                                                                      \
		locstak();                                                     \
	}

/* for local use only since it hands
 * out a real address for the stack top
 */
extern unsigned char * locstak(void);

/* Will allocate the item being used and return its
 * address (safe now).
 */
#define fixstak() endstak(staktop)

/* For use after `locstak' to hand back
 * new stack top and then allocate item
 */
extern unsigned char * endstak(unsigned char *argp);

/* Copy a string onto the stack and
 * allocate the space.
 */
extern unsigned char * cpystak(unsigned char *x);

/* Allocate given ammount of stack space */
extern unsigned char * getstak(int asize);

/* A chain of ptrs of stack blocks that
 * have become covered by heap allocation.
 * `tdystak' will return them to the heap.
 */
extern struct blk *stakbsy;

/* Base of the entire stack */
extern unsigned char *stakbas;

/* Top of entire stack */
extern unsigned char *brkend;

/* Base of current item */
extern unsigned char *stakbot;

/* Top of current item */
extern unsigned char *staktop;

/* Used with tdystak */
extern unsigned char * savstak(void);

extern void tdystak(unsigned char *x);
extern void stakchk(void);
