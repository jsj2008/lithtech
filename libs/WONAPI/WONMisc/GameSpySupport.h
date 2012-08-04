#ifndef __WON_GAMESPYSUPPORT_H__
#define __WON_GAMESPYSUPPORT_H__
#include "WONShared.h"

#include "WONCommon/SmartPtr.h"
#include "WONCommon/CriticalSection.h"
#include "WONSocket/IPAddr.h"
#include "WONCommon/AsyncOp.h"
#include "WONServer/ServerContext.h"

// Gamespy includes
#include "gqueryreporting.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum GameSpyQueryType
{
	GameSpyQueryType_Info		= 1,
	GameSpyQueryType_Rules		= 2,
	GameSpyQueryType_Players	= 3
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GameSpySupport: public RefCount
{
protected:
	qr_t	mQRInstance;					// out parameter from qr_init (identifies instance)

	// Server information
	IPAddr	mIP;					// qr_init

	typedef std::map<std::string, std::string>	KeyValueMap;
	KeyValueMap		mQueryMap[4];

	std::string		mSecretKey;				// (Gamespy) Secret key for validation
	bool			mStateChanged;			// has the mode changed since the last pump?
	
	bool			mInitialized;			// Has qr_init been called?

	WONStatus	mStatus;			// WONStatus form of last error
	int					mGameSpyStatus;		// Gamespy value for last error
	unsigned long		mAutoPumpTime;		// Timeout for auto pump timer

	AsyncOpPtr	mPumpTimerOp;
	CriticalSection mDataCrit;

	AsyncOpPtr mGetServiceOp;
	ServerContextPtr mDirServers;
	bool mDisabled;
	time_t mLastGetServiceTime;


	void DumpKeyValueMapToBuffer(KeyValueMap &theMap, char* outbuf, unsigned int maxlen);

	// Default Query Handlers
	static void HandleQueryBasic  (char*,int,void*);	// does not call a user callback
	static void HandleQueryInfo   (char*,int,void*);
	static void HandleQueryRules  (char*,int,void*);
	static void HandleQueryPlayers(char*,int,void*);
	//static void HandleQueryStatus(char*,int,void*); // just calls all other queries

	void	BeginAutoPump();
	static void PumpTimerCompletion(AsyncOpPtr, RefCountPtr); // Auto pumps completion signal

	void LaunchGetServiceOp();
	static void StaticGetServiceOpCompletion(AsyncOpPtr, RefCountPtr);
	void GetServiceOpCompletion(AsyncOp *theOp);

	//GameSpyCallback		mStatusCallback;
public:
	GameSpySupport(const char* aGameName, const char* aGameVersion, const char* aSecretKey, ServerContext *theDirServers);

	// Calls qr_init
	WONStatus Startup(const IPAddr& ServAddr);
	void	  Shutdown();	// sends status update with "gamemode" as "exiting"

	// Error codes
	WONStatus	GetStatus()			{ return mStatus;		 }
	int					GetGameSpyStatus()  { return mGameSpyStatus; }

	// Pump & Auto pump (calls process_queries)
	void	Pump();
	void	SetAutoPumpTime(unsigned long theMilliseconds);
	bool	IsAutoPumping()	{ return ( mInitialized == true && mAutoPumpTime != 0); }

	// Key Value accessors
	void SetKeyValue(GameSpyQueryType theType, const char* theKey, const char* theValue);
	const char* GetKeyValue(GameSpyQueryType theType, const char* theKey);
	bool EraseKeyValue(GameSpyQueryType theType, const std::string &theKey);
	void EraseAllKeyValues(GameSpyQueryType theType);

	// Lock the KeyValues while doing multiple updates in order to 
	// ensure consistent data is being sent in query replies.
	// Unlock when done modifying.
	void LockKeyValues();
	void UnlockKeyValues();

	// Set the game state
	void	SetGameMode(const char* aGameMode);

protected:
	~GameSpySupport();
};

typedef SmartPtr<GameSpySupport>	GameSpySupportPtr;
}; // namespace WONAPI


#endif // __WONGAMESPYSUPPORT_H__
