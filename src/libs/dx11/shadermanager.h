//!-----------------------------------------------------
//!
//! \file shadermanager.h
//! shader manager singleton
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_SHADERMAN_H_ )
#define YOLK_DX11_SHADERMAN_H_
#pragma once

#if !defined( YOLK_DX11_GFXRESOURCE_H_ )
#include "gfxresources.h"
#endif

#if !defined( YOLK_DX11_SHADER_H_ )
#include "shader.h"
#endif

//forward decl
struct ShaderIncluder;

namespace Dx11 {
	class ShaderManager : public Core::Singleton<ShaderManager>
	{
	public:
		ShaderManager();
		~ShaderManager();

		ShaderFX* InternalLoadShaderFX( const Core::ResourceHandleBase* handle, const char* pFilename, bool preLoad );
		StaticShader* InternalLoadStaticShader( const Core::ResourceHandleBase* baseHandle, const char* pFilename, bool preLoad );

		Core::vector<D3D_SHADER_MACRO>	d3dXShaderMacros;
	private:
		ShaderIncluder*					m_pIncluder;
	};

} // end namespace Graphics

#endif //WIERD_GRAPHICS_SHADERMANAGER_H