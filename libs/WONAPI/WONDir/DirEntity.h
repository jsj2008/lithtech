#ifndef __WON_DIRENTITY_H__
#define __WON_DIRENTITY_H__
#include "WONShared.h"

#include <string>
#include <list>
#include <map>
#include "WONCommon/ByteBuffer.h"
#include "WONSocket/IPAddr.h"
#include "DirTypes.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct DirEntity : public RefCount
{
protected:
	virtual ~DirEntity() {}

public:
	char mType;
	std::wstring mPath;
	std::wstring mName;
	std::wstring mDisplayName;
	time_t mLifespan;
	DWORD mCRC;

	time_t mCreated;
	unsigned long mCreateIP;
	unsigned long mCreateId;

	time_t mTouched;
	unsigned long mTouchIP;
	unsigned long mTouchId;

	bool mReqUnique;  // Unique display names required? (applies only to directories)
	ByteBufferPtr mNetAddr;
	DirDataObjectList mDataObjects;
	DirACLList mACLs;

public:
	DirEntity();
	IPAddr GetNetAddrAsIP() const;
	IPAddr GetCreateIP() const { return IPAddr(htonl(mCreateIP), 0); }
	IPAddr GetTouchIP() const { return IPAddr(htonl(mTouchIP), 0); }

	bool IsDir() const { return mType=='D'; }
	bool IsService() const { return mType=='S'; }

};

typedef ConstSmartPtr<DirEntity> DirEntityPtr;
typedef SmartPtr<DirEntity> NonConstDirEntityPtr;

typedef std::list<DirEntityPtr> DirEntityList;
typedef std::map<std::wstring,DirEntityList> DirEntityMap;


}; // namespace WONAPI

#endif
