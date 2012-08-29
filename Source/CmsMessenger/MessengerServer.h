// MessengerServer.h
#ifndef __MESSENGER_SERVER_H__
#define __MESSENGER_SERVER_H__
///////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <set>
#include "Interfaces.h"
#include "../../Supports/MiniLib/Thread.hpp"
#include "../../Supports/MiniLib/SocketSSL.hpp"
#include "../../Supports/MiniLib/Network.hpp"
#include "../Common.h"

///////////////////////////////////////////////////////////////////////////

class MessengerServer;

class MessengerThread : private Thread
{
public:
	MessengerThread(SSL_CTX* ctx, const IPAddress& ip, SocketSSLEx* sock);

public:
	bool Start();
	void Stop();
	using Thread::IsRunning;

public:
	IdsInfo GetInfo() const { return IdsInfo(id_, ip_); }
	const std::string& GetId() const { return id_; }

private:
	virtual int Run();
	bool CheckAlive();
public:
	bool Process(const std::vector<char>& data, std::vector<char>& result);
	bool Upload(const std::vector<char>& data, const std::string& path,
		const std::string& filename, std::vector<char>& result);
	bool Download(const std::string& path,
		const std::string& filename, std::vector<char>& result);

private:
	SSL_CTX* ctx_;
	IPAddress ip_;
	SocketSSLEx* sock_;
	std::string id_;
	ThreadLock lock_;
	time_t lastTick_;
};

///////////////////////////////////////////////////////////////////////////

class MessengerServer : private Thread, public IProcessor
{
public:
	MessengerServer();

public:
	bool Start(const char* certFile, unsigned short port, const char* transferFolder);
	void Stop();
public:
	void RemoveExitedThreads();

public:
	virtual std::vector<IdsInfo> GetIdsList();
	virtual bool Process(const std::string& id,
		const std::vector<char>& data, std::vector<char>& result);
	virtual bool Upload(const std::string& id,
		const std::vector<char>& data, const std::string& filename, std::vector<char>& result);
	virtual bool Download(const std::string& id,
		const std::string& filename, std::vector<char>& result);

private:
	virtual int Run();
private:
	MessengerThread* LockThread(const std::string& id);
	void UnlockThread(MessengerThread*);

private:
	SSL_CTX* ctx_;
	SocketSSL sock_;

	ThreadLock lock_;
	std::vector<MessengerThread*> threads_;
	std::vector<SocketSSLEx*> sockList_;
	std::set<MessengerThread*> lockedThreads_;

	std::string transferFolder_;
};

///////////////////////////////////////////////////////////////////////////
#endif//__MESSENGER_SERVER_H__

