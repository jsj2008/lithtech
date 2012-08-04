#ifndef __WON_ASYNCOPCONTAINER_H__
#define __WON_ASYNCOPCONTAINER_H__
#include "WONShared.h"

#include "AsyncOp.h"
#include <set>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AsyncOpContainer
{
private:
	CriticalSection mDataCrit;

	Event mSignal;

	typedef std::list<AsyncOpPtr> OpQueue;

	OpQueue mOpQueue;
	AsyncOpPtr mCurPumpOp;
	bool mPurging;
		
public:
	AsyncOpContainer();
	virtual ~AsyncOpContainer();

	void QueueOp(AsyncOp* theOp);
	bool WaitForOps(DWORD theTime);
	void Pump(DWORD theWaitTime, int theMaxOps = 0);

	void PurgeOps(); // kills all ops
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
