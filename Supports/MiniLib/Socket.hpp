// Socket.hpp
#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__
///////////////////////////////////////////////////////////////////////////
// NOTE: Socket is a very simple wrap class for socket.
//       It is not thread-safe!
// NOTE: SocketEx is a socket wrap class, which can send/recv fixed size
//       data, also with timeout option. It is not thread-safe either!
///////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

typedef int socklen_t;
#pragma comment(lib, "ws2_32.lib")

const int MSG_NOSIGNAL = 0;
const int MSG_DONTWAIT = 0;

#else//_WIN32

#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int SOCKET;
const SOCKET INVALID_SOCKET = -1;
const int    SOCKET_ERROR   = -1;

#endif//_WIN32

#include <cassert>
#include <string>
#include <vector>
///////////////////////////////////////////////////////////////////////////

class SocketLibrary
{
#ifdef _WIN32
private:
	SocketLibrary() { WSAData d; WSAStartup(MAKEWORD(2,0),&d); }
	~SocketLibrary() { WSACleanup(); }
#endif
public:
	static bool Init() { static SocketLibrary dummy; return (&dummy != NULL); }
};

///////////////////////////////////////////////////////////////////////////

class Socket
{
protected:
	SOCKET sockfd_;
public:
	explicit Socket(SOCKET s = INVALID_SOCKET) : sockfd_(s) { SocketLibrary::Init(); }
	~Socket() { Close(); }
private:
	Socket(const Socket&); // disable copy
	void operator=(const Socket&);

public:
	bool IsValid() const { return (INVALID_SOCKET != sockfd_); }

	bool Create(bool dgram)
	{
		assert( ! IsValid());
		sockfd_ = socket(AF_INET, (dgram ? SOCK_DGRAM : SOCK_STREAM), 0);
		return (sockfd_ != INVALID_SOCKET);
	}
	void Close()
	{
		if (IsValid())
		{
#ifdef _WIN32
			closesocket(sockfd_);
#else
			shutdown(sockfd_, 0);
			close(sockfd_);
#endif
			sockfd_ = INVALID_SOCKET;
		}
	}
public:
	void Attach(SOCKET sockfd)
	{
		assert( ! IsValid());
		sockfd_ = sockfd;
	}
	SOCKET Detach()
	{
		SOCKET sockfd = sockfd_;
		sockfd_ = INVALID_SOCKET;
		return sockfd;
	}
public:
	bool Bind(unsigned short port, const char* ip = NULL)
	{
		if ( ! IsValid()) return false;
		sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = (((ip == NULL) || (*ip == '\0')) ? INADDR_ANY : inet_addr(ip));
		addr.sin_port = htons(port);
		int n = bind(sockfd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
		return (SOCKET_ERROR != n);
	}
	bool Bind(sockaddr_in& addr)
	{
		if ( ! IsValid()) return false;
		int n = bind(sockfd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
		return (SOCKET_ERROR != n);
	}
	bool Listen(int backlog = SOMAXCONN)
	{
		if ( ! IsValid()) return false;
		int n = listen(sockfd_, backlog);
		return (SOCKET_ERROR != n);
	}
	bool Connect(const char* host, unsigned short port, int timeout = 0) // in ms
	{
		if ( ! IsValid()) return false;
		if ((0 != host) && ('\0' != host[0]))
		{
			hostent* h = gethostbyname(host);
			if (h != NULL)
			{
				for (short i = 0; h->h_addr_list[i] != NULL; ++i)
				{
					sockaddr_in addr = { 0 };
					addr.sin_family = AF_INET;
					memcpy(&addr.sin_addr.s_addr, h->h_addr_list[i], sizeof(addr.sin_addr.s_addr));
					addr.sin_port = htons(port);
					if (Connect(addr, timeout) != false) return true;
				}
			}
			return false;
		}
		else
		{
			sockaddr_in addr = { 0 };
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(port);
			int n = Connect(addr, timeout);
			return (SOCKET_ERROR != n);
		}
	}
	bool Connect(const sockaddr_in& addr, int timeout = 0) // in ms
	{
		if ( ! IsValid()) return false;
		if (timeout <= 0)
		{
			int n = connect(sockfd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
			return (SOCKET_ERROR != n);
		}
		else
		{
			unsigned long args = 1;
			if ( ! IOCtlSocket(FIONBIO, &args)) return false;

			if (connect(sockfd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(sockaddr_in)) != 0)
			{
				struct timeval t;
				t.tv_sec  = timeout / 1000;
				t.tv_usec = (timeout % 1000) * 1000;

				fd_set rset;
				FD_ZERO(&rset);
				FD_SET(sockfd_, &rset);
				fd_set wset = rset;
				int n = select(static_cast<int>(sockfd_ + 1), &rset, &wset, NULL, &t);
				if (n <= 0) return false;

				sockaddr_in name;
				if ( ! GetPeerName(name)) return false;
			}
			args = 0;
			if ( ! IOCtlSocket(FIONBIO, &args)) return false;
			return true;
		}
	}
	SOCKET Accept(sockaddr_in& name)
	{
		if ( ! IsValid()) return INVALID_SOCKET;
		memset(&name, 0, sizeof(sockaddr_in));
		name.sin_family = AF_INET;
		socklen_t addrlen = sizeof(sockaddr_in);
		return (accept(sockfd_, reinterpret_cast<sockaddr*>(&name), &addrlen));
	}
public:
	bool GetSockName(sockaddr_in& name)
	{
		if ( ! IsValid()) return false;
		socklen_t namelen = sizeof(sockaddr_in);
		int n = getsockname(sockfd_, reinterpret_cast<sockaddr*>(&name), &namelen);
		return (SOCKET_ERROR != n);
	}
	bool GetPeerName(sockaddr_in& name)
	{
		if ( ! IsValid()) return false;
		socklen_t namelen = sizeof(sockaddr_in);
		int n = getpeername(sockfd_, reinterpret_cast<sockaddr*>(&name), &namelen);
		return (SOCKET_ERROR != n);
	}
public:
	int Send(const char* buffer, int size, int flags = 0)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		return send(sockfd_, buffer, size, flags);
	}
	int Recv(char* buffer, int size, int flags = 0)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		return recv(sockfd_, buffer, size, flags);
	}
	int SendDgram(const char* buffer, int size, const sockaddr_in& name, int flags = 0)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		return sendto(sockfd_, buffer, size, flags, reinterpret_cast<const sockaddr*>(&name), sizeof(sockaddr_in));
	}
	int RecvDgram(char* buffer, int size, sockaddr_in& name, int flags = 0)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		socklen_t fromLen = sizeof(sockaddr_in);
		return recvfrom(sockfd_, buffer, size, flags, reinterpret_cast<sockaddr*>(&name), &fromLen);
	}
public:
	bool SetSockOpt(int level, int optname, const void* optval, int optlen)
	{
		if ( ! IsValid()) return false;
		int n = setsockopt(sockfd_, level, optname, reinterpret_cast<const char*>(optval), optlen);
		return (SOCKET_ERROR != n);
	}
	bool GetSockOpt(int level, int optname, void* optval, socklen_t* optlen)
	{
		if ( ! IsValid()) return false;
		int n = getsockopt(sockfd_, level, optname, reinterpret_cast<char*>(optval), optlen);
		return (SOCKET_ERROR != n);
	}
	bool IOCtlSocket(long cmd, void* argp)
	{
		if ( ! IsValid()) return false;
#ifdef _WIN32
		return (ioctlsocket(sockfd_, cmd, (unsigned long*)argp) >= 0);
#else
		return (ioctl(sockfd_, cmd, argp) >= 0);
#endif
	}
public:
	// Return value of WaitData:
	// -1  error
	//  0  no data
	//  1  has data
	int WaitData(long sec, long usec, bool toRecv)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		timeval timeout;
		timeout.tv_sec = sec + usec / 1000000;
		timeout.tv_usec = usec % 1000000;
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(sockfd_, &rset);
		int n = select(static_cast<int>(sockfd_ + 1),
			(toRecv ? &rset : NULL), (toRecv ? NULL : &rset), NULL, &timeout);
		if (SOCKET_ERROR == n) return SOCKET_ERROR;
		if (sockfd_ == INVALID_SOCKET) return SOCKET_ERROR;
		n = FD_ISSET(sockfd_, &rset);
		return (0 != n);
	}
	int WaitData(long msec, bool toRecv)
	{
		long sec = msec / 1000;
		long usec = (msec % 1000) * 1000;
		return WaitData(sec, usec, toRecv);
	}
};

//////////////////////////////////////////////////////////////////////////
class Tick // in millisecond
{
public:
	Tick() : value_(0) { }

public:
	bool operator <  (const Tick& tick) const { return ((value_ - tick.value_) <  0); }
	bool operator <= (const Tick& tick) const { return ((value_ - tick.value_) <= 0); }
	bool operator >  (const Tick& tick) const { return ((value_ - tick.value_) >  0); }
	bool operator >= (const Tick& tick) const { return ((value_ - tick.value_) >= 0); }
	bool operator == (const Tick& tick) const { return ((value_ - tick.value_) == 0); }
	bool operator != (const Tick& tick) const { return ((value_ - tick.value_) != 0); }

public:
	Tick operator + (int diff) const { Tick tick = *this; tick.value_ += diff; return tick; }
	Tick operator - (int diff) const { Tick tick = *this; tick.value_ -= diff; return tick; }

public:
	int operator - (const Tick& tick) const { return (value_ - tick.value_); }

public:
	const Tick& operator += (int diff) { value_ += diff; return *this; }
	const Tick& operator -= (int diff) { value_ -= diff; return *this; }

private:
	int value_;

public:
	static Tick Now()
	{
		Tick tick;
#ifdef _WIN32
		tick.value_ = static_cast<int>(GetTickCount());
#else
		timeval tv;
		gettimeofday(&tv, NULL);
		tick.value_ = static_cast<int>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
		return tick;
	}
};

///////////////////////////////////////////////////////////////////////////

template <class SocketT>
class SocketExT : public SocketT
{
public:
	explicit SocketExT(SOCKET s = INVALID_SOCKET) : SocketT(s) { }

public:
	int SendEx(const void* buffer, int size, int timeout = 0, int* sendBytes = NULL) // in ms
	{
		const int SOCKET_FLAG = MSG_NOSIGNAL | (timeout ? MSG_DONTWAIT : 0);
		const Tick endTime = Tick::Now() + timeout;

		const char* const start = reinterpret_cast<const char*>(buffer);
		const char* const end = start + size;

		if (sendBytes) *sendBytes = 0;
		const char* p = start;
		while (p < end)
		{
			if (timeout > 0)
			{
				int n = SocketT::WaitData(endTime - Tick::Now(), false);
				if (n < 0) return -1;
				if (Tick::Now() >= endTime) break;
				if (n == 0) continue;
			}
			int n = SocketT::Send(p, static_cast<int>(end - p), SOCKET_FLAG);
			if (n < 0) return -1;
			p += n;
			if (sendBytes) *sendBytes += n;
		}
		return static_cast<int>(p - start);
	}

	int RecvEx(void* buffer, int size, int timeout = 0, int* recvBytes = NULL) // in ms
	{
		const int SOCKET_FLAG = MSG_NOSIGNAL | (timeout ? MSG_DONTWAIT : 0);
		const Tick endTime = Tick::Now() + timeout;

		const char* const start = reinterpret_cast<const char*>(buffer);
		const char* const end = start + size;

		if (recvBytes) *recvBytes = 0;
		char* p = reinterpret_cast<char*>(buffer);
		while (p < end)
		{
			if (timeout > 0)
			{
				int n = SocketT::WaitData(endTime - Tick::Now(), true);
				if (n < 0) return -1;
				if (Tick::Now() >= endTime) break;
				if (n == 0) continue;
			}
			int n = SocketT::Recv(p, static_cast<int>(end - p), SOCKET_FLAG);
			if (n < 0) return -1;
			if (n == 0)
			{
				if (timeout > 0) return -1;
				break;
			}
			p += n;
			if (recvBytes) *recvBytes += n;
		}
		return static_cast<int>(p - start);
	}

public:
	bool SendPackage(const void* buffer, size_t size, int timeout = 0, bool closeWhenError = true) // in ms
	{
		if (sizeof(size) != SendEx(&size, sizeof(size), timeout))
		{
			if (closeWhenError) SocketT::Close();
			return false;
		}
		if (static_cast<int>(size) != SendEx(buffer, size, timeout))
		{
			if (closeWhenError) SocketT::Close();
			return false;
		}
		return true;
	}
	bool SendPackage(const std::vector<char>& buffer, int timeout = 0, bool closeWhenError = true) // in ms
	{
		return SendPackage(&buffer[0], buffer.size(), timeout, closeWhenError);
	}
	bool RecvPackage(std::vector<char>& buffer, int timeout = 0, bool closeWhenError = true) // in ms
	{
		size_t size = 0;
		if (sizeof(size) != RecvEx(&size, sizeof(size), timeout))
		{
			if (closeWhenError) SocketT::Close();
			return false;
		}
		buffer.resize(size);
		if (static_cast<int>(size) != RecvEx(&buffer[0], size, timeout))
		{
			if (closeWhenError) SocketT::Close();
			return false;
		}
		return true;
	}
public:
	bool RecvLine(std::string& text, int timeout = 0) // in ms
	{
		text.clear();
		for (;;)
		{
			char c;
			int n = RecvEx(&c, 1, timeout);
			if (n < 0) return false;
			if (n == 0) break;

			if ((c == '\0') || (c == '\n')) break;
			if (c == '\r') // EOL for DOS ("\r\n")
			{
				if (RecvEx(&c, 1, timeout) < 0) return false;
				break;
			}
			text += c;
		}
		return true;
	}
};

typedef SocketExT<Socket> SocketEx;

///////////////////////////////////////////////////////////////////////////

template <class SocketT>
class SocketSession
{
private:
	SocketT& sock_;
public:
	SocketSession(SocketT& sock) : sock_(sock) { }
	~SocketSession() { sock_.Close(); }
private:
	SocketSession(const SocketSession<SocketT>&);
	void operator = (const SocketSession<SocketT>&);
};

///////////////////////////////////////////////////////////////////////////
#endif//__SOCKET_HPP__

