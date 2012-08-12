//!-----------------------------------------------------
//!
//! \file material.h
//! the engine graphics material class
//!
//!-----------------------------------------------------
#if !defined(WIERD_GRAPHICS_MATERIAL_H)
#define WIERD_GRAPHICS_MATERIAL_H

#pragma once

#if !defined( WIERD_GRAPHICS_VBMANAGER_H)
#include "vbmanager.h"
#endif

#if !defined( WIERD_GRAPHICS_IBMANAGER_H)
#include "ibmanager.h"
#endif

#if !defined( WIERD_GRAPHICS_SHADER_H )
#include "shader.h"
#endif

#if !defined( WIERD_CORE_AABB_H )
#include "core/aabb.h"
#endif

namespace Graphics {
	// forward decl
	struct WobMaterial;
	class LightContext;
	class RenderContext;

	//! To the graphics system as material is the basic unit of rendering
	//! a mesh. 
	class Material {
	public:

		Material();
		virtual ~Material();

		virtual void Render( RenderContext* context, const Core::string& renderPassName ) = 0;

		//! Shader used for this material
		ShaderFXHandlePtr			m_ShaderHandle;

		//! maximum number of texture supported
		static const int MAX_TEXTURES = 16;

		//! texture handles 
		uint32_t numTextures;
		TextureHandlePtr				m_TextureHandles[MAX_TEXTURES];
		ShaderFX::SemanticHandle		m_FXTextureHandles[MAX_TEXTURES];		//!< FX handle for where each texture above should go

		// culling box
		Core::AABB							localAabb;

		static const char* s_TextureParameterArray[];
	};

	//! Contains the geoemtry data (vertices etc.) as well as 
	//! actual material data (shaders and parameters) for a wob material
	class MaterialWob : public Material {
	public:
		//! ctor
		MaterialWob();

		//! dtor
		~MaterialWob();

		//! sets up the material from a WobMaterial
		void CreateFromWobMaterial( WobMaterial& wobMaterial );

		void Render( RenderContext* context, const Core::string& renderPassName );

		uint32_t GetVertexSize() const {
			return m_uiVertexSize;
		}

	private:
		//! handle to pass the vertex decleration manager
		Core::unordered_map<Core::string, uint32_t>	m_VDeclHandles;
		//! size of each vertex
		uint32_t				m_uiVertexSize;
		//! vertex buffer for this material
		VBManager::VBInstance	m_VertexBuffer;
		//! index buffer for this materail
		IBManager::IBInstance	m_IndexBuffer;
		WobMaterial*			origWobMaterial;
		uint32_t				materialId;
		static uint32_t			highMaterialId; // simple incremntal id field
	};
} // end namespace Graphics


#endif // WIERD_GRAPHICS_MATERIAL_H