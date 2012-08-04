#ifndef __WON_ENTITYSETDATAOBJECTSOP_H__
#define __WON_ENTITYSETDATAOBJECTSOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum DirDataObjectSetMode;

class EntitySetDataObjectsOp : public ServerRequestOp
{
protected:
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;
	DirDataObjectSetMode mDataObjectMode;
	DirDataObjectList mDataObjects;

	bool mIsService;

protected:
	void Init();
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	EntitySetDataObjectsOp(ServerContext *theDirContext, bool isService);
	EntitySetDataObjectsOp(const IPAddr &theAddr, bool isService);

	void SetPath(const std::wstring &thePath) { mPath = thePath; }

	// DataObjectSetMode defined in DirTypes.h
	void SetDataObjectMode(DirDataObjectSetMode theMode) { mDataObjectMode = theMode; } //
	void AddDataObject(const std::string &theType, const ByteBuffer* theData) { mDataObjects.push_back(DirDataObject(theType,theData)); }
	void ClearDataObjects() { mDataObjects.clear(); }
	
	const std::wstring& GetPath() const { return mPath; }
	DirDataObjectSetMode GetDataObjectMode() { return mDataObjectMode; }
	const DirDataObjectList& GetDataObjects() const { return mDataObjects; }
};

typedef SmartPtr<EntitySetDataObjectsOp> EntitySetDataObjectsOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ServiceSetDataObjectsOp : public EntitySetDataObjectsOp
{
public:
	ServiceSetDataObjectsOp(ServerContext *theDirContext) : EntitySetDataObjectsOp(theDirContext, true) {}
	ServiceSetDataObjectsOp(const IPAddr &theAddr) : EntitySetDataObjectsOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }

	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<ServiceSetDataObjectsOp> ServiceSetDataObjectsOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class DirSetDataObjectsOp : public EntitySetDataObjectsOp
{
public:
	DirSetDataObjectsOp(ServerContext *theDirContext) : EntitySetDataObjectsOp(theDirContext, false) {}
	DirSetDataObjectsOp(const IPAddr &theAddr) : EntitySetDataObjectsOp(theAddr, false) {}
};

typedef SmartPtr<DirSetDataObjectsOp> DirSetDataObjectsOpPtr;


};   // namespace WONAPI



#endif
