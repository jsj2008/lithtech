#include "CDKey.h"
#include "WONCommon/CRC.h"

using namespace WONAPI;
using namespace std;

namespace
{
	const char* SKIPCHAR_MAP = "- \t";
	const char*         STRINGKEY_MAP  = "CVCNCVCNCVCNCVCNNNNN";
	const unsigned int  STRINGKEY_LEN  = strlen(STRINGKEY_MAP);
	const int           DASH_OFFSET    = 4;
	const unsigned char BETA_MASK      = 0x01;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void AddBits(char *theBuf, int &theOffset, int theBits, int theNumBits)
{
	int aBytePos = theOffset/8;
	int aBitPos = theOffset%8;

	int aMask = (theBits<<aBitPos)&0xff;
	theBuf[aBytePos] |= aMask;

	int extra = aBitPos + theNumBits - 8;
	if(extra>0)
	{
		aMask = (theBits>>(theNumBits - extra))&0xff;
		theBuf[aBytePos+1] |= aMask;
	}
		
	theOffset+=theNumBits;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool ProcessCChar(char *theBuf, int& theOffset, char theChar)
{
	int aVal = 0;

	// Determine mask based on char
	switch (toupper(theChar))
	{
		case 'B':
			aVal = 0;   break;
		case 'C':
			aVal = 1;   break;
		case 'D':
			aVal = 2;   break;
		case 'F':
			aVal = 3;   break;
		case 'G':
			aVal = 4;   break;
		case 'J':
			aVal = 5;   break;
		case 'L':
			aVal = 6;   break;
		case 'M':
			aVal = 7;   break;
		case 'N':
			aVal = 8;   break;
		case 'P':
			aVal = 9;   break;
		case 'R':
			aVal = 10;  break;
		case 'S':
			aVal = 11;  break;
		case 'T':
			aVal = 12;  break;
		case 'W':
			aVal = 13;  break;
		case 'X':
			aVal = 14;  break;
		case 'Z':
			aVal = 15;  break;
		default:
			return false;
	}

	AddBits(theBuf,theOffset,aVal,4);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool ProcessVChar(char *theBuf, int& theOffset, char theChar)
{
	int aVal = 0;

	// Determine mask based on char
	switch (toupper(theChar))
	{
		case 'A':
			aVal = 0;  break;
		case 'E':
			aVal = 1;  break;
		case 'U':
			aVal = 2;  break;
		case 'Y':
			aVal = 3;  break;
		default:
			return false;
	}

	AddBits(theBuf,theOffset,aVal,2);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool ProcessNChar(char *theBuf, int& theOffset, char theChar)
{
	if(!isdigit(theChar) || theChar<='1') // has to be '2'-'9'
		return false;

	int aVal = (theChar - '0') - 2;
	AddBits(theBuf,theOffset,aVal,3);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CDKey::CDKey()
{
	mIsValid = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CDKey::CDKey(const string& theProduct) : mProduct(theProduct)
{
	mIsValid = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CDKey::Invalidate()
{
	mIsValid = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CDKey::SetProductString(const std::string &theProductString)
{
	Invalidate();
	mProduct = theProductString;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::CalcKeyFromBuffer(const char *theBuf)
{
	mKey = new ByteBuffer(theBuf,8);

	CRC16 aCRC;
	aCRC.Put(mProduct.data(),mProduct.length());
	aCRC.Put(theBuf,3);
	aCRC.Put(theBuf+4,4);

	// Take the middle 8 bits of the the 16 bit CRC to get the 8-bit light check
	char aLightCheck = (aCRC.Get() & 0x0fff) >> 4;
	mIsValid = aLightCheck==theBuf[3];
	return mIsValid;	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::Init(const string& theKey)
{
	mIsValid = false;
	mKey = NULL;

	// remove skip chars such as dashes
	string aKey;
	int aPos = 0;
	while(aPos < theKey.length()) 
	{
		unsigned char aChar = theKey[aPos];
		if(strchr(SKIPCHAR_MAP,aChar)==NULL) // not a skip char
			aKey+=aChar;

		aPos++;
	}

	// Validate length
	if (aKey.size() != STRINGKEY_LEN)
	{
		mIsValid = false;
		return false;
	}

	char aBuf[8];
	memset(aBuf,0,8);
	
	int anOffset = 0;
	for (int i=0; i < STRINGKEY_LEN; i++)
	{
		bool aTst = true;
		switch (STRINGKEY_MAP[i])
		{
			case 'C':
				aTst = ProcessCChar(aBuf, anOffset, aKey[i]);  break;
			case 'V':
				aTst = ProcessVChar(aBuf, anOffset, aKey[i]);  break;
			case 'N':
				aTst = ProcessNChar(aBuf, anOffset, aKey[i]);  break;
		}

		if (! aTst)
		{
			mIsValid = false;
			return false;
		}
	}

	return CalcKeyFromBuffer(aBuf);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int ValFromBits(const char *theBuf, int &theOffset, int theNumBits)
{
	int aBytePos = theOffset/8;
	int aBitPos = theOffset%8;

	int aVal = ((unsigned char)theBuf[aBytePos]>>aBitPos)&((1<<theNumBits)-1)&0xff;
	
	int extra = theNumBits + aBitPos - 8;
	if(extra>0)
	{
		int extraBits = theBuf[aBytePos+1] & ((1<<extra)-1);
		extraBits <<= theNumBits-extra;
		aVal |= extraBits;
	}

	theOffset+=theNumBits;
	return aVal;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char BuildCChar(const char *theBuf, int& theOffset) 
{
	char aChar = 0;

	// Determine mask based on char
	switch (ValFromBits(theBuf, theOffset, 4))
	{
	case 0:
		aChar = 'B';   break;
	case 1:
		aChar = 'C';   break;
	case 2:
		aChar = 'D';   break;
	case 3:
		aChar = 'F';   break;
	case 4:
		aChar = 'G';   break;
	case 5:
		aChar = 'J';   break;
	case 6:
		aChar = 'L';   break;
	case 7:
		aChar = 'M';   break;
	case 8:
		aChar = 'N';   break;
	case 9:
		aChar = 'P';   break;
	case 10:
		aChar = 'R';   break;
	case 11:
		aChar = 'S';   break;
	case 12:
		aChar = 'T';   break;
	case 13:
		aChar = 'W';   break;
	case 14:
		aChar = 'X';   break;
	case 15:
		aChar = 'Z';   break;
	}

	return aChar;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char BuildVChar(const char *theBuf, int& theOffset) 
{
	char aChar = 0;

	// Determine mask based on char
	switch (ValFromBits(theBuf, theOffset, 2))
	{
		case 0:
			aChar = 'A';   break;
		case 1:
			aChar = 'E';   break;
		case 2:
			aChar = 'U';   break;
		case 3:
			aChar = 'Y';   break;
	}

	return aChar;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char BuildNChar(const char *theBuf, int& theOffset)
{
	return ValFromBits(theBuf, theOffset, 3) + '0' + 2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
string CDKey::GetString() const
{
	if(mKey==NULL)
		return "";

	const char *aBuf = mKey->data();
	int anOffset = 0;
	string aKey = "";

	for (int i=0; i < STRINGKEY_LEN;)
	{
		switch (STRINGKEY_MAP[i])
		{
			case 'C':
				aKey += BuildCChar(aBuf, anOffset);  break;
			case 'V':
				aKey += BuildVChar(aBuf, anOffset);  break;
			case 'N':
				aKey += BuildNChar(aBuf, anOffset);  break;
		}

		// Add dash every four chars
		if ((((++i) % DASH_OFFSET) == 0) && (i < STRINGKEY_LEN))
			aKey += '-';
	}

	return aKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CDKey::GetSymmetricKey(Blowfish &theSymKey, bool useNewMethod) const
{
	CRC16     aCRC;
	RawBuffer aBuf;

	if(useNewMethod)
	{
#ifdef WIN32
		char path[MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		GetWindowsDirectory(path,MAX_PATH);
		_splitpath(path,drive,dir,fname,ext);

		std::string aDrive = drive;
		if(!aDrive.empty())
		{
			if(aDrive[aDrive.length()-1]!='\\')
				aDrive += '\\';

			DWORD aSerialNumber = 0;
			GetVolumeInformation(aDrive.c_str(),NULL,NULL,&aSerialNumber,NULL,NULL,NULL,0);
			aBuf.append((const unsigned char*)(&aSerialNumber),4);
		}
#endif
	}

	// CRC the product and use it as 1st 2 bytes of key
	aCRC.Put(mProduct.data(),mProduct.length());
	unsigned short aCheckSum = aCRC.Get();
	aBuf.append(reinterpret_cast<unsigned char*>(&aCheckSum), sizeof(aCheckSum));

	// CRC each of 1st 3 chars of product and add them to key.
	for (int i=0; (i < 3) && (i < mProduct.size()); i++)
	{
		char aChar = mProduct[i];
		aCRC.Put(&aChar,1);
		aCheckSum = aCRC.Get();
		aBuf.append((const unsigned char*)(&aCheckSum), sizeof(aCheckSum));
	}

	theSymKey.SetKey(aBuf.data(),aBuf.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr CDKey::GetEncrypted() const
{

	if(!IsValid())
		return NULL;

	Blowfish aSymKey;
	GetSymmetricKey(aSymKey, GetUseNewEncryptMethod());
	return aSymKey.Encrypt(mKey->data(),mKey->length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::InitEncrypted(const void *theEncryptedKey, unsigned long theEncryptLen)
{
	mIsValid = false;
	mKey = NULL;

	// Punt if no key
	if(theEncryptedKey==NULL)
		return false;

	bool success = false;
	bool useNewMethod = (GetUseNewEncryptMethod());
	Blowfish aSymKey;

	// Decrypt key using new or old method as specified.
	// Try to build the key if decrypt succeeds.
	GetSymmetricKey(aSymKey,useNewMethod);
	ByteBufferPtr aDecrypt = aSymKey.Decrypt(theEncryptedKey,theEncryptLen);
	if (aDecrypt.get()!=NULL)
		success = CalcKeyFromBuffer(aDecrypt->data());

	// If we haven't succeeded yet try the opposite method
	if (!success)
	{
		useNewMethod = (!useNewMethod);
		GetSymmetricKey(aSymKey,useNewMethod);
		aDecrypt = aSymKey.Decrypt(theEncryptedKey,theEncryptLen);
		if(aDecrypt.get()!=NULL)
			success = CalcKeyFromBuffer(aDecrypt->data());
	}

	// If our last try was the old method, notify.
	if (!useNewMethod)
		NotifyUsedOldMethodToDecrypt();

	return success;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// default path for cd keys
const char*		CDKey::mCDKeyPath = "SOFTWARE\\Sierra\\CDKeys";
bool CDKey::SaveToRegistry(RegKeyRoot theRoot, const char* thePath) 
{
	if(!IsValid())
		return false;

	RegKey aKey(theRoot, thePath, true);
	if(!aKey.IsOpen())
		return false;

	ByteBufferPtr anEncrypt = GetEncrypted();
	if(anEncrypt.get()==NULL)
		return false;

	return aKey.SetValue(mProduct,anEncrypt->data(),anEncrypt->length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::LoadFromRegistry(RegKeyRoot theRoot, const char* thePath)
{
	if(mProduct.empty())
		return false;

	RegKey aKey(theRoot, thePath);
	if(!aKey.IsOpen())
		return false;

	ByteBufferPtr anEncrypt;
	if(aKey.GetValue(mProduct,anEncrypt)!=RegKey::Ok)
		return false;

	return InitEncrypted(anEncrypt->data(),anEncrypt->length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CDKey::ClearFromRegistry(RegKeyRoot theRoot, const char* thePath)
{
	if(mProduct.empty())
		return;

	RegKey aKey(theRoot, thePath);
	aKey.DeleteValue(mProduct);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::SaveToFile(const std::string &theFileName)
{
	if(!IsValid())
		return false;
	
	FILE *aFile = fopen(theFileName.c_str(),"wb");
	if(aFile==NULL)
		return false;

	ByteBufferPtr anEncrypt = GetEncrypted();	
	if(anEncrypt.get()==NULL)
		return false;

	if(fwrite(anEncrypt->data(),1,anEncrypt->length(),aFile)!=anEncrypt->length())
		return false;

	fclose(aFile);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::LoadFromFile(const std::string &theFileName)
{
	FILE *aFile = fopen(theFileName.c_str(),"rb");
	if(aFile==NULL)
		return false;

	char aBuf[2048];
	int aReadLen = fread(aBuf,1,2048,aFile);
	fclose(aFile);
	
	return InitEncrypted(aBuf,aReadLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool CDKey::IsBeta() const
{
	if(mKey.get()==NULL)
		return false;
	else
		return (*(mKey->data()) & BETA_MASK);
}
