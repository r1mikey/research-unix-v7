#include	<stdio.h>

FILE * fopen(const char *file, const char *mode)
{
	FILE *_findiop(), *_endopen();

	return(_endopen(file, mode, _findiop()));
}
