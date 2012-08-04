#ifndef __STRINGPARSER_H__
#define __STRINGPARSER_H__

#include "WONCommon/StringUtil.h"
#include "WONTypes.h"
#include <list>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class StringParser
{
private:
	const char *mStr;
	int mPos;
	bool mIsWString;

public:
	StringParser() { mStr = ""; mPos = 0; mIsWString = false; }
	StringParser(const char *theStr) : mStr(theStr), mPos(0), mIsWString(false) { }
	StringParser(const wchar_t *theWStr) : mStr((char*)theWStr), mPos(0), mIsWString(true) { }

	void Set(const char *theStr) { mStr = theStr; mPos = 0; mIsWString = false; }
	void Set(const wchar_t *theWStr) { mStr = (const char*)theWStr; mPos = 0; mIsWString = true; }

	bool EndOfString();
	char GetChar(int theOffset = 0);
	wchar_t GetWChar(int theOffset = 0);
	void IncrementPos(int theAmount = 1);

	void SetIsWString(bool isWString) { mIsWString = isWString; }
	bool GetIsWString() { return mIsWString; }

	bool StartsWith(const char *theStr);
	bool Is(const char *theStr);

	void SkipWhitespace();
	void SkipNonWhitespace();
	bool CheckNextChar(char theChar);

	bool ReadValue(bool &theBool);

	bool ReadValue(int &theInt);
	bool ReadValue(long &theLong) { int anInt; bool aRet = ReadValue(anInt); theLong = anInt; return aRet; }
	bool ReadValue(unsigned long &theLong) { int anInt; bool aRet = ReadValue(anInt); theLong = anInt; return aRet; }
	bool ReadValue(short &theShort) { int anInt; bool aRet = ReadValue(anInt); theShort = anInt; return aRet; }
	bool ReadValue(unsigned short &theShort) { int anInt; bool aRet = ReadValue(anInt); theShort = anInt; return aRet; }
	bool ReadValue(char &theByte) { int anInt; bool aRet = ReadValue(anInt); theByte = anInt; return aRet; }
	bool ReadValue(unsigned char &theByte) { int anInt; bool aRet = ReadValue(anInt); theByte = anInt; return aRet; }

	bool ReadValue(std::string &theStr);
	bool ReadValue(std::wstring &theStr);

	bool ReadValue(WONTypes::BoolList &theList);
	bool ReadValue(WONTypes::IntList &theList);
	bool ReadValue(WONTypes::StringList &theList);
	bool ReadValue(WONTypes::WStringList &theList);

	bool ReadValue(WONTypes::BoolListList &theList);
	bool ReadValue(WONTypes::IntListList &theList);
	bool ReadValue(WONTypes::StringListList &theList);
	bool ReadValue(WONTypes::WStringListList &theList);

	bool ReadString(std::string &theStr) { return ReadValue(theStr); }

public:
	static void ValueToString(const bool &theVal, std::string &theStr);
	static void ValueToString(const int &theVal, std::string &theStr);
	static void ValueToString(const unsigned long &theVal, std::string &theStr);
	static void ValueToString(const long &theVal, std::string &theStr);
	static void ValueToString(const unsigned short &theVal, std::string &theStr);
	static void ValueToString(const short &theVal, std::string &theStr);
	static void ValueToString(const unsigned char &theVal, std::string &theStr);
	static void ValueToString(const char &theVal, std::string &theStr);
	static void ValueToString(const std::string &theVal, std::string &theStr);
	static void ValueToString(const std::wstring &theVal, std::string &theStr);

	static void ValueToString(const WONTypes::BoolList &theVal, std::string &theStr);
	static void ValueToString(const WONTypes::IntList &theVal, std::string &theStr);
	static void ValueToString(const WONTypes::StringList &theVal, std::string &theStr);
	static void ValueToString(const WONTypes::WStringList &theVal, std::string &theStr);

	static void ValueToString(const WONTypes::BoolListList &theVal, std::string &theStr);
	static void ValueToString(const WONTypes::IntListList &theVal, std::string &theStr);
	static void ValueToString(const WONTypes::StringListList &theVal, std::string &theStr);
	static void ValueToString(const WONTypes::WStringListList &theVal, std::string &theStr);
};

}; // namespace WONAPI

#endif
