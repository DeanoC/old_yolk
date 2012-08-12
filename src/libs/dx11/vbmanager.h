//!-----------------------------------------------------
//!
//! \file vbmanager.h
//! keeps the number of actual vertex buffers allocated
//! to a minimum
//!
//!-----------------------------------------------------
#if !defined( WIERD_GRAPHICS_VBMANAGER_H )
#define WIERD_GRAPHICS_VBMANAGER_H

#if PLATFORM == WINDOWS
#	pragma once
#endif

// forward decl
struct ID3D11Buffer;

namespace Graphics
{
class RenderContext;

//! This singleton manages all vertex buffers
//! it is constructed inside the Gfx CreateScreen
class VBManager : public Core::Singleton<VBManager>
{
public:
	class VBInstance
	{
	public:
		enum LOCK_FLAGS
		{
			NONE		= 0x0,
			DISCARD		= 0x1,
			NOOVERWRITE	= 0x2
		};
		friend VBManager;

		void* Lock( RenderContext* context, LOCK_FLAGS flags = NONE );
		void Unlock( RenderContext* context );

		void Bind( RenderContext* context, uint32_t uiStreamNum, uint32_t stride );
		void Unbind( RenderContext* context, uint32_t uiStreamNum );

	private:
		uint32_t	m_iStartInBytes;	//!< start of this instance in the vertex buffer
		uint32_t	m_iSizeInBytes;		//!< size of this instance in the vertex buffer
		ID3D11Buffer* m_pVB;			//!< Owned by the vertex buffer manager DO NOT RELEASE
		bool		m_bDynamic;			//!< static or dynamic vertex buffer
	};

	~VBManager();

	// allocate some space (note on Dx11 this is purely for dynamics...)
	const VBInstance AllocVertices( const uint32_t iSizeInBytes, const bool bDynamic = false );

	// allocate and fill in vertex data (for static read-only data)
	const VBInstance AllocVertices( void* _data, const uint32_t _sizeInBytes );

	void FreeVertices( const VBInstance& vbInstance );

	void Reset();

private:
	struct VBAlloc
	{
		ID3D11Buffer* m_pVB;
		uint32_t m_maxFreeSpace;
		uint32_t m_iFreeSpace;
		bool	m_bDynamic;			//!< static or dynamic vertex buffer
		uint32_t m_iFirstUnused;
		uint32_t m_iRefCount;

		VBAlloc( ID3D11Buffer* vb_, const uint32_t free_, bool bDynamic_) :
			m_pVB(vb_), m_maxFreeSpace(free_), m_iFreeSpace(free_), m_bDynamic(bDynamic_), m_iFirstUnused(0), m_iRefCount(0)
			{
			}
	};
	typedef Core::list<VBAlloc> ListVBAlloc;

	ListVBAlloc m_ListVBAllocs;
};

}

#endif // end WIERD_GRAPHICS_VBMANAGER_H