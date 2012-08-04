#include "ConfigObject.h"
#include "WONCommon/StringUtil.h"
#include "StringParser.h"

using namespace std;
using namespace WONAPI;
using namespace WONTypes;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ConfigObject::ConfigObject()
{
	Rewind();
	mParent = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::Reset()
{
	Clear();
	mObjectName.erase();
	mInheritedName.erase();

	KeyValMap::iterator aKeyValItr = mKeyValMap.begin();
	while(aKeyValItr!=mKeyValMap.end())
	{
		ConfigValueBase *aConfigVal = aKeyValItr->second;
		aConfigVal->InitializeDefault();

		++aKeyValItr;
	}

	KeyGroupMap::iterator aKeyGroupItr = mKeyGroupMap.begin();
	while(aKeyGroupItr!=mKeyGroupMap.end())
	{
		GroupType &aGroupType = aKeyGroupItr->second;

		if(aGroupType.mObjectPtr!=NULL)
			*(aGroupType.mObjectPtr) = NULL;
	
		++aKeyGroupItr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::CopyKeyValsFrom(ConfigObject *theCopyFrom)
{
	KeyValMap &aMap1 = mKeyValMap;
	KeyValMap &aMap2 = theCopyFrom->mKeyValMap;

	KeyValMap::iterator anItr1 = aMap1.begin();
	KeyValMap::iterator anItr2 = aMap2.begin();
	while(anItr1!=aMap1.end() && anItr2!=aMap2.end())
	{
		int aComp = StringCompareNoCase(anItr1->first, anItr2->first);
		if(aComp < 0)
			++anItr1;
		else if(aComp > 0)
			++anItr2;
		else
		{
			ConfigValueBase *aVal1 = anItr1->second;
			ConfigValueBase *aVal2 = anItr2->second;
			aVal1->CopyValue(aVal2);

			++anItr1;
			++anItr2;
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::QueryValue(KeyValMap::const_iterator theItr, std::string &theValR)
{
	ConfigValueBase *aVal = theItr->second;
	aVal->WriteValue(theValR);
	if(aVal->GetIsDefault()) theValR += "  [DEF]";
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::QueryValue(const std::string &theKey, std::string &theValR)
{
	KeyValMap::const_iterator anItr = mKeyValMap.find(theKey);
	if(anItr!=mKeyValMap.end())
		return QueryValue(anItr,theValR);
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::QueryAllValues(std::string &theValues)
{
	KeyValMap::const_iterator anItr = mKeyValMap.begin();
	while(anItr!=mKeyValMap.end())
	{
		theValues += anItr->first;
		theValues += ": ";
		QueryValue(anItr,theValues);
		theValues += "\n";

		++anItr;
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ConfigObject::SetValueResult ConfigObject::SetValue(const std::string &theKey, StringParser &theParser)
{
	KeyValMap::const_iterator anItr = mKeyValMap.find(theKey);
	if(anItr!=mKeyValMap.end())
	{
		ConfigValueBase *aVal = anItr->second;
		if(aVal->ReadValue(theParser))
		{
			aVal->SetIsDefault(false);
			return SetValue_Success;
		}
		else
			return SetValue_ErrorSetting;
	}
	else
		return SetValue_NotFound;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::Clear()
{
	mChildList.clear();
	Rewind();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::Rewind()
{
	mChildItr = mChildList.begin();
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ConfigObject* ConfigObject::NextChild()
{
	if(mChildItr==mChildList.end())
		return NULL;

	return *mChildItr++;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::HasMoreChildren()
{
	return mChildItr!=mChildList.end();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::AddChild(ConfigObject *theChild)
{
	theChild->mParent = this;
	mChildList.push_back(theChild);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::ReadGroupStart(ConfigParser &theParser, std::string &theGroupName, std::string &theInheritedName)
{
	string aToken;
	if(!theParser.ReadToken(aToken))
		return false;

	if(aToken!="{") // if not '{' then must be [name] [: inherited name] {
	{
		StringParser aParser(aToken.c_str());
		if(aToken!=":") // if not ':' then must be the ObjectName
		{
			aParser.ReadValue(theGroupName);
			if(!theParser.ReadToken(aToken)) // read ':' or '{'
				return false;
		}

		if(aToken==":")
		{
			std::string anotherToken;
			if(!theParser.ReadToken(anotherToken)) // read inherited name
				return false;

			aParser.Set(anotherToken.c_str());
			aParser.ReadValue(mInheritedName);

			if(!theParser.ReadToken(aToken)) // read '{'
				return false;
		}
	}

	if(aToken!="{")
	{
		theParser.AbortRead("Expecting '{' but got " + aToken);
		return false;
	}	

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::ParseGroup(ConfigParser& theParser, bool checkBraces, bool skip, bool doPreAndPostParse)
{
	if(checkBraces)
	{
		if(!ReadGroupStart(theParser, mObjectName, mInheritedName))
			return false;
	}

	if(doPreAndPostParse && !skip)
	{
		if(!PreParse(theParser))
			return false;
	}

	// Read Key/Val's and Child groups
	while(!theParser.IsAborted() && !theParser.IsStopped())
	{
		string aKey;
		if(!theParser.ReadToken(aKey))
		{
			if(checkBraces)
			{
				theParser.AbortRead("Expecting '}'");
				return false;
			}
			else
				break;
		}

		StringToUpperCaseInPlace(aKey);
		if(aKey=="}") // end of group
		{
			if(!checkBraces)
			{
				theParser.AbortRead("Unexpected '}'");
				return false;
			}
			else
				break;
		}	
		else if(aKey[aKey.length()-1]==':') // key-value pair
		{
			string aVal;
			aKey.resize(aKey.length()-1);
			if(!theParser.ReadToken(aVal,true,skip))
				return false;

			if(!skip)
			{
				StringParser aParser(aVal.c_str());
				if(!HandleKeyVal(theParser, aKey, aParser))
					theParser.AddWarning("Unexpected key/val: " + aKey + " " + aVal);
			}
		}
		else	// child group
		{
			if(!skip)
			{
				if(!HandleChildGroup(theParser, aKey))
				{
					theParser.AddWarning("Unexpected child group: " + aKey);
					SkipGroup(theParser);
				}
			}
			else			
				SkipGroup(theParser);
		}
	}

	if(doPreAndPostParse && !skip)
	{
		if(!PostParse(theParser))
			return false;
	}

	return !theParser.IsAborted();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::SkipGroup(ConfigParser& theParser, bool checkBraces)
{	

	ConfigObjectPtr anObject = new ConfigObject;
	return anObject->ParseGroup(theParser,checkBraces,true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::IncludeFile(ConfigParser &theParser, const std::string &theFilePath)
{
	string aFilePath = theFilePath;
	int aSlashPos = aFilePath.find_last_of("/");
	if(aSlashPos==string::npos)
		aSlashPos = aFilePath.find_last_of("\\");

	if(theParser.GetIgnorePathOnIncludeFile() && aSlashPos!=string::npos)
		aFilePath = aFilePath.substr(aSlashPos+1, aFilePath.length() - (aSlashPos+1));

	aFilePath = theParser.GetFilePath() + aFilePath;

	ConfigParser aNewParser;
	aNewParser.SetParent(&theParser);
	if(aNewParser.OpenFile(aFilePath))
	{
		bool success = ParseGroup(aNewParser, false, false, false);
		theParser.CopyWarnings(aNewParser);

		if(!success)
		{
			theParser.AbortRead(aNewParser.GetAbortReason());
			return false;
		}

		if(aNewParser.IsStopped())
			theParser.StopRead();
	}
	else
	{
		theParser.AbortRead("File not found: " + aFilePath);
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::ParseChild(ConfigParser &theParser, ConfigObject *theChild)
{
	theChild->mParent = this;
	theChild->Reset();
	if(!theChild->ParseGroup(theParser))
		return false;
	else
	{
		AddChild(theChild);
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::HandleChildGroup(ConfigParser &theParser, KeyGroupMap::iterator &theItr)
{
	GroupType &aGroupType = theItr->second;
	ConfigObjectPtr anObject;
	if(aGroupType.mFactory!=NULL)
		anObject = aGroupType.mFactory();
	else
		anObject = aGroupType.mObject;

	anObject->Reset();
	anObject->mParent = this;
	if(!anObject->ParseGroup(theParser))
		return false;

	if(aGroupType.mObjectPtr!=NULL)
		*(aGroupType.mObjectPtr) = anObject;
	else if(aGroupType.mObject.get()==NULL)
		AddChild(anObject);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::HandleChildGroup(ConfigParser &theParser, const std::string& theGroupType)
{
	KeyGroupMap::iterator anItr = mKeyGroupMap.find(theGroupType);
	if(anItr!=mKeyGroupMap.end())
		return HandleChildGroup(theParser, anItr);
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::HandleKeyVal(ConfigParser &theParser, StringParser &theVal, KeyValMap::iterator &theItr)
{
	ConfigValueBase *aVal = theItr->second;
	aVal->SetIsDefault(false);
	if(!aVal->ReadValue(theVal))
		theParser.AddWarning("Failed to parse value for key: " + theItr->first);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::HandleKeyVal(ConfigParser &theParser, const std::string& theKey, StringParser &theVal)
{
	if(theKey=="DEFINE")
	{
		std::string aName;
		std::string aVal;
		if(!theVal.ReadString(aName) || !theVal.ReadString(aVal))
		{
			theParser.AddWarning("Invalid define.");
			return true;
		}

		if(!theParser.AddDefine(aName,aVal))
			return false;

		return true;
	}
	else if(theKey=="INCLUDEFILE")
	{
		std::string aPath;
		theVal.ReadString(aPath);
		return IncludeFile(theParser,aPath);
	}
	else
	{
		KeyValMap::iterator anItr = mKeyValMap.find(theKey);
		if(anItr!=mKeyValMap.end())
			return HandleKeyVal(theParser, theVal, anItr);
		else
			return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::PreParse(ConfigParser &theParser)
{
	return true;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ConfigObject::PostParse(ConfigParser &theParser)
{
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterKeyGroup(const char *theKey, ConfigObjectFactory theFactory, ConfigObjectPtr *theGroup)
{
	if(mKeyGroupMap.insert(KeyGroupMap::value_type(theKey,GroupType(theFactory,theGroup))).second==false)
	{
		// insert failed
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterKeyGroup(const char *theKey, ConfigObject *theGroup)
{
	if(mKeyGroupMap.insert(KeyGroupMap::value_type(theKey,GroupType(theGroup))).second==false)
	{
		// insert failed
	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterKeyValue(const char *theKey, ConfigValueBase *theValue)
{
	ConfigValueBasePtr aVal = theValue; // make sure we account for reference even if someone passed in new ConfigValue
	if(mKeyValMap.insert(KeyValMap::value_type(theKey,aVal)).second==false)
	{
		// insert failed
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterBoolValue(const char *theKey, bool &theBool, bool theDefVal)
{
	RegisterKeyValue(theKey, new BoolConfigValue(theBool,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterIntValue(const char *theKey, int &theInt, int theDefVal)
{
	RegisterKeyValue(theKey, new IntConfigValue(theInt,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterLongValue(const char *theKey, long &theLong, long theDefVal)
{
	RegisterKeyValue(theKey, new LongConfigValue(theLong, theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterShortValue(const char *theKey, short &theShort, short theDefVal)
{
	RegisterKeyValue(theKey, new ShortConfigValue(theShort, theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterByteValue(const char *theKey, char &theByte, char theDefVal)
{
	RegisterKeyValue(theKey, new ByteConfigValue(theByte,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterULongValue(const char *theKey, unsigned long &theLong, unsigned long theDefVal)
{
	RegisterKeyValue(theKey, new ULongConfigValue(theLong,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterUShortValue(const char *theKey, unsigned short &theShort, unsigned short theDefVal)
{
	RegisterKeyValue(theKey, new UShortConfigValue(theShort,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterUByteValue(const char *theKey, unsigned char &theByte, unsigned char theDefVal)
{
		RegisterKeyValue(theKey, new UByteConfigValue(theByte,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterStringValue(const char *theKey, std::string &theStr, const char *theDefVal)
{
	RegisterKeyValue(theKey, new StringConfigValue(theStr,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterWStringValue(const char *theKey, std::wstring &theStr, const wchar_t *theDefVal)
{
	RegisterKeyValue(theKey, new WStringConfigValue(theStr,theDefVal));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterBoolListValue(const char *theKey, BoolList &theBoolList, const char *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new BoolListConfigValue(theBoolList, theDefVal, additive));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterIntListValue(const char *theKey, IntList &theIntList, const char *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new IntListConfigValue(theIntList, theDefVal, additive));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterStringListValue(const char *theKey, StringList &theStringList, const char *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new StringListConfigValue(theStringList, theDefVal, additive));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterWStringListValue(const char *theKey, WStringList &theStringList, const wchar_t *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new WStringListConfigValue(theStringList, theDefVal, additive));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterBoolListListValue(const char *theKey, BoolListList &theBoolListList, const char *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new BoolListListConfigValue(theBoolListList, theDefVal, additive));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterIntListListValue(const char *theKey, IntListList &theIntListList, const char *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new IntListListConfigValue(theIntListList, theDefVal, additive));
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterStringListListValue(const char *theKey, StringListList &theStringListList, const char *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new StringListListConfigValue(theStringListList, theDefVal, additive));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConfigObject::RegisterWStringListListValue(const char *theKey, WStringListList &theStringListList, const wchar_t *theDefVal, bool additive)
{
	RegisterKeyValue(theKey, new WStringListListConfigValue(theStringListList, theDefVal, additive));
}

