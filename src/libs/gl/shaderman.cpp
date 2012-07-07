/**
 @file	Z:\Projects\wierd\source\gl\shaderman.cpp

 @brief	Implements the shader manager.
 */
#include "gl.h"
#include "program.h"
#include "core/resources.h"
#include "gpu_constants.h"
#include "gfx.h"
#include "cl/platform.h"
#include "cl/programman.h"

#include "shaderman.h"

// outside namespace
DECLARE_SHARED_WITH_CL( clforgl );
DECLARE_SHARED_WITH_CL( constant_blocks );
DECLARE_FRAGMENT( vs_basic );
DECLARE_FRAGMENT( fs_basic );
DECLARE_FRAGMENT( vs_dbg_linedraw );
DECLARE_FRAGMENT( fs_vertcol );
DECLARE_PROGRAM( basic );
DECLARE_PROGRAM( debuglines );

namespace {
const int MAX_GLSL_INCLUDES = 128;

const char* programTypeStrings[Gl::MAX_PROGRAM_TYPE] = { "vs_", "fs_", "gs_", "tc_", "te_" };
const Gl::PROGRAM_TYPE programTypes[Gl::MAX_PROGRAM_TYPE] = {
	Gl::PT_VERTEX, Gl::PT_FRAGMENT, Gl::PT_GEOMETRY, Gl::PT_TESS_CONTROL, Gl::PT_TESS_EVAL };
const char* wholeProgramShaders[Gl::MAX_PROGRAM_TYPE] = { "vertex()", "fragment()", "geometry()", "tesscontrol()" , "tesseval()" };
const char* ver420 = "#version 420 core\n";
const char* pragDebug ="#pragma debug(on)\n";
const GLenum shaderType[ Gl::MAX_PROGRAM_TYPE ] = { 
	GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER };

}

namespace Gl {
ShaderMan::ShaderMan() {
}

void ShaderMan::initDefaultPrograms() {
	REGISTER_SHARED_WITH_CL( clforgl );
	REGISTER_SHARED_WITH_CL( constant_blocks );
	REGISTER_FRAGMENT( vs_basic );
	REGISTER_FRAGMENT( fs_basic );
	REGISTER_FRAGMENT( vs_dbg_linedraw );
	REGISTER_FRAGMENT( fs_vertcol );

	REGISTER_PROGRAM( basic );
	REGISTER_PROGRAM( debuglines );
}

const char* ShaderMan::getProgramSource( const std::string& prgName ) const {
	NameSourceMap::const_iterator cIt = programSrc.find( prgName );
	if( cIt == programSrc.end() ) {
		// no source under that name :(
		LOG(FATAL) << "Gl Program source " << prgName << " not found";
		return NULL;
	}
	return cIt->second;
}
void ShaderMan::registerProgramSource( const char* name, const char* source ) {
	programSrc[ name ] = source;
}


/// this get called if this program is not in the (in-memory) resource cache
Program* ShaderMan::internalCreate( const Core::ResourceHandleBase* baseHandle, const char* pName, const Program::CreationStruct* creation) {
	using namespace Core;
	// TODO: baseHandle is used for async creation, not used currently

	// find program stage via, stage is _Xs_ or it at start just Xs_ where X is one
	// of v, f, g, tc, te 
	std::string programName( pName );

	PROGRAM_TYPE programStage = MAX_PROGRAM_TYPE;

	for( int i = 0; i < MAX_PROGRAM_TYPE; ++i ) {
		if( std::string::size_type stageLoc = 
				programName.find( programTypeStrings[i] ) != std::string::npos ) {
			// start must be shaderType or preceeded by a _
			if( stageLoc == 1 || programName[stageLoc-2] == '_' ) {
				programStage = programTypes[i];
				break;
			}
		}
	}

	if( programStage == MAX_PROGRAM_TYPE ) {
		// this means use a program object rather than a program pipeline object.
		return internalCreateWholeProgram( baseHandle, pName, creation );
	}

	const char* src = getProgramSource( programName );
	if( src == NULL )
		return NULL;

	Program* prg = CORE_NEW Program();
	std::string srcString( src );

	std::string glstrsrc[ MAX_GLSL_INCLUDES ];
	int count = 0;

	// build array of source string from includes etc.
	preprocess( srcString, count, glstrsrc );

	// convert into form GL consumes
	const char* glsrc[ MAX_GLSL_INCLUDES ];
	for( int i = 0; i < count ; ++ i ) {
		glsrc[i] = glstrsrc[i].c_str();
	}

	prg->type = (MEM_NAME_TYPE) programStage;
	prg->name = glCreateShaderProgramv( programStage, count, glsrc );
	prg->wholeProgram = false;

	glValidateProgram( prg->name );
	GLint validStatus = GL_FALSE;
	glGetProgramiv( prg->name, GL_VALIDATE_STATUS, &validStatus);
	if(validStatus == GL_FALSE) {
        int logLength = 0;
        glGetProgramiv(prg->name, GL_INFO_LOG_LENGTH, &logLength);
        char* log = CORE_STACK_NEW_ARRAY( char, logLength );
        glGetProgramInfoLog(prg->name, logLength, &logLength, log);
        LOG(ERROR) << "Validate program :  " << programName << " :\n" << log;
//		DebugBreak();
		return NULL;
	}
	if( creation || creation->numTransformFeedbackItems > 0 ) {
		TODO_ASSERT( creation->numTransformFeedbackItems > 0 );
	}

	buildUniformTables( prg );

	return prg;
}
Program* ShaderMan::internalCreateWholeProgram( const Core::ResourceHandleBase* baseHandle, const char* pName, const Program::CreationStruct* creation) {
	using namespace Core;
	// TODO: baseHandle is used for async creation, not used currently

	std::string programName( pName );

	Program* prg = CORE_NEW Program();

	const char* src = getProgramSource( programName );
	if( src == NULL )
		return NULL;

	std::vector<std::string> srcFragments;
	std::string srcString( src );

	std::string shaderSrcs[ Gl::MAX_PROGRAM_TYPE ];
	// each type of shader is defined in the source as SHADER() @ #include "blash.glsl" @
	for( int i = 0; i < Gl::MAX_PROGRAM_TYPE; ++i ) {
		std::string::size_type index = 0;
		if( (index = srcString.find( wholeProgramShaders[i] )) != std::string::npos ) {
			std::string::size_type spmark0 = srcString.find( '@', index );
			std::string::size_type spmark1 = srcString.find( '@', spmark0+1 );
			CORE_ASSERT( spmark0 != std::string::npos );
			CORE_ASSERT( spmark1 != std::string::npos );
			CORE_ASSERT( spmark0 != spmark1 );
			std::string shaderSrc( &src[spmark0+1], &src[spmark1] );
			shaderSrcs[i] = shaderSrc;
		}
	}

	prg->generateName( MNT_PROGRAM_OBJECT );
	prg->wholeProgram = true;

	// now create the shader objects using basically the same process as the pipeline objects
	for( int i = 0; i < Gl::MAX_PROGRAM_TYPE; i++ ) {
		prg->wholeProgramShaders[i] = 0;

		if( !shaderSrcs[i].empty() ) {
			std::string glstrsrc[ MAX_GLSL_INCLUDES ];
			int count = 0;

			// build array of source string from includes etc.
			preprocess( shaderSrcs[i], count, glstrsrc );

			// convert into form GL consumes
			const char* glsrc[ MAX_GLSL_INCLUDES ];
			for( int j = 0; j < count ; ++ j ) {
				glsrc[j] = glstrsrc[j].c_str();
			}

			Memory::Name shaderName = glCreateShader( shaderType[i] );
			glShaderSource( shaderName, count, glsrc, NULL );
			GL_CHECK
			glCompileShader( shaderName );
			GL_CHECK

			GLint validStatus = GL_FALSE;
			glGetShaderiv( shaderName, GL_COMPILE_STATUS, &validStatus);
			if(validStatus == GL_FALSE) {
				int logLength = 0;
				glGetShaderiv(shaderName, GL_INFO_LOG_LENGTH, &logLength);
				if( logLength > 0 ) {
					char* log = CORE_STACK_NEW_ARRAY( char, logLength );
					glGetShaderInfoLog(shaderName, logLength, &logLength, log);
					LOG(ERROR) << "Compile shader :  " << programName << " ( " << wholeProgramShaders[i] << " ) :\n" << log;
				} else {
					LOG(ERROR) << "Compile shader :  " << programName << " ( " << wholeProgramShaders[i] << " ) :\n No Log from GL\n";
				}
				return NULL;
			}

			glAttachShader( prg->getName(), shaderName );
			GL_CHECK
			prg->wholeProgramShaders[i] = shaderName;
		}
	}

	if( creation && creation->numTransformFeedbackItems > 0 ) {
		glTransformFeedbackVaryings( prg->getName(), 
										creation->numTransformFeedbackItems, 
										(const char**) creation->transformFeedbackItemNames,
										creation->interleavedItems ? GL_INTERLEAVED_ATTRIBS : GL_SEPARATE_ATTRIBS );
		GL_CHECK
	}

	glLinkProgram( prg->getName() );
	GL_CHECK

	glValidateProgram( prg->getName() );
	GL_CHECK

	GLint validStatus = GL_FALSE;
	glGetProgramiv( prg->getName(), GL_LINK_STATUS, &validStatus);
	if(validStatus == GL_FALSE) {
        int logLength = 0;
        glGetProgramiv(prg->name, GL_INFO_LOG_LENGTH, &logLength);
        char* log = CORE_STACK_NEW_ARRAY( char, logLength );
        glGetProgramInfoLog(prg->name, logLength, &logLength, log);
        LOG(ERROR) << "Link program :  " << programName << " :\n" << log;
//		DebugBreak();
		return NULL;
	}

	buildUniformTables( prg );

	return prg;

}
void ShaderMan::preprocessIncludes( std::string& src, std::list<std::string>& fragments ) {
	using namespace Core;
	std::string::size_type index = 0;
	while( (index = src.find( "#include" )) != std::string::npos ) {
		src[index+0] = '/';
		src[index+1] = '/';
		std::string::size_type spmark0 = src.find( '"', index );
		std::string::size_type spmark1 = src.find( '.', spmark0+1 );
		CORE_ASSERT( spmark0 != std::string::npos );
		CORE_ASSERT( spmark1 != std::string::npos );
		CORE_ASSERT( spmark0 != spmark1 );
		std::string fragName( &src[spmark0+1], &src[spmark1] );
		std::replace( fragName.begin(), fragName.end(), '/', '_' );
		std::replace( fragName.begin(), fragName.end(), '\\', '_' );
		fragments.push_back( fragName );
	}
}

void ShaderMan::preprocessPatchSource( std::string& src ) {
	using namespace Core;
	std::string::size_type index = 0;
	while( (index = src.find( "//#version" )) != std::string::npos ) {
		src[index+0] = ' ';
		src[index+1] = ' ';
	}
	index = 0;
	while( (index = src.find( "//#extension" )) != std::string::npos ) {
		src[index+0] = ' ';
		src[index+1] = ' ';
	}
	while( (index = src.find( "#include" )) != std::string::npos ) {
		src[index+0] = '/';
		src[index+1] = '/';
	}

}

void ShaderMan::preprocess( std::string& srcString, int& count, std::string* glstrsrc ) {
	count = 0;
	glstrsrc[count++] = ver420;
	IF_DEBUG( 
		glstrsrc[count++] = pragDebug;
	);

	std::list<std::string> srcFragments;
	preprocessIncludes( srcString, srcFragments );

	for( auto i = srcFragments.begin(); i != srcFragments.end(); ++i ) {
		const char* incSrc = getProgramSource(*i);
		std::string incSrcStr( incSrc );
		preprocessIncludes( incSrcStr, srcFragments );
	}
	auto initial = srcFragments.front(); srcFragments.pop_front();
	srcFragments.push_back( initial );

	for( auto i = srcFragments.begin(); i != srcFragments.end(); ++i ) {
		const char* incSrc = getProgramSource(*i);
		std::string incSrcStr( incSrc );
		preprocessPatchSource( incSrcStr );
		glstrsrc[count] = incSrcStr;
		glstrsrc[count] += std::string("\n");
		count++;
		CORE_ASSERT( count < MAX_GLSL_INCLUDES-1 );
	}

	srcString += "\n";
	glstrsrc[count++] = srcString.c_str();

}

void ShaderMan::buildUniformTables( Program* prg ) {
	// now work out constant blocks and check for standard layout and matrices
	GLint numUniformBlocks = 0;
	glGetProgramiv( prg->name, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);
	prg->usedBuffers = 0;
	for( int i = 0; i < numUniformBlocks; ++ i ) {
		GLint loc = 0;
		glGetActiveUniformBlockiv( prg->name, i, GL_UNIFORM_BLOCK_BINDING, &loc );
		prg->usedBuffers |= BIT(loc);
	}

	if( prg->usedBuffers & BIT( CF_STATIC ) ) {
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, Static, dummy ); 
	}
	if( prg->usedBuffers & BIT( CF_PER_FRAME ) ) {
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerFrame, frameCount ); 
	}
	if( prg->usedBuffers & BIT( CF_PER_PIPELINE ) ) {
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerPipeline, matrixProjection ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerPipeline, matrixProjectionInverse ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerPipeline, matrixProjectionIT ); 
	}
	if( prg->usedBuffers & BIT( CF_PER_VIEWS ) ) {
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerViews, matrixView ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerViews, matrixViewInverse ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerViews, matrixViewIT ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerViews, matrixViewProjection ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerViews, matrixViewProjectionInverse ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerViews, matrixViewProjectionIT ); 
	}
	if( prg->usedBuffers & BIT( CF_PER_TARGETS ) ) {
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, PerTargets, targetDims ); 
	}
	if( prg->usedBuffers & BIT( CF_STD_OBJECT ) ) {
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorld ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldInverse ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldIT ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldView ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldViewInverse ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldViewIT ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldViewProjection ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldViewProjectionInverse ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixWorldViewProjectionIT ); 
		WIERD_GL_VALIDATE_PRG_OFFSETS( prg->name, StdObject, matrixPreviousWorldViewProjection ); 
	}
}

}