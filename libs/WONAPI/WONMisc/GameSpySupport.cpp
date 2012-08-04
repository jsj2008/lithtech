

#include "GameSpySupport.h"
#include "WONDir/GetServiceOp.h"
#include "WONCommon/StringUtil.h"

  
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GameSpySupport::GameSpySupport(const char* aGameName, const char* aGameVersion, const char* aSecretKey, ServerContext *theDirServers)
	:	mIP(""), mSecretKey(aSecretKey), mInitialized(false), 
		mStatus(WS_None), mAutoPumpTime(0),	mGameSpyStatus(0),
		mDirServers(theDirServers), mDisabled(false)
{
	// Set Basic KeyValues
	mQueryMap[0]["gamename"] = aGameName;
	mQueryMap[0]["gamever"] = aGameVersion;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GameSpySupport::~GameSpySupport()
{ }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GameSpySupport::Startup(const IPAddr& anIPAddr)
{
	// Store the server address
	mLastGetServiceTime = 0;
	mDisabled = true; // Start off disabled until first GetServiceOp returns
	mIP = anIPAddr;

	// Initialize the gamespy sdk
	mGameSpyStatus = qr_init(&mQRInstance, mIP.GetHostString().c_str(), mIP.GetPort(), 
						mQueryMap[0]["gamename"].c_str(), mSecretKey.c_str(), 
						HandleQueryBasic, HandleQueryInfo, HandleQueryRules, HandleQueryPlayers, this);

	// Success
	if (0 == mGameSpyStatus)
	{
		mInitialized = true;
		mStatus = WS_Success;

		// Should we autopump?
		BeginAutoPump();
	}
	else
	{
		// Handle error
		switch(mGameSpyStatus)
		{
		case E_GOA_WSOCKERROR:	//1
			mStatus = WS_GameSpySupport_WinSockError;
		case E_GOA_BINDERROR:	//2
			mStatus = WS_GameSpySupport_BindError;
		case E_GOA_DNSERROR:	//3
			mStatus = WS_GameSpySupport_DNSError;
		case E_GOA_CONNERROR:	//4
			mStatus = WS_GameSpySupport_ConnError;
		default:	// Unknown error, maybe should create new status WS_Unhandled
			mStatus = WS_GameSpySupport_UnhandledError;
		}
	} 

	return mStatus;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::Shutdown()
{
	// Kill the timer op
	SetAutoPumpTime(0);				// stop future pump timers

	AutoCrit aCrit(mDataCrit);
	if(mGetServiceOp.get()!=NULL)
	{
		mGetServiceOp->Kill();
		mGetServiceOp = NULL;
	}
	aCrit.Leave();

	// "exiting" causes gamespy to deregister
	SetGameMode("exiting");

	// Do a final pump to send state change to gamespy
	Pump();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::HandleQueryBasic(char *outbuf, int maxlen, void *pThat)
{
	GameSpySupport* pSupport = (GameSpySupport*)pThat;

	// Load all keys from mBasicQueryMap into the buffer
	pSupport->DumpKeyValueMapToBuffer(pSupport->mQueryMap[0], outbuf, maxlen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::HandleQueryInfo(char *outbuf, int maxlen, void *pThat)
{
	GameSpySupport* pSupport = (GameSpySupport*)pThat;

	// Load all keys from mBasicQueryMap into the buffer
	pSupport->DumpKeyValueMapToBuffer(pSupport->mQueryMap[GameSpyQueryType_Info], outbuf, maxlen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::HandleQueryRules(char *outbuf, int maxlen, void *pThat)
{
	GameSpySupport* pSupport = (GameSpySupport*)pThat;

	// Load all keys from mBasicQueryMap into the buffer
	pSupport->DumpKeyValueMapToBuffer(pSupport->mQueryMap[GameSpyQueryType_Rules], outbuf, maxlen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::HandleQueryPlayers(char *outbuf, int maxlen, void *pThat)
{
	GameSpySupport* pSupport = (GameSpySupport*)pThat;

	// Load all keys from mBasicQueryMap into the buffer
	pSupport->DumpKeyValueMapToBuffer(pSupport->mQueryMap[GameSpyQueryType_Players], outbuf, maxlen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Sends all info
/*
void GameSpySupport::HandleQueryStatus(char *outbuf, int maxlen, void *pThat)
{
	GameSpySupport* pSupport = (GameSpySupport*)pThat;

	// Add extra pre-callback functionality here
	// ...

	// Call the users installed callback
	if (pSupport->mStatusCallback != NULL)
		(pSupport->mStatusCallback)(outbuf, maxlen, pSupport->mUserParam);
}
*/


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::BeginAutoPump()
{
	AutoCrit aCrit(mDataCrit);

	if (mPumpTimerOp.get() != NULL)
	{
		mPumpTimerOp->Kill();
		mPumpTimerOp = NULL;
	}

	if (mAutoPumpTime != 0)
	{
		mPumpTimerOp = new AsyncOp();
		mPumpTimerOp->SetCompletion(new OpRefCompletion(PumpTimerCompletion,this));
		mPumpTimerOp->RunAsTimer(mAutoPumpTime);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::SetAutoPumpTime(unsigned long theMilliseconds)
{
	AutoCrit aCrit(mDataCrit);

	mAutoPumpTime = theMilliseconds;
	BeginAutoPump();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::PumpTimerCompletion(AsyncOpPtr theOp, RefCountPtr theParam)
{
	if (theOp.get()	== NULL)
		return;

	if (theOp->Killed())
		return;

	GameSpySupport *pThat = (GameSpySupport*)theParam.get();
	pThat->Pump();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::StaticGetServiceOpCompletion(AsyncOpPtr theOp, RefCountPtr theGameSpy)
{
	GameSpySupport *aGameSpy = (GameSpySupport*)theGameSpy.get();
	GetServiceOp *anOp = (GetServiceOp*)theOp.get();
	aGameSpy->GetServiceOpCompletion(anOp);
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::GetServiceOpCompletion(AsyncOp *theOp)
{
	AutoCrit aCrit(mDataCrit);

	if(theOp->Killed())
		return;

	if(theOp!=mGetServiceOp.get())
		return;

	mDisabled = theOp->GetStatus()==WS_DirServ_ServiceNotFound;
	mGetServiceOp = NULL;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::LaunchGetServiceOp()
{
	if(mGetServiceOp.get()!=NULL)
		mGetServiceOp->Kill();

	GetServiceOpPtr anOp = new GetServiceOp(mDirServers);
	anOp->SetPath(L"/GameSpy");
	std::string &aName = mQueryMap[0]["gamename"];
	anOp->SetName(StringToWString(aName));
	anOp->SetNetAddr(new ByteBuffer(aName.data(),aName.length()));
	anOp->SetCompletion(new OpRefCompletion(StaticGetServiceOpCompletion, this));
	mGetServiceOp = anOp;
	anOp->RunAsync(60000);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::Pump()
{
	AutoCrit aCrit(mDataCrit);

	// process queries
// Don't allow it to be disabled on the titan side of things.
//	if(!mDisabled)
	{
		qr_process_queries(mQRInstance);

		// Is a state changed heartbeat needed?
		if (mStateChanged)
		{
			qr_send_statechanged(mQRInstance);
			mStateChanged = false;
		}
	}

	// Is a pump timer needed?
	if (IsAutoPumping())
		mPumpTimerOp->RunAsTimer(mAutoPumpTime);

	// Check if GameSpy is disabled
	time_t aTime = time(NULL);
	if(aTime - mLastGetServiceTime >= 3600)
	{
		mLastGetServiceTime = aTime;
		LaunchGetServiceOp();
	}
}
		
		
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::SetGameMode(const char* aGameMode)
{
	SetKeyValue(GameSpyQueryType_Info, "gamemode", aGameMode);

	// Signal for a state-change heartbeat
	mStateChanged = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::LockKeyValues()
{
	mDataCrit.Enter();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::UnlockKeyValues()
{
	mDataCrit.Leave();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::SetKeyValue(GameSpyQueryType theType, const char* theKey, const char* theValue)
{
	AutoCrit aCrit(mDataCrit);
	mQueryMap[theType][theKey] = theValue;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char* GameSpySupport::GetKeyValue(GameSpyQueryType theType, const char* theKey)
{
	KeyValueMap::iterator anItr = mQueryMap[theType].find(theKey);
	if(anItr==mQueryMap[theType].end())
		return NULL;
	else
		return anItr->second.c_str();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool GameSpySupport::EraseKeyValue(GameSpyQueryType theType, const std::string &theKey)
{
	AutoCrit aCrit(mDataCrit);
	return mQueryMap[theType].erase(theKey)>0;
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::EraseAllKeyValues(GameSpyQueryType theType)
{
	AutoCrit aCrit(mDataCrit);
	mQueryMap[theType].clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GameSpySupport::DumpKeyValueMapToBuffer(KeyValueMap& theMap, char* outbuf, unsigned int maxlen)
{
	AutoCrit aCrit(mDataCrit);

	unsigned int pos = 0;
	unsigned int pairlength = 0;

	KeyValueMap::iterator anItr = theMap.begin();
	for (; anItr != theMap.end(); ++anItr)
	{
		pairlength = anItr->first.length() + anItr->second.length() + 2; // add 2 for the seperating '\'
		
		if (pairlength < (maxlen-pos))
			sprintf(outbuf+pos, "\\%s\\%s", anItr->first.c_str(), anItr->second.c_str());

		pos += pairlength;
	}
}
