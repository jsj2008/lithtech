#ifndef __WON_EVENT_LINUX_H__
#define __WON_EVENT_LINUX_H__
#include "WONShared.h"
#include "CriticalSection.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Event
{
private:
	CriticalSection mCrit;
	pthread_cond_t mCond;
	bool mIsSet;
	bool mIsManualReset;
	
	Event* refersTo;

public:
	explicit Event(bool manualReset = false, bool initiallySet = false); 	
	~Event() { pthread_cond_destroy(&mCond); } 

	void Set();
	void Reset();

	bool WaitFor(DWORD timeout);
};

}; // namespace WONAPI

#endif
