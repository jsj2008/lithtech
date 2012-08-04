#ifndef __WON_FINDENTITYOP_H__
#define __WON_FINDENTITYOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FindEntityOp : public ServerRequestOp
{
protected:
	DWORD mGetFlags;
	DirFindMatchMode mMatchMode;
	unsigned char mFindFlags;

	std::wstring mPath;
	std::wstring mName;
	std::wstring mDisplayName;
	ByteBufferPtr mNetAddr;

	DirDataObjectList mFindDataObjects;
	DirDataTypeSet mGetDataTypes;

	DirEntityPtr mService;
	DirEntityList mDirEntityList;

	bool mIsService;
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

public:
	FindEntityOp(ServerContext *theDirContext, bool isService);
	FindEntityOp(const IPAddr &theAddr, bool isService);

	// DirGetFlags defined in DirTypes.h
	void SetGetFlags(DWORD theFlags) { mGetFlags = theFlags; } 
	void SetMatchMode(DirFindMatchMode theMode) { mMatchMode = theMode; }

	// DirFindFlags defined in DirTypes.h
	void SetFindFlags(unsigned char theFlags) { mFindFlags = theFlags; }

	void SetPath(const std::wstring &thePath) { mPath = thePath; }

	void SetMatchName(const std::wstring &theName) { mName = theName; }
	void SetMatchDisplayName(const std::wstring &theDisplayName) { mDisplayName = theDisplayName; }
	
	void AddFindDataObject(const std::string &theType, const ByteBuffer* theData) { mFindDataObjects.push_back(DirDataObject(theType,theData)); }
	void ClearFindDataObjects() { mFindDataObjects.clear(); }	

	void AddGetDataObject(const std::string &theType) { mGetDataTypes.insert(theType); }
	void ClearGetDataObjects() { mGetDataTypes.clear(); }

	DWORD GetGetFlags() { return mGetFlags; }
	DirFindMatchMode GetMatchMode() { return mMatchMode; }
	unsigned char GetFindFlags() { return mFindFlags; }
	const std::wstring& GetPath() const { return mPath; }
	const std::wstring& GetName() const { return mName; }
	const std::wstring& GetDisplayName() const { return mDisplayName; }

	// Result Methods
	DirEntityPtr GetService() const { return mService; }
	const DirEntityList GetDirEntityList() const { return mDirEntityList; }
};

typedef SmartPtr<FindEntityOp> FindEntityOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FindServiceOp : public FindEntityOp
{
public:
	FindServiceOp(ServerContext *theDirContext) : FindEntityOp(theDirContext, true) {}
	FindServiceOp(const IPAddr &theAddr) : FindEntityOp(theAddr, true) {}

	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }

	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<FindServiceOp> FindServiceOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FindDirOp : public FindEntityOp
{
public:
	FindDirOp(ServerContext *theDirContext) : FindEntityOp(theDirContext, false) {}
	FindDirOp(const IPAddr &theAddr) : FindEntityOp(theAddr, false) {}
};

typedef SmartPtr<FindDirOp> FindDirOpPtr;

};  // namespace WONAPI

#endif
