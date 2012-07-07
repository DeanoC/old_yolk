//!-----------------------------------------------------
//!
//! \file linear_allocator.h
//! A class for allocating RAM from a single block in
//! a simple linear fashion (i.e. simple allocs more from
//! the block, free are basically NOP)
//!
//!-----------------------------------------------------

#pragma once

#ifndef WEIRD_CORE_LINEAR_ALLOCATOR_H
#define WEIRD_CORE_LINEAR_ALLOCATOR_H


#include "core_utils.h"



namespace Core
{


//! a simple linear allocator for memory doesn't use pointer (just offset)
//! assume that you will only use it for chunks of memory less the size of an 
//! int can hold
template< int NUM_CHECKPOINTS = 8 >
class LinearAllocator
{
public:
	LinearAllocator( unsigned int size ) :
		m_iBlockSize( size ),
		m_iCurFree( 0 ),
		m_iCurCheckPoint( 0 )
	{
	}

	//! reserves amt of memory from the allocator
	unsigned int Alloc( unsigned int amt )
	{
		unsigned int iOffset = m_iCurFree;

		m_iCurFree += amt;
		assert( m_iCurFree < m_iBlockSize );
		return iOffset;
	}

	//! free the block, the offset must have come from Alloc
	void Free( unsigned int iOffset )
	{
		UNUSED( iOffset );
	}

	//! reset the linear allocator back to empty
	void Reset()
	{
		m_iCurFree = 0;
	}

	//! get unused ram (how much is left)
	unsigned int getUnused()
	{
		return (m_iBlockSize - m_iCurFree);
	}

	//! A checkpoint is a known place that can be reset to
	//! leaving anything before it alone
	void PushCheckpoint()
	{
		assert( m_iCurCheckPoint < NUM_CHECKPOINTS );
		m_iCheckPoint[ m_iCurCheckPoint++ ]  = m_iCurFree;
	}

	//! pops back to the last check point
	void PopCheckpoint()
	{
		assert( m_iCurCheckPoint > 0 );
		m_iCurFree = m_iCheckPoint[ m_iCurCheckPoint-- ];
	}

	//! resets memory to a specific checkpoint, all later checkpoints and allocs are wiped
	void ResetToCheckpoint( unsigned int iCheckpoint )
	{
		assert( iCheckpoint < m_iCurCheckPoint  );
		m_iCurFree = m_iCheckPoint[ iCheckpoint ];
		m_iCurCheckPoint = iCheckpoint;
	}

protected:
	const unsigned int m_iBlockSize;
	unsigned int m_iCurFree;
	unsigned int m_iCheckPoint[NUM_CHECKPOINTS];
	unsigned int m_iCurCheckPoint;

};


enum MemLinearAllocatorType
{
	MLAT_ALLOC,
	MLAT_NOALLOC,
};


template< MemLinearAllocatorType ALLOC_TYPE = MLAT_ALLOC, int NUM_CHECKPOINTS = 8 >
	class MemLinearAllocator;


template< int NUM_CHECKPOINTS >
class MemLinearAllocator<MLAT_NOALLOC,NUM_CHECKPOINTS > : public LinearAllocator<NUM_CHECKPOINTS>
{
public:
	MemLinearAllocator( unsigned int sizeInBytes, char* pBase ) : 
		LinearAllocator<NUM_CHECKPOINTS>( sizeInBytes ),
		m_pBase( pBase )
		{
		}
		~MemLinearAllocator()
		{
		}
	char* Alloc( unsigned int amt )
	{
		return( m_pBase + LinearAllocator<NUM_CHECKPOINTS>::Alloc(amt) );
	}
	void Free( char* pPtr )
	{
		UNUSED( pPtr );
	}
	char* GetBasePtr()
	{
		return m_pBase;
	}
private:
	using LinearAllocator<NUM_CHECKPOINTS>::Alloc;
	using LinearAllocator<NUM_CHECKPOINTS>::Free;

	char* m_pBase;
};


template< int NUM_CHECKPOINTS >
class MemLinearAllocator<MLAT_ALLOC, NUM_CHECKPOINTS> : public MemLinearAllocator< MLAT_NOALLOC, NUM_CHECKPOINTS>
{
public:
	MemLinearAllocator( unsigned int sizeInBytes ) : 
		MemLinearAllocator< MLAT_NOALLOC, NUM_CHECKPOINTS>( sizeInBytes, (char*) malloc(sizeInBytes) )
		{
		}
		~MemLinearAllocator()
		{
			free( MemLinearAllocator< MLAT_NOALLOC, NUM_CHECKPOINTS>::GetBasePtr() );
		}

};


}	//namespace Core


#endif
