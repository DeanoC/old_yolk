//!-----------------------------------------------------
//!
//! \file vdeclmanager.h
//! vertex decleration manager singleton
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_VDECLMANAGER_H_ )
#define YOLK_DX11_VDECLMANAGER_H_
#pragma once

#if !defined( YOLK_DX11_SHADER_H_)
#include "shader.h"
#endif

namespace Dx11 {

	class VDeclManager : public Core::Singleton<VDeclManager> {
	public:
		~VDeclManager();

		//! returns a handle to be passed to SetVertexDecleration 
		uint32_t GetVertexDeclarationHandle(	int numElements, WobVertexElement *type, 
												const ShaderFXHandlePtr shaderHandle, 
												const Core::string&  shaderGroup );

		//! SetVertexDecleration in the device
		void SetVertexDecleration( RenderContext* context, uint32_t handle );

		//! returns the size of a vertex based on its declaration
		uint32_t GetVertexSize( uint32_t handle );

		static uint32_t CalcVertexSize( int numElements, WobVertexElement *type );
		static void SetReferenceShader( const ShaderFXHandlePtr shaderHandle );
	private:
		struct VDeclItem {
			//! The D3D vertex decleration object
			ID3D11InputLayout*		m_inputLayout;
			//! copy of the creation vertex type structure
			int						m_numElements;
			WobVertexElement*		m_Type;
			//! pre-calculated vertex size 
			uint32_t				m_uiVertexSize;
		};

		typedef Core::unordered_map<uint32_t, VDeclItem> VDeclMap;
		VDeclMap	m_VDeclMap;	//!< Maps the hash handle to the decl struct

		static ShaderFXHandlePtr referenceShaderHandle;
	};

} // end namespace Graphics

#endif //WIERD_GRAPHICS_VDECLMANAGER_H