//!-----------------------------------------------------
//!
//! \file program.cpp
//!
//!-----------------------------------------------------

#include "scene.h"
#include "program.h"
#include "programman.h"

namespace Scene {

const void* Program::internalPreCreate( const char* pName, const Program::CreationInfo* load ) {
	return ProgramMan::get()->parseProgram( pName, load );
}

Program* Program::internalCreate( const void* data ) {
	auto creation = (const Program::CreationInfo*) data;
	return ProgramMan::get()->completeProgram( creation );
}

}