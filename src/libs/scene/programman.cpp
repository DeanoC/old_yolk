/**
 @file	programman.cpp

 @brief	Implements the shader manager.
 */
#include "scene.h"
#include "program.h"
#include "core/resources.h"
#include "scene/gpu_constants.h"
#include "scene/constantcache.h"

#include "programman.h"

#define DECLARE_SHARED( x ) 		extern "C" const char* binary_data_programs_ ## x;
#define DECLARE_PROGRAM( x) 		extern "C" const char* binary_data_programs_programs_ ## x; 
#define DECLARE_SHADER( x )			extern "C" const char* binary_data_programs_shaders_ ## x; 
#include "builtin_programs.h"
#undef DECLARE_SHARED
#undef DECLARE_PROGRAM
#undef DECLARE_SHADER

namespace {
const char* shaderTypeStrings[ Scene::MAX_SHADER_TYPES ] = { "vs_",		"fs_", 			"gs_", 			"hs_", 		"ds_", 		"cs_" 		};
const char* shaderSrcTag[ Scene::MAX_SHADER_TYPES ] 	 = { "vertex()", "fragment()", 	"geometry()", 	"hull()" , 	"domain()", "compute()" };

#define YOLK_VALIDATE_PRG_OFFSETS( p, c, v ) {										\
	uint32_t offset = ~0;																\
	offset = GetOffsetFromProgram( #c, prg, #v );										\
	CORE_ASSERT( offset == ~0 || offsetof( Scene::GPUConstants:: c, v ) == offset );	\
	used |= (offset != ~0);																\
}

// will return ~0(aka -1) if not used, or offset it used
uint32_t GetOffsetFromProgram( const char* block, const Scene::Program* prg, const char* name ) {
	return prg->getVariableOffset( block, name );
}

}
namespace Scene {

void ProgramMan::initDefaultPrograms() {
#define DECLARE_SHARED( x )				this->registerProgramSource( #x, binary_data_programs_ ## x  );
#define DECLARE_PROGRAM( x )			this->registerProgramSource( #x, binary_data_programs_programs_ ## x );
#define DECLARE_SHADER( x )				this->registerProgramSource( "shaders_" #x, binary_data_programs_shaders_ ## x );
#include "builtin_programs.h"
#undef DECLARE_SHARED
#undef DECLARE_PROGRAM
#undef DECLARE_SHADER
}

const char* ProgramMan::getProgramSource( const std::string& prgName ) const {
	NameSourceMap::const_iterator cIt = programSrc.find( prgName );
	if( cIt == programSrc.end() ) {
		// no source under that name :(
		LOG(FATAL) << "Program source " << prgName << " not found";
		return nullptr;
	}
	return cIt->second;
}
void ProgramMan::registerProgramSource( const char* name, const char* source ) {
	programSrc[ name ] = source;
}

void* ProgramMan::parseProgram( const char* pName, const Program::CreationInfo* ) {
	using namespace Core;

	const std::string programName( pName );
	const char* src = getProgramSource( programName );
	if( src == nullptr )
		return nullptr;

	std::vector<std::string> srcFragments;
	std::string srcString( src );

	std::string shaderSrcs[ MAX_SHADER_TYPES ];
	// each type of shader is defined in the source as SHADER() @ #include "blash.glsl" @
	for( int i = 0; i < MAX_SHADER_TYPES; ++i ) {
		std::string::size_type index = 0;
		if( (index = srcString.find( shaderSrcTag[i], index )) != std::string::npos ) {
			std::string::size_type spmark0 = srcString.find( '@', index );
			std::string::size_type spmark1 = srcString.find( '@', spmark0+1 );
			CORE_ASSERT( spmark0 != std::string::npos );
			CORE_ASSERT( spmark1 != std::string::npos );
			CORE_ASSERT( spmark0 != spmark1 );
			std::string shaderSrc( &src[spmark0+1], &src[spmark1] );
			shaderSrcs[i] = shaderSrc;
			index++;
		}
	}

	Program::CreationInfo* cs = CORE_NEW Program::CreationInfo();

	// now create the shader objects using basically the same process as the pipeline objects
	for( int i = 0; i < MAX_SHADER_TYPES; i++ ) {
		if( !shaderSrcs[i].empty() ) {
			// build array of source string from includes etc.
			std::string src;
			preprocess( shaderSrcs[i], src );
			cs->data[i] = compileShader( (SHADER_TYPES) i, src );			
		} else {
			cs->data[i] = nullptr;
		}
	}
	return cs;
}

Program* ProgramMan::completeProgram( const Program::CreationInfo* creation ) {
	Program* prg = linkProgram( creation );
	buildConstantTables( prg );
	return prg;
}

bool ProgramMan::preprocessIncludes( std::string& src ) {
	using namespace Core;
	bool found = false;
	std::string::size_type index = 0;
	while( (index = src.find( "#include", index )) != std::string::npos ) {
		std::string::size_type spmark0 = src.find( '"', index );
		std::string::size_type spmark1 = src.find( '.', spmark0+1 );
		std::string::size_type spmark2 = src.find( '"', spmark0+1 );
		if( spmark1 == std::string::npos ) {
			spmark1 = spmark2;
		}
		CORE_ASSERT( spmark0 != std::string::npos );
		CORE_ASSERT( spmark1 != std::string::npos );
		CORE_ASSERT( spmark0 != spmark1 );
		std::string fragName( &src[spmark0+1], &src[spmark1] );
		std::replace( fragName.begin(), fragName.end(), '/', '_' );
		std::replace( fragName.begin(), fragName.end(), '\\', '_' );
		const char* incSrc = getProgramSource( fragName );
		src.replace( index, (spmark2-index)+1, incSrc );

		found = true;
	}
	return found;
}

void ProgramMan::preprocess( const std::string& srcString, std::string& outsrc ) {
	// let the backend add some statement to the top
	outsrc = getSrcHeaderStatement() + srcString;

	// start of getting the includes from the file given
	do{} while( preprocessIncludes( outsrc ) );
	
	preprocessPatchSource( outsrc );


}

void ProgramMan::preprocessPatchSource( std::string& src ) {
	std::string::size_type index = 0;
	while( (index = src.find( "#include", index )) != std::string::npos ) {
		src[index+0] = '/';
		src[index+1] = '/';
		index++;
	}
}

void ProgramMan::buildConstantTables( Program* prg ) {
	using namespace Scene;

	prg->usedBuffers = 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "Static" ) 		? BIT( CF_STATIC ) 			: 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "PerFrame" )		? BIT( CF_PER_FRAME ) 		: 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "PerPipeline" ) 	? BIT( CF_PER_PIPELINE )	: 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "PerViews" ) 		? BIT( CF_PER_VIEWS ) 		: 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "PerTargets" ) 	? BIT( CF_PER_TARGETS ) 	: 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "StdObject" ) 		? BIT( CF_STD_OBJECT ) 		: 0;
	prg->usedBuffers |= prg->usesConstantBuffer( "PerMaterial" ) 	? BIT( CF_PER_MATERIAL)		: 0;

	if( prg->usedBuffers & BIT( CF_STATIC ) ) {
		bool used = false;
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_STATIC );
		}
	}
	if( prg->usedBuffers & BIT( CF_PER_FRAME ) ) {
		bool used = false;
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerFrame, frameCount ); 
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_PER_FRAME );
		}
	}
	if( prg->usedBuffers & BIT( CF_PER_PIPELINE ) ) {
		bool used = false;
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_PER_PIPELINE );
		}
	}
	if( prg->usedBuffers & BIT( CF_PER_VIEWS ) ) {
		bool used = false;
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixProjection ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixProjectionInverse ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixProjectionIT ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixView ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixViewInverse ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixViewIT ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixViewProjection ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixViewProjectionInverse ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerViews, matrixViewProjectionIT ); 
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_PER_VIEWS );
		}
	}
	if( prg->usedBuffers & BIT( CF_PER_TARGETS ) ) {
		bool used = false;
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerTargets, targetDims ); 
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_PER_TARGETS );
		}
	}
	if( prg->usedBuffers & BIT( CF_STD_OBJECT ) ) {
		bool used = false;
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorld ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldInverse ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldIT ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldView ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldViewInverse ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldViewIT ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldViewProjection ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldViewProjectionInverse ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixWorldViewProjectionIT ); 
		YOLK_VALIDATE_PRG_OFFSETS( prg, StdObject, matrixPreviousWorldViewProjection ); 
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_STD_OBJECT );
		}
	}
	if( prg->usedBuffers & BIT( CF_PER_MATERIAL ) ) {
		bool used = false;
		YOLK_VALIDATE_PRG_OFFSETS( prg, PerMaterial, materialIndex ); 
		if( !used ) {
			prg->usedBuffers &= ~ BIT( CF_PER_MATERIAL );
		}
	}
}

}