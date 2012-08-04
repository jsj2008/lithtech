#ifndef __WON_AUTHCERTIFICATEBASE_H__
#define __WON_AUTHCERTIFICATEBASE_H__
#include "WONShared.h"

#include "AuthBase.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class AuthCertificateBase : public AuthBase
{
protected:
	virtual ~AuthCertificateBase() { }

public:
	AuthCertificateBase(const void *theData = NULL, unsigned long theDataLen = 0) : AuthBase(theData,theDataLen), mUserId(0) {}
	
	unsigned long GetUserId() const { return mUserId; }
	const ElGamal& GetPubKey() const { return mPubKey; }

	virtual bool VerifyPermissions(DWORD, DWORD, unsigned short) const { return false; }


protected:
	unsigned long         mUserId;       // WON User ID (WONUserSeq)
	ElGamal mPubKey;					 // Public key
};

typedef SmartPtr<AuthCertificateBase> AuthCertificateBasePtr;

}; // namespace WONAPI

#endif
