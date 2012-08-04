#ifndef __WON_ASYNCSOCKET_H__
#define __WON_ASYNCSOCKET_H__
#include "WONShared.h"

#include "WONStatus.h"
#include "WONCommon/CriticalSection.h"
#include "WONCommon/SmartPtr.h"
#include "WONCommon/ByteBuffer.h"
#include "IPAddr.h"

#include <list>

namespace WONAPI
{

class MsgTransform;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AsyncSocket;
typedef SmartPtr<AsyncSocket> AsyncSocketPtr;

class AsyncSocket : public RefCount
{
public:
	enum SocketType { TCP, UDP };


protected:
	CriticalSection mDataCrit;

	SOCKET mSocket;
	SocketType mType;
	IPAddr mDestAddr;
	unsigned char mLengthFieldSize;
	bool mConnected;
	bool mOpen;

	time_t mLastRecvTime;
	time_t mFirstBlockedSendTime;

	typedef std::list<MsgTransform*> MsgTransformQueue;
	MsgTransformQueue mMsgTransformQueue;

protected:
	static u_long mLocalAddress;

	static WONStatus TSGetLastError();
	static int TimeLeft(DWORD theTotalTime, DWORD theStartTime);
	virtual ~AsyncSocket();
	void ClosePrv();

	virtual void OpenHook() { }
	virtual void CloseHook() { }
	virtual void KillHook() { }

public:
	static WONStatus StartWinsock(void);
	static WONStatus StopWinsock(void);
	static int mWinsockStartCount;

	// Set the address to use for binding sockets; theAddr must be in host byte
	// order.  Defaults to INADDR_ANY.
	// Use this method to handle multihomed machines where you widsh to control
	// with network connection is used for bound sockets.  Note that changing
	// this setting only affects future sockets; existing sockets remain bound
	// to their original address
	static void   SetLocalAddress(u_long theAddr) { mLocalAddress = theAddr; }
	static u_long GetLocalAddress(void) { return mLocalAddress; }

public:
	AsyncSocket(SocketType theType=TCP);	

	void SetType(SocketType theType);
	SocketType GetType(void);

	bool IsValid(void);
	bool IsConnected(void);
	bool IsOpen(void);
	const IPAddr& GetDestAddr();
	unsigned short GetLocalPort();

	SOCKET GetDescriptor() { return mSocket; }

	// Base commands [GetNewDescriptor <--> ::socket(...) ]
	WONStatus GetNewDescriptor(void);
	WONStatus Bind(unsigned short thePort, bool allowReuse = false);
	WONStatus Bind(const IPAddr &theAddr, bool allowReuse = false);
	WONStatus Listen(int theBacklog = 1000);
	WONStatus Connect(const IPAddr &theAddr);
	WONStatus Accept(AsyncSocketPtr &theAsyncSocket);
	void Disconnect();
	void Close();
	void Kill();
	
	// Check status of blocking commands
	WONStatus WaitForConnect(DWORD theWaitTime);
	void WaitForAccept(DWORD theWaitTime);
	void WaitForWrite(DWORD theWaitTime);
	void WaitForRead(DWORD theWaitTime);

	// Raw Sends
	WONStatus SendBytes(const void *theBuf, int theLen, int *theSentLen = NULL);
	WONStatus RecvBytes(void *theBuf, int theLen, int *theRecvLen = NULL);   

	WONStatus SendBytesTo(const void *theBuf, int theLen, const IPAddr &theAddr);
	WONStatus RecvBytesFrom(void *theBuf, int theLen, IPAddr *theSockAddr, int *theRecvLen = NULL);

/*
	WONStatus SendBytesOverlap(const void *theBuf, int theLen, SocketOverlap *theOverlap);
	WONStatus RecvBytesOverlap(void *theBuf, int theLen, SocketOverlap *theOverlap);
	WONStatus RecvBytesFromOverlap(void *theBuf, int theLen, IPAddr *theSockAddr, SocketOverlap *theOverlap);
*/

	// Length Prefixed Messages
	void SetLengthFieldSize(unsigned char theLengthFieldSize);
	unsigned char GetLengthFieldSize() const;

	void PushMsgTransform(MsgTransform *theTransform);
	void RemoveAllMsgTransforms();
	WONStatus RunRecvMsgTransform(ByteBufferPtr &theMsg);
	WONStatus RunSendMsgTransform(ByteBufferPtr &theMsg);

	virtual AsyncSocket* Duplicate() { return new AsyncSocket(mType); }

	time_t GetLastRecvTime() { return mLastRecvTime; }
	time_t GetFirstBlockedSendTime() { return mFirstBlockedSendTime; }

	WONStatus SetNagling(bool on);

protected:
	typedef void(*CloseCallback)(AsyncSocket*);
	CloseCallback mThreadCloseCallback;
	RefCountPtr mThreadData;

public:
// Only should be used by SocketThreads
	void SetThreadData(RefCount *theData) { mThreadData = theData; }
	RefCount* GetThreadData() { return mThreadData; }
	
	void SetThreadCloseCallback(CloseCallback theCallback) { mThreadCloseCallback = theCallback; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class MsgTransform
{
public:
	virtual ~MsgTransform() { }

	virtual WONStatus Send(ByteBufferPtr&) { return WS_Success; }
	virtual WONStatus Recv(ByteBufferPtr&) { return WS_Success; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// This class is used so that the user doesn't have to worry about
// initializing winsock or stopping winsock.  Just instantiate a
// variable of this type in main() and it does the rest.

class InitWinsock 
{
public:
	InitWinsock() { AsyncSocket::StartWinsock(); }
	~InitWinsock() { AsyncSocket::StopWinsock(); }
};

}; // namespace WONAPI

#endif
