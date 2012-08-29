// Network.hpp
#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__
//////////////////////////////////////////////////////////////////////////
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif

#include <iostream>

#include <ostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <Nb30.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "netapi32.lib")
#else
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif
//////////////////////////////////////////////////////////////////////////

class IPAddress
{
public:
	IPAddress() { addr_.l = 0; text_[0] = '\0'; }

	IPAddress(const char* ip)          { Set(ip); }
	IPAddress(const wchar_t* ip)       { Set(ip); }

	IPAddress(const sockaddr_in& addr) { addr_.l = 0; text_[0] = '\0'; Set(addr); }
	IPAddress(const in_addr& addr)     { addr_.l = 0; text_[0] = '\0'; Set(addr); }

	IPAddress(unsigned char u1, unsigned char u2, unsigned char u3, unsigned char u4)
	{
		addr_.l = 0;
		text_[0] = '\0';
		Set(u1, u2, u3, u4);
	}

public:
	void Set(const char* ip)
	{
		memset(text_, 0, sizeof(text_));
#ifdef _WIN32
		strncpy_s(text_, (ip ? ip : ""), sizeof(text_) - 1);
#else
		strncpy(text_, (ip ? ip : ""), sizeof(text_) - 1);
#endif
		addr_.l = inet_addr(text_);
		text_[0] = '\0';
	}

	void Set(const wchar_t* ip)
	{
		wchar_t buffer[sizeof(text_)];
		memcpy(buffer, ip, sizeof(buffer));

		memset(text_, 0, sizeof(text_));

		for (size_t i = 0; i < sizeof(text_) - 1; ++i)
		{
			if ( ! buffer[i]) break;
			assert((buffer[i] == L'.') || ((buffer[i] >= L'0') && (buffer[i] <= L'9')));
			text_[i] = static_cast<char>(buffer[i]);
		}
		addr_.l = inet_addr(text_);
		text_[0] = '\0';
	}

	void Set(const sockaddr_in& addr)
	{
		if (addr_.l != addr.sin_addr.s_addr)
		{
			addr_.l = addr.sin_addr.s_addr;
			text_[0] = '\0';
		}
	}

	void Set(const in_addr& addr)
	{
		if (addr_.l != addr.s_addr)
		{
			addr_.l = addr.s_addr;
			text_[0] = '\0';
		}
	}

	void Set(unsigned char u1, unsigned char u2, unsigned char u3, unsigned char u4)
	{
		if ((addr_.u[0] != u1) || (addr_.u[1] != u2) || (addr_.u[2] != u3) || (addr_.u[3] != u4))
		{
			addr_.u[0] = u1;
			addr_.u[1] = u2;
			addr_.u[2] = u3;
			addr_.u[3] = u4;
			text_[0] = '\0';
		}
	}

	void Clear()
	{
		addr_.l = 0;
		text_[0] = '\0';
	}

	std::string ToString() const
	{
		if (text_[0] == '\0')
		{
#ifdef _WIN32
			sprintf_s(text_, sizeof(text_), "%u.%u.%u.%u", addr_.u[0], addr_.u[1], addr_.u[2], addr_.u[3]);
#else
			sprintf(text_, "%u.%u.%u.%u", addr_.u[0], addr_.u[1], addr_.u[2], addr_.u[3]);
#endif
		}
		return text_;
	}

public:
	bool operator == (const IPAddress& ip) const { return (addr_.l == ip.addr_.l); }
	bool operator != (const IPAddress& ip) const { return (addr_.l != ip.addr_.l); }

	unsigned char operator[](size_t index) const
	{
		assert(index < sizeof(addr_.u));
		return addr_.u[index];
	}
private:
	union Addr
	{
		unsigned char u[4];
		unsigned long l;
	} addr_;

	mutable char text_[16]; // max size of "xxx.xxx.xxx.xxx"

public:
	bool IsLocalAddress() const { return (addr_.l == 0x0100007F); }
	unsigned long GetNetworkAddress() const { return addr_.l; }

public:
	static const IPAddress& LocalHost() { const static IPAddress ip(127, 0, 0, 1); return ip; }
};

inline std::ostream& operator << (std::ostream& os, const IPAddress& ip)
{
	return (os << ip.ToString());
}

//////////////////////////////////////////////////////////////////////////


class MacAddress
{
public:
	typedef unsigned char Buffer[6];

public:
	MacAddress() { memset(data_, 0, sizeof(data_)); text_[0] = '\0'; }

	MacAddress(long long mac)         { Set(mac); }
	MacAddress(const char* mac)    { Set(mac); }
	MacAddress(const wchar_t* mac) { Set(mac); }
	MacAddress(const Buffer& mac)  { Set(mac); }

	MacAddress(unsigned char c1, unsigned char c2, unsigned char c3,
		unsigned char c4, unsigned char c5, unsigned char c6)
	{
		Set(c1, c2, c3, c4, c5, c6);
	}

public:
	void Set(long long mac)
	{
#ifdef _WIN32
		assert( (mac & 0xFFFF000000000000) == 0 );
#else
		assert( (mac & 0xFFFF000000000000LL) == 0 );
#endif
		memcpy(data_, &mac, sizeof(data_));
		text_[0] = '\0';
	}

	void Set(const Buffer& data)
	{
		memcpy(data_, data, sizeof(data_));
		text_[0] = '\0';
	}

	void Set(unsigned char c1, unsigned char c2, unsigned char c3,
		unsigned char c4, unsigned char c5, unsigned char c6)
	{
		data_[0] = c1;
		data_[1] = c2;
		data_[2] = c3;
		data_[3] = c4;
		data_[4] = c5;
		data_[5] = c6;
		text_[0] = '\0';
	}

	void Set(const char* s)
	{
		memset(data_, 0, sizeof(data_));
		text_[0] = '\0';
		if (s != NULL)
		{
			const char* p = s;

			for (int i = 0; i < 6; ++i)
			{
				for (;;)
				{
					if (*p == '\0') return;
					if (IsNumberForMac(*p)) break;
					++p;
				}

				if (IsNumberForMac(p[1]))
				{
					data_[i] = static_cast<unsigned char>(ParseNumber(p[0]) * 16 + ParseNumber(p[1]));
					p += 2;
				}
				else
				{
					data_[i] = static_cast<unsigned char>(ParseNumber(p[0]));
					++p;
				}
			}
		}
	}

	void Set(const wchar_t* s)
	{
		char* p = NULL;
		if (s != NULL)
		{
			size_t length = wcslen(s);
			p = new char[length + 1];
			for (size_t i = 0; i <= length; ++i)
			{
				if (s[i] != static_cast<char>(s[i]))
				{
					delete[] p;
					p = NULL;
					break;
				}
				p[i] = static_cast<char>(s[i]);
			}
		}
		Set(p);
		delete[] p;
		p = NULL;
	}

private:
	static bool IsNumberForMac(char c)
	{
		if ((c >= '0') && (c <= '9')) return true;
		if ((c >= 'A') && (c <= 'F')) return true;
		if ((c >= 'a') && (c <= 'f')) return true;
		return false;
	}
	static bool IsSeparatorForMac(char c)
	{
		if ((c == ' ')) return true;
		if ((c == '-')) return true;
		if ((c == ':')) return true;
		if ((c == '.')) return true;
		return false;
	}
	static int ParseNumber(char c)
	{
		if ((c >= '0') && (c <= '9')) return (c - '0');
		if ((c >= 'A') && (c <= 'F')) return (c - 'A') + 10;
		if ((c >= 'a') && (c <= 'f')) return (c - 'a') + 10;
		return -1;
	}

public:
	unsigned char operator[](size_t index) const
	{
		assert(index < sizeof(data_));
		return data_[index];
	}

	std::string ToString() const
	{
		if (text_[0] == '\0')
		{
#ifdef _WIN32
			sprintf_s(text_, sizeof(text_), "%02X-%02X-%02X-%02X-%02X-%02X",
				data_[0], data_[1], data_[2], data_[3], data_[4], data_[5]);
#else
			sprintf(text_, "%02X-%02X-%02X-%02X-%02X-%02X",
				data_[0], data_[1], data_[2], data_[3], data_[4], data_[5]);
#endif
		}
		return text_;
	}

	long long GetMacValue() const
	{
		long long value = 0;
		memcpy(&value, data_, sizeof(data_));
		return value;
	}

public:
	bool operator == (const MacAddress& mac) const
	{
		return (memcmp(data_, mac.data_, sizeof(data_)) == 0);
	}
	bool operator != (const MacAddress& mac) const
	{
		return (memcmp(data_, mac.data_, sizeof(data_)) != 0);
	}

private:
	unsigned char data_[6];
	mutable char text_[18]; // max size of "XX-XX-XX-XX-XX-XX"
};

inline std::ostream& operator << (std::ostream& os, const MacAddress& mac)
{
	return (os << mac.ToString());
}

//////////////////////////////////////////////////////////////////////////

class Network
{
public:
	static void Init()
	{
#ifdef _WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
#endif
	}

public:
	static bool GetLocalIP(std::vector<IPAddress>& ipList)
	{
#ifdef _WIN32
		char name[64];
		if (gethostname(name, sizeof(name)) != 0)
		{
			return false;
		}

		hostent* h = gethostbyname(name);
		if (h == NULL) return false;

		ipList.clear();
		for (int i = 0; h->h_addr_list[i] != 0; ++i)
		{
			unsigned char* u = reinterpret_cast<unsigned char*>(h->h_addr_list[i]);
			IPAddress ip(u[0], u[1], u[2], u[3]);
			if ( ! ip.IsLocalAddress())
			{
				ipList.push_back(ip);
			}
		}
#else
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) return false;

		for (char c = '0'; c <= '9'; ++c)
		{
			struct ifreq ifr;
			strcpy(ifr.ifr_name, "eth0");
			ifr.ifr_name[3] = c;
			if (ioctl(sockfd, SIOCGIFADDR, reinterpret_cast<unsigned long*>(&ifr)) >= 0)
			{
				sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&ifr.ifr_addr);
				ipList.push_back( IPAddress(*addr) );
			}
		}
#endif
		return true;
	}

public:
#ifdef _WIN32
	static bool GetLocalMac(std::vector<MacAddress>& macList)
	{
		LANA_ENUM lanaEnum;
		memset(&lanaEnum, 0, sizeof(lanaEnum));

		// Enum NetCards of current machine
		NCB ncb;
		memset(&ncb, 0, sizeof(ncb));
		ncb.ncb_command = NCBENUM;
		ncb.ncb_buffer  = reinterpret_cast<PUCHAR>(&lanaEnum);
		ncb.ncb_length  = sizeof(lanaEnum);
		UCHAR result = Netbios(&ncb);
		if (result != 0) return false;

		for (UCHAR i = 0; i < lanaEnum.length; ++i)
		{
			// Reset NetCard
			memset(&ncb, 0, sizeof(ncb));
			ncb.ncb_command = NCBRESET;
			ncb.ncb_lana_num = lanaEnum.lana[i];
			result = Netbios(&ncb);
			if (result != 0) continue;

			// Get NetCard Info
			struct ASTAT
			{
				ADAPTER_STATUS adapt;
				NAME_BUFFER    nameBuff[30];
			}
			adapter;
			memset(&ncb, 0, sizeof(ncb));
			ncb.ncb_command = NCBASTAT;
			ncb.ncb_lana_num = lanaEnum.lana[i];
			ncb.ncb_callname[0] = '*';
			memset(ncb.ncb_callname + 1, ' ', sizeof(ncb.ncb_callname) - 1);
			ncb.ncb_buffer = reinterpret_cast<PUCHAR>(&adapter);
			ncb.ncb_length = sizeof(adapter);
			result = Netbios(&ncb);
			if (result != 0) continue;

			macList.push_back(MacAddress(adapter.adapt.adapter_address));
		}
		return true;
	}

#else
	static bool GetLocalMac(std::vector<MacAddress>& macList)
	{
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) return false;

		for (char c = '0'; c <= '9'; ++c)
		{
			struct ifreq ifr;
			strcpy(ifr.ifr_name, "eth0");
			ifr.ifr_name[3] = c;

			if (ioctl(sockfd, SIOCGIFHWADDR, reinterpret_cast<unsigned long*>(&ifr)) >= 0)
			{
				macList.push_back( MacAddress(ifr.ifr_ifru.ifru_hwaddr.sa_data) );
			}
		}
		return true;
	}
#endif
};

//////////////////////////////////////////////////////////////////////////
#endif//__NETWORK_HPP__

