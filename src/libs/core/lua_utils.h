//!-----------------------------------------------------
//!
//! \file lua_utils.h
//! just some helper functions for debugging lua state
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_LUA_UTILS_H
#define WIERD_CORE_LUA_UTILS_H


// forward decl
struct lua_State;


namespace Core
{

namespace LuaUtils
{

// currently no luabind on PSP
#if PLATFORM != PSP
//! dumps the current lua stack to the log
void StackDump( lua_State* L );

//! dumps a lua object to the log
void LuaObjectPrint( const luabind::object& object );

//! dumps a lua proxy object to the log
void LuaObjectPrint( luabind::detail::proxy_object& pobject );

void CopyNilElementsOfTable( const luabind::object& src, luabind::object& dest );
void CopyElementsOfTable( const luabind::object& src, luabind::object& dest );
#endif

}	//namespace LuaUtils

}	//namespace Core


#endif
