#ifndef __WON_CRITICALSECTION_MAC_H__
#define __WON_CRITICALSECTION_MAC_H__
#include "WONShared.h"
#include "Platform_Mac.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CriticalSection
{
// FIX: Mac!
/*
private:
	pthread_mutex_t mCrit;    // The Critical Section

	// Disable Copy and Assignment
	CriticalSection(const CriticalSection&);
	CriticalSection& operator=(const CriticalSection&);

	friend class Event;
*/
public:
	// Constructor / Destructor
	CriticalSection() {}
	~CriticalSection() {}

	void Enter() {}
	void Leave() {} 
};

};  // namespace WONAPI

#endif
