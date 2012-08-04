#ifndef __WON_AUTH2CERTIFICATE_H__
#define __WON_AUTH2CERTIFICATE_H__
#include "WONShared.h"

#include "AuthCertificateBase.h"
#include <map>


namespace WONAPI
{

typedef std::map<DWORD,unsigned short> CommunityTrustMap;
typedef std::map<std::wstring,std::wstring> NicknameMap;
typedef std::map<DWORD, ByteBufferPtr> UserDataMap; // map from community id to user data
typedef std::map<DWORD, DWORD> KeyIdMap;	// map from community id to unique key

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Auth2Certificate : public AuthCertificateBase
{
private:
	std::wstring			mUserName;		// WON User Name
	CommunityTrustMap		mCommunityTrustMap;
	NicknameMap				mNicknameMap;
	UserDataMap				mUserDataMap;
	KeyIdMap				mKeyIdMap;

protected:
	virtual bool UnpackHook(ReadBuffer &theData);
	virtual ~Auth2Certificate();

public:
	Auth2Certificate(const void *theData = NULL, unsigned long theDataLen = 0) : AuthCertificateBase(theData,theDataLen) { Unpack(); }
	const CommunityTrustMap& GetCommunityTrustMap() const { return mCommunityTrustMap; }
	time_t ComputeAuthDelta() const { return GetIssueTime() - time(NULL); }

	bool VerifyPermissions(DWORD theUserId, DWORD theCommunity, unsigned short theMinTrust) const;
	const std::wstring& GetUserName() const { return mUserName; }

	std::wstring GetNickname(const std::wstring &theKey) const;
	const NicknameMap& GetNicknameMap() const { return mNicknameMap; }

	ByteBufferPtr GetUserData(DWORD theCommunityId) const;
	const UserDataMap& GetUserDataMap() const { return mUserDataMap; }

	DWORD GetKeyId(DWORD theCommunityId) const;
	const KeyIdMap& GetKeyIdMap() const { return mKeyIdMap; }

	// The following are provided to simplify the interface 
	std::wstring   GetFirstNickname()		const;
	unsigned short GetFirstCommunityTrust()	const;
	ByteBufferPtr  GetFirstUserData()		const;
};

typedef ConstSmartPtr<Auth2Certificate> Auth2CertificatePtr;


}; // namespace WONAPI

#endif
