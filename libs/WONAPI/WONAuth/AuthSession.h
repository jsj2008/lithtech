#ifndef __WON_AUTHSESSION_H__
#define __WON_AUTHSESSION_H__
#include "WONShared.h"

#include "WONStatus.h"
#include "WONCommon/SmartPtr.h"
#include "WONCommon/ReadBuffer.h"
#include "WONCrypt/Blowfish.h"

namespace WONAPI
{



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum AuthType
{
	AUTH_TYPE_NONE = 0,
	AUTH_TYPE_SESSION = 1,
	AUTH_TYPE_PERSISTENT = 2,
	AUTH_TYPE_PERSISTENT_NOCRYPT = 3
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class AuthSession : public RefCount
{
protected:
	AuthType mAuthType;
	unsigned char mLengthFieldSize;

	unsigned short mId;
	unsigned short mInSeq;
	unsigned short mOutSeq;
	Blowfish mKey;

	time_t mLastUseTime;

	virtual ~AuthSession();

public:
	AuthSession(AuthType theType, unsigned short theId, Blowfish theKey, unsigned char theLengthFieldSize);

	WONStatus Encrypt(ByteBufferPtr &theMsg);
	WONStatus Decrypt(ByteBufferPtr &theMsg);

	time_t GetLastUseTime() const { return mLastUseTime; }
	const Blowfish& GetKey() const { return mKey; }
	AuthType GetAuthType() const { return mAuthType; }

	unsigned short GetSessionId() { return mId; }
};

typedef SmartPtr<AuthSession> AuthSessionPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
