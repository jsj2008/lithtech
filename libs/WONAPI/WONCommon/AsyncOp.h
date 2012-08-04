#ifndef __WON_ASYNCOP_H__
#define __WON_ASYNCOP_H__
#include "WONShared.h"

#define WIN32_LEAN_AND_MEAN
#include <list>
#include <map>

#include "CriticalSection.h"
#include "SmartPtr.h"
#include "Event.h"
#include "Completion.h"
#include "WONStatus.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum AsyncRunMode
{
	OP_MODE_ASYNC = 1,	
	OP_MODE_BLOCK = 2
};

enum AsyncTimeout
{
	OP_TIMEOUT_INFINITE = 0xffffffff,
};

class AsyncOpContainer;
class AsyncOp;
typedef SmartPtr<AsyncOp> AsyncOpPtr;
typedef CompletionBase<AsyncOpPtr> OpCompletionBase;
typedef SmartPtr<OpCompletionBase> OpCompletionBasePtr;


class AsyncOp : public RefCount
{
private:
	CriticalSection mStatusCrit;
	OpCompletionBasePtr mCompletion;
	AsyncOpContainer* mOpContainer;

	DWORD mStartTick;
	DWORD mTimeout;
	AsyncRunMode mMode;
	WONStatus mStatus;
	bool mAwaitingCompletion;
	bool mKilled;
	bool mRunAsyncImmediately;

private:
	// Used internally by the TimerThread
	friend class TimerThread;
	typedef std::multimap<DWORD,AsyncOpPtr> TimerMap;
	TimerMap *mTimerMap;
	TimerMap::iterator mTimerMapItr;
	void AddToTimerMap(TimerMap &theMap, DWORD theTimeout);
	void RemoveFromTimerMap(bool erase);
	void QueueInOpContainer();


protected:
	virtual ~AsyncOp();
	virtual void RunHook() { }
	virtual void CleanupHook() { }
	void StartRunAsync(); // called from async container
	friend class AsyncOpContainer;

public:
	AsyncOp();

	void SetMode(AsyncRunMode theMode);
	void SetTimeout(DWORD theTimeout);
	void SetRunAsyncImmediately(bool runImmediately); // should op be queued before running async or not?

	AsyncRunMode GetMode() const { return mMode; }
	DWORD GetTimeout() const { return mTimeout; }
	DWORD TimeLeft(); // Time left before operation times out
	DWORD RealTimeLeft(); // Doesn't return 0 is Op is running Async... returns actual time left
	bool IsAsync() { return mMode==OP_MODE_ASYNC; }
	bool IsBlock() { return mMode==OP_MODE_BLOCK; }

	void SetCompletion(OpCompletionBase *theCompletion) { mCompletion = theCompletion; }
	OpCompletionBase* GetCompletion() const { return mCompletion; }
	void SetAsyncOpContainer(AsyncOpContainer *theContainer) { mOpContainer = theContainer; }

	// Run Methods
	WONStatus Run(AsyncRunMode theMode, DWORD theTimeout);
	void RunAsync(DWORD theTimeout);
	bool RunBlock(DWORD theTimeout);

	// Timer Methods
	void RunAsTimer(DWORD theTimeout); // simply puts op in TimerThread -> will finish with WS_TimedOut or WS_Killed

	// Wrapping it up
	void Finish(WONStatus theStatus); // Finish operation if still pending
	void ForceFinish(WONStatus theStatus); // This essentially lets you queue up an op for completion even if it's not running
	void Complete(); // Call completion
	void Kill(bool finish = true); // Kill this operation 

	// Status accessors
	WONStatus GetStatus() const { return mStatus; }
	bool Succeeded() const { return mStatus==WS_Success; }
	bool TimedOut() const { return mStatus==WS_TimedOut; }
	bool Pending() const { return mStatus==WS_Pending; }
	bool Killed() const { return mKilled; }
 
	bool AwaitingCompletion() const { return mAwaitingCompletion; }
	bool Runnable() const { return !Pending() && !AwaitingCompletion(); }

};

typedef std::list<AsyncOpPtr> AsyncOpList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef Completion<AsyncOpPtr> OpCompletion;
typedef ParamCompletion<AsyncOpPtr,RefCountPtr> OpRefCompletion;
typedef BlockingCompletion<AsyncOpPtr> OpBlockingCompletion;
typedef SmartPtr<OpBlockingCompletion> OpBlockingCompletionPtr;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
class TimeoutOp : public AsyncOp
{
private:
	AsyncOpPtr mOpToTimeout;

protected:
	virtual void RunHook(); 
	virtual void CleanupHook();

public:
	TimeoutOp(AsyncOp *theOpToTimeout);
};
*/

}; // namespace WONAPI

#endif
