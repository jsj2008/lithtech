#ifndef __WON_EVENT_WINDOWS_H__
#define __WON_EVENT_WINDOWS_H__
#include "WONShared.h"
#include "Platform_Windows.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Event
{
private:
	HANDLE mEvent;

public:
	explicit Event(bool manualReset = false, bool initiallySet = false) { mEvent = CreateEvent(NULL,manualReset,initiallySet,NULL); }
	virtual ~Event() { CloseHandle(mEvent); }

	void Set() { SetEvent(mEvent); }
	void Reset() { ResetEvent(mEvent); }

	bool WaitFor(DWORD theMilliseconds) { return WaitForSingleObjectEx(mEvent,theMilliseconds,TRUE)==WAIT_OBJECT_0; }

public:
	HANDLE GetHandle() { return mEvent; } // non-portable

//	static DWORD WaitForMultipleEvents(DWORD numEvents, Event *theEvents[], bool waitForAll, DWORD dwMilliseconds);
};


}; // namespace WONAPI

#endif
