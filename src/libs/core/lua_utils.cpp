//!-----------------------------------------------------
//!
//!	\file lua_utils.cpp	some utilitys for fiddling with
//! lua
//!	(c) 2005 Dean Calver
//!-----------------------------------------------------
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

namespace Core { namespace LuaUtils {

#if PLATFORM != PSP

//! from the lua book, dumps the current lua stack
void StackDump( lua_State* L )
{
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) 
	{  /* repeat for each level */
		int t = lua_type(L, i);

		luabind::object obj(L, i);
		LuaObjectPrint(obj);
		Log << "  ";  /* put a separator */
	}
	Log << "\n";  /* end the listing */
}

//! dumps a lua object to the log
void LuaObjectPrint( const luabind::object& obj )
{
	using namespace luabind;
	switch( obj.type() )
	{
	case LUA_TNIL:
		Log << "LUA : nil\n";
		break;
	case LUA_TBOOLEAN:
		Log << "LUA : (" << object_cast<bool>(obj) << ")\n";
		break;
	case LUA_TLIGHTUSERDATA:
		Log << "LUA : light user data (" << object_cast<unsigned int>(obj) << ")\n";
		break;
	case LUA_TNUMBER:
		Log << "LUA : " << object_cast<float>(obj) << "\n";
		break;
	case LUA_TSTRING:
		Log << "LUA : " << object_cast<std::string>(obj) << "\n";
		break;
	case LUA_TTABLE:
		Log << "LUA : table ------\n";
		{
			object::iterator obIt = obj.begin();
			while( obIt != obj.end() )
			{
				LuaObjectPrint( (*obIt) );
				++obIt;
			}
		}
		Log << "LUA : ------------\n";
		break;
	case LUA_TFUNCTION:
		Log << "LUA : function (" << object_cast<unsigned int>(obj) << ")\n";
		break;
	case LUA_TUSERDATA:
		Log << "LUA : user data (" << object_cast<unsigned int>(obj) << ")\n";
		break;
	case LUA_TTHREAD:
		Log << "LUA : thread type\n";
		break;
	default:
		Log << "LUA : Unknown type\n";
	}

}

//! dumps a lua object to the log
void LuaObjectPrint( luabind::detail::proxy_object& object )
{
	LuaObjectPrint( static_cast<luabind::object>(object) );
}

//! copy a table coping thing only if the dest item isn't nil
void CopyNilElementsOfTable( const luabind::object& src, luabind::object& dest )
{
	if( src.type() == LUA_TTABLE )
	{
		luabind::object::iterator srcIt = src.begin();
		while( srcIt != src.end() )
		{
			if( dest[ srcIt.key() ].type() == LUA_TTABLE )
			{
				CopyNilElementsOfTable( *srcIt, dest.at( srcIt.key() ) );
			} else if( (*srcIt).type() == LUA_TTABLE )
			{
				dest[ srcIt.key() ] = luabind::newtable( Core::LuaS() );
				CopyNilElementsOfTable( *srcIt, dest.at( srcIt.key() ) );
			} else if( dest[ srcIt.key() ].type() == LUA_TNIL )
			{
				dest[ srcIt.key() ] = (*srcIt);
			}
			++srcIt;
		}
	} else
	{
		dest = src;
	}
}

//! copy a table, copies all sub tables etc.
void CopyElementsOfTable( const luabind::object& src, luabind::object& dest )
{
	if( src.type() == LUA_TTABLE )
	{
		luabind::object::iterator srcIt = src.begin();
		while( srcIt != src.end() )
		{
			if( dest[ srcIt.key() ].type() == LUA_TTABLE )
			{
				CopyElementsOfTable( *srcIt, dest.at( srcIt.key() ) );
			} else if( (*srcIt).type() == LUA_TTABLE )
			{
				dest[ srcIt.key() ] = luabind::newtable( Core::LuaS() );
				CopyElementsOfTable( *srcIt, dest.at( srcIt.key() ) );
			} else if( dest[ srcIt.key() ].type() == LUA_TNIL )
			{
				dest[ srcIt.key() ] = (*srcIt);
			}
			++srcIt;
		}
	} else
	{
		dest = src;
	}
}

#endif // PLATFORM != PSP
} } // end namespace Core::LuaUtils

#endif // CORE_USE_LUA
