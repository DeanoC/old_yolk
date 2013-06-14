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
	if ( QueryPerformanceFrequency( &tmp ) != TRUE ) {
		throw std::exception( "QueryPerformanceFrequency unsupported." );
	}
	s_TickFrequency = tmp.QuadPart;
#elif PLATFORM == POSIX
	s_TickFrequency = 1000000; // micro seconds
#endif

	m_nRealTicks = getInstantTicks();

	m_nTimeTicks = m_nRealTicks;

	m_TimeScale = TimeScale;

	// ensure all times / states are in the normal stable state
	update();
	update();
}


/* Functions */
/// Updates internal 'frame' time ; returns delta time in seconds
float Clock::update()
{
	// Real Ticks
	const int64_t dTicks = updateTicks();

	// Time Ticks
	const float dTime = time( dTicks ) * m_TimeScale;
	m_nTimeTicks += ticks( dTime );

	return dTime;
}

/// Updates internal 'frame' time ; returns FRAME_PERIOD in seconds
float Clock::step()
{
	// Real Ticks (update internal states)
	updateTicks();

	// Time Ticks
	m_nTimeTicks += ticks( FRAME_PERIOD );

	return FRAME_PERIOD;
}


/* PRIVATE */

// Updates internal Tick counters ; returns delta Ticks
int64_t Clock::updateTicks()
{
	// Real Ticks
	const int64_t nOldRealTick = m_nRealTicks;

	m_nRealTicks = getInstantTicks();

	return (m_nRealTicks - nOldRealTick);
}


//! sleep the game for n seconds
void Clock::sleep( float fTimeInSecs )
{
#if PLATFORM == WINDOWS	
	::Sleep( (DWORD)(fTimeInSecs * 1000.f) );
#else
	std::this_thread::sleep_for( std::chrono::milliseconds( (int)(fTimeInSecs*1000.f) ) );
#endif
}



}	//namespace Core
