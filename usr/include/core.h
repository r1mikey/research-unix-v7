/* machine dependent stuff for core files */
#define TXTRNDSIZ 4196L
#define stacktop(siz) (0x60000000L)
#define stackbas(siz) (0x60000000L-siz)
