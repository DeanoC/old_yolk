///-------------------------------------------------------------------------------------------------
/// \file	core\core_utils.h
///
/// \brief	Declares the core utils class. 
///
/// \details	
///		just some general helper macros and functions
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef YOLK_CORE_UTILS_H_
#define YOLK_CORE_UTILS_H_

///-------------------------------------------------------------------------------------------------
/// \def	NUM_ARRAY_ELEMENTS( array ) ( sizeof( (array) ) / sizeof( (array)[0] ) )
///
/// \brief	Returns the number of elements in a staic array. 
///
/// \remarks	Deano, 03/10/2012. 
///
/// \param	array	The array must be static.
/// \todo	Google has a version that detects it is a static array, this just bugs out if not... 
////////////////////////////////////////////////////////////////////////////////////////////////////
#define NUM_ARRAY_ELEMENTS( array ) ( sizeof( (array) ) / sizeof( (array)[0] ) )

///-------------------------------------------------------------------------------------------------
/// \def	BIT( num ) (1 << (num))
///
/// \brief	return the bit of the number. 
///
/// \remarks	Deano, 03/10/2012. 
///
/// \param	num	returns the bit representing this index. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#define BIT( num ) (1UL << (num))
#define BIT64( num ) (1ULL << (num))

///-------------------------------------------------------------------------------------------------
/// \def	UNUSED( i ) (void)i
///
/// \brief	marks the variable unused. 
///
/// \remarks	Deano, 03/10/2012. 
///
/// \param	i	The variable to mark as unused. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#define UNUSED( i ) (void)i

///-------------------------------------------------------------------------------------------------
/// \def	MACRO_TEXT(x) #x
///
/// \brief	Converts parameter in text. 
///
/// \remarks	Deano, 03/10/2010. 
///
/// \param	x	The parameter. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MACRO_TEXT(x) #x

///-------------------------------------------------------------------------------------------------
/// \def	MACRO_VALUE(x) MACRO_TEXT(x)
///
/// \brief	returns the text of the value of the parameter. 
///
/// \remarks	Deano, 03/10/2010. 
///
/// \param	x	The parameter. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MACRO_VALUE(x) MACRO_TEXT(x)


namespace Core {


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE int floor2Int( float value )
///
/// \brief	float to ine. 
///
/// \param	value	The float value to convert. 
///
/// \return	the integer value of the float (truncate AKA floor). 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE int floor2Int( float value )
{
	int output;
#if CPU_FAMILY == CPU_X86 && COMPILER == MS_COMPILER
	_asm
	{
		fld value
		fistp output
	}
#else
	output = static_cast<int>( floorf(value) );
#endif

	return output;
}


///-------------------------------------------------------------------------------------------------
/// \fn	CALL CORE_INLINE bool isPow2( unsigned int iNum )
///
/// \brief	Query if 'iNum' is power of two. 
///
/// \param	iNum	Number to test. 
///
/// \return	true if pow 2, false if not. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CALL CORE_INLINE bool isPow2( unsigned int iNum ) 
{
	return ((iNum & (iNum - 1)) == 0);
}


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE unsigned int getNextPow2( unsigned int iNum )
///
/// \brief	Gets the next pow 2. 
///
/// \todo	Fill in detailed method description. 
///
/// \param	iNum	Number to get next pow 2 off. 
///
/// \return	The next pow 2 of iNum. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE unsigned int getNextPow2( unsigned int iNum ) 
{
	iNum -= 1;
	iNum |= iNum >> 16;
	iNum |= iNum >> 8;
	iNum |= iNum >> 4;
	iNum |= iNum >> 2;
	iNum |= iNum >> 1;

	return iNum + 1;
}



///-------------------------------------------------------------------------------------------------
/// \struct	GenerateID
///
/// \brief	Generate identifier. 
/// takes 4 chars and generate a unsigned int ala IFF at compile time.
/// To use just code Core::GenerateID<'A','B','C','D'>::value
///
////////////////////////////////////////////////////////////////////////////////////////////////////
template< const char c0, const char c1 = ' ', const char c2 = ' ', const char c3 = ' '>
struct GenerateID {
	//! part of the generate ID template magic
	template<unsigned int c, unsigned int shift, unsigned int last>
	struct ShiftAndOr {
		static const unsigned int value = (c << shift) | last;
	};

	static const unsigned int value =	ShiftAndOr<c3, 24, 
										ShiftAndOr<c2, 16,
										ShiftAndOr<c1, 8,
										ShiftAndOr<c0, 0, 0 >::value >::value >::value >::value;
};


///-------------------------------------------------------------------------------------------------
/// \struct GenerateTemplateHash
///
/// \brief	 the template hash generator. 
/// a template to generate simple HASH string upto 16 chars.
/// To use simple code Core::GenerateTemplateHash< 'H', 'E', 'L', 'L' >::value
///
/// \return	The generate template hash. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template<	const char c0     ,  const char c1='\0',  const char c2='\0',  const char c3='\0', 
			const char c4='\0',  const char c5='\0',  const char c6='\0',  const char c7='\0',
			const char c8='\0',  const char c9='\0',  const char c10='\0', const char c11='\0',
			const char c12='\0', const char c13='\0', const char c14='\0', const char c15='\0' >
struct GenerateTemplateHash
{
	template< unsigned int c,unsigned int last>
	struct HashPipe
	{
		static const unsigned int prime = 0x01000193;
		static const unsigned int value = (c == 0) ? last : ((((uint64_t)last*prime)&0xFFFFFFFF)^c);
	};
	static const unsigned int value =	HashPipe<c0,
										HashPipe<c1,
										HashPipe<c2,
										HashPipe<c3,
										HashPipe<c4,
										HashPipe<c5,
										HashPipe<c6,
										HashPipe<c7,
										HashPipe<c8,
										HashPipe<c9,
										HashPipe<c10,
										HashPipe<c11,
										HashPipe<c12,
										HashPipe<c13,
										HashPipe<c14,
										HashPipe<c15,0	>::value >::value >::value >::value >::value >::value 
														>::value >::value >::value >::value >::value >::value 
														>::value >::value >::value >::value;
};

//! useful for binary loaders
//! currently all file offsets are 32 bit so maximum file size = 4GiB
template<typename T>
T* fixupPointer( void* pBase, const uint32_t offset )
{
	return (T*)(((char*)pBase) + (intptr_t)offset);
}

#if PLATFORM_OS != MS_WINDOWS
static unsigned int LogTable256[] = 
{
	0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};
#endif


///-------------------------------------------------------------------------------------------------
/// \fn	CORE_INLINE unsigned int log2(unsigned int v)
///
/// \brief	return Log2 of v. 
/// return log2 of an int. this is equivalent to finding the highest bit that has been set
/// or the number to shift 1 left by to get the nearest lower power of 2
/// \param	v	The number to get the log2 of. 
///
/// \return	log2(v). 
////////////////////////////////////////////////////////////////////////////////////////////////////
CORE_INLINE unsigned int log2(unsigned int v)
{
#if PLATFORM_OS == MS_WINDOWS
	unsigned long log2 = 0;

	_BitScanReverse( &log2, v );

	return unsigned int( log2 );
#else
	unsigned int r;     // r will be lg(v)
	unsigned int t, tt; // temporaries

	if ((tt = v >> 16) != 0)
	{
		r = ((t = tt >> 8)!=0) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	}
	else 
	{
		r = ((t = v >> 8)!=0) ? 8 + LogTable256[t] : LogTable256[v];
	}
	return r;
#endif
}


///-------------------------------------------------------------------------------------------------
/// \fn	CALL CORE_INLINE unsigned int alignTo(unsigned int k, unsigned int align)
///
/// \brief	Align input to align val. 
///
/// \param	k		The value to align. 
/// \param	align	The alignment boundary. 
///
/// \return	k aligned to align. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template< typename T, typename T2 >
CALL CORE_INLINE T alignTo(T k, T2 align)
{
	return ((k+align-1) & ~(align-1));
}

}	//namespace Core


#endif
