//!-----------------------------------------------------
//!
//! \file timer.h
//! A global timer service
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_TIMER_H
#define WIERD_CORE_TIMER_H


namespace Core
{


//! Clock Class using RDTSC on x86/x86-64
class Clock : public Singleton< Clock >
{

public:
	Clock( float TimeScale = 1.0f );


	/* Static */
	//! Convenience functions to translate ticks to time
	static float time( int64_t nTicks );

	//! Convenience functions to translate time to ticks
	static int64_t ticks( float Time );

	//! Get instantaneous ticks
	static int64_t getInstantTicks();

	/* Members */
	//! Property
	int64_t getTimeTicks() const;

	//! ditto
	float getTimeScale() const;
	//! ditto
	void setTimeScale( float Scale );

	/* Functions */
	//! Updates internal 'frame' time ; returns delta time in seconds
	float update();

	//! Updates internal 'frame' time ; returns FRAME_PERIOD in seconds
	float step();


private:
	static const float FRAME_PERIOD;
	static int64_t s_TickFrequency;

private:
	// Updates internal Tick counters ; returns delta Ticks
	int64_t updateTicks();


private:
	int64_t m_nRealTicks;
	int64_t m_nTimeTicks;

	float m_TimeScale;

};


#include "clock.inl"


}	//namespace Core


#endif
