#pragma once
//!-----------------------------------------------------
//!
//! \file debugprims.h
//! this implements the Core::DebugRender for the scene
//!
//!-----------------------------------------------------

#if !defined( YOLK_SCENE_DEBUGPRIMS_H_ )
#define YOLK_SCENE_DEBUGPRIMS_H_

#if !defined( YOLK_CORE_DEBUG_RENDER_H_ )
#	include "core/debug_render.h"
#endif
#if !defined( YOLK_SCENE_DATABUFFER_H_ )
#	include "databuffer.h"
#endif
#if !defined( YOLK_SCENE_VERTEXINPUT_H_ )
#	include "vertexinput.h"
#endif
#if !defined( YOLK_SCENE_PROGRAM_H_ )
#	include "program.h"
#endif
#if !defined( YOLK_SCENE_RENDERCONTEXT_H_ )
#	include "rendercontext.h"
#endif

namespace Scene {
	class DebugPrims :	public Core::DebugRenderInterface {
	public:
		DebugPrims();
		~DebugPrims();

		//! draw a NDC (projection) space line (resolution independent)
		virtual void ndcLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b ) override;
		//! draw a 3D world space line
		virtual void worldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b ) override;
		//! print some text onto some form of screen console
		virtual void print( const char* pText ) override;
		//! world sphere
		virtual void worldSphere( const Core::Colour& colour, const Math::Vector3& pos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override;
		//! world cylinder
		virtual void worldCylinder( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override;
		//! world box
		virtual void worldBox( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override;

		virtual int getNumberOfVarPrints() const override { return 0; };
		virtual void varPrint( const int _index, const Math::Vector2& _pos, const char* _text ) override {};


		// Actually draw the debug renderables
		void render( RenderContext* context ) ;

	private:

		static const int MAX_DEBUG_VERTICES			= 1*1024*1024;
		static const int NUM_TYPES_OF_DEBUG_PRIM	= 2; // NDC Line, World Line
		static const int NDC_LINE_START_INDEX		= MAX_DEBUG_VERTICES * 0;
		static const int WORLD_LINE_START_INDEX		= MAX_DEBUG_VERTICES * 1;

		struct Vertex {
			Math::Vector3	pos;
			uint32_t		colour;
		};						//! format to the line vertices
		Vertex						vertices[ MAX_DEBUG_VERTICES * 2 ];

		std::atomic<int>			numLineVertices;			//!< Number of vertices currently used by the line drawer
		std::atomic<int>			numLineWorldVertices;		//!< Number of vertices currently used by the world space line drawer
		DebugRenderInterface*		pPrevDRI;					//!< the previous Debug Render Interface

		DataBufferHandlePtr			vertexBufferHandle;
		VertexInputHandlePtr		vaoHandle;
		ProgramHandlePtr			debugProgramHandle;	
	};
};

#endif //YOLK_SCENE_DEBUGPRIMS_H_