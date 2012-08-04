#ifndef __WON_QUEUESOCKET_H__
#define __WON_QUEUESOCKET_H__
#include "WONShared.h"

#include <list>
#include "WONCommon/SmartPtr.h"
#include "WONCommon/CriticalSection.h"
#include "WONCommon/AsyncOp.h"
#include "AsyncSocket.h"
#include "CloseOp.h"

namespace WONAPI
{

class SocketOp;
typedef SmartPtr<SocketOp> SocketOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class QueueSocket : public AsyncSocket
{
private:
	OpCompletionBasePtr mDefaultQueueCompletion;
	OpCompletionBasePtr mCurQueueCompletion;
	OpCompletionBasePtr mRepeatCompletion;
	OpCompletionBasePtr mCloseCompletion;

	static void StaticCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	void Callback(SocketOp* theOp);

	static void StaticCloseTimerCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	void CloseTimerCallback(AsyncOp *theTimer);

private:
	SocketOpPtr mRepeatOp;
	SocketOpPtr mQueueOp;
	AsyncOpPtr mCloseTimer;
	typedef std::list<SocketOpPtr> OpQueue;
	OpQueue mOpQueue;

	void RunRepeatOp();
	void RunQueueOp();

protected:
	virtual void KillHook();
	virtual void CloseHook();

public:	
	QueueSocket(SocketType theType=TCP);
	void QueueOp(SocketOp *theOp, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	void QueueOpFast(SocketOp *theOp, DWORD theTimeout = OP_TIMEOUT_INFINITE); 
	void SetRepeatOp(SocketOp *theOp);

	void SetQueueCompletion(OpCompletionBase *theCompletion);
	void SetRepeatCompletion(OpCompletionBase *theCompletion);
	void SetCloseCompletion(OpCompletionBase *theCompletion);

	void QueueClose(DWORD theTimeout = OP_TIMEOUT_INFINITE);

	void StartCloseTimer(DWORD theTimeout);
	void CancelCloseTimer();

	virtual AsyncSocket* Duplicate() { return new QueueSocket(mType); }
};

typedef SmartPtr<QueueSocket> QueueSocketPtr;


}; // namespace WONAPI

#endif
