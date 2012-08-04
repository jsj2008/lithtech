#ifndef __WON_GETVERSIONOP_H__
#define __WON_GETVERSIONOP_H__
#include "WONShared.h"

#include "WONCommon/AsyncOpTracker.h"
#include "WONCommon/ByteBuffer.h"
#include "WONServer/ServerContext.h"
#include "WONDir/GetDirOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::set<std::string> VersionSet;
typedef std::map<std::string,std::string> VersionDescriptionMap;
enum VersionResult
{
	Version_Invalid,
	Version_Valid_NotLatest,
	Version_Valid_Latest
};	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class VersionBaseOp : public AsyncOpWithTracker
{
public:
	typedef int(*VersionCompare)(const std::string &s1, const std::string &s2);

protected:
	std::string mProduct;
	std::string mExtraConfig;
	ServerContextPtr mDirServers;

	typedef std::set<std::string> FilterStringSet;
	FilterStringSet mFilterStringSet;
	

	VersionSet mValidVersionSet;
	VersionSet mFilteredVersionSet;
	std::string mHighestValidVersion;
	VersionCompare mVersionCompare;

	VersionDescriptionMap mVersionDescriptionMap;
	VersionDescriptionMap mFilteredVersionDescriptionMap;

	std::wstring GetVersionDir();
	void BuildValidVersionSet(const DirEntity *theEntity);
	static int DefaultVersionCompare(const std::string &s1, const std::string &s2) { return s1.compare(s2); }
	void BuildVersionFilterSet(const std::string &theVersionString, FilterStringSet &theVersionFilterSet);

public:
	VersionBaseOp() { mVersionCompare = DefaultVersionCompare; }

	void SetDirServers(ServerContext *theDirServers) { mDirServers = theDirServers; }
	void SetProduct(const std::string &theProduct) { mProduct = theProduct; }
	void SetExtraConfig(const std::string &theExtraConfig) { mExtraConfig = theExtraConfig; }

	const std::string& GetProduct() { return mProduct; }
	const std::string& GetExtraConfig() { return mExtraConfig; }
	ServerContext* GetDirServers() { return mDirServers; }
	
	void SetValidVersionSet(const VersionSet &theSet) { mValidVersionSet = theSet; }
	void SetFiltereVersionSet(const VersionSet &theSet) { mFilteredVersionSet = theSet; }
	const VersionSet& GetValidVersionSet() { return mValidVersionSet; }
	const VersionSet& GetFilteredVersionSet() { return mFilteredVersionSet; }
	const std::string& GetHighestValidVersion() { return mHighestValidVersion; }

	const VersionDescriptionMap& GetVersionDescriptionMap() { return mVersionDescriptionMap; }

	void SetVersionCompare(VersionCompare theCompareFunc) { mVersionCompare = theCompareFunc; }

	void SetFilterString(const std::string &theFilterString);
	bool IsFiltered(const std::string &theVersion, const std::string &theVersionData);
	VersionResult CheckVersion(const std::string &theVersionId);
	bool VersionIsLatest(const std::string &theVersionId);
	bool VersionIsValid(const std::string &theVersionId);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetVersionOp : public VersionBaseOp
{
protected:
	void DoFinish(GetDirOp *theOp);

	virtual bool CallbackHook(AsyncOp *theOp, int theId);
	virtual void RunHook();

public:
	GetVersionOp(ServerContext *theDirServers, const std::string &theProduct);

};

typedef SmartPtr<GetVersionOp> GetVersionOpPtr;

}; // namespace WONAPI

#endif

