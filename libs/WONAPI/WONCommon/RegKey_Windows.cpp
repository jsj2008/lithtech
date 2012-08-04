#include "RegKey_Windows.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HKEY RegKey::GetRootHKey(RegKeyRoot theRoot)
{
#ifdef WIN32
	switch(theRoot)
	{
		case REGKEY_CLASSES_ROOT: return HKEY_CLASSES_ROOT;
		case REGKEY_CURRENT_USER: return HKEY_CURRENT_USER; 
		case REGKEY_LOCAL_MACHINE: return HKEY_LOCAL_MACHINE;
		case REGKEY_USERS: return HKEY_USERS;
		case REGKEY_PERFORMANCE_DATA: return HKEY_PERFORMANCE_DATA;
		case REGKEY_CURRENT_CONFIG: return HKEY_CURRENT_CONFIG;
		case REGKEY_DYN_DATA: return HKEY_DYN_DATA;
	}
#endif

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::RegKey(void) : mLeafName(), mBufLen(0), mBufP(NULL), mOpen(false), mKey(NULL), mKeyIndex(0), mValueIndex(0) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::RegKey(RegKeyRoot theRoot, const string& theKey, bool createKey, bool nonVolatile) :
	mKey(), mOpen(false), mBufP(NULL), mBufLen(0), mLeafName(), mKeyIndex(0), mValueIndex(0) 
{
	OpenNewKey(theRoot,theKey,createKey,nonVolatile);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::~RegKey(void)
{
	if(mOpen)
		RegCloseKey(mKey);

	delete [] mBufP;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RegKey::CloseKey(void)
{
	if(mOpen)
	{
		RegCloseKey(mKey);
		mOpen = false;

	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD RegKey::GetToBuf(const string& theName, unsigned long &theLengthR) const
{
	DWORD aType;
	DWORD aLen;

	// First get length of data
	if(RegQueryValueEx(mKey, theName.c_str(), NULL, &aType,
	                   NULL, &aLen) != ERROR_SUCCESS)
	{
		return REG_NONE;

	}

	theLengthR = aLen;

	// Allocate the buffer
	if(mBufLen < aLen)
	{
		delete [] mBufP;
		mBufLen = aLen;
		mBufP   = new unsigned char [aLen];

	}

	// Fetch the value
	if(RegQueryValueEx(mKey, theName.c_str(), NULL, &aType,
	                   static_cast<unsigned char *>(mBufP), &aLen) == ERROR_SUCCESS)
	{
		return aType;

	}
	else
	{
		return REG_NONE;

	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::OpenNewKey(RegKeyRoot theRoot, const string& theKey, bool createKey, bool nonVolatile)
{
	return OpenNewKey(GetRootHKey(theRoot),theKey,createKey,nonVolatile);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::OpenNewKey(HKEY theRoot, const string& theKey, bool createKey, bool nonVolatile)
{
#ifdef WIN32
	CloseKey();

	if(theKey.size() > 0)
	{
		if(createKey)
		{
			char aClass;
			DWORD wasCreated;
			DWORD anOptionValue((nonVolatile) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE);

			mOpen = (RegCreateKeyEx(theRoot, theKey.c_str(), NULL, &aClass, anOptionValue, KEY_ALL_ACCESS, NULL, &mKey, &wasCreated) == ERROR_SUCCESS);

		}
		else
			mOpen = (RegOpenKeyEx(theRoot, theKey.c_str(), NULL, KEY_ALL_ACCESS, &mKey) == ERROR_SUCCESS);

		if(mOpen)
			mLeafName = theKey;
	}
#endif

	return mOpen;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetValue(const string& theName, string& theValR) const
{
	GetResult ret = NotFound;

	theValR.erase();
	if(!IsOpen())
		return NotFound;

	unsigned long aLength;
	switch (GetToBuf(theName, aLength))
	{
		case REG_SZ:
			theValR = reinterpret_cast<char *>(mBufP);
			ret = Ok;
			break;
		case REG_NONE:
			ret = NotFound;
			break;
		default:
			ret = BadType;
			break;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetValue(const string& theName, unsigned long& theValR) const
{

	GetResult ret = NotFound;

	theValR = 0;
	if(!IsOpen())
		return NotFound;

	unsigned long aLength;
	switch (GetToBuf(theName, aLength))
	{
		case REG_DWORD:
			memcpy(&theValR, mBufP, sizeof(unsigned long));
			ret = Ok;
			break;
		case REG_NONE:
			ret = NotFound;
			break;
		default:
			ret = BadType;
			break;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetValue(const std::string& theName, ByteBufferPtr &theValR) const
{
	GetResult ret = NotFound;

	if(!IsOpen())
		return NotFound;

	unsigned long aLenR = 0;
	switch (GetToBuf(theName, aLenR))
	{
		case REG_BINARY:
			if(aLenR)
			{
				theValR = new ByteBuffer(mBufP,aLenR);
				ret = Ok;
			}
			else
				ret = NoSize;

			break;
		case REG_NONE:
			ret = NotFound;
			break;
		default:
			ret = BadType;
			break;
	}

	return ret;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::SetValue(const string& theName, const string& theValue){

	if(!IsOpen())
		return false;

	if(RegSetValueEx(mKey, theName.c_str(), NULL, REG_SZ,
	                  reinterpret_cast<CONST BYTE*>(theValue.c_str()),
	                  (theValue.size()+1)) == ERROR_SUCCESS)
	{
		return true;

	}
	else
		return false;

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::SetValue(const string& theName, unsigned long theValue)
{
	if(!IsOpen())
		return false;
	
	if(RegSetValueEx(mKey, theName.c_str(), NULL, REG_DWORD,
	                  reinterpret_cast<CONST BYTE*>(&theValue),
	                  sizeof(theValue)) == ERROR_SUCCESS)
	{
		return true;
	}
	else
		return false;

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::SetValue(const std::string& theName, const void* theValueP, unsigned long theLength)
{
	if(!IsOpen())
		return false;

	if(RegSetValueEx(mKey, theName.c_str(), NULL, REG_BINARY,
	                 (const unsigned char*)theValueP,
	                 theLength) == ERROR_SUCCESS)
	{
		return true;

	}
	else
		return false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::DeleteValue(const string& theName)
{
	if(!IsOpen())
		return false;

	return (RegDeleteValue(mKey, theName.c_str()) == ERROR_SUCCESS);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::OpenSubKey(const std::string& theSubKey, bool createSubKey, bool nonVolatile)
{
	bool subkeyOpen(false);

#ifdef WIN32
	if(theSubKey.size() > 0 && mOpen)
	{
		HKEY aSubKey;
		
		if(createSubKey)
		{
			char aClass;
			DWORD wasCreated;
			DWORD anOptionValue((nonVolatile) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE);

			subkeyOpen = (RegCreateKeyEx(mKey, theSubKey.c_str(), NULL, &aClass, anOptionValue, KEY_ALL_ACCESS, NULL, &aSubKey, &wasCreated) == ERROR_SUCCESS);
		}
		else
			subkeyOpen = (RegOpenKeyEx(mKey, theSubKey.c_str(), NULL, KEY_ALL_ACCESS, &aSubKey) == ERROR_SUCCESS);

		if(subkeyOpen)
		{
			CloseKey();
			mLeafName = theSubKey;
			mKey = aSubKey;
			mOpen = subkeyOpen;
		}
	}
#endif

	return subkeyOpen;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::GetSubKey(const std::string& theSubKeyName, RegKey& theSubKey, bool createSubKey, bool nonVolatile) const 
{
	if(theSubKeyName.size() > 0 && mOpen)
		return theSubKey.OpenNewKey(mKey,theSubKeyName, createSubKey, nonVolatile);

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::DeleteSubKey(const std::string& theSubKey) const 
{

	if(theSubKey.size() > 0 && mOpen)
		return (RegDeleteKey(mKey, theSubKey.c_str()) == ERROR_SUCCESS);

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RegKey::DeleteSubKey(RegKey& theSubKey) const 
{
	if(mOpen)
	{
		if(theSubKey.mOpen)
			theSubKey.CloseKey();

		return (RegDeleteKey(mKey, theSubKey.GetLeafName().c_str()) == ERROR_SUCCESS);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetFirstSubKey(RegKey& theSubKey) const 
{
	mKeyIndex = 0;
	return ((GetNextSubKey(theSubKey) == NoMore) ? NotFound : Ok);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetNextSubKey(RegKey& theSubKey) const 
{
	FILETIME aFileTime;
	char aNameBuffer[256];
	DWORD aNameLength(sizeof(aNameBuffer));
	GetResult ret(NoMore);

	if(RegEnumKeyEx(mKey, mKeyIndex, aNameBuffer, &aNameLength, 0, NULL, NULL, &aFileTime) != ERROR_NO_MORE_ITEMS)
	{
		mKeyIndex++;
		if(theSubKey.OpenNewKey(mKey,aNameBuffer))
			ret = Ok;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetFirstValueName(std::string& theName, DataType& theType) const 
{
	mValueIndex = 0;
	return ((GetNextValueName(theName, theType) == NoMore) ? NotFound : Ok);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RegKey::GetResult RegKey::GetNextValueName(std::string& theName, DataType& theType) const 
{
	char aNameBuffer[256];
	DWORD aNameLength(sizeof(aNameBuffer)), aDataType;
	GetResult ret(NoMore);

	if(RegEnumValue(mKey, mValueIndex, aNameBuffer, &aNameLength, 0, &aDataType, NULL, NULL) != ERROR_NO_MORE_ITEMS)
	{
		switch(aDataType)
		{
			case REG_DWORD:
				theType=Long;
				break;
			case REG_SZ:
				theType=String;
				break;
			case REG_BINARY:
				theType=Binary;
				break;
			default:
				theType=Other;
				break;
		}
		mValueIndex++;
		theName = aNameBuffer;
		ret = Ok;
	}

	return ret;
}


