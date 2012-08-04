#ifndef __WON_AUTHCERTIFICATE_H__
#define __WON_AUTHCERTIFICATE_H__
#include "WONShared.h"

#include "AuthCertificateBase.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AuthCertificate : public AuthCertificateBase
{
private:
	unsigned long         mCommunityId;  // User's community ID
	unsigned short        mTrustLevel;   // User's trust level

protected:
	virtual ~AuthCertificate();
	virtual bool UnpackHook(ReadBuffer &theData);

public:
	AuthCertificate(const void *theData = NULL, unsigned long theDataLen = 0) : AuthCertificateBase(theData,theDataLen), mCommunityId(0), mTrustLevel(0) { Unpack(); }
	unsigned long GetCommunityId() const { return mCommunityId; }
	unsigned short GetTrustLevel() const { return mTrustLevel; }
	time_t ComputeAuthDelta() const { return GetIssueTime() - time(NULL); }

	virtual bool VerifyPermissions(DWORD theUserId, DWORD theCommunity, unsigned short theMinTrust) const;
};

typedef ConstSmartPtr<AuthCertificate> AuthCertificatePtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
