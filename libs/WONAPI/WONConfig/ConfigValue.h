#ifndef __WON_CONFIGVALUE_H__
#define __WON_CONFIGVALUE_H__

#include "WONCommon/SmartPtr.h"
#include "StringParser.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ConfigValueBase : public RefCount
{
protected:
	bool mIsDefault;

	virtual void InitializeDefaultHook() = 0;
	virtual bool CopyValueHook(ConfigValueBase *theCopyFrom) = 0;

public:
	ConfigValueBase() : mIsDefault(false) { }

	void InitializeDefault() { InitializeDefaultHook(); mIsDefault = true; }

	bool GetIsDefault() { return mIsDefault; }
	void SetIsDefault(bool isDefault) { mIsDefault = isDefault; }

	void CopyValue(ConfigValueBase *theCopyFrom) { if(CopyValueHook(theCopyFrom)) mIsDefault = theCopyFrom->mIsDefault; }

	virtual bool ReadValue(StringParser &theParser) = 0;
	virtual void WriteValue(std::string &theStr) = 0;
};
typedef SmartPtr<ConfigValueBase> ConfigValueBasePtr;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T>
class ConfigValue : public ConfigValueBase
{
protected:
	T& mVal;

	virtual bool CopyValueHook(ConfigValueBase *theVal) 
	{ 
		ConfigValue<T> *aVal = dynamic_cast<ConfigValue<T>*>(theVal);
		if(aVal)
		{
			mVal = aVal->mVal;
			return true;
		}
		return false;
	}

public:
	ConfigValue(T &theVal) : mVal(theVal) { }

};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> 
class SimpleConfigValue : public ConfigValue<T>
{
protected:
	T mDefVal;

	virtual void InitializeDefaultHook() { mVal = mDefVal; }

public:
	SimpleConfigValue(T &theVal, T theDefVal) : ConfigValue<T>(theVal), mDefVal(theDefVal) { }

	virtual bool ReadValue(StringParser &theParser) { return theParser.ReadValue(mVal); }
	virtual void WriteValue(std::string &theStr) { StringParser::ValueToString(mVal, theStr); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> 
class ConfigStringValue : public ConfigValue<std::basic_string<T> >
{
protected:
	typedef std::basic_string<T> StrType;
	const T* mDefVal;

	virtual void InitializeDefaultHook() 
	{ 
		if(mDefVal) 
			mVal = mDefVal; 
		else 
			mVal.erase(); 
	}

public:
	ConfigStringValue(StrType &theVal, const T* theDefVal) : ConfigValue<StrType>(theVal), mDefVal(theDefVal) { }

	virtual bool ReadValue(StringParser &theParser) { return theParser.ReadValue(mVal); }
	virtual void WriteValue(std::string &theStr) { StringParser::ValueToString(mVal, theStr); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> 
class ConfigListValue : public ConfigValue<T>
{
protected:
	const char* mDefStr;
	bool mIsWString;
	bool mAdditive;

	virtual void InitializeDefaultHook() 
	{ 
		mVal.clear();
		if(mDefStr)
		{
			StringParser aParser(mDefStr);
			aParser.SetIsWString(mIsWString);
			aParser.ReadValue(mVal); 
		}
	}

public:
	ConfigListValue(T &theVal, const char* theDefVal, bool additive = false) : ConfigValue<T>(theVal), mDefStr(theDefVal), mAdditive(additive), mIsWString(false) { }
	ConfigListValue(T &theVal, const wchar_t* theDefVal, bool additive = false) : ConfigValue<T>(theVal), mDefStr((const char*)theDefVal), mAdditive(additive), mIsWString(true) { }

	virtual bool ReadValue(StringParser &theParser) { if(!mAdditive) mVal.clear(); return theParser.ReadValue(mVal); }
	virtual void WriteValue(std::string &theStr) { StringParser::ValueToString(mVal, theStr); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef SimpleConfigValue<bool> BoolConfigValue;
typedef SimpleConfigValue<int> IntConfigValue;
typedef SimpleConfigValue<char> ByteConfigValue;
typedef SimpleConfigValue<unsigned char> UByteConfigValue;
typedef SimpleConfigValue<short> ShortConfigValue;
typedef SimpleConfigValue<unsigned short> UShortConfigValue;
typedef SimpleConfigValue<long> LongConfigValue;
typedef SimpleConfigValue<unsigned long> ULongConfigValue;

typedef ConfigStringValue<char> StringConfigValue;
typedef ConfigStringValue<wchar_t> WStringConfigValue;

typedef ConfigListValue<WONTypes::BoolList> BoolListConfigValue;
typedef ConfigListValue<WONTypes::IntList> IntListConfigValue;
typedef ConfigListValue<WONTypes::StringList> StringListConfigValue;
typedef ConfigListValue<WONTypes::WStringList> WStringListConfigValue;
typedef ConfigListValue<WONTypes::BoolListList> BoolListListConfigValue;
typedef ConfigListValue<WONTypes::IntListList> IntListListConfigValue;
typedef ConfigListValue<WONTypes::StringListList> StringListListConfigValue;
typedef ConfigListValue<WONTypes::WStringListList> WStringListListConfigValue;

}; // namespace WONAPI

#endif
