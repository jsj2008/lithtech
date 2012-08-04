#ifndef __WON_CRYPTTRANSFORM_H__
#define __WON_CRYPTTRANSFORM_H__
#include "WONShared.h"

#include "WONSocket/AsyncSocket.h"
#include "AuthSession.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CryptTransform : public MsgTransform
{
private:
	AuthSessionPtr mSession;

protected:
	virtual ~CryptTransform() { }

public:	
	CryptTransform(AuthSession* theSession) : mSession(theSession) { }

	virtual WONStatus Recv(ByteBufferPtr &theMsg) {	return mSession->Decrypt(theMsg); }
	virtual WONStatus Send(ByteBufferPtr &theMsg) { 	return mSession->Encrypt(theMsg); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
