#ifndef __WON_AUTHBASE_H__
#define __WON_AUTHBASE_H__
#include "WONShared.h"

#include <time.h>
#include "WONCommon/ReadBuffer.h"
#include "WONCrypt/ElGamal.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AuthBase : public RefCount
{
private:
	ByteBufferPtr mRawBuf;
	unsigned short mDataLen;
	unsigned short mFamily;
	time_t mIssueTime;   
	time_t mExpireTime;  
	bool mIsValid;

protected:
	virtual ~AuthBase();
	virtual bool UnpackHook(ReadBuffer &theData);
	bool Unpack();

public:
	AuthBase(const void *theData = NULL, unsigned long theDataLen = 0);
	bool IsValid() const { return mIsValid; }

	ByteBufferPtr GetRawBuf() const { return mRawBuf; }

	const void* GetRaw() const { return mRawBuf?mRawBuf->data():NULL; }
	unsigned long GetRawLen() const { return mRawBuf?mRawBuf->length():0; }
	const void* GetData() const { return mRawBuf?mRawBuf->data():NULL; }
	unsigned long GetDataLen() const { return mRawBuf?mDataLen:0; }
	const void* GetSig() const { return mRawBuf?(mRawBuf->data()+mDataLen):0; }
	unsigned long GetSigLen() const { return mRawBuf?(mRawBuf->length()-mDataLen):0; }

	bool Verify(const ElGamal &theKey) const;
	bool IsExpired(time_t theExtraTime=0) const;

	unsigned short GetFamily() const { return mFamily; }
	time_t GetIssueTime() const { return mIssueTime; }
	time_t GetExpireTime() const { return mExpireTime; }
};

typedef ConstSmartPtr<AuthBase> AuthBasePtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
