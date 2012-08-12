//!-----------------------------------------------------
//!
//! \file gfx_debug_render.h
//! this implements the Core::DebugRender for the games
//!
//!-----------------------------------------------------

#if !defined( YOLK_DX11_DEBUG_RENDER_H_ )
#define YOLK_DX11_DEBUG_RENDER_H_
#pragma once

#if !defined( YOLK_CORE_DEBUG_RENDER_H_ )
#include "core/debug_render.h"
#endif

#if !defined( YOLK_DX11_VBMANAGER_H_ )
#include "vbmanager.h"
#endif

#if !defined( YOLK_DX11_SHADER_H_ )
#include "shader.h"
#endif

// forward decl
struct ID3DXLine;

namespace Dx11 {
	class GfxDebugRender : public Core::DebugRenderInterface, public Core::Singleton<GfxDebugRender> {
	public:
		GfxDebugRender();
		~GfxDebugRender();

		//! draw a NDC (projection) space line (resolution independent)
		virtual void NDCLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b );
		//! draw a 3D world space line
		virtual void WorldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b );
		//! print some text onto some form of screen console
		virtual void Print( const char* pText );
		//! world sphere
		virtual void WorldSphere( const Core::Colour& colour, const Math::Vector3& pos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix() );
		//! world cylinder
		virtual void WorldCylinder( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() );
		//! world box
		virtual void WorldBox( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() );

		// Actually draw the debug renderables
		void Flush();

		void GpuLineDraw( const D3D11BufferSmartPtr& indirectParams, const D3D11BufferSmartPtr& lineBuffer);

	private:
		void FlushGpuLineBufs();

		static const int MAX_DEBUG_VERTICES			= 1024;
		static const int NUM_TYPES_OF_DEBUG_PRIM	= 4; // NDC Line, World Line, NDC Quad, World Quad
		static const int NDC_LINE_START_INDEX		= MAX_DEBUG_VERTICES * 0;
		static const int WORLD_LINE_START_INDEX		= MAX_DEBUG_VERTICES * 1;
		static const int NDC_QUAD_START_INDEX		= MAX_DEBUG_VERTICES * 2;
		static const int WORLD_QUAD_START_INDEX		= MAX_DEBUG_VERTICES * 3;

		struct Vertex {
			Math::Vector3	pos;
			uint32_t		colour;
		} *pVertices;						//! format and pointer to the line vertices

		struct GpuLineBufs {
			GpuLineBufs( const D3D11BufferSmartPtr& _indirectParams, const D3D11BufferSmartPtr& _lineBuffer )
					: indirectParams( _indirectParams ), lineBuffer( _lineBuffer ) {};
			D3D11BufferSmartPtr indirectParams;
			D3D11BufferSmartPtr lineBuffer;
		};

		VBManager::VBInstance		vertexBuffer;				//! Vertex buffer for the line vertices
		int							numLineVertices;			//!< Number of vertices currently used by the line drawer
		int							numLineWorldVertices;		//!< Number of vertices currently used by the world space line drawer
		uint32_t					lineVDeclHandle;			//!< Vertex decleration for the line
		ShaderFXHandlePtr			lineShader;					//!< Shader resource for the line shader
		DebugRenderInterface*		pPrevDRI;					//!< the previous Debug Render Interface
		RenderContext*				debugContext;
		Core::vector<GpuLineBufs>	gpuLineBufs;
	};
};

#endif //WIERD_GRAPHICS_DEBUG_RENDER_H