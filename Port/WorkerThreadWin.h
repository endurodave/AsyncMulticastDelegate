#ifndef _WORKER_THREAD_H
#define _WORKER_THREAD_H

// @see https://www.codeproject.com/Articles/1095196/Win-Thread-Wrapper-with-Synchronized-Start
// David Lafreniere, Oct 2019

#include "DelegateOpt.h"
#if USE_WIN32_THREADS

#include "ThreadWin.h"

/// @brief A worker thread 
class WorkerThread : public ThreadWin
{
public:
	/// Constructor
	/// @param[in] threadName - the thread name. 
	WorkerThread(const CHAR* threadName);

private:
	/// The worker thread entry function
	virtual unsigned long Process (void* parameter);

	/// Timer callback called when the timer expires. 
	static void CALLBACK TimerExpired(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
};

#endif

#endif