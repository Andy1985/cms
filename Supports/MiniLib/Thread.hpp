// Thread.hpp
#ifndef __THREAD_HPP__
#define __THREAD_HPP__
///////////////////////////////////////////////////////////////////////////
/* Usage:

class SampleThread : private Thread
{
public:
	bool Start()
	{
		if (Thread::IsRunning()) return false;
		exitFlag_ = false;
		return Thread::StartThread();
	}

	void Stop()
	{
		exitFlag_ = true;
		Thread::WaitForExit();
	}

	virtual int Run()
	{
		while (exitFlag_ == false)
		{
			// do something in thread....
		}
		return 0;
	}

private:
	volatile bool exitFlag_;
};

void Foo()
{
	SampleThread thread;
	thread.Start();

	// waiting for exit....

	thread.Stop();
}

*/
// NOTE: class Thread is not thread-safe!
//       So starting one thread object in same thread is recommended
///////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define THREAD_MAIN_PROC DWORD WINAPI
typedef DWORD ThreadProcReturnType;

#else

#include <pthread.h>
#include <unistd.h>

#define THREAD_MAIN_PROC void*
typedef void* ThreadProcReturnType;

#endif

#include <cassert>

///////////////////////////////////////////////////////////////////////////

class Thread
{
public:
	Thread() : threadHandle_(0) { }
	virtual ~Thread() { assert(threadHandle_ == 0); }
private:
	Thread(const Thread&); // disable copy
	void operator = (const Thread&);

public:
	bool StartThread()
	{
		if (threadHandle_ != 0) return false;

#ifdef _WIN32
		DWORD threadId = 0;
		threadHandle_ = CreateThread(NULL, 0, ThreadMain, this, 0, &threadId);
		if (threadHandle_ == NULL)
		{
			return false;
		}
#else
		if (pthread_create(&threadHandle_, NULL, ThreadMain, this) != 0)
		{
			return false;
		}
#endif
		return true;
	}

	void WaitForExit()
	{
		while (IsRunning())
		{
			Sleep(1);
		}
	}

	bool IsRunning() const
	{
		return (threadHandle_ != 0);
	}

public:
	bool SetHighPriority()
	{
#ifdef _WIN32
		if (SetThreadPriority(threadHandle_, THREAD_PRIORITY_HIGHEST) == FALSE)
		{
			return false;
		}
#else
		struct sched_param param;
		param.sched_priority = sched_get_priority_max(SCHED_FIFO);
		if (pthread_setschedparam(threadHandle_, SCHED_FIFO, &param) != 0)
		{
			return false;
		}
#endif
		return true;
	}

	void TerminateThread()
	{
#ifdef _WIN32
		::TerminateThread(threadHandle_, 0);
#else
		pthread_cancel(threadHandle_);
#endif
		threadHandle_ = 0;
	}

private:
	static THREAD_MAIN_PROC ThreadMain(void* param)
	{
		Thread* thisPointer = reinterpret_cast<Thread*>(param);

		ThreadProcReturnType n = (ThreadProcReturnType)thisPointer->Run();

#ifdef _WIN32
		CloseHandle(thisPointer->threadHandle_);
#else
		pthread_detach(thisPointer->threadHandle_);
#endif
		thisPointer->threadHandle_ = 0;
		return n;
	}

	virtual int Run() = 0;

private:
#ifdef _WIN32
	volatile HANDLE threadHandle_;
#else
	pthread_t threadHandle_;
#endif

public:
	static void Sleep(long milliseconds)
	{
#ifdef _WIN32
		::Sleep(milliseconds);
#else
		usleep(milliseconds * 1000);
#endif
	}
};

///////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

class ThreadLock
{
public:
	ThreadLock()  { InitializeCriticalSection(&criticalSection_); }
	~ThreadLock() { DeleteCriticalSection    (&criticalSection_); }
private:
	ThreadLock(const ThreadLock&); // disable copy
	void operator = (const ThreadLock&);
public:
	void Lock()   { EnterCriticalSection     (&criticalSection_); }
	void Unlock() { LeaveCriticalSection     (&criticalSection_); }
private:
	CRITICAL_SECTION criticalSection_;
};

#else//_WIN32

#include <pthread.h>

class ThreadLock
{
public:
	ThreadLock()  { pthread_mutex_init   (&mutex_, NULL); }
	~ThreadLock() { pthread_mutex_destroy(&mutex_);       }
private:
	ThreadLock(const ThreadLock&); // disable copy
	void operator = (const ThreadLock&);
public:
	void Lock()   { pthread_mutex_lock   (&mutex_);       }
	void Unlock() { pthread_mutex_unlock (&mutex_);       }
private:
	pthread_mutex_t mutex_;
};

#endif//_WIN32
///////////////////////////////////////////////////////////////////////////

class ThreadLockScope
{
public:
	ThreadLockScope(ThreadLock& lock) : lock_(lock) { lock_.Lock();   }
	~ThreadLockScope()                              { lock_.Unlock(); }
private:
	ThreadLockScope(const ThreadLockScope&); // disable copy
	void operator = (const ThreadLockScope&);
private:
	ThreadLock& lock_;
};

///////////////////////////////////////////////////////////////////////////
#endif//__THREAD_HPP__

