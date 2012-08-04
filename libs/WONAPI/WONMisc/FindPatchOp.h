#ifndef __WON_FINDPATCHOP_H__
#define __WON_FINDPATCHOP_H__
#include "WONShared.h"

#include "WONCommon/AsyncOpTracker.h"
#include "GetVersionOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::map<std::string,ByteBufferPtr> PatchDataMap;
	
struct PatchInfo
{
	std::string mFromVersion;
	std::string mToVersion;
	std::wstring mName;
	unsigned long mChecksum;
	unsigned long mSize;
	ByteBufferPtr mPatchLocation;

	PatchDataMap mData;

	PatchInfo() : mChecksum(0), mSize(0) {}
};

typedef std::list<PatchInfo> PatchList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FindPatchOp : public VersionBaseOp
{
private:
	PatchList mPatchList;
	bool ValidFromVersion(const std::string &theFromVersion, const PatchInfo &theInfo);

protected:
	void DoFinish(GetDirOp *theOp);

	virtual bool CallbackHook(AsyncOp *theOp, int theId);
	virtual void RunHook();

public:
	FindPatchOp(ServerContext *theDirServers, const std::string &theProduct);
	FindPatchOp(GetVersionOp *theVersionOp);

	const PatchList& GetPatchList() { return mPatchList; } // all available patches
	void GetPatchList(const std::string &theFromVersion, PatchList &aList); // all patches from this version
	void GetValidPatchList(const std::string &theFromVersion, PatchList &aList); // all patches from this version to a valid version
	void GetHighestPatchList(const std::string &theFromVersion, PatchList &aList); // all patches to the highest version available from this version
	void GetHighestPatchList(const std::string &theFromVersion, const std::string &theToVersion, PatchList &aList); // all patches to the highest version <= theToVersion available from this version
};

typedef SmartPtr<FindPatchOp> FindPatchOpPtr;

}; // namespace WONAPI

#endif
