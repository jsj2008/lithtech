#ifndef __WON_GETCERTOP_H__
#define __WON_GETCERTOP_H__
#include "WONShared.h"

#include "AuthContext.h" 
#include "WONServer/ServerRequestOp.h"


namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetCertOp : public ServerRequestOp
{
private:
	AuthContextPtr mAuthContext;
	Blowfish mSessionKey;
	AuthPubKeyBlockPtr mKeyBlock;
	time_t mAuthDelta;
	AuthPeerDataPtr mPeerData;
	RawBuffer mChallengeSeed;

	std::wstring mNewPassword;
	bool mCreateAccount;


	enum
	{
		GETTING_PUB_KEYS = 1,
		GETTING_CHALLENGE = 2,
		GETTING_CERTIFICATE = 3
	} mState;

	WONStatus SendPubKeyRequest();
	WONStatus SendLoginRequest();
	WONStatus SendLoginConfirm();

	WONStatus HandlePubKeyReply();
	WONStatus HandleLoginChallenge(ReadBuffer &theMsg);
	WONStatus HandleLoginReply(ReadBuffer &theMsg);

protected:
	~GetCertOp() {}
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void RunHook();
	virtual void CleanupHook();
	virtual void Reset();

public:

	GetCertOp(AuthContext *theContext);
	const AuthPeerData* GetPeerData() const { return mPeerData; }

	// This method should *only* be used for testing. It does not comply with COPPA.
	void SetCreateAccount(bool theVal) { mCreateAccount = theVal; }
	void SetNewPassword(const std::wstring &theNewPassword) { mNewPassword = theNewPassword; }
	void SetAuthContext(AuthContext *theContext) { mAuthContext = theContext; }

	const Blowfish& GetSessionKey() const { return mSessionKey; }
};

typedef SmartPtr<GetCertOp> GetCertOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class LightGetCertOp : public AsyncOp
{
private:
	AuthPeerDataPtr mPeerData;

public:
	void SetPeerData(const AuthPeerData *theData) { mPeerData = theData; }
	const AuthPeerData* GetPeerData() const { return mPeerData; }
};


}; // namespace WONAPI

#endif
