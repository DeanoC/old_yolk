//!-----------------------------------------------------
//!
//!	\file luastate.cpp	Bind weird and lua together
//!	(c) 2004 Dean Calver
//!-----------------------------------------------------

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "core/core.h"

#if defined( CORE_USE_LUA )

// lua and luabind includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
}
#if PLATFORM != PSP
#include "luabind/luabind.hpp"
#endif

#include "luastate.h"
#include "lua_utils.h"


//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Local Classes and structures
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Local Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

namespace
{
#if PLATFORM != PSP
	void lua_print( const luabind::object& obj )
	{
		Core::LuaUtils::LuaObjectPrint( obj );
	}

	void lua_ALERT( const luabind::object& obj )
	{
		using namespace luabind;
		Log << "Lua ALERT : " << object_cast<Core::string>(obj) << "\n";
	}
#endif

	int lua_panic (lua_State *L) 
	{
		UNUSED(L);
		CORE_ASSERT( false && "lua has panic'ed, we are stuffed!" );
		return 0;
	}
}

namespace Core
{
//! ctor
LuaStateContainer::LuaStateContainer() :
	m_LuaState( 0 )
{
	m_LuaState = lua_open();

	lua_atpanic( m_LuaState, lua_panic );
	luaopen_base( m_LuaState );
	luaopen_table( m_LuaState );
	luaopen_io( m_LuaState );
	luaopen_string( m_LuaState );
	luaopen_math( m_LuaState );
	luaopen_debug( m_LuaState );
//	luaopen_loadlib( m_LuaState );

	// initialise the socket library
//	luaopen_socket( m_LuaState );
#if PLATFORM != PSP

	using namespace luabind;
	luabind::open( m_LuaState );
	module(m_LuaState)
	[
		def("print", &lua_print ),
		def("_ALERT", &lua_ALERT )
	];

#endif
}

//! dtor
LuaStateContainer::~LuaStateContainer()
{
	lua_close( m_LuaState );
	m_LuaState = 0;
}

}
/**
Short description.
Detailed description
@param param description
@return description
@exception description*/

#endif // CORE_USE_LUA
