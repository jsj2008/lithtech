#ifndef __WON_CRITICALSECTION_WINDOWS_H__
#define __WON_CRITICALSECTION_WINDOWS_H__
#include "WONShared.h"
#include "Platform_Windows.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CriticalSection
{
private:
	CRITICAL_SECTION mCrit;    // The Critical Section

	// Disable Copy and Assignment
	CriticalSection(const CriticalSection&);
	CriticalSection& operator=(const CriticalSection&);

	friend class Event;

public:
	// Constructor / Destructor
	CriticalSection() { InitializeCriticalSection(&mCrit); }
	~CriticalSection() { DeleteCriticalSection(&mCrit); }

	void Enter() { EnterCriticalSection(&mCrit); }
	void Leave() { LeaveCriticalSection(&mCrit); }
};

};  // namespace WONAPI

#endif
