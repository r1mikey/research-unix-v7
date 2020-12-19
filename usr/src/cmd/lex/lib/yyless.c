
extern char yytext[];
extern int yyleng;
extern int yyprevious;

void
yyless(int x)
{
	char *lastch, *ptr;
	lastch = yytext + yyleng;
	if (x >= 0 && x <= yyleng)
		ptr = x + yytext;
	else
		ptr = (char *)x;
	while (lastch > ptr)
		yyunput(*--lastch);
	*lastch = 0;
	if (ptr > yytext)
		yyprevious = *--lastch;
	yyleng = ptr - yytext;
}
