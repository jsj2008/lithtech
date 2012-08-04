#include <assert.h>
#if defined(WIN32) || defined(_LINUX)
#include <sys/timeb.h>
#endif // WIN32 || _LINUX
#include "AsyncSocket.h"
#include "NetStats.h"
#include <time.h>
using namespace WONAPI;

#if (defined _LINUX) && (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1) // MSG_NOSIGNAL supposedly broken for glibc-2.0
#define SENDRECV_FLAGS MSG_NOSIGNAL
#else
#define SENDRECV_FLAGS 0
#endif

int    AsyncSocket::mWinsockStartCount = 0;
u_long AsyncSocket::mLocalAddress      = INADDR_ANY;


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
AsyncSocket::AsyncSocket(SocketType theType) 
{
	mSocket = INVALID_SOCKET;
	mLengthFieldSize = 4;
	mType = theType;
	mConnected = false;
	mOpen = false;
	mThreadCloseCallback = NULL;

	mFirstBlockedSendTime = 0;
	mLastRecvTime = time(NULL);

	NetStats::IncrementNumConstructedSockets(1);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
AsyncSocket::~AsyncSocket() 
{
	Close();
	MsgTransformQueue::iterator anItr = mMsgTransformQueue.begin();
	while(anItr!=mMsgTransformQueue.end())
	{
		delete *anItr;
		++anItr;
	}

	NetStats::IncrementNumConstructedSockets(-1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int AsyncSocket::TimeLeft(DWORD theTotalTime, DWORD theStartTime)
{
	int deltaTime = GetTickCount() - theStartTime;
	if(deltaTime >= theTotalTime)
		return 0;
	else
		return theTotalTime - deltaTime;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::SetLengthFieldSize(unsigned char theLengthFieldSize)
{
	mLengthFieldSize = theLengthFieldSize;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
unsigned char AsyncSocket::GetLengthFieldSize() const
{
	return mLengthFieldSize; 
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::PushMsgTransform(MsgTransform *theTransform)
{
	AutoCrit aCrit(mDataCrit);
	mMsgTransformQueue.push_back(theTransform);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::RemoveAllMsgTransforms()
{
	AutoCrit aCrit(mDataCrit);
	MsgTransformQueue::iterator anItr = mMsgTransformQueue.begin();
	while(anItr!=mMsgTransformQueue.end())
	{
		delete *anItr;
		++anItr;
	}
	mMsgTransformQueue.clear();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::RunRecvMsgTransform(ByteBufferPtr &theMsg)
{
	AutoCrit aCrit(mDataCrit);
	MsgTransformQueue::iterator anItr = mMsgTransformQueue.end();
	if(anItr!=mMsgTransformQueue.begin())
	{
		do
		{
			--anItr;
			WONStatus aStatus = (*anItr)->Recv(theMsg);
			if(aStatus!=WS_Success)
				return aStatus;
		} while(anItr!=mMsgTransformQueue.begin());
	}

	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::RunSendMsgTransform(ByteBufferPtr &theMsg)
{
	AutoCrit aCrit(mDataCrit);
	MsgTransformQueue::iterator anItr = mMsgTransformQueue.begin();
	while(anItr!=mMsgTransformQueue.end())
	{
		WONStatus aStatus = (*anItr)->Send(theMsg);
		if(aStatus!=WS_Success)
			return aStatus;

		++anItr;
	}

	return WS_Success;
}



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::TSGetLastError()
{
	WONStatus anError = (WONStatus)WSAGetLastError();
	if(anError==WS_WSAEWOULDBLOCK || anError==WS_WSAEINPROGRESS)
		anError = WS_TimedOut;

	return anError;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
bool AsyncSocket::IsValid(void) 
{
	return mSocket!=INVALID_SOCKET;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
bool AsyncSocket::IsConnected(void)
{
	return mConnected;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
bool AsyncSocket::IsOpen()
{
	return mOpen;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
const IPAddr& AsyncSocket::GetDestAddr()
{
	return mDestAddr;
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
unsigned short AsyncSocket::GetLocalPort(void) 
{
	if(!IsValid())
		return 0;

	SOCKADDR aSockAddr;
	int aVal;
	socklen_t aLen = sizeof(SOCKADDR);

	aVal = getsockname(mSocket,(SOCKADDR*)&aSockAddr,&aLen);
	if(aVal!=0) return 0;

	return ntohs(((SOCKADDR_IN*)(&aSockAddr))->sin_port);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

WONStatus AsyncSocket::StartWinsock(void) 
{
	if(mWinsockStartCount!=0)
	{
		mWinsockStartCount++;
		return WS_Success;
	}

	int aVal = 0;

#ifdef _XBOX
	aVal = XnetInitialize(NULL,true);
	if(aVal!=0)
		return (WONStatus)aVal;
#endif // _XBOX


#ifdef WIN32
	WSADATA aDat;
	aVal = WSAStartup(MAKEWORD(1,1),&aDat);
	if(aVal!=0)
		return (WONStatus)aVal;
#endif	// WIN32

	mWinsockStartCount++;
	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::StopWinsock(void) 
{
	if(mWinsockStartCount<=0 || --mWinsockStartCount!=0)
		return WS_Success;

	int aVal = 0;

#ifdef WIN32
	aVal = WSACleanup();
	if(aVal!=0) 
		return (WONStatus)aVal;
#endif // WIN32

#if defined(_XBOX)
	aVal = XnetCleanup();
	return (WONStatus)aVal;
#endif

	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

AsyncSocket::SocketType AsyncSocket::GetType(void) 
{
	return mType;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::SetType(SocketType theType) 
{
	AutoCrit aCrit(mDataCrit);
	if(mType!=theType)
		Close();

	mType = theType;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::SetNagling(bool on) 
{
	int aVal, aSockOpt = !on;
	aVal = setsockopt(mSocket,IPPROTO_TCP,TCP_NODELAY,(char*) &aSockOpt,sizeof(aSockOpt));

	if(aVal==SOCKET_ERROR) 
		return TSGetLastError();
	else
		return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::GetNewDescriptor() 
{
	AutoCrit aCrit(mDataCrit);

	int aVal, aSockOptVal;
	u_long anIoctlVal;
	
	if(IsValid())
		Close();

	mOpen = true;

	switch(mType) 
	{
		case TCP: mSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); break;
		case UDP: mSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); break;
		default: assert(0);
	}

	if(mSocket==INVALID_SOCKET) 
		return TSGetLastError();
	
	NetStats::IncrementNumOpenSockets(1);

	// Tell the socket not to linger on close
	if(mType==TCP) 
	{
		linger aLingerStruct;
		aLingerStruct.l_onoff = 0;
//		aLingerStruct.l_onoff = 1;
//		aLingerStruct.l_linger = 1200;
		aVal = setsockopt(mSocket,SOL_SOCKET,SO_LINGER,(char*)&aLingerStruct,sizeof(linger));
		if(aVal==SOCKET_ERROR) 
			return TSGetLastError();
	}

	// Tell the socket to enable broadcast
	if(mType==UDP) 
	{
		aSockOptVal = 1;
		aVal = setsockopt(mSocket,SOL_SOCKET,SO_BROADCAST,(char*)&aSockOptVal,sizeof(int));
		if(aVal==SOCKET_ERROR) 
			return TSGetLastError();
	}

	// Tell the socket not to block
	anIoctlVal = 1;
	aVal = ioctlsocket(mSocket,FIONBIO,&anIoctlVal);
	if(aVal==SOCKET_ERROR)
		return TSGetLastError();

	OpenHook();
	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::Bind(const IPAddr &theAddr, bool allowReuse)
{
	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
	{
		WONStatus anError = GetNewDescriptor();
		if(anError!=WS_Success) return anError;
	}

	int aVal;

	if (allowReuse)
	{
		int aBool = 1;
		aVal = setsockopt( mSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &aBool, sizeof(int) );
		if (aVal!=0)
			return TSGetLastError();
	}

	aVal = ::bind(mSocket,(const SOCKADDR*)&theAddr.GetSockAddrIn(),sizeof(SOCKADDR_IN));
	if(aVal!=0)
		return TSGetLastError();

	return WS_Success;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::Bind(unsigned short thePort, bool allowReuse) 
{
	SOCKADDR_IN aSockAddrIn;
	
	memset((char *)&aSockAddrIn, 0, sizeof(aSockAddrIn));
	aSockAddrIn.sin_family      = AF_INET;
	aSockAddrIn.sin_addr.s_addr = htonl(mLocalAddress);
	aSockAddrIn.sin_port        = htons(thePort);
	return Bind(IPAddr(aSockAddrIn),allowReuse);
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::Connect(const IPAddr &theAddr) 
{
	AutoCrit aCrit(mDataCrit);
	
	if(mType==TCP && mConnected) Disconnect();
	if(mSocket==INVALID_SOCKET) 
	{
		WONStatus anError = GetNewDescriptor();
		if(anError!=WS_Success) return anError;
		if(mLocalAddress!=INADDR_ANY)
		{
			anError = Bind(0);
			if(anError!=WS_Success) return anError;
		}
	}

	mDestAddr = theAddr; 
	mConnected = true;
	int aVal = ::connect(mSocket,(const SOCKADDR*)&theAddr.GetSockAddrIn(),sizeof(SOCKADDR));	
	if(aVal==SOCKET_ERROR) 
	{
		WONStatus anError = TSGetLastError();
		return anError;
	}
	else
		return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::Listen(int theBacklog)
{
	AutoCrit aCrit(mDataCrit);

	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	if (mType == UDP)
		return WS_AsyncSocket_DatagramNotAllowed;

	int anError = ::listen(mSocket, theBacklog);
	if ( anError != 0 )
		return TSGetLastError();
	else
		return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::Accept(AsyncSocketPtr &theAsyncSocket)
{
	AutoCrit aCrit(mDataCrit);

	SOCKET aSocket;
	SOCKADDR_IN aSockAddr;
	socklen_t aSockAddrSize;

	aSockAddrSize = sizeof(SOCKADDR_IN);
	aSocket = ::accept(mSocket, (SOCKADDR*)&aSockAddr, &aSockAddrSize );
	if (aSocket == INVALID_SOCKET)
		return TSGetLastError();

	int aVal;
	u_long anIoctlVal;

	// Tell the socket not to linger on close... 
	linger aLingerStruct;
	aLingerStruct.l_onoff = 0;
//	aLingerStruct.l_onoff = 1;
//	aLingerStruct.l_linger = 1200;
	aVal = setsockopt(aSocket,SOL_SOCKET,SO_LINGER,(char*)&aLingerStruct,sizeof(linger));

	// Tell the socket not to block
	anIoctlVal = 1;
	aVal = ioctlsocket(aSocket,FIONBIO,&anIoctlVal);


	theAsyncSocket = Duplicate();
	theAsyncSocket->mSocket = aSocket;
	theAsyncSocket->mDestAddr = IPAddr(aSockAddr);
	theAsyncSocket->mOpen = true;
	theAsyncSocket->mConnected = true;
	NetStats::IncrementNumOpenSockets(1);
	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::WaitForConnect(DWORD theWaitTime) 
{

	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;
	SOCKET aSocket = mSocket;
	aCrit.Leave();


	timeval aTimeout;
	aTimeout.tv_sec = theWaitTime/1000;
	aTimeout.tv_usec = (theWaitTime%1000)*1000;
  
	fd_set aWriteSet;
	fd_set anExceptSet;

	FD_ZERO(&aWriteSet);
	FD_ZERO(&anExceptSet);
	FD_SET(aSocket,&aWriteSet);
	FD_SET(aSocket,&anExceptSet);
	
	int aVal = select(FD_SETSIZE,NULL,&aWriteSet,&anExceptSet,&aTimeout);

	int aConnectError = 0;
	socklen_t aLen = sizeof(socklen_t);

#ifdef _LINUX
	// Extended error information (need to get SO_ERROR on LINUX... doesn't go in the except set)
	int anotherVal = getsockopt(mSocket,SOL_SOCKET,SO_ERROR,(char*)&aConnectError,&aLen);
	if(aConnectError!=0)
		return (WONStatus)aConnectError;
#endif
 
	if(FD_ISSET(aSocket,&anExceptSet))
	{
//		Sleep(1); // This seems to make getsockopt more likely to work (on some machines)

#ifndef _XBOX
	// Extended error information
	int anotherVal = getsockopt(mSocket,SOL_SOCKET,SO_ERROR,(char*)&aConnectError,&aLen);
	if(aConnectError!=0)
		return (WONStatus)aConnectError;
#endif


		if(aConnectError==0) 
			aConnectError = WS_AsyncSocket_ConnectFailed;

		return (WONStatus)aConnectError;
	}

	if(aVal==1)
		return WS_Success;
	else
		return WS_TimedOut;

}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::WaitForAccept(DWORD theWaitTime) 
{
	WaitForRead(theWaitTime);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::WaitForWrite(DWORD theWaitTime) 
{
	if(theWaitTime==0)
		return;

	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return;

	timeval aTimeout;
	aTimeout.tv_sec = theWaitTime/1000;
	aTimeout.tv_usec = (theWaitTime%1000)*1000;
  
	fd_set aWriteSet;

	FD_ZERO(&aWriteSet);
	FD_SET(mSocket,&aWriteSet);
	aCrit.Leave();
	 
	select(FD_SETSIZE,NULL,&aWriteSet,NULL,&aTimeout);	
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::WaitForRead(DWORD theWaitTime) 
{
	if(theWaitTime==0)
		return;

	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return;

	timeval aTimeout;
	aTimeout.tv_sec = theWaitTime/1000;
	aTimeout.tv_usec = (theWaitTime%1000)*1000;
	fd_set aReadSet;

	FD_ZERO(&aReadSet);
	FD_SET(mSocket,&aReadSet);
	aCrit.Leave();
	
	int aVal = select(FD_SETSIZE,&aReadSet,NULL,NULL,&aTimeout);	
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::SendBytes(const void *theBuf, int theLen, int *theSentLen)
{
	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	int aVal = send(mSocket, (char*)theBuf, theLen, SENDRECV_FLAGS);
	if(aVal<0)
	{
		if(mFirstBlockedSendTime==0)
			mFirstBlockedSendTime = time(NULL);

		return TSGetLastError();
	}
	else
	{
		mFirstBlockedSendTime = 0;
		NetStats::IncrementBytesSent(aVal);
	}

	if(theSentLen!=NULL) 
		*theSentLen = aVal;

	if(aVal<theLen)
	{
		if(mType==UDP)
			return WS_AsyncSocket_PartialSendTo;
		else
			return WS_TimedOut;
	}

	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::RecvBytes(void *theBuf, int theLen, int *theRecvLen)
{
	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	int aVal = recv(mSocket, (char*)theBuf, theLen, SENDRECV_FLAGS);
	if(aVal<0)
		return TSGetLastError();
	else if(aVal==0) 
		return WS_AsyncSocket_Shutdown;
	else
	{
		NetStats::IncrementBytesReceived(aVal);
		mLastRecvTime = time(NULL);
	}

	if(theRecvLen!=NULL) 
		*theRecvLen = aVal;

	if(aVal!=theLen && mType==TCP)
		return WS_TimedOut;
	else	
		return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

WONStatus AsyncSocket::SendBytesTo(const void *theBuf, int theLen, const IPAddr &theAddr)
{
	AutoCrit aCrit(mDataCrit);
	if(mType==TCP)
		return WS_AsyncSocket_StreamNotAllowed;

	if(!IsValid())
	{
		WONStatus anError = GetNewDescriptor();
		if(anError!=WS_Success) return anError;
	}

	int aVal = sendto(mSocket,(char*)theBuf,theLen,SENDRECV_FLAGS,(SOCKADDR*)&theAddr.GetSockAddrIn(),sizeof(SOCKADDR_IN));
	if(aVal<0)
		return TSGetLastError();

	NetStats::IncrementBytesSent(aVal);
	if(aVal==theLen) 
		return WS_Success;
	else 
		return WS_AsyncSocket_PartialSendTo;
}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::RecvBytesFrom(void *theBuf, int theLen, IPAddr *theAddr, int *theRecvLen)
{
	AutoCrit aCrit(mDataCrit);
	if(mType==TCP)
		return WS_AsyncSocket_StreamNotAllowed;

	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	int aVal;
	socklen_t aFromLen;

	aFromLen = sizeof(SOCKADDR_IN);
	SOCKADDR_IN anAddr;

	aVal = recvfrom(mSocket,(char*)theBuf,theLen,SENDRECV_FLAGS,(SOCKADDR*)&anAddr,&aFromLen);
	if(aVal>=0) 
	{
		NetStats::IncrementBytesReceived(aVal);

		if(theAddr!=NULL) 
			theAddr->Set(anAddr);
		
		if(theRecvLen!=NULL) 
			*theRecvLen = aVal;
		
		return WS_Success;
	}
	else 
		return TSGetLastError();
}

/*
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::SendBytesOverlap(const void *theBuf, int theLen, SocketOverlap *theOverlap)
{
	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	int aSentLen = 0;
	theOverlap->mBuf.buf = theBuf;
	theOverlap->mBuf.len = theLen;
	if(WSASend(mSocket, &theOverlap->mBuf, 1, &aSentLen, 0, theOverlap, NULL)==0)
	{
		if(aSentLen==theLen)
			return WS_Success;
		else	
			return WS_TimedOut;
	}

	if(GetLastError()==WSA_IO_PENDING)
		return WS_TimedOut;
	else
		return TSGetLastError();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::RecvBytesOverlap(void *theBuf, int theLen, SocketOverlap *theOverlap)
{
	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	int aRecvLen = 0;
	theOverlap->mBuf.buf = theBuf;
	theOverlap->mBuf.len = theLen;
	if(WSARecv(mSocket, &theOverlap->mBuf, 1, &aRecvLen, 0, theOverlap, NULL)==0)
	{
		if(aRecvLen==theLen)
			return WS_Success;
		else	
			return WS_TimedOut;
	}

	if(GetLastError()==WSA_IO_PENDING)
		return WS_TimedOut;
	else
		return TSGetLastError();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus AsyncSocket::RecvBytesFromOverlap(void *theBuf, int theLen, IPAddr *theAddr, SocketOverlap *theOverlap)
{
	AutoCrit aCrit(mDataCrit);
	if(!IsValid())
		return WS_AsyncSocket_InvalidSocket;

	int aRecvLen = 0;
	theOverlap->mBuf.buf = theBuf;
	theOverlap->mBuf.len = theLen;
	socklen_t aFromLen;

	aFromLen = sizeof(SOCKADDR_IN);
	SOCKADDR_IN anAddr;

	if(WSARecvFrom(mSocket, &theOverlap->mBuf, 1, &aRecvLen, 0, &anAddr, &aFromLen, theOverlap, NULL)==0)
	{
		if(theAddr!=NULL) 
			theAddr->Set(anAddr);

		theOverlap->mBuf.len = aFromLen;

		return WS_Success;
	}

	if(GetLastError()==WSA_IO_PENDING)
		return WS_TimedOut;
	else
		return TSGetLastError();
}
*/

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void AsyncSocket::Disconnect() 
{
	AutoCrit aCrit(mDataCrit);
	if(mType==TCP)
		Close();
	else 
	{
		int aVal;
		SOCKADDR aSockAddr;

		memset(&aSockAddr,0,sizeof(SOCKADDR));

		// This disconnects the datagram socket from a specific address
		aVal = ::connect(mSocket,&aSockAddr,sizeof(SOCKADDR));
		if(aVal!=0) 
			Close();
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::ClosePrv()
{
	if(IsValid())
	{
		::shutdown(mSocket, SD_SEND);
		if(::closesocket(mSocket)!=0 && TSGetLastError()==WS_TimedOut)
		{
			// Do abortive close if closesocket had a blocking error
			linger aLingerStruct;
			aLingerStruct.l_onoff = 1;
			aLingerStruct.l_linger = 0;
			int aVal = setsockopt(mSocket,SOL_SOCKET,SO_LINGER,(char*)&aLingerStruct,sizeof(linger));
			::closesocket(mSocket);
		}

		mSocket = INVALID_SOCKET;

		NetStats::IncrementNumOpenSockets(-1);

	}

	mConnected = false;
	mOpen = false;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::Close() 
{
	if(mThreadCloseCallback)
		mThreadCloseCallback(this);

	AutoCrit aCrit(mDataCrit);
	CloseHook();
	ClosePrv();
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void AsyncSocket::Kill()
{
	if(mThreadCloseCallback)
		mThreadCloseCallback(this);

	AutoCrit aCrit(mDataCrit);
	CloseHook();
	KillHook();
	ClosePrv();
}
