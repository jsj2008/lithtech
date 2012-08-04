#include "NetStats.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CriticalSection NetStats::mDataCrit;

bool NetStats::mEnabled = false;

int NetStats::mNumConstructedSockets = 0;
int NetStats::mNumOpenSockets = 0;

__int64 NetStats::mNumBytesSent = 0;
__int64 NetStats::mNumBytesReceived = 0;
__int64 NetStats::mNumMessagesSent = 0;
__int64 NetStats::mNumMessagesReceived = 0;

