#pragma once
//!-----------------------------------------------------
//!
//! \file debugprims.h
//! this implements the Core::DebugRender for the gl path
//!
//!-----------------------------------------------------

#if !defined(WIERD_GL_DEBUG_PRIMS_H)
#define WIERD_GL_DEBUG_PRIMS_H

#if !defined( WIERD_CORE_DEBUG_RENDER_H )
#include "core/debug_render.h"
#endif

#include "databuffer.h"
#include "vao.h"
#include "program.h"

namespace Gl {
	class DebugPrims :	public Core::DebugRenderInterface {
	public:
		DebugPrims();
		~DebugPrims();

		//! draw a NDC (projection) space line (resolution independent)
		virtual void ndcLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b );
		//! draw a 3D world space line
		virtual void worldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b );
		//! print some text onto some form of screen console
		virtual void print( const char* pText );
		//! world sphere
		virtual void worldSphere( const Core::Colour& colour, const Math::Vector3& pos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix() );
		//! world cylinder
		virtual void worldCylinder( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() );
		//! world box
		virtual void worldBox( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() );

		// Actually draw the debug renderables
		void flush();// RenderContext* context );

	private:

		static const int MAX_DEBUG_VERTICES			= 1*1024*1024;
		static const int NUM_TYPES_OF_DEBUG_PRIM	= 2; // NDC Line, World Line
		static const int NDC_LINE_START_INDEX		= MAX_DEBUG_VERTICES * 0;
		static const int WORLD_LINE_START_INDEX		= MAX_DEBUG_VERTICES * 1;

		struct Vertex {
			Math::Vector3	pos;
			uint32_t		colour;
		} *pVertices;						//! format and pointer to the line vertices

		std::atomic<int>			numLineVertices;			//!< Number of vertices currently used by the line drawer
		std::atomic<int>			numLineWorldVertices;		//!< Number of vertices currently used by the world space line drawer
		DebugRenderInterface*		pPrevDRI;					//!< the previous Debug Render Interface

		DataBufferHandlePtr			vertexBufferHandle;
		DataBufferPtr				vertexBuffer;
		VaoHandlePtr				vaoHandle;
		ProgramHandlePtr			debugProgramHandle;	
	};
};

#endif //WIERD_GL_DEBUG_PRIMS_H