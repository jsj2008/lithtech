#ifndef __WON_MODIFYENTITYOP_H__
#define __WON_MODIFYENTITYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ModifyEntityOp : public ServerRequestOp
{
protected:
	DWORD mFlags;
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;

	std::wstring mNewName;
	std::wstring mNewDisplayName;
	ByteBufferPtr mNewNetAddr;
	unsigned long mNewLifespan;

	DirDataObjectList mDataObjects;

	bool mIsService;
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	ModifyEntityOp(ServerContext *theDirContext, bool isService);
	ModifyEntityOp(const IPAddr &theAddr, bool isService);

	// DirUpdateFlags defined in DirTypes.h
	void SetFlags(DWORD theFlags) { mFlags = theFlags; } 
	void SetPath(const std::wstring &thePath) { mPath = thePath; }

	void SetNewName(const std::wstring &theNewName) { mNewName = theNewName; }
	void SetNewDisplayName(const std::wstring &theNewDisplayName) { mNewDisplayName = theNewDisplayName; }
	void SetNewLifespan(unsigned long theLifespan) { mNewLifespan = theLifespan; }
	void AddDataObject(const std::string &theType, const ByteBuffer* theData) { mDataObjects.push_back(DirDataObject(theType,theData)); }
	void ClearDataObjects() { mDataObjects.clear(); }
	
	DWORD GetFlags() const { return mFlags; }
	const std::wstring& GetPath() const { return mPath; }
	const std::wstring& GetNewName() const { return mNewName; }
	const std::wstring& GetNewDisplayName() const { return mNewDisplayName; }
	unsigned long GetNewLifespan() const { return mNewLifespan; }
	const DirDataObjectList& GetDataObjects() const { return mDataObjects; }
};

typedef SmartPtr<ModifyEntityOp> ModifyEntityOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ModifyServiceOp : public ModifyEntityOp
{
public:
	ModifyServiceOp(ServerContext *theDirContext) : ModifyEntityOp(theDirContext, true) {}
	ModifyServiceOp(const IPAddr &theAddr) : ModifyEntityOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }
	void SetNewNetAddr(ByteBufferPtr theNetAddr) { mNewNetAddr = theNetAddr; }
	void SetNewNetAddr(const IPAddr& theNetAddr) { mNewNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }

	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
	const ByteBuffer* GetNewNetAddr() const { return mNewNetAddr; }
};

typedef SmartPtr<ModifyServiceOp> ModifyServiceOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ModifyDirOp : public ModifyEntityOp
{
public:
	ModifyDirOp(ServerContext *theDirContext) : ModifyEntityOp(theDirContext, false) {}
	ModifyDirOp(const IPAddr &theAddr) : ModifyEntityOp(theAddr, false) {}
};

typedef SmartPtr<ModifyDirOp> ModifyDirOpPtr;

};  // namespace WONAPI

#endif
