#include "SocketThreadWinsock2.h"
#include <crtdbg.h>
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketWaitThread::SocketWaitThread(CriticalSection &theCrit) : mDataCrit(theCrit)
{
	mNumObjects = 1;
#ifndef __WON_SINGLETHREADED__
	mWaitArray[0] = mSignalEvent.GetHandle();
#endif
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

		CloseHandle(mWaitArray[aPos]);
		mSocketArray[aPos]->SetThreadData(NULL);

		if(aPos!=mNumObjects-1)
		{
			// Swap released handled with the last handle (mNumObjects-1)
			AsyncSocket* aSocket = mSocketArray[mNumObjects-1];
			mSocketArray[aPos] = aSocket;

			// Maintain invariant on the non-released handle
			SocketData* aData = (SocketData*)aSocket->GetThreadData();
			aData->mArrayPos = aPos;
			mWaitArray[aPos] = aData->mEvent;
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
	int aHandleIndex;
	while(true)
	{
		DWORD aWaitResult = WaitForMultipleObjects(mNumObjects, mWaitArray, false, INFINITE);
		switch(aWaitResult) 
		{
			case WAIT_FAILED:				
				return; 

			case WAIT_TIMEOUT:
				break;

			default:
				AutoCrit aCrit(mDataCrit);

				aHandleIndex = (aWaitResult - WAIT_OBJECT_0);
				if(aHandleIndex==0)  // REHUP
				{
					if(mStopped)
						return;
				}
				else
				{
					AsyncSocket* aSocket = mSocketArray[aHandleIndex];
					SocketThreadWinsock2::Notify(aSocket);
				}
				aCrit.Leave();
		}

		ReleaseSockets();
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SocketWaitThread::AddSocket(AsyncSocket *theSocket)
{
	AutoCrit aCrit(mDataCrit);

	if(mNumObjects==MAXIMUM_WAIT_OBJECTS)
		return false;

	int aPos = mNumObjects;

	SocketDataPtr aData = new SocketData;
	aData->mEvent = CreateEvent(NULL,false,false,NULL);
	WSAEventSelect(theSocket->GetDescriptor(), aData->mEvent, FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT | FD_ACCEPT);
	aData->mThread = this;
	aData->mArrayPos = aPos;
	theSocket->SetThreadData(aData);

	mSocketArray[aPos] = theSocket;
	mWaitArray[aPos] = aData->mEvent;
	mNumObjects++;

	aCrit.Leave();
	mSignalEvent.Set();
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
	mSignalEvent.Set();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketWaitThread::SocketData::SocketData()
{
	mEvent = NULL;
	mThread = NULL;
	mArrayPos = -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SocketThreadWinsock2::~SocketThreadWinsock2()
{
	PurgeOps();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketThreadWinsock2::PurgeOps()
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
void SocketThreadWinsock2::StaticSocketCloseCallback(AsyncSocket *theSocket)
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
void SocketThreadWinsock2::AddSocketOp(SocketOp *theOp)
{
	AutoCrit aCrit(mDataCrit);

	AsyncSocket* aSocket = theOp->GetSocket();
	SocketWaitThread::SocketData* aData = (SocketWaitThread::SocketData *)aSocket->GetThreadData();
	bool foundThread = false;
	if(aData==NULL || aData->mThread==NULL)
	{
		aSocket->SetThreadCloseCallback(StaticSocketCloseCallback);
		ThreadList::iterator anItr = mThreadList.begin();
		while(anItr!=mThreadList.end())
		{
			if((*anItr)->AddSocket(aSocket))
			{
				foundThread = true;
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

		if(!foundThread)
		{
			mThreadList.push_front(new SocketWaitThread(mDataCrit));
			mThreadList.front()->Start();
			mThreadList.front()->AddSocket(aSocket);
		}
	}

	aData = (SocketWaitThread::SocketData *)aSocket->GetThreadData();
	aData->mOps.insert(theOp);
	theOp->mInSocketThread = true;
	SetEvent(aData->mEvent);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadWinsock2::RemoveSocketOp(SocketOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	AsyncSocket* aSocket = theOp->GetSocket();
	SocketWaitThread::SocketData* aData = (SocketWaitThread::SocketData *)aSocket->GetThreadData();
	theOp->mInSocketThread = false;
	if(aData!=NULL)
	{
		aData->mOps.erase(theOp);

//		if(aData->mOps.empty())
//			aData->mThread->RemoveSocket(aSocket);
	}	
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadWinsock2::Notify(AsyncSocket *theSocket)
{
	SocketWaitThread::SocketData* aData = (SocketWaitThread::SocketData *)theSocket->GetThreadData();
	if(aData==NULL)
		return;

	SocketWaitThread::OpList::iterator anItr = aData->mOps.begin();
	while(anItr!=aData->mOps.end())
	{
		if(DoOp(*anItr))
			aData->mOps.erase(anItr++);
		else
			++anItr;
	}
}

