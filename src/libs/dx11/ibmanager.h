//!-----------------------------------------------------
//!
//! \file ibmanager.h
//! keeps the number of actual index buffers allocated
//! to a minimum
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_IBMANAGER_H)
#define WIERD_GRAPHICS_IBMANAGER_H

#if PLATFORM == WINDOWS
#	pragma once
#endif

#include <list>

// forward decl
struct IDirect3DIndexBuffer9;

namespace Graphics
{
//! This singleton manages all index buffers
//! it is constructed inside the Gfx CreateScreen
class IBManager : public Core::Singleton<IBManager>
{
public:
	class IBInstance
	{
	public:
		enum LOCK_FLAGS
		{
			NONE		= 0x0,
			DISCARD		= 0x1,
			NOOVERWRITE	= 0x2
		};
		friend IBManager;

		void* Lock( RenderContext* context, LOCK_FLAGS flags = NONE );
		void Unlock( RenderContext* context );

		void Bind( RenderContext* context );

		uint32_t GetStartIndex() const {
			if( m_32bit ) {
				return m_iStartInBytes / sizeof(uint32_t);
			} else {
				return m_iStartInBytes / sizeof(uint16_t);
			}
		}
		uint32_t GetTriangleCount() const {
			if( m_32bit ) {
				return m_iSizeInBytes / (sizeof(uint32_t) *3);
			} else {
				return m_iSizeInBytes / (sizeof(uint16_t) *3);
			}
		}

	private:
		uint32_t		m_iStartInBytes;	//!< start of this instance in the index buffer
		uint32_t		m_iSizeInBytes;		//!< size of this instance in the index buffer
		ID3D11Buffer*	m_pIB;				//!< Owned by the index buffer manager DO NOT RELEASE
		bool			m_bDynamic;			//!< static or dynamic vertex buffer
		bool			m_32bit;			//!< indices are 32 bit
	};

	~IBManager();

	const IBInstance AllocIndices( const uint32_t iSizeInBytes, const bool b32Bit = false, const bool bDynamic = false );
	const IBInstance AllocIndices( void* _data, const uint32_t _sizeInBytes, const bool b32Bit = false );
	void FreeIndices( const IBInstance& ibInstance );

	void Reset();

private:
	struct IBAlloc
	{
		ID3D11Buffer* m_pIB;
		uint32_t m_iFreeSpace;
		bool	m_bDynamic;			//!< static or dynamic vertex buffer
		bool	m_32bit;
		uint32_t m_iFirstUnused;
		uint32_t m_iRefCount;

		IBAlloc( ID3D11Buffer* ib_, const uint32_t free_, bool bDynamic_, bool b32bit_) :
			m_pIB(ib_), m_iFreeSpace(free_), m_bDynamic(bDynamic_), m_32bit(b32bit_), m_iFirstUnused(0), m_iRefCount(0)
			{}
	};
	typedef std::list<IBAlloc> ListIBAlloc;

	ListIBAlloc m_ListIBAllocs;
};

}

#endif // end WIERD_GRAPHICS_VBMANAGER_H