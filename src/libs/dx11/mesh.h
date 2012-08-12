//!-----------------------------------------------------
//!
//! \file mesh.h
//! the engine graphics mesh class
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_MESH_H)
#define WIERD_GRAPHICS_MESH_H

#if PLATFORM == WINDOWS
#	pragma once
#endif
#if !defined( WIERD_GRAPHICS_GFXRESOURCES_H )
#include "gfxresources.h"
#endif

#if !defined( WIERD_GRAPHICS_RENDERABLE_H )
#include "renderable.h"
#endif

#if !defined( WIERD_GRAPHICS_LIGHTCONTEXT_H )
#include "lightcontext.h"
#endif

namespace Graphics
{
	class Mesh : public Renderable {
	public:
		//! Default ctor creates an empty mesh
		Mesh();
		//! loads the mesh provided
		Mesh( const char* pFilename );
		
		//! dtor
		~Mesh();

		//! inherited from Renderable
		virtual void Render( RenderContext* context, const Core::string& passName );
		
		virtual void DebugDraw( RenderContext* context );

		virtual uint32_t GetActualRenderablesOfType( R_TYPE _type, uint32_t arraySize, Renderable** outArray ) {
			if( _type == R_MESH && arraySize > 0) {
				outArray[0] = this;
				return 1;
			} else {
				return 0;
			}
		}

		WobResourcePtr GetResource() const {
			return m_MeshHandle->Acquire();
		}

	protected:
		WobResourceHandlePtr		m_MeshHandle;
		Math::Matrix4x4				m_TransformMatrix;
		Core::TransformNode			m_SimpleTransformNode;
		Math::Matrix4x4				m_prevWVP;
	};
}; // end namespace Graphics

#endif