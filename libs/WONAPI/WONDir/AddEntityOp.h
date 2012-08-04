#ifndef __WON_ADDENTITYOP_H__
#define __WON_ADDENTITYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AddEntityOp : public ServerRequestOp
{
protected:
	DWORD mFlags;
	std::wstring mPath;
	std::wstring mName;
	std::wstring mDisplayName;
	ByteBufferPtr mNetAddr;

	unsigned long mLifespan;
	DirDataObjectList mDataObjects;

	bool mIsService;
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	AddEntityOp(ServerContext *theDirContext, bool isService);
	AddEntityOp(const IPAddr &theAddr, bool isService);

	// DirUpdateFlags defined in DirTypes.h
	void SetFlags(DWORD theFlags) { mFlags = theFlags; } 	
	void SetPath(const std::wstring &thePath) { mPath = thePath; } 
	void SetName(const std::wstring &theName) { mName = theName; }
	void SetDisplayName(const std::wstring &theDisplayName) { mDisplayName = theDisplayName; }
	void SetLifespan(unsigned long theLifespan) { mLifespan = theLifespan; }
	void AddDataObject(const std::string &theType, const ByteBuffer* theData) { mDataObjects.push_back(DirDataObject(theType,theData)); }
	void ClearDataObjects() { mDataObjects.clear(); }
	
	DWORD GetFlags() const { return mFlags; }
	const std::wstring& GetPath() const { return mPath; }
	const std::wstring& GetName() const { return mName; }
	const std::wstring& GetDisplayName() const { return mDisplayName; }
	const DirDataObjectList& GetDataObjects() const { return mDataObjects; }
};

typedef SmartPtr<AddEntityOp> AddEntityOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AddServiceOp : public AddEntityOp
{
public:
	AddServiceOp(ServerContext *theDirContext) : AddEntityOp(theDirContext, true) {}
	AddServiceOp(const IPAddr &theAddr) : AddEntityOp(theAddr, true) {}

	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }
	void SetNetAddrPort(unsigned short thePort);

	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<AddServiceOp> AddServiceOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AddDirOp : public AddEntityOp
{
public:
	AddDirOp(ServerContext *theDirContext) : AddEntityOp(theDirContext, false) {}
	AddDirOp(const IPAddr &theAddr) : AddEntityOp(theAddr, false) {}
};

typedef SmartPtr<AddDirOp> AddDirOpPtr;


};  // namespace WONAPI

#endif
