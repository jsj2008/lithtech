#ifndef __WON_CRITICALSECTION_H__
#define __WON_CRITICALSECTION_H__
#include "WONShared.h"

#ifndef __WON_SINGLETHREADED__

#if defined(WIN32)
#include "CriticalSection_Windows.h"
#elif defined(_LINUX)
#include "CriticalSection_Linux.h"
#elif defined(macintosh) && (macintosh == 1)
#include "CriticalSection_Mac.h"
#endif

#else  // __WON_SINGLETHREADED__

namespace WONAPI
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CriticalSection
{
private:
	// Disable Copy and Assignment
	CriticalSection(const CriticalSection&);
	CriticalSection& operator=(const CriticalSection&);

public:
	// Constructor / Destructor
	CriticalSection() { }
	~CriticalSection() { }

	void Enter() { }
	void Leave() { }
};

}; // namespace WONAPI

#endif  // __WON_SINGLETHREADED__


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AutoCrit
{
private:
	CriticalSection& mCritR;    // The CriticalSection
	unsigned int     mEnterCt;  // How many times entered?

	// Disable default ctor, copy, and assignment
	AutoCrit();
	AutoCrit(const AutoCrit&);
	AutoCrit& operator=(const AutoCrit&);

public:
	// Constructor / Destructor
	explicit AutoCrit(CriticalSection& theCrit, bool enterNow = true) : mCritR(theCrit), mEnterCt(0) { if (enterNow) Enter(); }
	~AutoCrit() { while (mEnterCt > 0) { mCritR.Leave();  mEnterCt--; } }

	void Enter() { mCritR.Enter();  mEnterCt++; }
	void Leave() { if (mEnterCt > 0) { mCritR.Leave();  mEnterCt--; } }
};


};  // namespace WONAPI

#endif
