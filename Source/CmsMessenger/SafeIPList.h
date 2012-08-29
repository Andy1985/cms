// SafeIPList.h
#ifndef __SAFE_IP_LIST_H__
#define __SAFE_IP_LIST_H__
///////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#define USE_OPEN_SSL
#include "../../Supports/MiniLib/Network.hpp"

///////////////////////////////////////////////////////////////////////////

class SafeIP
{
public:
	SafeIP();

public:
	bool Set(const char*);
	bool Check(const IPAddress& ip) const;

	std::string ToString() const;

private:
	bool Set(const std::string& field, int& minValue, int& maxValue);
private:
	int value_[4][2];
};

///////////////////////////////////////////////////////////////////////////

class SafeIPList
{
public:
	bool Load(const char* filename);

	void Dump() const;
public:
	bool Check(const IPAddress& ip) const;

private:
	std::vector<SafeIP> list_;
};

///////////////////////////////////////////////////////////////////////////
#endif//__SAFE_IP_LIST_H__

