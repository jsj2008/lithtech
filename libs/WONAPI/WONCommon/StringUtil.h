#ifndef __WON_STRINGUTIL_H__
#define __WON_STRINGUTIL_H__
#include "WONShared.h"

#include "Platform.h"
#include <string>

namespace WONAPI
{

std::wstring StringToWString(const std::string& theStr);
std::string WStringToString(const std::wstring& theStr);
std::string StringToUpperCase(const std::string& theStr);
std::string StringToLowerCase(const std::string& theStr);
std::wstring WStringToUpperCase(const std::wstring& theStr);
std::wstring WStringToLowerCase(const std::wstring& theStr);

void StringToUpperCaseInPlace(std::string &theStr);
void StringToLowerCaseInPlace(std::string &theStr);
void WStringToUpperCaseInPlace(std::wstring &theStr);
void WStringToLowerCaseInPlace(std::wstring &theStr);

int StringCompareNoCase(const std::string &s1, const std::string &s2);
int WStringCompareNoCase(const std::wstring &s1, const std::wstring &s2);

inline const wchar_t* WStringGetCStr(const std::wstring &theStr)
{
#ifdef _LINUX // this is a work around for a bug in some versions of basic_string
	if(theStr.length()==0)
		return L"";
	else 
	{
		if(theStr.capacity() < theStr.length() + 1)
			((std::wstring&)theStr).reserve(theStr.length() + 1);

		((std::wstring&)theStr)[theStr.length()] = L'\0';
		return theStr.data();
	}
#else
	return theStr.c_str();
#endif
}

class StringLessNoCase
{
public:
	bool operator()(const std::string &s1, const std::string &s2) const { return StringCompareNoCase(s1,s2)<0; }
};

class WStringLessNoCase
{
public:
	bool operator()(const std::wstring &s1, const std::wstring &s2) const { return WStringCompareNoCase(s1,s2)<0; }
};


class MultiString
{
private:
	std::string mAsciiStr;
	std::wstring mUnicodeStr;

public:
	MultiString(const std::string &theStr) : mAsciiStr(theStr) {}
	MultiString(const std::wstring &theStr) : mUnicodeStr(theStr) {}

	const std::string& GetAscii();
	const std::wstring& GetUnicode();

	operator const std::string&() { return mAsciiStr; }
	operator const std::wstring&() { return mUnicodeStr; }

};

}; // namespace WONAPI

#endif
