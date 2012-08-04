#ifndef __WON_SERVERCONTEXT_H__
#define __WON_SERVERCONTEXT_H__
#include "WONShared.h"

#include "WONCommon/SmartPtr.h"
#include "WONCommon/CriticalSection.h"
#include "WONSocket/IPAddr.h"
#include "WONDir/DirEntity.h"

#include <map>


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ServerContext : public RefCount
{
private:
	bool mNeedShuffle;
	bool mHasShuffled;

protected:
	CriticalSection mDataCrit;

	typedef std::map<IPAddr,AddrList::iterator> AddrMap;
	AddrMap mAddrMap;
	AddrList mAddrList;

public:
	ServerContext() : mNeedShuffle(true), mHasShuffled(false) { }
	ServerContext(const IPAddr &theAddr) : mNeedShuffle(true), mHasShuffled(false) { AddAddress(theAddr); }

	void Clear();

	bool AddAddress(const IPAddr &theAddr);
	bool RemoveAddress(const IPAddr &theAddr);
	void RemoveAllAddresses();

	void CopyAddresses(AddrList &theCopy);
	void CopyAddresses(AddrSet &theCopy);
	void NotifyFailed(const IPAddr &theAddr);
	void SetNeedShuffle(bool needShuffle) { mNeedShuffle = needShuffle; if (mNeedShuffle) mHasShuffled = false; }

	bool AddAddressesFromDir(const DirEntityList &theDir, const wchar_t *theNameFilter = NULL);
	bool AddAddressesFromDir(const DirEntityMap &theDir, const std::wstring &thePath, const wchar_t *theNameFilter = NULL);

	bool AtomicClearAndAddAddressesFromDir(const DirEntityList &theDir, const wchar_t *theNameFilter = NULL);
	bool AtomicClearAndAddAddressesFromDir(const DirEntityMap &theDir, const std::wstring &thePath, const wchar_t *theNameFilter = NULL);


	unsigned long GetNumAddresses() { return mAddrMap.size(); }
	bool IsEmpty() { return mAddrMap.empty(); }
};

typedef SmartPtr<ServerContext> ServerContextPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
