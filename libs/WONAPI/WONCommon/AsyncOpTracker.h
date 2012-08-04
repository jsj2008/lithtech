#ifndef __WON_ASYNCOPTRACKER_H__
#define __WON_ASYNCOPTRACKER_H__
#include "WONShared.h"

#include "BiMap.h"
#include "AsyncOp.h"
#include "CriticalSection.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class AsyncOpTracker
{
private:
	CriticalSection mDataCrit;

	typedef BiMultiMap<DWORD,AsyncOpPtr,std::less<DWORD>,AsyncOpPtr::Comp> OpMap;
	OpMap mOpMap;

public:
	AsyncOp* Track(AsyncOp *theOp, DWORD theId = 0);
	bool Untrack(AsyncOp *theOp);

	void Kill(AsyncOp *theOp);
	void Kill(DWORD theId);
	void KillAll();

	int GetNumTrack() { return mOpMap.size(); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AsyncOpWithTracker : public AsyncOp
{
private:
	class TrackerCompletion : public OpCompletionBase
	{
		private:
			SmartPtr<AsyncOpWithTracker> mOp;
			int mParam;
		
		public:
			TrackerCompletion(AsyncOpWithTracker *theOp, int theParam) : mOp(theOp), mParam(theParam) {}
			virtual void Complete(AsyncOpPtr theOp) { mOp->Callback(theOp, mParam); }
	};

	friend class TrackerCompletion;

private:
	void Callback(AsyncOp *theOp, int theParam);

protected:
	AsyncOpTracker mOpTracker;
	AsyncOp* Track(AsyncOp *theOp, int theId = 0);
	virtual bool CallbackHook(AsyncOp *, int) { return false; }

	virtual void CleanupHook();
};

}; // namespace WONAPI

#endif

