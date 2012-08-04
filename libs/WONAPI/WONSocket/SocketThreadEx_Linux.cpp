#include "SocketThreadEx_Linux.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketWaitThread::SocketWaitThread(CriticalSection &theCrit) : mDataCrit(theCrit)
{
	mMaxSockets = 101;
	mPollArray = new pollfd[mMaxSockets];
	mSocketArray = new AsyncSocketPtr[mMaxSockets];

	mSignalSocket = new AsyncSocket(AsyncSocket::UDP);

	// Bind SignalSocket and connect it to itself
	IPAddr anAddr = IPAddr::GetLocalAddr();
	anAddr.SetThePort(0); // choose random port
	mSignalSocket->Bind(anAddr);
	anAddr.SetThePort(mSignalSocket->GetLocalPort());
	mSignalSocket->Connect(anAddr);

	mNumObjects = 1;
	mPollArray[0].fd = mSignalSocket->GetDescriptor();
	mPollArray[0].events = POLLIN;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketWaitThread::~SocketWaitThread()
{
	AutoCrit aCrit(mDataCrit);
	for(int i=1; i<mNumObjects; i++)
	{
		AsyncSocket *aSocket = mSocketArray[i];
		RemoveSocket(aSocket);
	}

	ReleaseSockets();

	delete [] mPollArray;
	delete [] mSocketArray;

	DeleteOldPollArrays();
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::Signal()
{
	static char aBuf = 1;
	mSignalSocket->SendBytes(&aBuf,1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::ReleaseSockets(void) 
{
	AutoCrit aCrit(mDataCrit);

	if(mReleaseSet.empty())
		return;

	ReverseSet::iterator anItr = mReleaseSet.begin();
	int aCount = 0;
	while(anItr!=mReleaseSet.end()) 
	{
		int aPos = *anItr;

//		CloseHandle(mPollArray[aPos]);
		mSocketArray[aPos]->SetThreadData(NULL);

		if(aPos!=mNumObjects-1)
		{
			// Swap released handled with the last handle (mNumObjects-1)
			AsyncSocket* aSocket = mSocketArray[mNumObjects-1];
			mSocketArray[aPos] = aSocket;

			// Maintain invariant on the non-released handle
			SocketData* aData = (SocketData*)aSocket->GetThreadData();
			aData->mArrayPos = aPos;
			mPollArray[aPos].events = aData->mEvents;
			mPollArray[aPos].fd = aSocket->GetDescriptor();
		}

		mNumObjects--;
		mSocketArray[mNumObjects] = NULL;

		++anItr;
		++aCount;
	}

	mReleaseSet.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::ThreadFunc()
{
	while(!mStopped)
		Pump(INFINITE);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::Pump(DWORD theWaitTime)
{
	AutoCrit aCrit(mDataCrit);
	pollfd *anArray = mPollArray;
	int aNumObjects = mNumObjects;
	
	aCrit.Leave();
	int aVal = poll(anArray, aNumObjects, theWaitTime);
	aCrit.Enter();

	if(aVal>0)
	{
		pollfd *aPoll = anArray;
		if(aPoll->revents!=0)
		{
			static char aBuf;
			while(mSignalSocket->RecvBytes(&aBuf,1)==WS_Success)
			{
			}

			aVal--;
		}

		if(aVal>0)
		{
			aPoll++;
			for(int i=1; i<aNumObjects; i++)
			{
				if(aPoll->revents!=0)
				{
					AsyncSocket* aSocket = mSocketArray[i];
					SocketThreadEx::Notify(aSocket);

					aVal--;
					if(aVal<=0)
						break;
				}
				aPoll++;
			}
		}
	}
	else if(aVal<0) // got an error --> try all ops
	{
		for(int i=1; i<aNumObjects; i++)
		{
			AsyncSocket* aSocket = mSocketArray[i];
			SocketThreadEx::Notify(aSocket);
		}
	}		

	ReleaseSockets();
	DeleteOldPollArrays();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::DeleteOldPollArrays()
{
	if(mOldPollArrays.empty())
		return;

	AutoCrit aCrit(mDataCrit);
	PollArrayList::iterator anItr = mOldPollArrays.begin();
	while(anItr!=mOldPollArrays.end())
	{
		delete [] *anItr;
		++anItr;
	}
	mOldPollArrays.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SocketWaitThread::AddSocket(AsyncSocket *theSocket, bool growIfNeeded)
{
	AutoCrit aCrit(mDataCrit);

	if(mNumObjects==mMaxSockets)
	{
		if(!growIfNeeded)
			return false;
		
		mOldPollArrays.push_back(mPollArray);
		mMaxSockets*=2;
		mPollArray = new pollfd[mMaxSockets];
	}

	int aPos = mNumObjects;

	SocketDataPtr aData = new SocketData;
	aData->mThread = this;
	aData->mArrayPos = aPos;
	theSocket->SetThreadData(aData);

	mSocketArray[aPos] = theSocket;
	mPollArray[aPos].fd = theSocket->GetDescriptor();
	mPollArray[aPos].events = 0;

	mNumObjects++;

//	aCrit.Leave();
//	Signal();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::RemoveSocket(AsyncSocket *theSocket)
{
	AutoCrit aCrit(mDataCrit);

	SocketData* aData = (SocketData*)theSocket->GetThreadData();
	if(aData==NULL || aData->mThread!=this)
		return;

	aData->mThread = NULL;

	mReleaseSet.insert(aData->mArrayPos);
	OpList::iterator anItr = aData->mOps.begin();
	while(anItr!=aData->mOps.end())
	{
		(*anItr)->Kill();
		++anItr;
	}
	aData->mOps.clear();

	aCrit.Leave();
	Signal();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketWaitThread::SocketData::SocketData()
{
	mThread = NULL;
	mArrayPos = -1;
	mEvents = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketWaitThread::SocketData::CalcEvents(SocketOp *eraseThisOp)
{
	SocketData::OpList::iterator anItr = mOps.begin();
	while(anItr!=mOps.end())
	{
		SocketOp *anOp = *anItr;
		if(anOp==eraseThisOp)
			mOps.erase(anItr++);
		else
		{
			if(anOp->NeedSocketEvent(SocketEvent_Read))
				mEvents |= POLLIN;
			if(anOp->NeedSocketEvent(SocketEvent_Write))
				mEvents |= POLLOUT;
		}

		++anItr;
	}

	if(mThread!=NULL)
		mThread->mPollArray[mArrayPos].events = mEvents;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketThreadEx::~SocketThreadEx()
{
	PurgeOps();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::PurgeOps()
{
	ThreadList::iterator anItr = mThreadList.begin();
	while(anItr!=mThreadList.end())
	{
		SocketWaitThread *aThread = *anItr;
		aThread->Stop();
		delete aThread;
		++anItr;
	}
	mThreadList.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::StaticSocketCloseCallback(AsyncSocket *theSocket)
{
	SocketWaitThread::SocketData *aData = (SocketWaitThread::SocketData *)theSocket->GetThreadData();
	if(aData!=NULL)
	{
		SocketWaitThread *aThread = aData->mThread;
		if(aThread!=NULL)
			aThread->RemoveSocket(theSocket);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::AddSocketOp(SocketOp *theOp)
{
	AutoCrit aCrit(mDataCrit);

	AsyncSocket* aSocket = theOp->GetSocket();
	SocketWaitThread::SocketData* aData = (SocketWaitThread::SocketData *)aSocket->GetThreadData();
	SocketWaitThread *aThread = aData?aData->mThread:NULL;
	if(aThread==NULL)
	{
		aSocket->SetThreadCloseCallback(StaticSocketCloseCallback);
		ThreadList::iterator anItr = mThreadList.begin();
		while(anItr!=mThreadList.end())
		{
			if((*anItr)->AddSocket(aSocket, mSingleThreaded))
			{
				aThread = *anItr;
				if(anItr!=mThreadList.begin())
				{
					mThreadList.push_front(*anItr);
					mThreadList.erase(anItr++);
				}
				break;
			}
			else
				++anItr;
		}

		if(aThread==NULL)
		{
			aThread = new SocketWaitThread(mDataCrit);
			mThreadList.push_front(aThread);
			if(!mSingleThreaded)
				aThread->Start();

			aThread->AddSocket(aSocket);
		}
	}

	aData = (SocketWaitThread::SocketData *)aSocket->GetThreadData();
	aData->mOps.push_back(theOp);
	aData->CalcEvents();
	theOp->mInSocketThread = true;
	
	aCrit.Leave();
	aThread->Signal();
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::RemoveSocketOp(SocketOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	AsyncSocket* aSocket = theOp->GetSocket();
	SocketWaitThread::SocketData* aData = (SocketWaitThread::SocketData *)aSocket->GetThreadData();
	theOp->mInSocketThread = false;
	if(aData!=NULL)
		aData->CalcEvents(theOp);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::Notify(AsyncSocket *theSocket)
{
	SocketWaitThread::SocketData* aData = (SocketWaitThread::SocketData *)theSocket->GetThreadData();
	if(aData==NULL)
		return;

	SocketWaitThread::OpList::iterator anItr = aData->mOps.begin();
	bool needRecalc = false;
	while(anItr!=aData->mOps.end())
	{
		if(DoOp(*anItr))
		{
			needRecalc = true;
			aData->mOps.erase(anItr++);
		}
		else
			++anItr;
	}

	if(needRecalc)
		aData->CalcEvents();
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::Pump(DWORD theWaitTime)
{
	if(!mThreadList.empty())
		mThreadList.front()->Pump(theWaitTime);
}


