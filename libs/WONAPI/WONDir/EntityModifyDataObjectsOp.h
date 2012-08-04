#ifndef __WON_ENTITYMODIFYDATAOBJECTSOP_H__
#define __WON_ENTITYMODIFYDATAOBJECTSOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "DirTypes.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class EntityModifyDataObjectsOp : public ServerRequestOp
{
private:
	class DataObjectMod
	{
	public:
		DirDataObject mObject;
		bool mIsInsert;
		unsigned short mOffset;

		DataObjectMod() { mIsInsert = false; mOffset = 0; }
		DataObjectMod(const DirDataObject &theObj, bool isInsert, unsigned short theOffset) :
			mObject(theObj), mIsInsert(isInsert), mOffset(theOffset) {}
	};

	typedef std::list<DataObjectMod> DataObjectModList;

protected:
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;
	DataObjectModList mDataObjectMods;

	bool mIsService;

protected:
	void Init();
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();

public:
	EntityModifyDataObjectsOp(ServerContext *theDirContext, bool isService);
	EntityModifyDataObjectsOp(const IPAddr &theAddr, bool isService);

	void SetPath(const std::wstring &thePath) { mPath = thePath; }

	void AddDataObjectMod(const std::string &theType, 
		const ByteBuffer* theData, 
		bool isInsert, 
		unsigned short theOffset) { mDataObjectMods.push_back(DataObjectMod(DirDataObject(theType,theData),isInsert,theOffset)); }
	
	void ClearDataObjectMods() { mDataObjectMods.clear(); }
	
	const std::wstring& GetPath() const { return mPath; }
};

typedef SmartPtr<EntityModifyDataObjectsOp> EntityModifyDataObjectsOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ServiceModifyDataObjectsOp : public EntityModifyDataObjectsOp
{
public:
	ServiceModifyDataObjectsOp(ServerContext *theDirContext) : EntityModifyDataObjectsOp(theDirContext, true) {}
	ServiceModifyDataObjectsOp(const IPAddr &theAddr) : EntityModifyDataObjectsOp(theAddr, true) {}

	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }

	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }
};

typedef SmartPtr<ServiceModifyDataObjectsOp> ServiceModifyDataObjectsOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class DirModifyDataObjectsOp : public EntityModifyDataObjectsOp
{
public:
	DirModifyDataObjectsOp(ServerContext *theDirContext) : EntityModifyDataObjectsOp(theDirContext, false) {}
	DirModifyDataObjectsOp(const IPAddr &theAddr) : EntityModifyDataObjectsOp(theAddr, false) {}
};

typedef SmartPtr<DirModifyDataObjectsOp> DirModifyDataObjectsOpPtr;


};   // namespace WONAPI



#endif
