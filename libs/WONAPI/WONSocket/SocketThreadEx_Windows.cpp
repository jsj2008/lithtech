#include "SocketThreadEx_Windows.h"
using namespace WONAPI;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
SocketThreadEx::SocketThreadEx()
{
	AsyncSocket::StartWinsock();
	mSignalSocket = new AsyncSocket(AsyncSocket::UDP);

	// Bind SignalSocket and connect it to itself
	IPAddr anAddr = IPAddr::GetLocalAddr();
	anAddr.SetThePort(0); // choose random port
	mSignalSocket->Bind(anAddr);
	anAddr.SetThePort(mSignalSocket->GetLocalPort());
	mSignalSocket->Connect(anAddr);

	for(int i=0; i<3; i++)
	{
		mSocketArray[i].reserve(64);
		mOpArray[i].reserve(64);
	}

	mSocketArray[SocketEvent_Read].push_back(mSignalSocket->GetDescriptor());
	mOpArray[SocketEvent_Read].push_back(NULL);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
SocketThreadEx::~SocketThreadEx()
{
	PurgeOps();
	AsyncSocket::StopWinsock();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::PurgeOps()
{
	for(int i=0; i<3; i++)
	{
		OpMap::iterator anItr = mOpMap[i].begin();
		while(anItr!=mOpMap[i].end())
		{
			SocketOp *anOp = anItr->second;
			if(anOp->mInSocketThread)
			{
				anOp->mInSocketThread = false;
				anOp->Kill();
			}

			++anItr;
		}

		mOpMap[i].clear();
		if(i!=SocketEvent_Read)
		{
			mSocketArray[i].clear();
			mOpArray[i].clear();
		}
	}	
	
	// Leave signal socket
	mSocketArray[SocketEvent_Read].resize(1);
	mOpArray[SocketEvent_Read].resize(1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::AddSocketOp(SocketOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	SOCKET aSocket = theOp->GetSocket()->GetDescriptor();
	for(int i=0; i<3; i++)
	{
		if(theOp->NeedSocketEvent((SocketEventType)i))
		{
			std::pair<OpMap::iterator,bool> aRet;
			aRet = mOpMap[i].insert(OpMap::value_type(aSocket,theOp));
			if(!aRet.second) // old op still here
			{
				aRet.first->second->Kill();
				RemoveSocketOp(aRet.first->second,i);
				aRet.first->second = theOp;
			}

			mOpMap[i][aSocket] = theOp;

			theOp->mArrayPos[i] = mSocketArray[i].size();
			mSocketArray[i].push_back(aSocket);
			mOpArray[i].push_back(theOp);
		}
	}

	theOp->mInSocketThread = true;
	Signal();
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::RemoveSocketOp(SocketOp *theOp)
{
	RemoveSocketOp(theOp, -1);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::RemoveSocketOp(SocketOp *theOp, int dontErase)
{
	AutoCrit aCrit(mDataCrit);
	SocketOpPtr anOp = theOp;
	for(int i=0; i<3; i++)
	{
		if(theOp->NeedSocketEvent((SocketEventType)i))
		{
			int aPos = theOp->mArrayPos[i];
			if(aPos>=0)
			{
				if(dontErase!=i)
					mOpMap[i].erase(mSocketArray[i][aPos]);

				int aSize = mSocketArray[i].size();
				
				if(aPos!=aSize-1)
				{
					swap(mSocketArray[i][aPos],mSocketArray[i][aSize-1]);
					swap(mOpArray[i][aPos],mOpArray[i][aSize-1]);

					mOpArray[i][aPos]->mArrayPos[i] = aPos;
				}

				mSocketArray[i].pop_back();
				mOpArray[i].pop_back();
				theOp->mArrayPos[i] = -1;
			}
		}
	}

	theOp->mInSocketThread = false;
	Signal();
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::Signal()
{
	static char aBuf = 1;
	mSignalSocket->SendBytes(&aBuf,1);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::Pump(DWORD theWaitTime)
{

	AutoCrit aCrit(mDataCrit);
	int i,j;

	for(i=0; i<3; i++)
		m_fdset[i].Copy(mSocketArray[i]);

	aCrit.Leave();
	
	timeval timeoutval;
	timeoutval.tv_sec = theWaitTime/1000;
	timeoutval.tv_usec = (theWaitTime%1000)*1000;

	int aVal = select(0, m_fdset[0].mSet, m_fdset[1].mSet, m_fdset[2].mSet, &timeoutval);

	if(aVal>0)
	{
		aCrit.Enter();
		OpMap::iterator anItr;
		for(i=0; i<3; i++)
		{
			fd_set *aSet = m_fdset[i].mSet;
			for(j=0; j<aSet->fd_count; j++)
			{
				anItr = mOpMap[i].find(aSet->fd_array[j]);
				if(anItr!=mOpMap[i].end())
				{
					if(DoOp(anItr->second))
					{
						RemoveSocketOp(anItr->second, i);
						mOpMap[i].erase(anItr);
					}
				}
			}
		}
		aCrit.Leave();

		static char aBuf;
		while(mSignalSocket->RecvBytes(&aBuf,1)==WS_Success)
		{
		}
	}
	else if(aVal<0)
	{
		aCrit.Enter();
		OpMap::iterator anItr;
		for(i=0; i<3; i++)
		{
			anItr = mOpMap[i].begin();
			while(anItr!=mOpMap[i].end())
			{
				if(DoOp(anItr->second))
				{
					RemoveSocketOp(anItr->second, i);
					mOpMap[i].erase(anItr++);
				}
				else
					++anItr;
			}
		}
		aCrit.Leave();
	}
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SocketThreadEx::ThreadFunc()
{
	while(!mStopped)
		Pump(1000);
}

