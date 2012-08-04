#ifndef __WON_SocketThreadWinsock2_H__
#define __WON_SocketThreadWinsock2_H__


#include "WONShared.h"

#include <set>
#include <list>
#include <vector>
#include "SocketThread.h"
#include "WONCommon/BiMap.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketWaitThread : public Thread
{
public:
	typedef std::set<SocketOpPtr> OpList;
	struct SocketData : public RefCount
	{
		SocketWaitThread *mThread;
		HANDLE mEvent;
		OpList mOps;
		int mArrayPos;

		SocketData();

	protected:
		~SocketData() { }
	};
	typedef SmartPtr<SocketData> SocketDataPtr;

protected:
	HANDLE mWaitArray[MAXIMUM_WAIT_OBJECTS];
	AsyncSocketPtr mSocketArray[MAXIMUM_WAIT_OBJECTS];
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

	bool AddSocket(AsyncSocket *theSocket);
	void RemoveSocket(AsyncSocket *theSocket);

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketThreadWinsock2 : public SocketThread
{
private:

	typedef std::list<SocketWaitThread*> ThreadList;
	ThreadList mThreadList;

private:
	virtual bool NeedThread() { return false; } 

	static void StaticSocketCloseCallback(AsyncSocket *theSocket);

public:
	virtual ~SocketThreadWinsock2();
	virtual void PurgeOps();

	virtual void AddSocketOp(SocketOp *theSocketOp);
	virtual void RemoveSocketOp(SocketOp *theSocketOp);

	static void Notify(AsyncSocket *theSocket);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI


#endif
