//!-----------------------------------------------------
//!
//! \file development_context.h
//! A development context controls the screen output,
//! keyboard, mouse and pad input. In the real game there
//! is only one context (GAME), in development there can
//! be as many required.
//!
//!-----------------------------------------------------

#pragma once

#ifndef YOLK_CORE_DEVELOPMENT_CONTEXT_H_
#define YOLK_CORE_DEVELOPMENT_CONTEXT_H_


#include "input_listener.h"

namespace Scene {
	class Camera;
}
namespace Core {

//! interface class that each context fills in
class DevelopmentContextInterface : public InputListener {
public:
	virtual ~DevelopmentContextInterface() {};

	//! call when switched to and from this context.
	virtual void enable( bool on ) = 0;

	//! called every frame this context is active
	virtual void update( float fTimeInSecs ) = 0;

	//! Context will call for your to display you debug info.
	virtual void display() {};
	
	//! allows a context to return a scene camera for use with a renderer (optional, if not returns nullptr)
	virtual std::shared_ptr<Scene::Camera> getCamera() const { return std::shared_ptr<Scene::Camera>(); }
};


//! Singleton controller of the development context system
class DevelopmentContext : public Singleton<DevelopmentContext> {
public:
	DevelopmentContext();

	~DevelopmentContext();

	//! add a new context
	void addContext( const char* pName, std::shared_ptr<DevelopmentContextInterface> pInterface );

	//! update the context
	void update(float fTimeInSecs);

	//! render any context stuff
	void display();

	//! move to next context programmatically
	void activateNextContext();

	//! activate a named context
	void activateContext( const char* pName );

	//! returns a named context
	std::shared_ptr<DevelopmentContextInterface> getContext( const char* pName ) const;

	//! return the current context
	std::shared_ptr<DevelopmentContextInterface> getContext() const;

private:
	static const int MAX_CONTEXTS = 10;
	struct Context* contexts;

	unsigned int 	currentContext;
	bool			contextChanged;

};


} // namespace Core


#endif
