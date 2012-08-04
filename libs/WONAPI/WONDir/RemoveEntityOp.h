#ifndef __WON_REMOVEENTITYOP_H__
#define __WON_REMOVEENTITYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RemoveEntityOp : public ServerRequestOp
{
protected:
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;

	bool mIsService;
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	RemoveEntityOp(ServerContext *theDirContext, bool isService);
	RemoveEntityOp(const IPAddr &theAddr, bool isService);

	void SetPath(const std::wstring &thePath) { mPath = thePath; }	
	const std::wstring& GetPath() const { return mPath; }
};

typedef SmartPtr<RemoveEntityOp> RemoveEntityOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RemoveServiceOp : public RemoveEntityOp
{
public:
	RemoveServiceOp(ServerContext *theDirContext) : RemoveEntityOp(theDirContext, true) {}
	RemoveServiceOp(const IPAddr &theAddr) : RemoveEntityOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }


	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<RemoveServiceOp> RemoveServiceOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RemoveDirOp : public RemoveEntityOp
{
public:
	RemoveDirOp(ServerContext *theDirContext) : RemoveEntityOp(theDirContext, false) {}
	RemoveDirOp(const IPAddr &theAddr) : RemoveEntityOp(theAddr, false) {}
};

typedef SmartPtr<RemoveDirOp> RemoveDirOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; 	// namespace WONAPI

#endif
