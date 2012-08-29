// String.hpp
#ifndef __STRING_HPP__
#define __STRING_HPP__
///////////////////////////////////////////////////////////////////////////
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <functional>
///////////////////////////////////////////////////////////////////////////

template <typename T>
class string_has_t : public std::unary_function<T, bool>
{
private:
	std::basic_string<T> s_;
public:
	string_has_t(const std::basic_string<T>& s) : s_(s) { }
	bool operator()(const T& val) const { return (s_.find(val) != std::basic_string<T>::npos); }
};

template <typename T>
class string_has_not_t : public std::unary_function<T, bool>
{
private:
	std::basic_string<T> s_;
public:
	string_has_not_t(const std::basic_string<T>& s) : s_(s) { }
	bool operator()(const T& val) const { return (s_.find(val) == std::basic_string<T>::npos); }
};

//-------------------------------------------------------------------------

template <typename T> bool string_has    (const std::basic_string<T>& s, T c) { return string_has_t<T>(s)(c); }
template <typename T> bool string_has_not(const std::basic_string<T>& s, T c) { return string_has_not_t<T>(s)(c); }

inline bool string_has(const std::string& s,  char c)    { return string_has<char>   (s, c); }
inline bool string_has(const std::wstring& s, wchar_t c) { return string_has<wchar_t>(s, c); }

inline bool string_has_not(const std::string& s,  char c)    { return string_has_not<char>   (s, c); }
inline bool string_has_not(const std::wstring& s, wchar_t c) { return string_has_not<wchar_t>(s, c); }

///////////////////////////////////////////////////////////////////////////

template <typename T>
inline std::basic_string<T> string_trim_left(const std::basic_string<T>& s, T c)
{
	std::string::size_type pos = s.find_first_not_of(c);
	return ((pos == std::string::npos) ? std::basic_string<T>() : s.substr(pos));
}

template <typename T>
inline std::basic_string<T> string_trim_right(const std::basic_string<T>& s, T c)
{
	std::string::size_type pos = s.find_last_not_of(c);
	return ((pos == std::string::npos) ? std::basic_string<T>() : s.substr(0, pos + 1));
}

template <typename T>
inline std::basic_string<T> string_trim_left(const std::basic_string<T>& s, const std::basic_string<T>& c)
{
	typename std::basic_string<T>::const_iterator pos = std::find_if(s.begin(), s.end(), string_has_not_t<T>(c));
	return (pos == s.end() ? std::basic_string<T>() : s.substr(pos - s.begin()));
}

template <typename T>
inline std::basic_string<T> string_trim_right(const std::basic_string<T>& s, const std::basic_string<T>& c)
{
	typename std::basic_string<T>::const_reverse_iterator pos = std::find_if(s.rbegin(), s.rend(), string_has_not_t<T>(c));
	return (pos == s.rend() ? std::basic_string<T>() : s.substr(0, s.rend() - pos));
}

template <typename T, typename U>
inline std::basic_string<T> string_trim(const std::basic_string<T>& s, U c)
{
	return string_trim_left<T>(string_trim_right<T>(s, c), c);
}

//-------------------------------------------------------------------------

inline std::string  string_trim_left (const std::string& s,  char    c = ' ')  { return string_trim_left<char>(s, c); }
inline std::wstring string_trim_left (const std::wstring& s, wchar_t c = L' ') { return string_trim_left<wchar_t>(s, c); }
inline std::string  string_trim_right(const std::string& s,  char    c = ' ')  { return string_trim_right<char>(s, c); }
inline std::wstring string_trim_right(const std::wstring& s, wchar_t c = L' ') { return string_trim_right<wchar_t>(s, c); }
inline std::string  string_trim(const std::string& s,  char    c = ' ')  { return string_trim<char>(s, c); }
inline std::wstring string_trim(const std::wstring& s, wchar_t c = L' ') { return string_trim<wchar_t>(s, c); }

inline std::string  string_trim_left (const std::string& s,  const std::string& c)  { return string_trim_left<char>(s, c); }
inline std::wstring string_trim_left (const std::wstring& s, const std::wstring& c) { return string_trim_left<wchar_t>(s, c); }
inline std::string  string_trim_right(const std::string& s,  const std::string& c)  { return string_trim_right<char>(s, c); }
inline std::wstring string_trim_right(const std::wstring& s, const std::wstring& c) { return string_trim_right<wchar_t>(s, c); }
inline std::string  string_trim(const std::string& s,  const std::string& c)  { return string_trim<char>(s, c); }
inline std::wstring string_trim(const std::wstring& s, const std::wstring& c) { return string_trim<wchar_t>(s, c); }

///////////////////////////////////////////////////////////////////////////

template <typename T>
inline void string_makeupper(std::basic_string<T>& s)
{
	std::transform(s.begin(), s.end(), s.begin(), static_cast<int(*)(int)>(&std::toupper));
}

template <typename T>
inline void string_makelower(std::basic_string<T>& s)
{
	std::transform(s.begin(), s.end(), s.begin(), static_cast<int(*)(int)>(&std::tolower));
}

template <typename T>
inline std::basic_string<T> string_toupper(const std::basic_string<T>& s)
{
	std::basic_string<T> result;
	std::transform(s.begin(), s.end(), std::inserter(result, result.begin()), static_cast<int(*)(int)>(&std::toupper));
	return result;
}

template <typename T>
inline std::basic_string<T> string_tolower(const std::basic_string<T>& s)
{
	std::basic_string<T> result;
	std::transform(s.begin(), s.end(), std::inserter(result, result.begin()), static_cast<int(*)(int)>(std::tolower));
	return result;
}

//-------------------------------------------------------------------------

inline void string_makeupper(std::string&  s) { string_makeupper<char>(s); }
inline void string_makeupper(std::wstring& s) { string_makeupper<wchar_t>(s); }
inline void string_makelower(std::string&  s) { string_makelower<char>(s); }
inline void string_makelower(std::wstring& s) { string_makelower<wchar_t>(s); }

inline std::string  string_toupper(const std::string& s)  { return string_toupper<char>(s); }
inline std::wstring string_toupper(const std::wstring& s) { return string_toupper<wchar_t>(s); }
inline std::string  string_tolower(const std::string& s)  { return string_tolower<char>(s); }
inline std::wstring string_tolower(const std::wstring& s) { return string_tolower<wchar_t>(s); }

///////////////////////////////////////////////////////////////////////////

template <typename T>
inline int string_compare_no_case(const std::basic_string<T>& s1, const std::basic_string<T>& s2)
{
	for (size_t i = 0; ; ++i)
	{
		if ((i >= s1.size()) && (i >= s2.size())) break;

		if (i >= s1.size()) return -1;
		if (i >= s2.size()) return 1;

		T c1 = s1[i];
		T c2 = s2[i];
		if (std::isalpha(c1) && std::isalpha(c2))
		{
			c1 = static_cast<T>(std::toupper(static_cast<int>(c1)));
			c2 = static_cast<T>(std::toupper(static_cast<int>(c2)));
		}
		if (c1 < c2) return -1;
		if (c1 > c2) return 1;
	}
	return 0;
}

//-------------------------------------------------------------------------

inline int string_compare_no_case(const std::string&  s1, const std::string&  s2) { return string_compare_no_case<char>(s1, s2); }
inline int string_compare_no_case(const std::wstring& s1, const std::wstring& s2) { return string_compare_no_case<wchar_t>(s1, s2); }

///////////////////////////////////////////////////////////////////////////

template <typename T>
inline std::basic_string<T> string_replace(const std::basic_string<T>& s, const std::basic_string<T>& from, const std::basic_string<T>& to)
{
	std::basic_string<T> result = s;
	if ( ! from.empty())
	{
		typename std::basic_string<T>::size_type pos = 0;
		typename std::basic_string<T>::size_type fromLen = from.size();
		typename std::basic_string<T>::size_type toLen = to.size();
		for (;;)
		{
			pos = result.find(from, pos);
			if (pos == std::basic_string<T>::npos) break;
			result.replace(pos, fromLen, to);
			pos += toLen;
		}
	}
	return result;
}

//-------------------------------------------------------------------------

inline std::string  string_replace(const std::string&  s, const std::string& from,  const std::string& to)  { return string_replace<char>(s, from, to); }
inline std::wstring string_replace(const std::wstring& s, const std::wstring& from, const std::wstring& to) { return string_replace<wchar_t>(s, from, to); }

///////////////////////////////////////////////////////////////////////////

template <typename T>
inline std::string number_to_string(T value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

inline std::string number_to_string(double value)
{
	std::stringstream ss;
	ss << value;
	std::string text = ss.str();

	if (text.find('.') != std::string::npos)
	{
		string_trim_right(text, '0');
	}
	if (text.empty())
	{
		text.clear();
		text.push_back('0');
	}
	return text;
}

///////////////////////////////////////////////////////////////////////////

template <typename T>
inline std::vector<std::basic_string<T> > string_split(std::basic_string<T> s, T delim)
{
	typename std::vector<std::basic_string<T> > result;
	typename std::basic_stringstream<T> ss(s);
	typename std::basic_string<T> item;
	while (std::getline(ss, item, delim))
	{
		result.push_back(item);
	}
	return result;
}

template <typename T>
inline std::vector<std::basic_string<T> > string_split(std::basic_string<T> s, T delim, std::basic_string<T> quoteMarks)
{
	typename std::vector<std::basic_string<T> > result;
	typename std::basic_string<T> current;

	T mark = '\0';
	bool inString = false;

	for (size_t i = 0; i < s.size(); ++i)
	{
		if (inString)
		{
			current += s[i];
			if (s[i] == '\\')
			{
				if (++i >= s.size()) break;
				current += s[i];
			}
			else if (s[i] == mark)
			{
				mark = '\0';
				inString = false;
			}
		}
		else
		{
			if (s[i] == delim)
			{
				result.push_back(current);
				current.clear();
			}
			else
			{
				current += s[i];
				if (quoteMarks.find_first_of(s[i]) != std::basic_string<T>::npos)
				{
					mark = s[i];
					inString = true;
				}
			}
		}
	}
	if ( ! current.empty())
	{
		result.push_back(current);
	}
	return result;
}

inline std::vector<std::string> string_split(const std::string&  s, char delim = ' ', std::string quoteMarks = "")
{
	if (quoteMarks.empty())
	{
		return string_split<char>(s, delim);
	}
	else
	{
		return string_split<char>(s, delim, quoteMarks);
	}
}

inline std::vector<std::wstring> string_split(const std::wstring& s, wchar_t delim = L' ', std::wstring quoteMarks = L"")
{
	if (quoteMarks.empty())
	{
		return string_split<wchar_t>(s, delim);
	}
	else
	{
		return string_split<wchar_t>(s, delim, quoteMarks);
	}
}

///////////////////////////////////////////////////////////////////////////
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

inline std::wstring AtoW(const std::string& text)
{
	std::wstring wtext;
	wtext.resize(text.size() + 1);
	int length = ::MultiByteToWideChar(/*CP_ACP*/936, 0, text.c_str(), static_cast<int>(text.size() + 1), &wtext[0], wtext.size());
	wtext.resize(length - 1);
	return wtext;
}

inline std::string WtoA(const std::wstring& wtext)
{
	std::string text;
	text.resize((wtext.size() + 1) * 2);
	int length = ::WideCharToMultiByte(/*CP_ACP*/936, 0, wtext.c_str(), static_cast<int>(wtext.size() + 1), &text[0], text.size(), NULL, NULL);
	text.resize(length - 1);
	return text;
}

#else//_WIN32

#include <iconv.h>
#include <cstdio>

inline std::wstring AtoW(const std::string& text)
{
	std::wstring wtext;
	if ( ! text.empty())
	{
		iconv_t cd = iconv_open("Unicode", "GB18030");
		if (cd != reinterpret_cast<iconv_t>(-1))
		{
			std::string from = text;
			std::string to;
			to.resize((text.size() + 1) * 2);

			char* p1 = &from[0];
			size_t s1 = from.size();
			char* p2 = &to[0];
			size_t s2 = to.size();
			iconv(cd, &p1, &s1, &p2, &s2);

			to.resize(to.size() - s2);

			if ((to.size() > 2) && (to[0] == '\xFF') && (to[1] == '\xFE'))
			{
				to = to.substr(2);
			}
			for (size_t i = 0; i < to.size(); i += 2)
			{
				wtext += static_cast<wchar_t>(*reinterpret_cast<unsigned short*>(&to[i]));
			}
			iconv_close(cd);
		}
	}
	return wtext;
}

inline std::string WtoA(const std::wstring& wtext)
{
	std::string text;
	if ( ! wtext.empty())
	{
		iconv_t cd = iconv_open("GB18030", "Unicode");
		if (cd != reinterpret_cast<iconv_t>(-1))
		{
			for (size_t i = 0; i < wtext.size(); i++)
			{
				char wc[2];
				wc[0] = static_cast<char>(wtext[i]);
				wc[1] = static_cast<char>(wtext[i] >> 8);

				if ( ! wc[1])
				{
					text += wc[0];
				}
				else
				{
					char buffer[3] = { 0, 0, 0 };

					char* p1 = wc;
					size_t s1 = sizeof(wc);
					char* p2 = buffer;
					size_t s2 = sizeof(buffer);

					iconv(cd, &p1, &s1, &p2, &s2);

					text += buffer[0];
					text += buffer[1];
				}
			}
			iconv_close(cd);
		}
	}
	return text;
}
#endif//_WIN32

//-------------------------------------------------------------------------

namespace std
{
#if defined(_UNICODE) || defined(UNICODE)
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}

#if defined(_UNICODE) || defined(UNICODE)
inline std::tstring AtoT(const std::string&  text) { return AtoW(text); }
inline std::tstring WtoT(const std::wstring& text) { return     (text); }
inline std::string  TtoA(const std::tstring& text) { return WtoA(text); }
inline std::wstring TtoW(const std::tstring& text) { return     (text); }
#else
inline std::tstring AtoT(const std::string&  text) { return     (text); }
inline std::tstring WtoT(const std::wstring& text) { return WtoA(text); }
inline std::string  TtoA(const std::tstring& text) { return     (text); }
inline std::wstring TtoW(const std::tstring& text) { return AtoW(text); }
#endif

///////////////////////////////////////////////////////////////////////////
#endif//__STRING_HPP__

