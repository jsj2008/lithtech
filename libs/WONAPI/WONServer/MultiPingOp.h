#ifndef __WON_MULTIPINGOP_H__
#define __WON_MULTIPINGOP_H__
#include "WONShared.h"

#include "WONCommon/AsyncOpTracker.h"
#include "WONSocket/BlockingSocket.h"
#include "WONDir/DirEntity.h"
#include "ServerContext.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct MultiPingStruct : public RefCount
{
	IPAddr mAddr;
	int mPingTime;
	int mNumFailures;

	DWORD mStartPingTick;
	int mPingId;

	DirEntityPtr mDirEntity;

	MultiPingStruct(IPAddr theAddr, const DirEntity *theEntity = NULL) : mAddr(theAddr), mDirEntity(theEntity), mPingTime(-1), mNumFailures(0) {} 
};
typedef SmartPtr<MultiPingStruct> MultiPingStructPtr;
typedef std::list<MultiPingStructPtr> MultiPingList;
typedef std::map<IPAddr,MultiPingStructPtr> MultiPingMap;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class MultiPingOp : public AsyncOp
{
private:

	MultiPingList mPingList;
	MultiPingMap mPingMap;

	MultiPingList mFinishedList;

	unsigned short mBatchSize;
	unsigned char mMaxFailures;
	DWORD mMaxPingTime;
	unsigned short mMaxPingReplySize;
	bool mOrderListByPingTime;

	BlockingSocketPtr mSocket;
	AsyncOpPtr mRecvTimeoutOp;

	void PrepareBatch();
	void PingBatchAsync();
	WONStatus PingBatchBlock();

	void FinishBatch();
	DWORD ResponseTimeLeft(DWORD theStartTick);

	static void StaticRecvCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	static void StaticTimeoutCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	void RecvCallback(RecvBytesFromOp *theOp);
	void TimeoutCallback(AsyncOp *theOp);

	bool HandleResponsePrv(WONStatus theStatus, const ByteBuffer *theResponse, const IPAddr &theAddr);

protected:
	enum PingAction
	{
		PingAction_InvalidReply,	// --> keep waiting for valid reply
		PingAction_Done,			// --> done pinging this guy
		PingAction_More				// --> ping this guy some more
	};

	virtual ByteBufferPtr GetRequest(MultiPingStruct *theStruct);
	virtual PingAction HandleResponse(MultiPingStruct *theStruct, const ByteBuffer *theResponse);

	virtual void RunHook();
	virtual void CleanupHook();

public:
	MultiPingOp();
	
	void AddServer(const IPAddr &theAddr);
	void AddServers(const DirEntityList &theDir, const wchar_t *theNameFilter = NULL);
	void ClearServers();

	const MultiPingList& GetPingList() const { return mFinishedList; }
	ServerContextPtr GetServerContext(bool discardNonResponsiveServers = true);

	void SetBatchSize(unsigned short theSize) { mBatchSize = theSize; }
	void SetMaxFailures(unsigned char theMaxFailures) { mMaxFailures = theMaxFailures; }
	void SetMaxPingTime(DWORD theMilliseconds) { mMaxPingTime = theMilliseconds; }
};


typedef SmartPtr<MultiPingOp> MultiPingOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
