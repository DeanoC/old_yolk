//!-----------------------------------------------------
//!
//! \file development_context.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "development_context.h"
#include "sysmsg.h"
#if PLATFORM == WINDOWS
#include "platform_windows/keyboard_win.h"
#include "platform_windows/mouse_win.h"
#endif
#include <boost/shared_ptr.hpp>

namespace Core
{

struct Context {
	char* m_pName;
	std::shared_ptr<Core::DevelopmentContextInterface> m_pInterface;
};

DevelopmentContext::DevelopmentContext() :
	m_iCurrentContext(0),
	m_bContextChanged(false) {
	m_Contexts = CORE_NEW_ARRAY( Context[MAX_CONTEXTS] );
}

DevelopmentContext::~DevelopmentContext() {
	for(int i=0;i < MAX_CONTEXTS;i++)
	{
		if( m_Contexts[i].m_pInterface )
		{
			m_Contexts[i].m_pInterface.reset();
			CORE_DELETE_ARRAY m_Contexts[i].m_pName;
		}
	}

	CORE_DELETE_ARRAY( m_Contexts );
}

void DevelopmentContext::addContext( const char *pName, std::shared_ptr<DevelopmentContextInterface> pInterface ) {
	// find first free slot
	for(int i=0;i < MAX_CONTEXTS;i++)
	{
		if( !m_Contexts[i].m_pInterface )
		{
			m_Contexts[i].m_pInterface = pInterface;
			m_Contexts[i].m_pName = CORE_NEW_ARRAY char[ strlen(pName)+1 ];
			strcpy( m_Contexts[i].m_pName, pName );
			return;
		}
	}

	LOG(INFO) << "Run out of Development Contexts, ignored\n";
}

void DevelopmentContext::update( float fTimeInSecs ) {
	unsigned int iLastContext = m_iCurrentContext;

#if PLATFORM == WINDOWS
	// in all contexts escape is exit
	if( KeyboardWin::exists() && KeyboardWin::get()->keyDown( Core::KT_ESCAPE ) == true ) {
		SystemMessage::get()->Quit();
	}

	if( KeyboardWin::exists() && KeyboardWin::get()->keyDownOnce( Core::KT_TAB ) == true ) {
		activateNextContext();
	}

#endif

	if( m_bContextChanged ) {
		m_Contexts[iLastContext].m_pInterface->enable( false );
		m_Contexts[m_iCurrentContext].m_pInterface->enable( true );
		m_bContextChanged = false;
	}
	if( m_Contexts[m_iCurrentContext].m_pInterface ) {
		DevelopmentContextInterface*  pInterface = m_Contexts[m_iCurrentContext].m_pInterface.get();

#if PLATFORM == WINDOWS
		// simulate pad input on keyboard for windows
		float x = 0.f, y = 0.f;
		if( KeyboardWin::exists() ) {
			x += KeyboardWin::get()->keyDown(KT_A) ? 1.f : 0.f;
			x += KeyboardWin::get()->keyDown(KT_D) ? -1.f : 0.f;
			y += KeyboardWin::get()->keyDown(KT_W) ? 1.f : 0.f;
			y += KeyboardWin::get()->keyDown(KT_S) ? -1.f : 0.f;


			if( KeyboardWin::get()->keyDown(KT_RCONTROL) ) {
				pInterface->padButton1( 0 );
			}
			if( KeyboardWin::get()->keyDown(KT_RSHIFT) ) {
				pInterface->padButton2( 0 );
			}
			if( KeyboardWin::get()->keyDown(KT_DELETE) | 
				KeyboardWin::get()->keyDown(KT_LBRACKET) ) {
					pInterface->padButton3( 0 );
			}
			if( KeyboardWin::get()->keyDown(KT_INSERT) | 
				KeyboardWin::get()->keyDown(KT_RBRACKET) ) {
					pInterface->padButton4( 0 );
			}
			if( KeyboardWin::get()->keyDownOnce(KT_F1) ) {
				pInterface->debugButton1( 0 );
			}
			if( KeyboardWin::get()->keyDownOnce(KT_F2) ) {
				pInterface->debugButton2( 0 );
			}
			if( KeyboardWin::get()->keyDownOnce(KT_F3) ) {
				pInterface->debugButton3( 0 );
			}
			if( KeyboardWin::get()->keyDownOnce(KT_F4) ) {
				pInterface->debugButton4( 0 );
			}
			if( KeyboardWin::get()->keyDownOnce(KT_F5) ) {
				pInterface->debugButton5( 0 );
			}

			pInterface->padXAxisMovement( 0, x );
			pInterface->padYAxisMovement( 0, y );
		}

		float dx = 0;
		float dy = 0;
		if( KeyboardWin::exists() ) {
			dx += KeyboardWin::get()->keyDown(KT_LEFT) ? -0.005f : 0.f;
			dx += KeyboardWin::get()->keyDown(KT_RIGHT) ? 0.005f : 0.f;
			dy += KeyboardWin::get()->keyDown(KT_UP) ? 0.005f : 0.f;
			dy += KeyboardWin::get()->keyDown(KT_DOWN) ? -0.005f : 0.f;
		}

		//-=-=-=-=-=-=-=-=-=-=-=-=-
		// update the mouse input
		if( MouseWin::exists() ) {
			dx += MouseWin::get()->getDeltaXAxis();
			dy += MouseWin::get()->getDeltaYAxis();

			if( MouseWin::get()->getLeftMouseButton() )
				pInterface->mouseLeftButton( );
			if( MouseWin::get()->getRightMouseButton() )
				pInterface->mouseRightButton( );
		}

		if( fabsf(dx) > 1e-5f )
			pInterface->mouseDeltaX( dx );
		if( fabsf(dy) > 1e-5f )
			pInterface->mouseDeltaY( dy );
#endif

		pInterface->update( fTimeInSecs );
	}
}

void DevelopmentContext::activateNextContext() {
	unsigned int iStart = m_iCurrentContext;

	for(int i=0;i < MAX_CONTEXTS;i++) {
		iStart++;
		iStart = iStart % MAX_CONTEXTS;
		if( m_Contexts[iStart].m_pInterface ) {
			break;
		}
	}

	if( iStart != m_iCurrentContext ) {
		m_iCurrentContext = iStart;
		m_bContextChanged = true;
	}
}

void DevelopmentContext::activateContext( const char* pName ) {
	for(int i=0;i < MAX_CONTEXTS;i++) {
		if( m_Contexts[i].m_pInterface && (strcmp(pName, m_Contexts[i].m_pName)==0) ) {
			m_iCurrentContext = i;
			m_bContextChanged = true;
			return;
		}
	}
	LOG(INFO) << "No context of name: " << pName << "\n";
}

void DevelopmentContext::display() {
	if( m_Contexts[m_iCurrentContext].m_pInterface ) {
		m_Contexts[m_iCurrentContext].m_pInterface->display();
	}
}

std::shared_ptr<DevelopmentContextInterface> DevelopmentContext::getContext( const char* pName ) {
	for(int i=0;i < MAX_CONTEXTS;i++) {
		if( m_Contexts[i].m_pInterface && (strcmp(pName, m_Contexts[i].m_pName)==0) ) {
			return m_Contexts[i].m_pInterface;
		}
	}
	LOG(INFO) << "No context of name: " << pName << "\n";

	return std::shared_ptr<DevelopmentContextInterface>();
}

std::shared_ptr<DevelopmentContextInterface> DevelopmentContext::getContext() {
	return m_Contexts[m_iCurrentContext].m_pInterface;
}

};
