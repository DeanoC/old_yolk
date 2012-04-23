#ifdef _MSC_VER
#pragma warning (disable : 4786)
#endif

#include <map>
#include <cstring>

#include <cstdio>
#include <cstdlib>
#include <assert.h>


#include "binify.h"
#include "emit.h"

static Type s_DefaultType = typeU32;
static ByteOrder s_ByteOrder = LittleEndian;

static unsigned int g_debugMin = 0x958;
static unsigned int g_debugMax = 0x9c0;

#define BINIFY_LITTLE_ENDIAN
//#define BINIFY_BIG_ENDIAN


//--------------------------------------------
// Symbol table
typedef std::map< std::string, int > SymTab;
static SymTab s_SymTab;

void SetSymbol( const char* name, int val )
{
	if( g_Pass == 0 )
	{
		if( g_Debug )
			printf("%d: Set %s=%d\n", g_LineNum, name,val );

		SymTab::const_iterator it = s_SymTab.find( name );
		if( it != s_SymTab.end() )
			fprintf( stderr, "WARNING: redefining '%s'\n", name );

		s_SymTab[name] = val;
	}
}

bool LookupSymbol( const char* name, int& val )
{
//	printf("Lookup( %s )\n", name );
	if( g_Pass == 0 )
	{
		val = 0;
		return true;
	}

	SymTab::const_iterator it = s_SymTab.find( name );
	if( it == s_SymTab.end() )
		return false;
	val = it->second;
	return true;
}


// everything goes out through here.
static void ByteOut( uint8_t b ) {

	++g_Offset;

	if( g_Pass == 1 )
	{
		if( g_Debug ) {
			if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
				printf( " 0x%02x", (int)b );
			}
		}

		g_OutputBuf.push_back( b );
	}
}


// Dump out a multibyte value as a series of bytes.
// (value is assumed to be in native byteorder)
static void ValueOut( const void* value, int size )
{
	const uint8_t* p = (const uint8_t*)value;
	int i;
#ifdef BINIFY_LITTLE_ENDIAN
	if( s_ByteOrder == LittleEndian )
	{
		for( i=0; i<size; ++i )		// native order
			ByteOut( p[i] );
	}
	else	// BigEndian
	{
		for( i=size-1; i>=0; --i )
			ByteOut( p[i] );
	}
#else	// BINIFY_BIG_ENDIAN
	if( s_ByteOrder == LittleEndian )
	{
		for( i=size-1; i>=0; --i )
			ByteOut( p[i] );
	}
	else	// BigEndian
	{
		for( i=0; i<size; ++i )	// native order
			ByteOut( p[i] );
	}
#endif
}




void SetDefaultType( Type type )
{
	s_DefaultType = type;
}

void SetByteOrder( ByteOrder order )
{
	s_ByteOrder = order;
}

// pad with zeros to get to the specified boundary
void Align( int boundary )
{
	uint32_t pad = boundary - (g_Offset % boundary);
	if( pad==boundary )
		pad = 0;

	if( g_Debug && g_Pass == 1 )
	{
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: .align to %d (offset is %d implies %d pad bytes)\n  (",
				g_LineNum, boundary, g_Offset, pad );
		}
	}

	while( pad-- )
		ByteOut(0);

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf(")\n");
		}
}


// emit a run of zeros
void Blank( int count )
{
	if( g_Debug && g_Pass == 1 )
		printf( "%d: .blank( %d bytes ): ", g_LineNum, count );
	while( count-- )
		ByteOut(0);
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf(")\n");
		}

}


void EmitString( const char* str )
{
	const char* p;
	int len;
	int i;

	len = std::strlen(str);
	assert( len >= 2 );
	assert( str[0] == '\"' );
	assert( str[len-1] == '\"' );

	if( g_Debug && g_Pass == 1 )
		printf("%d: string ( ", g_LineNum );

	p = str+1;
	i=1;
	while( i<len-1 )
	{
		char c = *p++;
		++i;
		if( c == '\\' )
		{
			assert( i<len-1 );	// should be proper error check?
			++i;
			switch( *p++ )
			{
			case '0': c='\0'; break;
			case 'n': c='\n'; break;
			case 't': c='\t'; break;
			case 'r': c='\r'; break;
			case '\"': c='\"'; break;
			case '\\': c='\\'; break;
			}
		}
		ByteOut( c );
	}
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}


void EmitU8( uint32_t i )
{
	uint8_t b = (uint8_t)(i & 0xff);

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: u8( %d ):", g_LineNum, (int)i );
		}

	if( i > 0xff )
		fprintf( stderr, "WARNING: value out of range for U8!\n" );

	ByteOut( b );
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitU16( uint32_t i )
{
	uint16_t j = (uint16_t)(i&0xffff);

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: u16( %d ):", g_LineNum, (int)i );
		}

	if( i > 0xffff )
		fprintf( stderr, "WARNING: value out of range for U16!\n" );

	ValueOut( &j, 2 );

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitU32( uint32_t i )
{
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: u32( %d ): ", g_LineNum, (int)i );
		}

	ValueOut( &i, 4 );

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}


void EmitS8( int i )
{
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: s8( %d ):", g_LineNum, i );
		}

	if( i < -0x80 || i > 0x7f )
		fprintf( stderr, "WARNING: value out of range for S8!\n" );

	int8_t j = (int8_t)i;
	ByteOut( (uint8_t)j );
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitS16( int i )
{
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: s16( %d ):", g_LineNum, i );
		}
	if( i < -0x8000 || i > 0x7ffff )
		fprintf( stderr, "WARNING: value out of range for S16!\n" );

	int16_t j = (int16_t)i;
	ValueOut( &j, 2 );

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitS32( int i )
{
	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: s32( %d ): ", g_LineNum, (int)i );
		}

	ValueOut( &i, 4 );

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitFloat( double d )
{
	const float f = (float)d;
	const uint8_t* p = (const uint8_t*)&f;

	// TODO: range check the float

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: float( %f ):", g_LineNum, (float)d );
		}

	assert( sizeof(float)==4 );
	ValueOut( &f, sizeof(float) );

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitDouble( double d )
{
	const uint8_t* p = (const uint8_t*)&d;

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "%d: double( %f ):", g_LineNum, (float)d );
		}

	assert( sizeof(double)==8 );
	ValueOut( &d, sizeof(double) );

	if( g_Debug && g_Pass == 1 )
		if( g_OutputBuf.size() > g_debugMin && g_OutputBuf.size() < g_debugMax ) {
			printf( "\n" );
		}
}

void EmitIntDefault( int i )
{
	switch( s_DefaultType )
	{
		case typeU8: EmitU8( i ); break;
		case typeU16: EmitU16( i ); break;
		case typeU32: EmitU32( i ); break;
		case typeS8: EmitS8( i ); break;
		case typeS16: EmitS16( i ); break;
		case typeS32: EmitS32( i ); break;
		case typeFLOAT: EmitFloat( (float)i ); break;
		case typeDOUBLE: EmitDouble( (double)i ); break;
	}
}

void EmitFloatDefault( double f )
{
	// floats always output as floats, despite the default type.
	// only exception is if defaulttype is double.
	switch( s_DefaultType )
	{
		case typeDOUBLE:
			EmitDouble( f );
			break;
		default:
			EmitFloat( (float)f );
			break;
	}
}

void EndOfLine( bool blank )
{
	++g_LineNum;
}




