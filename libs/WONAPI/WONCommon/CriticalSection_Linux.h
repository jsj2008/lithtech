#ifndef __WON_CRITICALSECTION_LINUX_H__
#define __WON_CRITICALSECTION_LINUX_H__
#include "WONShared.h"
#include "Platform_Linux.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CriticalSection
{
private:
	pthread_mutex_t mCrit;    // The Critical Section

	// Disable Copy and Assignment
	CriticalSection(const CriticalSection&);
	CriticalSection& operator=(const CriticalSection&);

	friend class Event;

public:
	// Constructor / Destructor
	CriticalSection() { pthread_mutex_t tmpMutex = {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, {0, 0}}; mCrit = tmpMutex; }
	~CriticalSection() { pthread_mutex_destroy(&mCrit); }

	void Enter() { pthread_mutex_lock(&mCrit); }
	void Leave() { pthread_mutex_unlock(&mCrit); }
};

};  // namespace WONAPI

#endif
