// Interfaces.h
#ifndef __INTERFACES_H__
#define __INTERFACES_H__
///////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include "../../Supports/MiniLib/Network.hpp"

///////////////////////////////////////////////////////////////////////////

class IdsInfo
{
public:
	IdsInfo(const std::string& id, const IPAddress& ip) : id_(id), ip_(ip) { }
public:
	std::string id_;
	IPAddress ip_;
};

///////////////////////////////////////////////////////////////////////////

class IProcessor
{
public:
	virtual std::vector<IdsInfo> GetIdsList() = 0;
	virtual bool Process(const std::string& id,
		const std::vector<char>& data, std::vector<char>& result) = 0;
	virtual bool Upload(const std::string& id,
		const std::vector<char>& data, const std::string& filename, std::vector<char>& result) = 0;
	virtual bool Download(const std::string& id, const std::string& filename, std::vector<char>& result) = 0;
	
	virtual ~IProcessor() {}
};

///////////////////////////////////////////////////////////////////////////
#endif//__INTERFACES_H__

