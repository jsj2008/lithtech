#define __WON_MASTER_CPP__

#include "WONAPI.h"
#include "WONCommon/AsyncOpContainer.h"
#include "WONCommon/TimerThread.h"
#include "WONSocket/AsyncSocket.h"
#include "WONSocket/SocketThreadSimple.h"
#include "WONSocket/SocketThreadEx.h"
#include "WONCommon/Platform.h"

#ifdef _MSC_VER
#include <string>
#endif


using namespace WONAPI;

WONAPICore* WONAPICore::mInstance = NULL;
bool WONAPICore::mHaveWrittenVersion = false;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::Init()
{
	assert(mInstance==NULL);

	mVersionString = "wonapi121401";

	mInstance = this;
	mIsStarted = false;
	
	mDoPumpThread = false;
	mDoSocketAndTimerThreads = true;
	mSocketThread = NULL;

	AsyncSocket::StartWinsock();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONAPICore::WONAPICore(bool doPumpThread, bool doSocketAndTimerThreads, bool useSocketThreadEx)
{
	Init();

	mDoPumpThread = doPumpThread;
	mDoSocketAndTimerThreads = doSocketAndTimerThreads;
	if(useSocketThreadEx)
		mSocketThread = new SocketThreadEx;
	else
		mSocketThread = new SocketThreadSimple;

	Startup();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONAPICore::WONAPICore(const char*) // work around, need other constructor for WONAPICoreEx to call
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONAPICore::~WONAPICore()
{
	Shutdown();

	delete mSocketThread;
	AsyncSocket::StopWinsock();

	mInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::WriteVersion()
{
	if(mHaveWrittenVersion) // don't need to write the version multiple times
		return;

	mHaveWrittenVersion = true;
	std::string aVersionFilePath = WONAPICore::GetDefaultFileDirectory() + "_wonapiversion.txt";

	// Only write version file if it exists, this way we don't always clutter the
	// _wonapiversion.txt everywhere.  We can simply check the version by putting 
	// a blank file called _wonapiversion.txt in the directory.
	FILE* aVersionFileP = fopen(aVersionFilePath.c_str(), "r"); 
	if (aVersionFileP != NULL)
	{
		fclose(aVersionFileP);
		aVersionFileP = fopen(aVersionFilePath.c_str(), "w");
		if(aVersionFileP!=NULL)
		{
			fprintf(aVersionFileP,"%s\n",GetVersionString().c_str());
			fclose(aVersionFileP);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::Startup()
{
	if(mIsStarted) // Shouldn't start again if already started
		return;

	mIsStarted = true;

	// Write API version string to file if necessary
	WriteVersion();

	// Initialize socket thread
	if(mSocketThread==NULL) // make sure we have some sort of socket handling
		mSocketThread = new SocketThreadSimple;

	// Some socket threads behave differently in single threaded mode than in multi-threaded mode.
	// For instance SocketThreads which act as managers and spawn off other threads can no longer
	// do this in single threaded mode.
	mSocketThread->SetSingleThreaded(!mDoSocketAndTimerThreads); 

	// Start up the necessary threads
	if(mDoSocketAndTimerThreads) 
	{
		if(mSocketThread->NeedThread())
			mSocketThread->Start();

		mTimerThread.Start();
	}

	if(mDoPumpThread)
		mPumpThread.Start();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::Shutdown()
{
	if(!mIsStarted) // Shouldn't shutdown if not started
		return;

	mIsStarted = false;

	// Signal threads to stop.  Signal them all at the same time. I think this should be 
	// faster than stopping and waiting for each one to stop individually.
	mPumpThread.Stop(false);
	mTimerThread.Stop(false);
	mSocketThread->Stop(false);

	mPumpThread.WaitForStop();
	mTimerThread.WaitForStop();
	mSocketThread->WaitForStop();

	// Now purge all ops 
	mTimerThread.PurgeOps();
	mSocketThread->PurgeOps();
	mOpContainer.PurgeOps();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool WONAPICore::IsStarted()
{
	return mInstance?mInstance->mIsStarted:false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::Pump(unsigned long theMilliseconds, int theMaxOps)
{
	DWORD aTick = GetTickCount();
	DWORD anElapsed = 0;

	while(true)
	{
		DWORD aWaitTime = theMilliseconds - anElapsed;

		if(mDoSocketAndTimerThreads)
			mOpContainer.Pump(aWaitTime, theMaxOps);
		else
		{
			DWORD aTimerWait = mTimerThread.GetWaitTime();
			if(aTimerWait < aWaitTime)
				aWaitTime = aTimerWait;

			mSocketThread->Pump(aWaitTime);
			mTimerThread.Pump();
			mOpContainer.Pump(0, theMaxOps);
		}

		anElapsed = GetTickCount() - aTick;
		if(anElapsed >= theMilliseconds)
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool WONAPICore::WaitTillNeedPump(unsigned long theMilliseconds)
{
	return mOpContainer.WaitForOps(theMilliseconds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const std::string& WONAPICore::GetDefaultFileDirectory()
{
	static std::string anEmptyStr;
	if(mInstance==NULL)
		return anEmptyStr;
	else
		return mInstance->mFileDirectory;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::QueueInContainer(AsyncOp *theOp)
{
	if(IsStarted())
		mInstance->mOpContainer.QueueOp(theOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::AddToTimerThread(AsyncOp *theOp, DWORD theTimeout)
{
	if(IsStarted())
		mInstance->mTimerThread.AddTimerOp(theOp,theTimeout);
	else
		theOp->Kill();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::RemoveFromTimerThread(AsyncOp *theOp)
{
	if(IsStarted())
		mInstance->mTimerThread.RemoveTimerOp(theOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::AddToSocketThread(SocketOp *theOp)
{
	if(IsStarted())
		mInstance->mSocketThread->AddSocketOp(theOp);
	else
		theOp->Kill();
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::RemoveFromSocketThread(SocketOp *theOp)
{
	if(IsStarted())
		mInstance->mSocketThread->RemoveSocketOp(theOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONAPICoreEx::WONAPICoreEx() : WONAPICore("") // workaround for default constructor problem
{
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICoreEx::SetDefaultFileDirectory(const char* theDirectory)
{
	if(!mIsStarted)
	{
		mFileDirectory = theDirectory;
		if(!mFileDirectory.empty())
		{
			char anEndChar = mFileDirectory[mFileDirectory.length()-1];
			if(anEndChar!='\\' && anEndChar!='/')
				mFileDirectory += '/';
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICoreEx::SetDoPumpThread(bool doPumpThread)
{
	if(!mIsStarted)
		mDoPumpThread = doPumpThread;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICoreEx::SetDoSocketAndTimerThreads(bool doSocketAndTimerThreads)
{
	if(!mIsStarted)
		mDoSocketAndTimerThreads = doSocketAndTimerThreads;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICoreEx::SetSocketThread(SocketThread *theThread)
{
	if(!mIsStarted)
	{
		if(mSocketThread!=NULL)
			delete mSocketThread;

		mSocketThread = theThread;
	}
	else
		delete theThread;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WONAPICore::PumpThread::ThreadFunc()
{
	WONAPICore *anAPI = WONAPICore::GetInstance();
	while(true)
	{
		anAPI->Pump(20);
		if(mStopped)
			return;
	}
}

