#include <string>
#include <sstream>
#include <ostream>
#include <vector>

#include "FlexLexer.h"
#include "binify.h"
#include "parser.tab.hpp"

extern int yyparse (void *YYPARSE_PARAM);

int g_LineNum;
int g_Debug = 1;
int g_ShowHelp;
char g_InFile[512];
char g_OutFile[512];

std::ostream* g_OutFP;
std::vector<uint8_t> g_OutputBuf;

uint32_t g_Offset;	// current byte pos in binary output

int g_Pass;	// 0 or 1



void Binify( const std::string& txt, std::ostream& out ) {

	std::istringstream inp(txt);

	g_Offset = 0;
	g_LineNum = 0;
	g_Pass = 0;

	bindy parser0( &inp, &out );
	yyparse( &parser0 );

	g_Offset = 0;
	g_LineNum = 0;
	g_Pass = 1;

	std::istringstream inp1(txt);
	inp.seekg (0, std::ios::beg);

	bindy parser1( &inp1, &out );
	yyparse( &parser1 );

	if( g_OutputBuf.empty() == false ) {
		g_OutFP->write( (const char*)&g_OutputBuf[0], g_OutputBuf.size() );
	}

	g_OutputBuf.clear();

}
