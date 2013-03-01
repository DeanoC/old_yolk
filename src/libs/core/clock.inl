//!-----------------------------------------------------
//!
//! \file timer.inl
//! A global timer service
//!
//!-----------------------------------------------------



/* Static */
/// Convenience functions to translate ticks to time
CORE_INLINE float Clock::time( int64_t nTicks )
{
	return nTicks / float( s_TickFrequency );
}

/// Convenience functions to translate time to ticks
CORE_INLINE int64_t Clock::ticks( float Time )
{
	return int64_t( Time * s_TickFrequency );
}

/// Get instantaneous ticks
CORE_INLINE int64_t Clock::getInstantTicks()
{
	int64_t nTicks;
#if PLATFORM == WINDOWS
	LARGE_INTEGER tmp;
	QueryPerformanceCounter( &tmp );
	nTicks = tmp.QuadPart;
#elif PLATFORM == POSIX
    struct timeval time;
    /* Grab the current time. */
    gettimeofday(&time, NULL);
    nTicks = time.tv_usec + time.tv_sec * 1000000;   
#endif
	return nTicks;
}


/* Members */
/// Property
CORE_INLINE int64_t Clock::getTimeTicks() const
{
	return m_nTimeTicks;
}

/// ditto
CORE_INLINE float Clock::getTimeScale() const
{
	return m_TimeScale;
}
/// ditto
CORE_INLINE void Clock::setTimeScale( float Scale )
{
	m_TimeScale = Scale;
}
