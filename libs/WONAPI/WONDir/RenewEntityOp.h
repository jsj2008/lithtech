#ifndef __WON_RENEWENTITYOP_H__
#define __WON_RENEWENTITYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RenewEntityOp : public ServerRequestOp
{
protected:
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;
	unsigned long mLifespan;

	bool mIsService;
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	RenewEntityOp(ServerContext *theDirContext, bool isService);
	RenewEntityOp(const IPAddr &theAddr, bool isService);

	void SetPath(const std::wstring &thePath) { mPath = thePath; }	
	void SetLifespan(unsigned long theLifespan) { mLifespan = theLifespan; }

	const std::wstring& GetPath() const { return mPath; }
	unsigned long GetLifespan() { return mLifespan; }
};

typedef SmartPtr<RenewEntityOp> RenewEntityOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RenewServiceOp : public RenewEntityOp
{
public:
	RenewServiceOp(ServerContext *theDirContext) : RenewEntityOp(theDirContext, true) {}
	RenewServiceOp(const IPAddr &theAddr) : RenewEntityOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }


	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<RenewServiceOp> RenewServiceOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RenewDirOp : public RenewEntityOp
{
public:
	RenewDirOp(ServerContext *theDirContext) : RenewEntityOp(theDirContext, false) {}
	RenewDirOp(const IPAddr &theAddr) : RenewEntityOp(theAddr, false) {}
};

typedef SmartPtr<RenewDirOp> RenewDirOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; 	// namespace WONAPI

#endif
