#include "ServerContext.h"
#include <time.h>

#include <algorithm>
using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerContext::Clear()
{
	AutoCrit aCrit(mDataCrit);
	mAddrMap.clear();
	mAddrList.clear();
	mNeedShuffle = true;
	mHasShuffled = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerContext::AddAddress(const IPAddr &theAddr)
{
	if(!theAddr.IsValid())
		return false;

	AutoCrit aCrit(mDataCrit);
	std::pair<AddrMap::iterator,bool> aRet;
	aRet = mAddrMap.insert(AddrMap::value_type(theAddr,mAddrList.end()));
	if(aRet.second)
	{
		aRet.first->second = mAddrList.insert(mAddrList.end(),theAddr);
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerContext::RemoveAddress(const IPAddr &theAddr)
{
	AutoCrit aCrit(mDataCrit);
	AddrMap::iterator anItr = mAddrMap.find(theAddr);
	if(anItr==mAddrMap.end())
		return false;

	mAddrList.erase(anItr->second);
	mAddrMap.erase(anItr);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ServerContext::CopyAddresses(AddrList &theCopy)
{
	AutoCrit aCrit(mDataCrit);
	if(mNeedShuffle && !mHasShuffled)
	{
		vector<IPAddr> aVec;
		vector<IPAddr>::iterator aVecItr;
		aVec.reserve(mAddrList.size());

		AddrList::iterator aListItr = mAddrList.begin();
		while(aListItr!=mAddrList.end())
		{
			aVec.push_back(*aListItr);
			++aListItr;
		}

		srand(time(NULL));
		random_shuffle(aVec.begin(), aVec.end());
		mAddrMap.clear();
		mAddrList.clear();
		

		aVecItr = aVec.begin();
		while(aVecItr!=aVec.end())
		{
			mAddrMap[*aVecItr] = mAddrList.insert(mAddrList.end(),*aVecItr);
			++aVecItr;
		}

		mHasShuffled = true;
	}

	theCopy = mAddrList;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerContext::CopyAddresses(AddrSet &theCopy)
{
	AutoCrit aCrit(mDataCrit);
	theCopy.clear();
	AddrMap::iterator aMapItr = mAddrMap.begin();
	AddrSet::iterator aSetItr = theCopy.begin();
	while(aMapItr!=mAddrMap.end())
	{
		aSetItr = theCopy.insert(aSetItr,aMapItr->first);
		++aMapItr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerContext::NotifyFailed(const IPAddr &theAddr)
{
	AutoCrit aCrit(mDataCrit);
	AddrMap::iterator anItr = mAddrMap.find(theAddr);
	if(anItr==mAddrMap.end())
		return;

	mAddrList.erase(anItr->second);
	anItr->second = mAddrList.insert(mAddrList.end(), anItr->first);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerContext::AddAddressesFromDir(const DirEntityList &theDir, const wchar_t *theNameFilter)
{
	bool added = false;

	wstring aNameFilter;
	if(theNameFilter!=NULL)
		aNameFilter = theNameFilter;

	DirEntityList::const_iterator anItr = theDir.begin();
	while(anItr!=theDir.end())
	{
		const DirEntity *anEntity = *anItr;
		if(!anEntity->IsDir())
		{
			if(aNameFilter.empty() || aNameFilter==anEntity->mName)
			{
				if(AddAddress(anEntity->GetNetAddrAsIP()))
					added = true;
			}
		}

		++anItr;
	}

	return added;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerContext::AddAddressesFromDir(const DirEntityMap &theDir, const std::wstring &thePath, const wchar_t *theNameFilter)
{
	DirEntityMap::const_iterator anItr = theDir.find(thePath);
	if(anItr==theDir.end())
		return false;
	else
		return AddAddressesFromDir(anItr->second,theNameFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerContext::AtomicClearAndAddAddressesFromDir(const DirEntityList &theDir, const wchar_t *theNameFilter)
{
	AutoCrit aCrit(mDataCrit);
	Clear();
	return AddAddressesFromDir(theDir, theNameFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ServerContext::AtomicClearAndAddAddressesFromDir(const DirEntityMap &theDir, const std::wstring &thePath, const wchar_t *theNameFilter)
{
	AutoCrit aCrit(mDataCrit);
	Clear();
	return AddAddressesFromDir(theDir, thePath, theNameFilter);	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ServerContext::RemoveAllAddresses()
{
	Clear();
}
