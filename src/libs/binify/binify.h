#ifndef BINIFY_H
#define BINIFY_H
#include "Flexlexer.h"
#include <vector>


#define BINIFY_VERSION "0.2"

//#include <stdint.h>
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;


enum type_
{
	typeU8, typeU16, typeU32, typeS8, typeS16, typeS32, typeFLOAT, typeDOUBLE
};
typedef enum type_ Type;

enum byteorder_
{
	LittleEndian, BigEndian
};
typedef enum byteorder_ ByteOrder;

extern int g_LineNum;
extern int g_Debug;
extern int g_ShowHelp;
extern char g_InFile[512];
extern char g_OutFile[512];

extern std::ostream* g_OutFP;
extern uint32_t g_Offset;	// current byte pos in binary output
extern std::vector<uint8_t>	g_OutputBuf;

extern int g_Pass;	// 0 or 1

#define YYPARSE_PARAM bnd
#define YYLEX_PARAM bnd

class bindy : public yyFlexLexer {
public:
	bindy( std::istream* arg_yyin = 0, std::ostream* arg_yyout = 0 ) :
		yyFlexLexer(arg_yyin, arg_yyout) {
			g_OutFP = arg_yyout;
		};

	int yylex();

};

inline int yylex( void* bnd ) {
	return ((bindy*)bnd)->yylex();
}
#endif // BINIFY_H

