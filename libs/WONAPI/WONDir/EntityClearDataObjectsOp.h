#ifndef __WON_ENTITYCLEARDATAOBJECTSOP_H__
#define __WON_ENTITYCLEARDATAOBJECTSOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum DirDataObjectSetMode;

class EntityClearDataObjectsOp : public ServerRequestOp
{
protected:
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;

	DirDataTypeSet mDataTypes;

	bool mIsService;

protected:
	void Init();
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	EntityClearDataObjectsOp(ServerContext *theDirContext, bool isService);
	EntityClearDataObjectsOp(const IPAddr &theAddr, bool isService);

	void SetPath(const std::wstring &thePath) { mPath = thePath; }
	void AddDataType(const std::string &theType) { mDataTypes.insert(theType); }
	void ClearDataTypes() { mDataTypes.clear(); }
	
	const std::wstring& GetPath() const { return mPath; }
};

typedef SmartPtr<EntityClearDataObjectsOp> EntityClearDataObjectsOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ServiceClearDataObjectsOp : public EntityClearDataObjectsOp
{
public:
	ServiceClearDataObjectsOp(ServerContext *theDirContext) : EntityClearDataObjectsOp(theDirContext, true) {}
	ServiceClearDataObjectsOp(const IPAddr &theAddr) : EntityClearDataObjectsOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }

	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<ServiceClearDataObjectsOp> ServiceClearDataObjectsOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class DirClearDataObjectsOp : public EntityClearDataObjectsOp
{
public:
	DirClearDataObjectsOp(ServerContext *theDirContext) : EntityClearDataObjectsOp(theDirContext, false) {}
	DirClearDataObjectsOp(const IPAddr &theAddr) : EntityClearDataObjectsOp(theAddr, false) {}
};

typedef SmartPtr<DirClearDataObjectsOp> DirClearDataObjectsOpPtr;

}; // namespace WONAPI

#endif
