//!-----------------------------------------------------
//!
//! \file timer.cpp
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "clock.h"


namespace Core
{

const float Clock::FRAME_PERIOD = 1.0f / 30.0f;
int64_t Clock::s_TickFrequency;

Clock::Clock( float TimeScale )
{
#if PLATFORM == WINDOWS
	LARGE_INTEGER tmp;

	if ( QueryPerformanceFrequency( &tmp ) != TRUE )
	{
		throw std::exception( "QueryPerformanceFrequency unsupported." );
	}
	s_TickFrequency = tmp.QuadPart;

	QueryPerformanceCounter( &tmp );
	m_nRealTicks = tmp.QuadPart;
#endif
	m_nTimeTicks = m_nRealTicks;

	m_TimeScale = TimeScale;
}


/* Functions */
/// Updates internal 'frame' time ; returns delta time in seconds
float Clock::update()
{
	// Real Ticks
	const int64_t dTicks = updateTicks();

	// Time Ticks
	const float dTime = Clock::Get()->time( dTicks ) * m_TimeScale;
	m_nTimeTicks += Clock::Get()->ticks( dTime );

	return dTime;
}

/// Updates internal 'frame' time ; returns FRAME_PERIOD in seconds
float Clock::step()
{
	// Real Ticks (update internal states)
	updateTicks();

	// Time Ticks
	m_nTimeTicks += Clock::Get()->ticks( FRAME_PERIOD );

	return FRAME_PERIOD;
}


/* PRIVATE */

// Updates internal Tick counters ; returns delta Ticks
int64_t Clock::updateTicks()
{
	// Real Ticks
	const int64_t nOldRealTick = m_nRealTicks;
#if PLATFORM == WINDOWS
	LARGE_INTEGER tmp;
	QueryPerformanceCounter( &tmp );
	m_nRealTicks = tmp.QuadPart;
#endif

	return (m_nRealTicks - nOldRealTick);
}

/*
//! sleep the game for n seconds
void Clock::Sleep( float fTimeInSecs )
{
#if PLATFORM == WINDOWS	
	::Sleep( (DWORD)(fTimeInSecs * 1000.f) );
#endif
}
*/


}	//namespace Core
