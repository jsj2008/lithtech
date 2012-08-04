#include "Auth2Certificate.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Auth2Certificate::~Auth2Certificate()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Auth2Certificate::UnpackHook(ReadBuffer &theData)
{
	if(!AuthBase::UnpackHook(theData))
		return false;

	unsigned short aDataCount = theData.ReadShort();
	for(int i=0; i<aDataCount; i++)
	{
		unsigned short aTypeId = theData.ReadShort();
		unsigned short aDataLen = theData.ReadShort();
		int aPos = theData.pos();
		switch(aTypeId)
		{

			case 0:  // standard login data
			{
				mUserId = theData.ReadLong();
				unsigned short aKeyLen = theData.ReadShort();
				if(!mPubKey.SetPublicKey(theData.ReadBytes(aKeyLen),aKeyLen))
					return false;

				theData.ReadWString(mUserName);
				unsigned char aCommunityCount = theData.ReadByte();
				for(int i=0; i<aCommunityCount; i++)
					mCommunityTrustMap[theData.ReadLong()] = theData.ReadShort();
 
			}
			break;

			case 1: // user data
			{
				DWORD aCommunityId = theData.ReadLong();
				mUserDataMap[aCommunityId] = theData.ReadBuf(2);
			}
			break;

			case 2:	// nickname
			{
				wstring aKey, aVal;
				theData.ReadWString(aKey);
				theData.ReadWString(aVal);
				mNicknameMap[aKey] = aVal;
			}
			break;

			case 3: // KeyId data block
			{
				DWORD aCommunityId = theData.ReadLong();
				DWORD aKeyId = theData.ReadLong();
				mKeyIdMap[aCommunityId] = aKeyId;
			}
			break;
		}

		theData.ReadBytes(aDataLen - (theData.pos() - aPos));
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool Auth2Certificate::VerifyPermissions(DWORD theUserId, DWORD theCommunity, unsigned short theMinTrust) const
{
	if(theUserId!=0 && mUserId!=theUserId)
		return false;

	CommunityTrustMap::const_iterator anItr;
	if(theCommunity!=0)
	{
		anItr = mCommunityTrustMap.find(theCommunity);
		if(anItr==mCommunityTrustMap.end())
			return false;
		else
			return anItr->second >= theMinTrust;
	}
	else
	{
		anItr = mCommunityTrustMap.begin();
		while(anItr!=mCommunityTrustMap.end())
		{
			if(theMinTrust > anItr->second)
				return true;

			++anItr;
		}

		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::wstring Auth2Certificate::GetNickname(const std::wstring &theKey) const
{
	NicknameMap::const_iterator anItr = mNicknameMap.find(theKey);
	if(anItr==mNicknameMap.end())
		return L"";
	else
		return anItr->second;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr Auth2Certificate::GetUserData(DWORD theCommunityId) const
{
	UserDataMap::const_iterator anItr = mUserDataMap.find(theCommunityId);
	if(anItr==mUserDataMap.end())
		return NULL;
	else
		return anItr->second;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD Auth2Certificate::GetKeyId(DWORD theCommunityId) const
{
	KeyIdMap::const_iterator anItr = mKeyIdMap.find(theCommunityId);
	if(anItr==mKeyIdMap.end())
		return 0;
	else
		return anItr->second;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::wstring Auth2Certificate::GetFirstNickname() const
{
	if (mNicknameMap.empty())
		return L"";
	else
		return GetNickname(mNicknameMap.begin()->second);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned short Auth2Certificate::GetFirstCommunityTrust() const
{
	if (mCommunityTrustMap.empty())
		return 0;
	else
		return (mCommunityTrustMap.begin()->second);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr Auth2Certificate::GetFirstUserData() const
{
	if (mUserDataMap.empty())
		return NULL;
	else
		return (mUserDataMap.begin()->second);
}
