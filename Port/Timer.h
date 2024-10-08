#ifndef _TIMER_H
#define _TIMER_H

#include "DelegateLib.h"
#include "LockGuard.h"
#include <list>

using namespace DelegateLib;

/// @brief A timer class provides periodic timer callbacks on the client's 
/// thread of control. Timer is thread safe.
class Timer 
{
public:
	/// Client's register with Expired to get timer callbacks
	SinglecastDelegate0<void> Expired;

	/// Constructor
	Timer(void);

	/// Destructor
	~Timer(void);

	/// Starts a timer for callbacks on the specified timeout interval.
	/// @param[in]	timeout - the timeout in milliseconds.
	void Start(unsigned long timeout);

	/// Stops a timer.
	void Stop();

	/// Gets the enabled state of a timer.
	/// @return		TRUE if the timer is enabled, FALSE otherwise.
	bool Enabled() { return m_enabled; }

	/// Get the current time in ticks. 
	/// @return The current time in ticks. 
    static unsigned long GetTime();

	/// Computes the time difference in ticks between two tick values taking into
	/// account rollover.
	/// @param[in] 	time1 - time stamp 1 in ticks.
	/// @param[in] 	time2 - time stamp 2 in ticks.
	/// @return		The time difference in ticks.
	static unsigned long Difference(unsigned long time1, unsigned long time2);

	/// Called on a periodic basic to service all timer instances. 
	static void ProcessTimers();

private:
	// Prevent inadvertent copying of this object
	Timer(const Timer&);
	Timer& operator=(const Timer&);

	/// Called to check for expired timers and callback registered clients.
	void CheckExpired();

	/// List of all system timers to be serviced.
	static std::list<Timer*> m_timers;
	typedef std::list<Timer*>::iterator TimersIterator;

	/// A lock to make this class thread safe.
	static LOCK m_lock;

	/// TRUE if lock initialized.
	static bool m_lockInit;

	unsigned long m_timeout;		// in ticks
	unsigned long m_expireTime;		// in ticks
	bool m_enabled;
	static bool m_timerStopped;
};

#endif
