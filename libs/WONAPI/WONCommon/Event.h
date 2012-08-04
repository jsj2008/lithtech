#ifndef __WON_EVENT_H__
#define __WON_EVENT_H__
#include "WONShared.h"

#ifndef __WON_SINGLETHREADED__

#if defined(WIN32)
#include "Event_Windows.h"
#elif defined(_LINUX)
#include "Event_Linux.h"
#elif defined(macintosh) && (macintosh == 1)
#include "Event_Mac.h"
#endif

#else // __WON_SINGLETHREADED__

#include "Platform.h"
namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Event
{
private:
	bool mIsSet;
	bool mIsManualReset;

public:
	explicit Event(bool manualReset = false, bool initiallySet = false) : mIsSet(initiallySet), mIsManualReset(manualReset) { }
	virtual ~Event() { }

	void Set() { mIsSet = true; }
	void Reset() { mIsSet = false; }

	bool WaitFor(DWORD theMilliseconds) 
	{ 
		if (mIsSet)
		{
			if (!mIsManualReset)
				mIsSet = false;

			return true;
		}

		Sleep(theMilliseconds);
		return false;
	}
};

}; // namespace WONAPI



#endif // __WON_SINGLETHREADED__


#endif
