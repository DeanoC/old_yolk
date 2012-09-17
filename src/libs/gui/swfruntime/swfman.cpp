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
		SwfFrameItem::s_asPropertyStringMap = CORE_NEW SwfFrameItem::AsPropertyStringMap();
		SwfFrameItem::s_asPropertyStringMap->clear();
		(*SwfFrameItem::s_asPropertyStringMap)[ "_x" ] = 0;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_y" ] = 1;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_X" ] = 0;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_Y" ] = 1;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_xscale" ] = 2;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_yscale" ] = 3;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_currentframe" ] = 4;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_totalframes" ] = 5;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_alpha" ] = 6;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_visible" ] = 7;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_width" ] = 8;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_height" ] = 9;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_rotation" ] = 10;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_target" ] = 11;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_framesloaded" ] = 12;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_name" ] = 13;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_droptarget" ] = 14;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_url" ] = 15;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_highquality" ] = 16;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_focusrect" ] = 17;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_soundbuftime" ] = 18;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_quality" ] = 19;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_xmouse" ] = 20;
		(*SwfFrameItem::s_asPropertyStringMap)[ "_ymouse" ] = 21;
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

		rasterStateHandle.reset( RasteriserStateHandle::create( "_RS_Normal_NoCull" ) );
		renderStateHandle.reset( RenderTargetStatesHandle::create( "_RTS_PMOver_WriteAll" ) );

}
SwfMan::~SwfMan() {
}

void SwfMan::bind( Scene::RenderContext* _ctx ) {

	auto vi = vertexInputHandle.acquire();
	auto prg = mainProgramHandle.acquire();
	auto rs = rasterStateHandle.acquire();
	auto rts = renderStateHandle.acquire();
	_ctx->bind( rts );
	_ctx->bind( rs );
	_ctx->bind( vi );
	_ctx->bind( prg );

}

}