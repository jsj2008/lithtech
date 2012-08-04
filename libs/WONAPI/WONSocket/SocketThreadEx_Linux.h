#ifndef __WON_SocketThreadEx_Linux_H__
#define __WON_SocketThreadEx_Linux_H__

#include "WONShared.h"

#include <set>
#include <list>
#include <vector>
#include <sys/poll.h>

#include "SocketThread.h"
#include "WONCommon/BiMap.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketWaitThread : public Thread
{
public:
	typedef std::list<SocketOpPtr> OpList;
	struct SocketData : public RefCount
	{
		SocketWaitThread *mThread;
		OpList mOps;
		int mArrayPos;
		int mEvents;

		SocketData();
		void CalcEvents(SocketOp *eraseThisOp = NULL);

	protected:
		~SocketData() { }
	};
	typedef SmartPtr<SocketData> SocketDataPtr;
	friend class SocketData;

protected:
	typedef std::list<pollfd*> PollArrayList;
	PollArrayList mOldPollArrays;
	void DeleteOldPollArrays();

	unsigned long mMaxSockets;
	AsyncSocketPtr mSignalSocket;

	pollfd *mPollArray;
	AsyncSocketPtr *mSocketArray;
	int mNumObjects;
	CriticalSection &mDataCrit;

	class ReverseLess
	{
	public:
		bool operator()(int a, int b) const { return b<a; }
	};
	typedef std::set<int,ReverseLess> ReverseSet;
	ReverseSet mReleaseSet;

	virtual void ThreadFunc();
	void ReleaseSockets();

public:
	SocketWaitThread(CriticalSection &theCrit);
	virtual ~SocketWaitThread();

	bool AddSocket(AsyncSocket *theSocket, bool growIfNeeded = false);
	void RemoveSocket(AsyncSocket *theSocket);
	virtual void Signal();

	virtual void Pump(DWORD theWaitTime);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketThreadEx : public SocketThread
{
private:

	typedef std::list<SocketWaitThread*> ThreadList;
	ThreadList mThreadList;

private:
	static void StaticSocketCloseCallback(AsyncSocket *theSocket);

public:
	virtual ~SocketThreadEx();
	virtual void PurgeOps();

	virtual bool NeedThread() { return false; } 
	virtual void AddSocketOp(SocketOp *theSocketOp);
	virtual void RemoveSocketOp(SocketOp *theSocketOp);
	virtual void Pump(DWORD theWaitTime);

	static void Notify(AsyncSocket *theSocket);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI


#endif
