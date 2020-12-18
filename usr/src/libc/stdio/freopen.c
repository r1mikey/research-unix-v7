#include <stdio.h>

FILE * freopen(const char *file, const char *mode, FILE *iop)
{
	FILE *_endopen();

	fclose(iop);
	return(_endopen(file, mode, iop));
}
