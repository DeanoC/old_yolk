//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "development_context.h"
#include "sysmsg.h"
#include "keyboard.h"
//#include "platform_windows/mouse_win.h"

namespace Core
{

struct Context {
	char* m_pName;
	std::shared_ptr<Core::DevelopmentContextInterface> dcinterface;
};

DevelopmentContext::DevelopmentContext() :
	currentContext(0),
	contextChanged(false) {
	contexts = CORE_NEW_ARRAY( Context[MAX_CONTEXTS] );
}

DevelopmentContext::~DevelopmentContext() {
	for(int i=0;i < MAX_CONTEXTS;i++) {
		if( contexts[i].dcinterface ) {
			contexts[i].dcinterface.reset();
			CORE_DELETE_ARRAY contexts[i].m_pName;
		}
	}

	CORE_DELETE_ARRAY( contexts );
}

void DevelopmentContext::addContext( const char *pName, std::shared_ptr<DevelopmentContextInterface> pInterface ) {
	// find first free slot
	for(int i=0;i < MAX_CONTEXTS;i++)
	{
		if( !contexts[i].dcinterface )
		{
			contexts[i].dcinterface = pInterface;
			contexts[i].m_pName = CORE_NEW_ARRAY char[ strlen(pName)+1 ];
			strcpy( contexts[i].m_pName, pName );
			return;
		}
	}

	LOG(INFO) << "Run out of Development Contexts, ignored\n";
}

void DevelopmentContext::update( float fTimeInSecs ) {
	unsigned int lastContext = currentContext;

	// in all contexts escape is exit
	if( Keyboard::exists() && Keyboard::get()->keyDown( Core::KT_ESCAPE ) == true ) {
		SystemMessage::get()->quit();
	}

	if( Keyboard::exists() && Keyboard::get()->keyDownOnce( Core::KT_TAB ) == true ) {
		activateNextContext();
	}

	if( contextChanged ) {
		contexts[lastContext].dcinterface->enable( false );
		contexts[currentContext].dcinterface->enable( true );
		contextChanged = false;
	}

	if( contexts[currentContext].dcinterface ) {
		auto  pInterface = contexts[currentContext].dcinterface;

		// simulate pad input on keyboard for windows
		float x = 0.f, y = 0.f;
		if( Keyboard::exists() ) {
			x += Keyboard::get()->keyDown(KT_A) ? 1.f : 0.f;
			x += Keyboard::get()->keyDown(KT_D) ? -1.f : 0.f;
			y += Keyboard::get()->keyDown(KT_W) ? 1.f : 0.f;
			y += Keyboard::get()->keyDown(KT_S) ? -1.f : 0.f;


			if( Keyboard::get()->keyDown(KT_RCONTROL) ) {
				pInterface->padButton1( 0 );
			}
			if( Keyboard::get()->keyDown(KT_RSHIFT) ) {
				pInterface->padButton2( 0 );
			}
			if( Keyboard::get()->keyDown(KT_DELETE) ) {
					pInterface->padButton3( 0 );
			}
			if( Keyboard::get()->keyDown(KT_INSERT) ) {
					pInterface->padButton4( 0 );
			}
			if( Keyboard::get()->keyDownOnce(KT_F1) ) {
				pInterface->debugButton1( 0 );
			}
			if( Keyboard::get()->keyDownOnce(KT_F2) ) {
				pInterface->debugButton2( 0 );
			}
			if( Keyboard::get()->keyDownOnce(KT_F3) ) {
				pInterface->debugButton3( 0 );
			}
			if( Keyboard::get()->keyDownOnce(KT_F4) ) {
				pInterface->debugButton4( 0 );
			}
			if( Keyboard::get()->keyDownOnce(KT_F5) ) {
				pInterface->debugButton5( 0 );
			}

			if( fabsf(x) > 1e-5f )
				pInterface->padXAxisMovement( 0, x );
			if( fabsf(y) > 1e-5f )
				pInterface->padYAxisMovement( 0, y );
		}

		float dx = 0;
		float dy = 0;
		if( Keyboard::exists() ) {
			dx += Keyboard::get()->keyDown(KT_LEFT) ? -0.005f : 0.f;
			dx += Keyboard::get()->keyDown(KT_RIGHT) ? 0.005f : 0.f;
			dy += Keyboard::get()->keyDown(KT_UP) ? 0.005f : 0.f;
			dy += Keyboard::get()->keyDown(KT_DOWN) ? -0.005f : 0.f;
		}
/*
		//-=-=-=-=-=-=-=-=-=-=-=-=-
		// update the mouse input
		if( Mouse::exists() ) {
			dx += Mouse::get()->getDeltaXAxis();
			dy += Mouse::get()->getDeltaYAxis();

			if( Mouse::get()->getLeftMouseButton() )
				pInterface->mouseLeftButton( );
			if( Mouse::get()->getRightMouseButton() )
				pInterface->mouseRightButton( );
		}
*/
		if( fabsf(dx) > 1e-5f )
			pInterface->mouseDeltaX( dx );
		if( fabsf(dy) > 1e-5f )
			pInterface->mouseDeltaY( dy );

		pInterface->update( fTimeInSecs );
	}
}

void DevelopmentContext::activateNextContext() {
	unsigned int start = currentContext;

	for(int i=0;i < MAX_CONTEXTS;i++) {
		start++;
		start = start % MAX_CONTEXTS;
		if( contexts[start].dcinterface ) {
			break;
		}
	}

	if( start != currentContext ) {
		currentContext = start;
		contextChanged = true;
	}
}

void DevelopmentContext::activateContext( const char* pName ) {
	for(int i=0;i < MAX_CONTEXTS;i++) {
		if( contexts[i].dcinterface && (strcmp(pName, contexts[i].m_pName)==0) ) {
			currentContext = i;
			contextChanged = true;
			return;
		}
	}
	LOG(INFO) << "No context of name: " << pName << "\n";
}

void DevelopmentContext::display() {
	if( contexts[currentContext].dcinterface ) {
		contexts[currentContext].dcinterface->display();
	}
}

std::shared_ptr<DevelopmentContextInterface> DevelopmentContext::getContext( const char* pName ) const {
	for(int i=0;i < MAX_CONTEXTS;i++) {
		if( contexts[i].dcinterface && (strcmp(pName, contexts[i].m_pName)==0) ) {
			return contexts[i].dcinterface;
		}
	}
	LOG(INFO) << "No context of name: " << pName << "\n";

	return std::shared_ptr<DevelopmentContextInterface>();
}

std::shared_ptr<DevelopmentContextInterface> DevelopmentContext::getContext() const {
	return contexts[currentContext].dcinterface;
}

};
