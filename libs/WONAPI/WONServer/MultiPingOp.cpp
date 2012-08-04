#include "MultiPingOp.h"
#include <time.h>
using namespace WONAPI;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
MultiPingOp::MultiPingOp()
{
	mBatchSize = 20;
	mMaxFailures = 3;
	mMaxPingTime = 3000;
	mMaxPingReplySize = 2048;
	mOrderListByPingTime = true;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::AddServer(const IPAddr &theAddr)
{
	mPingList.push_back(new MultiPingStruct(theAddr));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::AddServers(const DirEntityList &theDir, const wchar_t *theNameFilter)
{
	wstring aNameFilter;
	if(theNameFilter!=NULL)
		aNameFilter = theNameFilter;

	DirEntityList::const_iterator anItr = theDir.begin();
	while(anItr!=theDir.end())
	{
		const DirEntity *anEntity = *anItr;
		if(!anEntity->IsDir())
		{
			if(aNameFilter.empty() || aNameFilter==anEntity->mName)
			{
				IPAddr anAddr = anEntity->GetNetAddrAsIP();
				if(anAddr.IsValid())
					mPingList.push_back(new MultiPingStruct(anAddr,anEntity));
			}
		}

		++anItr;
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::ClearServers()
{
	mPingList.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr MultiPingOp::GetRequest(MultiPingStruct *theStruct)
{
	theStruct->mPingId = rand();
	theStruct->mStartPingTick = GetTickCount();

	WriteBuffer aBuf;
	aBuf.AppendByte(3);
	aBuf.AppendByte(1);
	aBuf.AppendByte(5);
	aBuf.AppendLong(theStruct->mPingId);
	aBuf.AppendBool(false);
	return aBuf.ToByteBuffer();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
MultiPingOp::PingAction MultiPingOp::HandleResponse(MultiPingStruct *theStruct, const ByteBuffer *theResponse)
{
	ReadBuffer aBuf(theResponse->data(),theResponse->length());
	try
	{
		unsigned char aHeader = aBuf.ReadByte();
		unsigned char aService = aBuf.ReadByte();
		unsigned char aMsg = aBuf.ReadByte();

		if(aHeader!=3 || aService!=1 || aMsg!=6)
			return PingAction_InvalidReply;

		int anId = aBuf.ReadLong();
		if(anId!=theStruct->mPingId)
			return PingAction_InvalidReply;

		theStruct->mPingTime = GetTickCount()-theStruct->mStartPingTick;
		return PingAction_Done;
	}
	catch(ReadBufferException&)
	{
		return PingAction_InvalidReply;
	}

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::StaticRecvCallback(AsyncOpPtr theOp, RefCountPtr theParam)
{
	MultiPingOp *thisOp = (MultiPingOp*)theParam.get();
	RecvBytesFromOp *aRecvOp = (RecvBytesFromOp*)theOp.get();
	thisOp->RecvCallback(aRecvOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::RecvCallback(RecvBytesFromOp *theOp)
{
	if(!Pending() || theOp->GetSocket()!=mSocket.get())
		return;

	if(!HandleResponsePrv(theOp->GetStatus(),theOp->GetBytes(),theOp->GetAddr()))
		Finish(theOp->GetStatus());

	if(mPingMap.size()==0)
	{
		if(mPingList.size()==0)
			Finish(WS_Success);
		else
			PingBatchAsync();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::StaticTimeoutCallback(AsyncOpPtr theOp, RefCountPtr theParam)
{
	MultiPingOp *thisOp = (MultiPingOp*)theParam.get();
	thisOp->TimeoutCallback((AsyncOp*)theOp.get());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::TimeoutCallback(AsyncOp *theOp)
{
	if(!Pending() || theOp!=mRecvTimeoutOp.get())
		return;

	mRecvTimeoutOp = NULL;
	FinishBatch();
	PingBatchAsync();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::PrepareBatch()
{
	mPingMap.clear();

	MultiPingList::iterator anItr = mPingList.begin();
	int aCount = 0;

	while(anItr!=mPingList.end() && aCount<mBatchSize)
	{
		mPingMap[(*anItr)->mAddr] = *anItr;
		mPingList.erase(anItr++);
		aCount++;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::FinishBatch()
{
	MultiPingMap::iterator anItr = mPingMap.begin();
	while(anItr!=mPingMap.end())
	{
		MultiPingStruct *aStruct = anItr->second;
		aStruct->mNumFailures++;
		if(aStruct->mNumFailures >= mMaxFailures)
			mFinishedList.push_back(aStruct);
		else
			mPingList.push_back(aStruct);

		++anItr;
	}

	mPingMap.clear();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::PingBatchAsync()
{
	if(mRecvTimeoutOp.get()!=NULL)
	{
		mRecvTimeoutOp->Kill();
		mRecvTimeoutOp = NULL;
	}

	PrepareBatch();
	MultiPingMap::iterator anItr = mPingMap.begin();
	while(anItr!=mPingMap.end())
	{
		ByteBufferPtr aPing = GetRequest(anItr->second);
		mSocket->QueueOp(new SendBytesToOp(aPing,anItr->first), OP_TIMEOUT_INFINITE);
		++anItr;
	}

	mRecvTimeoutOp = new AsyncOp;
	mRecvTimeoutOp->SetCompletion(new OpRefCompletion(StaticTimeoutCallback,this));
	mRecvTimeoutOp->RunAsync(mMaxPingTime);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD MultiPingOp::ResponseTimeLeft(DWORD theStartTick)
{
	DWORD anElapsed = GetTickCount() - theStartTick;
	if(anElapsed > mMaxPingTime)
		return 0;


	DWORD anAbsTimeLeft = TimeLeft();
	DWORD aTimeLeft = mMaxPingTime - anElapsed;

	if(aTimeLeft < anAbsTimeLeft)
		return aTimeLeft;
	else
		return anAbsTimeLeft;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool MultiPingOp::HandleResponsePrv(WONStatus theStatus, const ByteBuffer *theResponse, const IPAddr &theAddr)
{
	if(theStatus!=WS_Success && theStatus!=WS_WSAEMSGSIZE)
		return false;

	if(theStatus==WS_Success)
	{
		MultiPingMap::iterator anItr = mPingMap.find(theAddr);
		if(anItr!=mPingMap.end())
		{
			PingAction anAction = HandleResponse(anItr->second,theResponse);
			if(anAction==PingAction_Done)
			{
				mFinishedList.push_back(anItr->second);
				mPingMap.erase(anItr);
			}
			else if(anAction==PingAction_More)
			{
				mPingList.push_back(anItr->second);
				mPingMap.erase(anItr);
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus MultiPingOp::PingBatchBlock()
{
	PrepareBatch();

	MultiPingMap::iterator anItr = mPingMap.begin();
	WONStatus aStatus;
	while(TimeLeft()>0 && anItr!=mPingMap.end())
	{
		ByteBufferPtr aPing = GetRequest(anItr->second);
		aStatus = mSocket->SendBytesTo(aPing, anItr->first, TimeLeft());
		if(aStatus!=WS_Success)
			return aStatus;

		++anItr;
	}

	DWORD aStartTick = GetTickCount();
	while(ResponseTimeLeft(aStartTick)>0 && !mPingMap.empty())
	{
		ByteBufferPtr aReply;
		IPAddr anAddr;
		aStatus = mSocket->RecvBytesFrom(aReply,mMaxPingReplySize,anAddr,ResponseTimeLeft(aStartTick));
		if(!HandleResponsePrv(aStatus,aReply,anAddr))
			return aStatus;

	}

	return aStatus;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::RunHook()
{
	srand(time(NULL));
	mSocket = new BlockingSocket(AsyncSocket::UDP);
	mSocket->Bind(0);
	mFinishedList.clear();

	if(IsAsync())
	{
		mSocket->SetRepeatCompletion(new OpRefCompletion(StaticRecvCallback,this));
		mSocket->SetRepeatOp(new RecvBytesFromOp(mMaxPingReplySize));
		PingBatchAsync();
		return;
	}

	WONStatus aStatus;
	while(TimeLeft()>0 && !mPingList.empty())
	{
		aStatus = PingBatchBlock();
		FinishBatch();
		if(aStatus!=WS_Success && aStatus!=WS_TimedOut)
		{
			Finish(aStatus);
			return; 
		}
	}

	Finish(WS_Success);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MultiPingOp::CleanupHook()
{
	if(mSocket.get()!=NULL)
	{
		mSocket->Close();
		mSocket = NULL;
	}

	if(mRecvTimeoutOp.get()!=NULL)
	{
		mRecvTimeoutOp->Kill();
		mRecvTimeoutOp = NULL;
	}

	FinishBatch();

	MultiPingList::iterator aListItr = mPingList.begin();
	while(aListItr!=mPingList.end())
	{
		mFinishedList.push_back(*aListItr);
		++aListItr;
	}
	mPingList.clear();

	if(mOrderListByPingTime)
	{
		typedef std::multimap<DWORD,MultiPingStructPtr> SortMap;
		SortMap aMap;
		aListItr = mFinishedList.begin();
		while(aListItr!=mFinishedList.end())
		{
			aMap.insert(SortMap::value_type((*aListItr)->mPingTime,*aListItr));
			++aListItr;
		}

		mFinishedList.clear();

		SortMap::iterator aMapItr = aMap.begin();
		while(aMapItr!=aMap.end())
		{
			mFinishedList.push_back(aMapItr->second);
			aMapItr++;
		}
	}

}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ServerContextPtr MultiPingOp::GetServerContext(bool discardNonResponsiveServers)
{
	ServerContextPtr aContext = new ServerContext;
	MultiPingList::iterator anItr = mFinishedList.begin();
	while(anItr!=mFinishedList.end())
	{
		MultiPingStruct *aStruct = *anItr;
		if(aStruct->mPingTime!=-1 || !discardNonResponsiveServers)
			aContext->AddAddress(aStruct->mAddr);

		++anItr;
	}

	aContext->SetNeedShuffle(false);
	return aContext;
}
