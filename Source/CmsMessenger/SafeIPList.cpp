// SafeIPList.cpp

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "SafeIPList.h"

///////////////////////////////////////////////////////////////////////////

SafeIP::SafeIP()
{
	value_[0][0] = value_[0][1] = 127;
	value_[1][0] = value_[1][1] = 0;
	value_[2][0] = value_[2][1] = 0;
	value_[3][0] = value_[3][1] = 1;
}

bool SafeIP::Set(const char* text)
{
	if ( ! text) return false;

	std::string s = text;
	size_t p1 = s.find_first_of('.');
	if (p1 == std::string::npos) return false;
	size_t p2 = s.find_first_of('.', p1 + 1);
	if (p2 == std::string::npos) return false;
	size_t p3 = s.find_first_of('.', p2 + 1);
	if (p3 == std::string::npos) return false;

	if ( ! Set(s.substr(0,      p1         ), value_[0][0], value_[0][1])) return false;
	if ( ! Set(s.substr(p1 + 1, p2 - p1 - 1), value_[1][0], value_[1][1])) return false;
	if ( ! Set(s.substr(p2 + 1, p3 - p2 - 1), value_[2][0], value_[2][1])) return false;
	if ( ! Set(s.substr(p3 + 1             ), value_[3][0], value_[3][1])) return false;
	return true;
}

bool SafeIP::Set(const std::string& field, int& minValue, int& maxValue)
{
	if (field == "") return false;
	if (field.find_first_of('*') != std::string::npos)
	{
		minValue = 0;
		maxValue = 255;
	}
	else
	{
		size_t pos = field.find_first_of('-');
		if (pos == std::string::npos)
		{
			int value = atoi(field.c_str());
			if ((value < 0) || (value > 255)) return false;
			minValue = maxValue = value;
		}
		else
		{
			int value1 = atoi(field.substr(0, pos).c_str());
			int value2 = atoi(field.substr(pos + 1).c_str());
			if ((value1 < 0) || (value1 > 255)) return false;
			if ((value2 < 0) || (value2 > 255)) return false;
			if (value1 < value2)
			{
				minValue = value1;
				maxValue = value2;
			}
			else
			{
				minValue = value2;
				maxValue = value1;
			}
		}
	}
	return true;
}

bool SafeIP::Check(const IPAddress& ip) const
{
	if (ip[0] < value_[0][0]) return false;
	if (ip[0] > value_[0][1]) return false;
	if (ip[1] < value_[1][0]) return false;
	if (ip[1] > value_[1][1]) return false;
	if (ip[2] < value_[2][0]) return false;
	if (ip[2] > value_[2][1]) return false;
	if (ip[3] < value_[3][0]) return false;
	if (ip[3] > value_[3][1]) return false;
	return true;
}

std::string SafeIP::ToString() const
{
	std::stringstream ss;
	for (size_t i = 0; i < 4; ++i)
	{
		if (i > 0)
		{
			ss << ".";
		}

		if (value_[i][0] == value_[i][1])
		{
			ss << value_[i][0];
		}
		else if ((value_[i][0] == 0) && (value_[i][1] == 255))
		{
			ss << "*";
		}
		else
		{
			ss << value_[i][0] << "-" << value_[i][1];
		}
	}
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////

bool SafeIPList::Load(const char* filename)
{
	list_.clear();

	SafeIP ip;
	list_.push_back(ip);

	FILE* fp = fopen(filename, "r");
	if (fp)
	{
		while ( ! feof(fp))
		{
			char buffer[1024];
			if (fgets(buffer, sizeof(buffer), fp))
			{
				std::string s = buffer;
				if ((s.size() > 0) && (s.substr(s.size() - 1) == "\n"))
				{
					s = s.substr(0, s.size() - 1);
				}

				if (ip.Set(buffer))
				{
					list_.push_back(ip);
				}
			}
		}
		fclose(fp);
	}
	return true;
}

void SafeIPList::Dump() const
{
	std::cout << "Web from following " << list_.size() << " IP(s) are allowed to manage IDS:" << std::endl;
	for (size_t i = 0; i < list_.size(); ++i)
	{
		std::cout << " (" << i + 1 << ") " << list_[i].ToString() << std::endl;
	}
}

bool SafeIPList::Check(const IPAddress& ip) const
{
	for (size_t i = 0; i < list_.size(); ++i)
	{
		if (list_[i].Check(ip))
		{
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////

