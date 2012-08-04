typedef union 
{
	char	szString[256];
} YYSTYPE;
#define	ERROR_TOKEN	258
#define	PROPERTIES	259
#define	ANIMATIONS	260
#define	TRANSITIONS	261
#define	STRING	262
#define	BRACE_LEFT	263
#define	BRACE_RIGHT	264
#define	SET_ADD	265
#define	SET_REMOVE	266
#define	SET_CONSTANT	267
#define	SET_NOT	268
#define	COMMA	269


extern YYSTYPE yyanimationlval;
