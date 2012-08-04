#ifndef __WON_AUTHPUBKEYBLOCK_H__
#define __WON_AUTHPUBKEYBLOCK_H__
#include "WONShared.h"

#include "AuthBase.h"
#include <list>


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AuthPubKeyBlock : public AuthBase
{
private:
	typedef std::list<ElGamal> KeyBlock;
	KeyBlock mKeyBlock;
	unsigned short mKeyBlockId;

protected:
	virtual ~AuthPubKeyBlock();
	virtual bool UnpackHook(ReadBuffer &theData);

public:
	AuthPubKeyBlock(const void *theData = NULL, unsigned long theDataLen = 0) : AuthBase(theData, theDataLen) { Unpack(); }
	unsigned short GetId() const { return mKeyBlockId; }
	bool Verify(const AuthBase* theAuthBase) const;
	const ElGamal& GetCurrentKey() const { return mKeyBlock.front(); }
	bool IsValidAndVerified(const ElGamal &theVerifierKey) const;
};

typedef ConstSmartPtr<AuthPubKeyBlock> AuthPubKeyBlockPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


}; // namespace WONAPI

#endif
