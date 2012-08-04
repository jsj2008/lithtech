#include "Event_Windows.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using namespace WONAPI;
/*
DWORD Event::WaitForMultipleEvents(DWORD numEvents, Event *theEvents[], bool waitForAll, DWORD dwMilliseconds)
{
	HANDLE h[MAXIMUM_WAIT_OBJECTS];
	for(DWORD i=0; i<numEvents; i++)
		h[i] = theEvents[i]->mEvent;

	DWORD aResult = WaitForMultipleObjects(numEvents,h,waitForAll,dwMilliseconds);
	aResult -= WAIT_OBJECT_0;
	if(aResult>=0 && aResult<numEvents)
		return aResult;
	else 
		return -1;
}*/

