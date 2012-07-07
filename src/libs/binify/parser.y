/* binify */

%{
#include <math.h>
#include <stdio.h>
#include "binify.h"

#include "emit.h"

void yyerror( const char* s );
%}


%union
{
	int intnum;
	double fpnum;
	char str[128];
	Type type;
}

%token TYPE
%token BLANK
%token ALIGN
%token LITTLEENDIAN
%token BIGENDIAN
%token <type> U8 U16 U32 S8 S16 S32 FLOAT DOUBLE
%token <intnum> INTNUM
%token <fpnum>	FPNUM
%token <str> STRING
%token <str> IDENTIFIER
%left '-' '+' 
%left '*' '/'
%left '|' 
%left '(' ')'
%left NEG

%type <intnum> intexp
%type <fpnum> fpexp
%type <type> type

%%

input:		/* empty */
		| input line
;

line:	'\n'					{ EndOfLine( true ); }
		|	run '\n'			{ EndOfLine( false ); }
	;


run:	item
		| run ',' item
;


type:	U8			{ $$=typeU8; }
		| U16		{ $$=typeU16; }
		| U32		{ $$=typeU32; }
		| S8		{ $$=typeS8; }
		| S16		{ $$=typeS16; }
		| S32		{ $$=typeS32; }
		| FLOAT		{ $$=typeFLOAT; }
		| DOUBLE	{ $$=typeDOUBLE; }
;

item:	'(' U8 ')' intexp		{ EmitU8( $4 ); }
		| '(' U16 ')' intexp	{ EmitU16( $4 ); }
		| '(' U32 ')' intexp	{ EmitU32( $4 ); }
		| '(' S8 ')' intexp		{ EmitS8( $4 ); }
		| '(' S16 ')' intexp	{ EmitS16( $4 ); }
		| '(' S32 ')' intexp	{ EmitS32( $4 ); }
		| '(' FLOAT ')' fpexp	{ EmitFloat( $4 ); }
		| '(' DOUBLE ')' fpexp	{ EmitDouble( $4 ); }
		| intexp				{ EmitIntDefault( $1 ); }
		| fpexp					{ EmitFloatDefault( $1 ); }
		| STRING				{ EmitString( $1 ); }
		| ALIGN intexp			{ Align( $2 ); }
		| BLANK intexp			{ Blank( $2 ); }
		| TYPE type				{ SetDefaultType( $2 ); }
		| LITTLEENDIAN			{ SetByteOrder( LittleEndian ); }
		| BIGENDIAN				{ SetByteOrder( BigEndian ); }
		| IDENTIFIER ':'		{ SetSymbol( $1, (int)g_Offset ); }
;





intexp: INTNUM					{ $$=$1; }
		| intexp '-' intexp		{ $$=$1-$3; }
		| intexp '+' intexp		{ $$=$1+$3; }
		| '(' intexp ')'		{ $$=$2; }
/*		| intexp '|' intexp		{ $$=$1|$3; }*/
		| IDENTIFIER			{
				//					printf("IDENTIFIER '%s'\n", $1 );
									int val;
									if( LookupSymbol( $1, val ) ) {
										$$=val;
									} else {
										fprintf( stderr, "%d: Couldn't find '%s'\n", g_LineNum, $1 );
										YYERROR;
									}
								}
;

fpexp: FPNUM			{ $$=$1; }
	 /*
	| fpexp '+' fpexp	{ $$=$1+$3; }
	| fpexp '-' fpexp	{ $$=$1-$3; }
	| fpexp '*' fpexp	{ $$=$1*$3; }
	| fpexp '/' fpexp	{ $$=$1/$3; }
	| '-' fpexp %prec NEG	{ $$=-$2; }
	| '(' fpexp ')'		{ $$=$2; }
	*/
;



%%

void yyerror( const char* s )
{
	fprintf( stderr, "%d: %s\n", g_LineNum, s );
}


