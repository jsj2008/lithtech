#include "FindPatchOp.h"
#include "WONCommon/StringUtil.h"
#include "WONCommon/LittleEndian.h"

using namespace WONAPI;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FindPatchOp::FindPatchOp(ServerContext *theDirServers, const std::string &theProduct)
{
	mDirServers = theDirServers;
	mProduct = theProduct;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FindPatchOp::FindPatchOp(GetVersionOp *theVersionOp)
{
	mDirServers = theVersionOp->GetDirServers();
	mProduct = theVersionOp->GetProduct();
	mExtraConfig = theVersionOp->GetExtraConfig();
	mValidVersionSet = theVersionOp->GetValidVersionSet();
	mVersionDescriptionMap = theVersionOp->GetVersionDescriptionMap();
	mFilteredVersionSet = theVersionOp->GetFilteredVersionSet();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FindPatchOp::ValidFromVersion(const std::string &theFromVersion, const PatchInfo &theInfo)
{
	if(theInfo.mFromVersion==theFromVersion)
		return true;
	else if(theInfo.mFromVersion.empty() && mVersionCompare(theInfo.mToVersion,theFromVersion)>0)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindPatchOp::GetPatchList(const std::string &theFromVersion, PatchList &aList)
{
	aList.clear();

	PatchList::iterator anItr = mPatchList.begin();
	while(anItr!=mPatchList.end())
	{
		const PatchInfo &anInfo = *anItr;
		if(ValidFromVersion(theFromVersion,anInfo))
			aList.push_back(anInfo);

		++anItr;
	}
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindPatchOp::GetValidPatchList(const std::string &theFromVersion, PatchList &aList)
{
	aList.clear();

	PatchList::iterator anItr = mPatchList.begin();
	while(anItr!=mPatchList.end())
	{
		const PatchInfo &anInfo = *anItr;
		if(ValidFromVersion(theFromVersion,anInfo) && mValidVersionSet.find(anInfo.mToVersion)!=mValidVersionSet.end())
			aList.push_back(anInfo);

		++anItr;
	}
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindPatchOp::GetHighestPatchList(const std::string &theFromVersion, const std::string &theToVersion, PatchList &aList) // all patches to the highest version <= theToVersion available from this version
{
	aList.clear();

	PatchList::iterator aPatchItr;

	std::string aHighestVersion;
	bool highestVersionHasFromVersion = false;

	for(aPatchItr = mPatchList.begin(); aPatchItr!=mPatchList.end(); ++aPatchItr)
	{
		const PatchInfo &anInfo = *aPatchItr;
		if(!ValidFromVersion(theFromVersion,anInfo))
			continue;

		if(mVersionCompare(anInfo.mToVersion, theToVersion)<=0) // only want versions <= theToVersion
			continue;

		// Don't show filtered versions
		if(mFilteredVersionSet.find(anInfo.mToVersion) != mFilteredVersionSet.end())
			continue;

		if(aHighestVersion.empty())
		{
			aHighestVersion = anInfo.mToVersion;
			highestVersionHasFromVersion = !anInfo.mFromVersion.empty();
		}

		int aComp = mVersionCompare(anInfo.mToVersion, aHighestVersion);
		if(aComp<0) // ToVersion less than highest version found
			continue;
		else if(aComp>0) // ToVersion greater than highest version found
		{
			aHighestVersion = anInfo.mToVersion;
			highestVersionHasFromVersion = !anInfo.mFromVersion.empty();
			aList.clear();
			aList.push_back(anInfo);
		}
		else // aComp = 0
		{
			if(!highestVersionHasFromVersion) // highest current patch is non-specific
			{
				if(!anInfo.mFromVersion.empty()) // specific patch for this version to high version
				{
					highestVersionHasFromVersion = true;
					aList.clear();
					aList.push_back(anInfo);
				}
				else
					aList.push_back(anInfo); // still no from version
			}
			else  // highest patch is specific --> only allow specific patches (no empty from versions)
			{
				if(!anInfo.mFromVersion.empty())
					aList.push_back(anInfo);
			}
		}
	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindPatchOp::GetHighestPatchList(const std::string &theFromVersion, PatchList &aList)
{
	GetHighestPatchList(theFromVersion,GetHighestValidVersion(),aList);
/*	aList.clear();

	PatchList::iterator aPatchItr;

	std::string aHighestVersion;
	bool highestVersionHasFromVersion = false;

	for(aPatchItr = mPatchList.begin(); aPatchItr!=mPatchList.end(); ++aPatchItr)
	{
		const PatchInfo &anInfo = *aPatchItr;
		if(!ValidFromVersion(theFromVersion,anInfo))
			continue;

		if(aHighestVersion.empty())
		{
			aHighestVersion = anInfo.mToVersion;
			highestVersionHasFromVersion = !anInfo.mFromVersion.empty();
		}

		int aComp = mVersionCompare(anInfo.mToVersion, aHighestVersion);
		if(aComp<0) // ToVersion less than highest version found
			continue;
		else if(aComp>0) // ToVersion greater than highest version found
		{
			aHighestVersion = anInfo.mToVersion;
			highestVersionHasFromVersion = !anInfo.mFromVersion.empty();
			aList.clear();
			aList.push_back(anInfo);
		}
		else // aComp = 0
		{
			if(!highestVersionHasFromVersion) // highest current patch is non-specific
			{
				if(!anInfo.mFromVersion.empty()) // specific patch for this version to high version
				{
					highestVersionHasFromVersion = true;
					aList.clear();
					aList.push_back(anInfo);
				}
				else
					aList.push_back(anInfo); // still no from version
			}
			else  // highest patch is specific --> only allow specific patches (no empty from versions)
			{
				if(!anInfo.mFromVersion.empty())
					aList.push_back(anInfo);
			}
		}
	}*/
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindPatchOp::DoFinish(GetDirOp *theOp)
{
	if(!theOp->Succeeded())
	{
		Finish(theOp->GetStatus());
		return;
	}

	const DirEntityList &aList = theOp->GetDirEntityList();
	DirEntityList::const_iterator anItr = aList.begin();
	while(anItr!=aList.end())
	{
		const DirEntity *anEntity = *anItr;
		if(anEntity->IsDir())
			BuildValidVersionSet(anEntity);
		else
		{
			PatchInfo anInfo;
			anInfo.mPatchLocation = anEntity->mNetAddr;

			const std::string &aPatchId = WStringToString(anEntity->mName);
			int aPos = aPatchId.find(L'~');
			if(aPos==wstring::npos)
				anInfo.mToVersion = aPatchId;
			else
			{
				anInfo.mFromVersion = aPatchId.substr(0,aPos);
				anInfo.mToVersion = aPatchId.substr(aPos+1);
			}

			anInfo.mName = anEntity->mDisplayName;
			DirDataObjectList::const_iterator anItr = anEntity->mDataObjects.begin();
			while(anItr!=anEntity->mDataObjects.end())
			{
				DirDataObject anObject = *anItr;
				if(anObject.mDataType=="_cs")
				{
					if(anObject.mData->length()==4)
						anInfo.mChecksum = LongFromLittleEndian(*(unsigned long*)anObject.mData->data());
				}
				else if(anObject.mDataType=="_ps")
				{
					if(anObject.mData->length()==4)
						anInfo.mSize = LongFromLittleEndian(*(unsigned long*)anObject.mData->data());
				}
				else
					anInfo.mData[anObject.mDataType] = anObject.mData;

				++anItr;
			}

			mPatchList.push_back(anInfo);
		}

		++anItr;
	}

	Finish(WS_Success);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FindPatchOp::CallbackHook(AsyncOp *theOp, int)
{
	DoFinish((GetDirOp*)theOp);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FindPatchOp::RunHook()
{
	mPatchList.clear();

	GetDirOpPtr anOp = new GetDirOp(mDirServers);

	anOp->SetPath(GetVersionDir());
	DWORD aGetFlags = DIR_GF_DECOMPSERVICES | DIR_GF_SERVADDNAME | DIR_GF_ADDDISPLAYNAME | DIR_GF_SERVADDNETADDR 
		| DIR_GF_ADDDATAOBJECTS | DIR_GF_ADDDOTYPE | DIR_GF_ADDDODATA;

	if(mValidVersionSet.empty())
		aGetFlags |= DIR_GF_DECOMPROOT;

	anOp->SetFlags(aGetFlags);

	if(IsAsync())
	{
		Track(anOp);
		anOp->RunAsync(OP_TIMEOUT_INFINITE);
		return;
	}

	anOp->RunBlock(TimeLeft());
	DoFinish(anOp);
}

