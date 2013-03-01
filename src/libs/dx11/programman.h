//!-----------------------------------------------------
//!
//! \file shadermanager.h
//! shader manager singleton
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_PROGRAMMAN_H_ )
#define YOLK_DX11_PROGRAMMAN_H_
#pragma once

#if !defined( YOLK_DX11_PROGRAM_H_ )
#	include "program.h"
#endif

#if !defined( YOLK_SCENE_PROGRAMMAN_H_ )
#	include "scene/programman.h"
#endif

namespace Dx11 {
	class ProgramMan : public Scene::ProgramMan {
	public:

	protected:
		virtual std::string getSrcHeaderStatement() const override { return std::string("#define D3D_SM5\n"); };

		virtual void* compileShader( Scene::SHADER_TYPES type, const std::string& shader ) override;
		virtual Program* linkProgram( const Program::CreationInfo* creation ) override;
	};

} // end namespace Dx11

#endif //YOLK_DX11_PROGRAMMAN_H_