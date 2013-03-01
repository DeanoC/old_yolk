typedef union
{
	int intnum;
	double fpnum;
	char str[128];
	Type type;
} YYSTYPE;
#define	TYPE	258
#define	BLANK	259
#define	ALIGN	260
#define	LITTLEENDIAN	261
#define	BIGENDIAN	262
#define	U8	263
#define	U16	264
#define	U32	265
#define	S8	266
#define	S16	267
#define	S32	268
#define	FLOAT	269
#define	DOUBLE	270
#define	INTNUM	271
#define	FPNUM	272
#define	STRING	273
#define	IDENTIFIER	274
#define	NEG	275


extern YYSTYPE yylval;
