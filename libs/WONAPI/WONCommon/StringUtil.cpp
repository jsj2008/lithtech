#include "StringUtil.h"

using namespace std;
namespace WONAPI
{

// WideToAscii
// Converts a WIDE (wchar_t*) buffer to an ASCII (char*) buffer
// Returns pointer to the ASCII buffer.
// ASCII buffer must be at least the size of the ASCII buffer.
static char* WideToAscii(char* aBufP, const wchar_t* wBufP, size_t nChars)
{
	if (! aBufP) return NULL;

	*aBufP = '\0';
	if (wBufP)
	{
#ifdef WIN32_NOT_XBOX
		// aBufP better point to a buffer at least the size of wBufP
		// or this will blow
		WideCharToMultiByte(CP_ACP, 0, wBufP, nChars, aBufP, nChars, NULL, NULL);
#else//if defined(_LINUX)
		const wchar_t* curSrc = wBufP;
		const wchar_t* srcEnd = wBufP + nChars;
		char* curDst = aBufP;
		while (curSrc != srcEnd)
		{
			*(curDst++) = *(curSrc++);
		}
//#else
//#error unknown platform
#endif
	}

	return aBufP;
}

// AsciiToWide
// Converts an ASCII (char*) buffer to a WIDE (wchar_t*) buffer
// Returns pointer to the WIDE buffer.
// WIDE buffer must be at least twice the size of the ASCII buffer.
static wchar_t* AsciiToWide(wchar_t* wBufP, const char* aBufP, size_t nChars)
{
	if (! wBufP) return NULL;

	*wBufP = '\0';
	if (aBufP)
	{
#ifdef WIN32_NOT_XBOX
		// wBufP better point to a buffer at least twice the size of aBufP
		// or this will blow
		MultiByteToWideChar(CP_ACP, 0, aBufP, nChars, wBufP, nChars);
#else//if defined(_LINUX)
		const char* curSrc = aBufP;
		const char* srcEnd = aBufP + nChars;
		wchar_t* curDst = wBufP;
		while (curSrc != srcEnd)
		{
			*(curDst++) = *(curSrc++);
		}
//#else
//#error unknown platform
#endif
	}

	return wBufP;
}

wstring StringToWString(const string& theStr)
{
	size_t aCopySize = theStr.length();
	if(!aCopySize)
		return wstring();
	
	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr.data(), aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}

string WStringToString(const wstring& theStr)
{
	size_t aCopySize = theStr.size();
	if (!aCopySize)
		return string();
		
	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr.data(), aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}

std::string StringToUpperCase(const std::string& theStr)
{
	string aNewStr;
	aNewStr.reserve(theStr.length());
	for(int i=0; i<theStr.length(); i++)
		aNewStr+=toupper(theStr[i]);

	return aNewStr;
}

std::string StringToLowerCase(const std::string& theStr)
{
	string aNewStr;
	aNewStr.reserve(theStr.length());
	for(int i=0; i<theStr.length(); i++)
		aNewStr+=tolower(theStr[i]);

	return aNewStr;
}

std::wstring WStringToUpperCase(const std::wstring& theStr)
{
	wstring aNewStr;
	aNewStr.reserve(theStr.length());
	for(int i=0; i<theStr.length(); i++)
		aNewStr+=towupper(theStr[i]);

	return aNewStr;
}

std::wstring StringToLowerCase(const std::wstring& theStr)
{
	wstring aNewStr;
	aNewStr.reserve(theStr.length());
	for(int i=0; i<theStr.length(); i++)
		aNewStr+=towlower(theStr[i]);

	return aNewStr;
}

void StringToUpperCaseInPlace(std::string &theStr)
{
	for(int i=0; i<theStr.length(); i++)
		theStr[i] = toupper(theStr[i]);
}

void StringToLowerCaseInPlace(std::string &theStr)
{
	for(int i=0; i<theStr.length(); i++)
		theStr[i] = tolower(theStr[i]);
}

void WStringToUpperCaseInPlace(std::wstring &theStr)
{
	for(int i=0; i<theStr.length(); i++)
		theStr[i] = towupper(theStr[i]);
}
void WStringToLowerCaseInPlace(std::wstring &theStr)
{
	for(int i=0; i<theStr.length(); i++)
		theStr[i] = towlower(theStr[i]);
}

const std::string& MultiString::GetAscii()
{
	if(mAsciiStr.empty())
		mAsciiStr = WStringToString(mUnicodeStr);

	return mAsciiStr;
}
	

const std::wstring& MultiString::GetUnicode()
{
	if(mUnicodeStr.empty())
		mUnicodeStr = StringToWString(mAsciiStr);

	return mUnicodeStr;
}

int StringCompareNoCase(const std::string &s1, const std::string &s2)
{
	string::const_iterator i1 = s1.begin();
	string::const_iterator i2 = s2.begin();

	while ((i1 != s1.end()) && (i2 != s2.end()))
	{
		if (toupper(*i1) != toupper(*i2))
			return ((toupper(*i1) < toupper(*i2)) ? -1 : 1);

		i1++;  i2++;
	}

	return ((s1.size() == s2.size()) ? 0 : ((s1.size() < s2.size()) ? -1 : 1));	
}

int WStringCompareNoCase(const std::wstring &s1, const std::wstring &s2)
{
	wstring::const_iterator i1 = s1.begin();
	wstring::const_iterator i2 = s2.begin();

	while ((i1 != s1.end()) && (i2 != s2.end()))
	{
		if (towupper(*i1) != towupper(*i2))
			return ((towupper(*i1) < towupper(*i2)) ? -1 : 1);

		i1++;  i2++;
	}

	return ((s1.size() == s2.size()) ? 0 : ((s1.size() < s2.size()) ? -1 : 1));	
}

};
