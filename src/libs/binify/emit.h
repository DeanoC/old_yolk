#ifndef EMIT_H
#define EMIT_H


void EmitIntDefault( int i );
void EmitFloatDefault( double f );

void EmitString( const char* str );
void EmitFloat( double d );
void EmitDouble( double d );
void EmitU8( uint32_t i );
void EmitU16( uint32_t i );
void EmitU32( uint32_t i );
void EmitS8( int i );
void EmitS16( int i );
void EmitS32( int i );

void SetDefaultType( Type type );
void SetByteOrder( ByteOrder order );
void Align( int boundary );
void Blank( int count );

void SetSymbol( const char* name, int val );
bool LookupSymbol( const char* name, int& val );

void EndOfLine( bool blank );

#endif // EMIT_H

