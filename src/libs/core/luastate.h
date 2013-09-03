//!-----------------------------------------------------
//!
//! \file luastate.h
//!   Copyright (C) 2004 by Dean Calver                                     
//!   deano@cloudpixies.com                                              
//!
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_LUASTATE_H
#define WIERD_CORE_LUASTATE_H


// forward declarations
struct lua_State;


namespace Core
{


class LuaStateContainer : public Singleton<LuaStateContainer>
{
public:
	LuaStateContainer();
	~LuaStateContainer();

	lua_State* getState(){ return m_LuaState; };

	bool isValid(){ return (m_LuaState != 0); };

private:
	lua_State* m_LuaState;
};
// nasty short cut to save my fingers :-)
inline lua_State* LuaS()
{
	return LuaStateContainer::Get()->getState();
}


}	//namespace Core


#endif
