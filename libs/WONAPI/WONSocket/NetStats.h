#ifndef __WON_NETSTATS_H__
#define __WON_NETSTATS_H__
#include "WONShared.h"

#include "WONCommon/CriticalSection.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class NetStats
{
protected:
	static CriticalSection mDataCrit;

	static bool mEnabled;

	static int mNumConstructedSockets;
	static int mNumOpenSockets;
	
	static __int64 mNumBytesSent;
	static __int64 mNumBytesReceived;
	static __int64 mNumMessagesSent;
	static __int64 mNumMessagesReceived;

public:
	static void Enable(bool isEnabled) { mEnabled = isEnabled; }

	static void IncrementNumConstructedSockets(int theAmount) { if(!mEnabled) return; mDataCrit.Enter(); mNumConstructedSockets+=theAmount; mDataCrit.Leave(); }
	static void IncrementNumOpenSockets(int theAmount) { if(!mEnabled) return; mDataCrit.Enter(); mNumOpenSockets+=theAmount; mDataCrit.Leave(); }

	static void IncrementBytesSent(int theAmount) { if(!mEnabled) return; mDataCrit.Enter(); mNumBytesSent+=theAmount; mDataCrit.Leave(); }
	static void IncrementBytesReceived(int theAmount) { if(!mEnabled) return; mDataCrit.Enter(); mNumBytesReceived+=theAmount; mDataCrit.Leave(); }
	static void IncrementMessagesSent(int theAmount) { if(!mEnabled) return; mDataCrit.Enter(); mNumMessagesSent+=theAmount; mDataCrit.Leave(); }
	static void IncrementMessagesReceived(int theAmount) { if(!mEnabled) return; mDataCrit.Enter(); mNumMessagesReceived+=theAmount; mDataCrit.Leave(); }

	static int GetNumConstructedSockets() { return mNumConstructedSockets; }
	static int GetNumOpenSockets() { return mNumOpenSockets; }

	static __int64 GetNumBytesSent() { return mNumBytesSent; }
	static __int64 GetNumBytesReceived() { return mNumBytesReceived; }
	static __int64 GetNumMessagesSent() { return mNumMessagesSent; }
	static __int64 GetNumMessagesReceived() { return mNumMessagesReceived; }
};

}; // namespace WONAPI

#endif
