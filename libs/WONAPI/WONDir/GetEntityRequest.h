///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __WON_GETENTITYREQUEST_H__
#define __WON_GETENTITYREQUEST_H__
#include "WONShared.h"
#include "WONCommon/WriteBuffer.h"
#include "WONStatus.h"
#include "DirEntity.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Data for each request
class GetEntityRequest : public RefCount
{
protected:
	DWORD mFlags;
	std::wstring mPath;

	DirDataTypeSet mDataTypes;

	DirEntityList mDirEntityList;
	DirEntityMap mDirEntityMap;

	WONStatus mStatus;

	// Destructor
	~GetEntityRequest();
public:
	enum PACKFLAG
	{ Pack_GetDirOp, Pack_GetMultiDirOp };

	// Path to the directory you want to retrieve
	void SetPath(const std::wstring &thePath) { mPath = thePath; }
	const std::wstring& GetPath()	 { return mPath; }

	// DirGetFlags defined in DirTypes.h
	void AddFlags   (DWORD theFlags) { mFlags |=  theFlags; }
	void RemoveFlags(DWORD theFlags) { mFlags &=~ theFlags; }
	void SetFlags   (DWORD theFlags) { mFlags  =  theFlags; }
	DWORD GetFlags()				 { return mFlags;		 }

	// Types of DataObjects to be retrieved
	void SetDataTypes(const DirDataTypeSet& theSet) { mDataTypes = theSet; }
	void AddDataType(const std::string &theType) { mDataTypes.insert(theType); }
	void ClearDataTypes() { mDataTypes.clear(); }

	// List of retrieved directory entities
	const DirEntityList& GetDirEntityList() { return mDirEntityList; }
	const DirEntityMap& GetDirEntityMap(); // map of path to entities in that path

	void Reset();
	void Pack(WriteBuffer* aMsg, PACKFLAG thePackType);

	WONStatus GetStatus() { return mStatus; }
	void SetStatus(WONStatus theStatus) { mStatus = theStatus; }
	// Taken from DirEntityReplyParser
public:
	WONStatus ParseMultiEntityReply(const void *theMsg, unsigned long theMsgLen);
	static WONStatus ParseSingleEntityReply(const void *theMsg, unsigned long theMsgLen, DirEntityPtr &theEntity);
	static bool OnlyDirs(DWORD theFlags)		{ 	return !(theFlags & (DIR_GF_DECOMPSERVICES)); }
	static bool OnlyServices(DWORD theFlags)	{	return !(theFlags & (DIR_GF_DECOMPROOT | DIR_GF_DECOMPSUBDIRS)); }

public:
	GetEntityRequest()
	{
		mFlags = DIR_GF_DECOMPROOT | DIR_GF_DECOMPSERVICES | DIR_GF_DECOMPSUBDIRS | DIR_GF_ADDTYPE
				| DIR_GF_ADDDISPLAYNAME | DIR_GF_ADDDATAOBJECTS | DIR_GF_SERVADDNAME | DIR_GF_SERVADDNETADDR 
				| DIR_GF_DIRADDNAME;
	}

};

typedef SmartPtr<GetEntityRequest> GetEntityRequestPtr;
typedef std::list<GetEntityRequestPtr> GetEntityRequestList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
} // namespace WONAPI


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __WON_GETENTITYREQUEST_H__
