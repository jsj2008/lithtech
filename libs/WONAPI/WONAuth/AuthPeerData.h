#ifndef __WON_AUTHPEERDATA_H__
#define __WON_AUTHPEERDATA_H__
#include "WONShared.h"

#include "AuthCertificate.h"
#include "Auth2Certificate.h"
#include "AuthPubKeyBlock.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AuthPeerData : public RefCount
{
private:
	AuthCertificatePtr mCertificate;
	Auth2CertificatePtr mCertificate2;
	AuthPubKeyBlockPtr mKeyBlock;	
	ElGamal mPrivateKey;
	time_t mAuthDelta;

protected:
	virtual ~AuthPeerData();

public:
	AuthPeerData(const AuthPubKeyBlock *theKeyBlock = NULL);
	AuthPeerData(const AuthCertificate *theCertificate, const AuthPubKeyBlock *theKeyBlock, 
		const ElGamal &theKey, time_t theAuthDelta, const Auth2Certificate *theCert2 = NULL);

	bool Verify(const AuthBase *theAuthBase) const;
	bool CertificateExpired(time_t theExtraTime = 0) const;
	bool Certificate2Expired(time_t theExtraTime = 0) const;
	bool IsValid() const;
	bool IsValid2() const;

	const AuthCertificate* GetCertificate() const { return mCertificate; }
	const Auth2Certificate* GetCertificate2() const { return mCertificate2; }
	const AuthPubKeyBlock* GetPubKeyBlock() const { return mKeyBlock; }
	const ElGamal& GetPrivateKey() const { return mPrivateKey; }
	time_t GetAuthDelta() const { return mAuthDelta; }
	time_t GetExpireDelta() const;
};

typedef ConstSmartPtr<AuthPeerData> AuthPeerDataPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
