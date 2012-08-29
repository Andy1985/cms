// ControlServer.h
#ifndef __CONTROL_SERVER_H__
#define __CONTROL_SERVER_H__
///////////////////////////////////////////////////////////////////////////

#include <vector>
#include <list>
#include "../../Supports/MiniLib/Thread.hpp"
#include "../../Supports/MiniLib/SocketSSL.hpp"
#include "Interfaces.h"
#include "SafeIPList.h"
#include "../Common.h"

///////////////////////////////////////////////////////////////////////////

class ControlThread : private Thread
{
public:
	ControlThread();

public:
	bool Start(SOCKET sock, IProcessor* processor);
	void Stop();
	using Thread::IsRunning;

private:
	virtual int Run();

private:
	SocketEx sock_;
	IProcessor* processor_;
};

///////////////////////////////////////////////////////////////////////////

class ControlServer : private Thread
{
public:
	ControlServer();

public:
	bool Start(unsigned short port, IProcessor* processor, const char* ipListFile);
	void Stop();
public:
	void RemoveExitedThreads();

private:
	virtual int Run();

private:
	SafeIPList safeIpList_;

	Socket sock_;
	IProcessor* processor_;

	ThreadLock lockThreads_;
	std::list<ControlThread*> threads_;
};

///////////////////////////////////////////////////////////////////////////
#endif//__CONTROL_SERVER_H__

