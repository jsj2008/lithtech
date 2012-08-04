#ifndef __WON_GETDIROP_H__
#define __WON_GETDIROP_H__
#include "WONShared.h"

#include <list>
#include "WONServer/ServerRequestOp.h"
#include "DirEntity.h"
#include "GetEntityRequest.h"

namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetDirOp : public ServerRequestOp
{
private:
	GetEntityRequestPtr mGetEntityRequest;

	/*
	DWORD mFlags;
	std::wstring mPath;

	typedef std::set<std::string> DataTypeSet;
	DataTypeSet mDataTypes;
	DirEntityList mDirEntityList;
	DirEntityMap mDirEntityMap;
	*/

	//bool mStoreInMap;
	
	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

public:
	GetDirOp(ServerContext *theDirContext);
	GetDirOp(const IPAddr &theAddr);

	// Path to the directory you want to retrieve
	void SetPath(const std::wstring &thePath) { mGetEntityRequest->SetPath(thePath); }
	const std::wstring& GetPath()	 { return mGetEntityRequest->GetPath();      }

	// DirGetFlags defined in DirTypes.h
	void AddFlags   (DWORD theFlags) { mGetEntityRequest->AddFlags   (theFlags); }
	void RemoveFlags(DWORD theFlags) { mGetEntityRequest->RemoveFlags(theFlags); }
	void SetFlags   (DWORD theFlags) { mGetEntityRequest->SetFlags   (theFlags); }
	DWORD GetFlags() { return mGetEntityRequest->GetFlags(); }

	// Types of DataObjects to be retrieved
	void AddDataType(const std::string &theType) { mGetEntityRequest->AddDataType(theType); }
	void ClearDataTypes() { mGetEntityRequest->ClearDataTypes(); }

	// List of retrieved directory entities
	const DirEntityList& GetDirEntityList() { return mGetEntityRequest->GetDirEntityList(); }
	const DirEntityMap&  GetDirEntityMap()  { return mGetEntityRequest->GetDirEntityMap();  } // map of path to entities in that path
};

typedef SmartPtr<GetDirOp> GetDirOpPtr;



}; // namespace WONAPI

#endif
