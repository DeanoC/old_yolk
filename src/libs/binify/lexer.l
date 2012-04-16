/* binify */

%{


#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>	// for isatty()
#endif
#include "binify.h"
#include "parser.tab.hpp"
%}

%option noyywrap
%option c++
%option yyclass="binify"
%option batch

DIGIT	[0-9]
HEXDIGIT [a-fA-F0-9]
LETTER	[a-zA-Z_]

%%



{DIGIT}+|0x{HEXDIGIT}+	{	/* dec or hex constant */
				yylval.intnum = strtoul(yytext, 0, 0);
				return INTNUM;
			}
[-]{DIGIT}+|0x{HEXDIGIT}+	{	/* dec or hex constant */
				yylval.intnum = strtol(yytext, 0, 0);
				return INTNUM;
			}


"-"?[0-9]*"."[0-9]+([eE]("+"|"-")[0-9]+)? {	/* float */
				yylval.fpnum = atof(yytext);
				return FPNUM;
			}

"-"?[0-9]+[eE]("+"|"-")[0-9]+ {	/* float */
				yylval.fpnum = atof(yytext);
				return FPNUM;
			}

"%"[10]+	{	/* binary constant */
				const char*p=yytext+1;
				yylval.intnum=0;
				while( *p )
				{
					yylval.intnum = yylval.intnum << 1;
					if( *p++ == '1' )
						yylval.intnum |= 1;
				}
				return INTNUM;
			}


"u8"		{ return U8; }
"u16"		{ return U16; }
"u32"		{ return U32; }
"s8"		{ return S8; }
"s16"		{ return S16; }
"s32"		{ return S32; }
"float"		{ return FLOAT; }
"double"		{ return DOUBLE; }


".align"	{ return ALIGN; }
".type"		{ return TYPE; }
".littleendian"	{ return LITTLEENDIAN; }
".bigendian"	{ return BIGENDIAN; }
".blank"	{ return BLANK; }

"//".*[\n]	{ return (int)'\n'; }

[ \t\r]		/* soak up whitespace */
[\n]		{ return (int)'\n'; }

"\"".*"\""	{ strcpy( yylval.str, yytext ); return STRING; }

{LETTER}({LETTER}|{DIGIT})*		{
				strcpy( yylval.str, yytext );
				return IDENTIFIER;
			}

.			{
				/* should bail out instead? */
				return (int)(yytext[0]);
			}


%%


