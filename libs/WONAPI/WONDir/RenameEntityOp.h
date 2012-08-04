#ifndef __WON_RENAMEENTITYOP_H__
#define __WON_RENAMEENTITYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RenameEntityOp : public ServerRequestOp
{
protected:
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;

	std::wstring mNewDisplayName;
	bool mIsUnique;

	bool mIsService;
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	RenameEntityOp(ServerContext *theDirContext, bool isService);
	RenameEntityOp(const IPAddr &theAddr, bool isService);

	void SetPath(const std::wstring &thePath) { mPath = thePath; }	
	void SetNewDisplayName(const std::wstring &theName) { mNewDisplayName = theName; }
	void SetUnique(bool requireUniqueDisplayName) { mIsUnique = requireUniqueDisplayName; }	


	const std::wstring& GetPath() const { return mPath; }
	const std::wstring& GetNewDisplayName() const { return mNewDisplayName; }
	bool GetUnique() const { return mIsUnique; }
};

typedef SmartPtr<RenameEntityOp> RenameEntityOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RenameServiceOp : public RenameEntityOp
{
public:
	RenameServiceOp(ServerContext *theDirContext) : RenameEntityOp(theDirContext, true) {}
	RenameServiceOp(const IPAddr &theAddr) : RenameEntityOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }


	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<RenameServiceOp> RenameServiceOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RenameDirOp : public RenameEntityOp
{
public:
	RenameDirOp(ServerContext *theDirContext) : RenameEntityOp(theDirContext, false) {}
	RenameDirOp(const IPAddr &theAddr) : RenameEntityOp(theAddr, false) {}
};

typedef SmartPtr<RenameDirOp> RenameDirOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; 	// namespace WONAPI

#endif
