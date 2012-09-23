// 
//  swfman.cpp
//  
//  Created by Deano on 19/09/2012.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "scene/programman.h"
#include "scene/vertexinput.h"
#include "gradienttexturemanager.h"
#include "frameitem.h"
#include "swfman.h"

#define DECLARE_SHARED( x ) 		extern "C" const char* binary_data_programs_ ## x;
#define DECLARE_PROGRAM( x) 		extern "C" const char* binary_data_programs_programs_ ## x; 
#define DECLARE_SHADER( x )			extern "C" const char* binary_data_programs_shaders_ ## x; 

DECLARE_PROGRAM( swfmain )
DECLARE_SHADER( vs_swfmain )
DECLARE_SHADER( fs_swfmain )

#undef DECLARE_SHARED
#undef DECLARE_PROGRAM
#undef DECLARE_SHADER

namespace Swf {

SwfMan::SwfMan() {

#if defined( USE_ACTIONSCRIPT )
		FrameItem::s_asPropertyStringMap = CORE_NEW FrameItem::AsPropertyStringMap();
		FrameItem::s_asPropertyStringMap->clear();
		(*FrameItem::s_asPropertyStringMap)[ "_x" ] = 0;
		(*FrameItem::s_asPropertyStringMap)[ "_y" ] = 1;
		(*FrameItem::s_asPropertyStringMap)[ "_X" ] = 0;
		(*FrameItem::s_asPropertyStringMap)[ "_Y" ] = 1;
		(*FrameItem::s_asPropertyStringMap)[ "_xscale" ] = 2;
		(*FrameItem::s_asPropertyStringMap)[ "_yscale" ] = 3;
		(*FrameItem::s_asPropertyStringMap)[ "_currentframe" ] = 4;
		(*FrameItem::s_asPropertyStringMap)[ "_totalframes" ] = 5;
		(*FrameItem::s_asPropertyStringMap)[ "_alpha" ] = 6;
		(*FrameItem::s_asPropertyStringMap)[ "_visible" ] = 7;
		(*FrameItem::s_asPropertyStringMap)[ "_width" ] = 8;
		(*FrameItem::s_asPropertyStringMap)[ "_height" ] = 9;
		(*FrameItem::s_asPropertyStringMap)[ "_rotation" ] = 10;
		(*FrameItem::s_asPropertyStringMap)[ "_target" ] = 11;
		(*FrameItem::s_asPropertyStringMap)[ "_framesloaded" ] = 12;
		(*FrameItem::s_asPropertyStringMap)[ "_name" ] = 13;
		(*FrameItem::s_asPropertyStringMap)[ "_droptarget" ] = 14;
		(*FrameItem::s_asPropertyStringMap)[ "_url" ] = 15;
		(*FrameItem::s_asPropertyStringMap)[ "_highquality" ] = 16;
		(*FrameItem::s_asPropertyStringMap)[ "_focusrect" ] = 17;
		(*FrameItem::s_asPropertyStringMap)[ "_soundbuftime" ] = 18;
		(*FrameItem::s_asPropertyStringMap)[ "_quality" ] = 19;
		(*FrameItem::s_asPropertyStringMap)[ "_xmouse" ] = 20;
		(*FrameItem::s_asPropertyStringMap)[ "_ymouse" ] = 21;
#endif
		using namespace Scene;

#define DECLARE_SHARED( x )				ProgramMan::get()->registerProgramSource( #x, binary_data_programs_ ## x  );
#define DECLARE_PROGRAM( x )			ProgramMan::get()->registerProgramSource( #x, binary_data_programs_programs_ ## x );
#define DECLARE_SHADER( x )				ProgramMan::get()->registerProgramSource( "shaders_" #x, binary_data_programs_shaders_ ## x );

		DECLARE_PROGRAM( swfmain )
		DECLARE_SHADER( vs_swfmain )
		DECLARE_SHADER( fs_swfmain )
		
#undef DECLARE_SHARED
#undef DECLARE_PROGRAM
#undef DECLARE_SHADER

		gradientTextureManager = CORE_NEW GradientTextureManager();

		VertexInput::CreationInfo vocs = {		
			1,	{	{ VE_POSITION, VT_FLOAT2 },  },
				{	{ nullptr, VI_AUTO_OFFSET, VI_AUTO_STRIDE, 0, VI_VERTEX_STREAM }, }
		};

		const std::string viName = "_SWFPlayer" + VertexInput::genEleString(vocs.elementCount, vocs.elements );
		vertexInputHandle = VertexInputHandle::create( viName.c_str(), &vocs );

		mainProgramHandle = ProgramHandle::load( "swfmain" );
		auto prg = mainProgramHandle.acquire();
		auto vi = vertexInputHandle.acquire();
		vi->validate( prg );

		rasterStateHandle.reset( RasteriserStateHandle::create( RENDER_STATE_NORMAL_NOCULL ) );
		norenderStateHandle.reset( RenderTargetStatesHandle::create( RENDER_TARGET_STATES_NOWRITE ) );
		renderStateHandle.reset( RenderTargetStatesHandle::create( RENDER_TARGET_STATES_PMOVER_WRITEALL ) );
		clampSamplerHandle.reset( SamplerStateHandle::create( SAMPLER_STATE_ANISO16_CLAMP ) );
		
		DepthStencilState::CreationInfo normalci = { (DepthStencilState::CreationInfo::FLAGS)0 };
		normalDepthStateHandle.reset( DepthStencilStateHandle::create( "_DS_SWF_Normal", &normalci ) );

		// reserve top two bits of stencil for other uses (None yet but possible clip paths?)
		DepthStencilState::CreationInfo countcrossingci = {
			DepthStencilState::CreationInfo::STENCIL_ENABLE, CF_ALWAYS, 0x3F, 0x3F, 
				SO_KEEP, SO_KEEP, SO_INC, CF_ALWAYS,
				SO_KEEP, SO_KEEP, SO_INC, CF_ALWAYS
		};
		countCrossingDepthStateHandle.reset( DepthStencilStateHandle::create( "_DS_SWF_CountCrossings", &countcrossingci ) );

		// reserve top two bits of stencil for other uses (None yet but possible clip paths?)
		// where stencil is odd render the pixel and clear the stencil buffer
		DepthStencilState::CreationInfo renderoddci = {
			DepthStencilState::CreationInfo::STENCIL_ENABLE, CF_ALWAYS, 0x1, 0x3F, 
				SO_ZERO, SO_ZERO, SO_ZERO, CF_EQUAL,
				SO_ZERO, SO_ZERO, SO_ZERO, CF_EQUAL
		};
		oddDepthStateHandle.reset( DepthStencilStateHandle::create( "_DS_SWF_RenderOdd", &renderoddci ) );

}
SwfMan::~SwfMan() {
}

void SwfMan::bind( Scene::RenderContext* _ctx ) {

	auto vi = vertexInputHandle.acquire();
	auto prg = mainProgramHandle.acquire();
	auto rs = rasterStateHandle.acquire();
	auto rts = renderStateHandle.acquire();
	auto ss = clampSamplerHandle.acquire();
	auto dss = normalDepthStateHandle.acquire();

	_ctx->bind( Scene::ST_FRAGMENT, 0, ss );
	_ctx->bind( dss );
	_ctx->bind( rts );
	_ctx->bind( rs );
	_ctx->bind( vi );
	_ctx->bind( prg );

}

}