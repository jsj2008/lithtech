#ifndef __WON_WONAPI_H__
#define __WON_WONAPI_H__
#include "WONShared.h"

#include "WONCommon/TimerThread.h"
#include "WONCommon/AsyncOpContainer.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SocketThread;
class SocketOp;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Note: WONAPICore is deprecated.  Please use WONAPICoreEx instead.  Note that you must call
// the Startup() method when using WONAPICoreEx.
class WONAPICore
{
public:
	WONAPICore(bool doPumpThread = false, bool doSocketAndTimerThreads = true, bool useSocketThreadEx = false);
	virtual ~WONAPICore();

	virtual void Startup();
	virtual void Shutdown();

	void Pump(unsigned long theMilliseconds, int theMaxOps = 0);
	bool WaitTillNeedPump(unsigned long theMilliseconds);

	static const std::string& GetDefaultFileDirectory();				// Get dir for API files
	const std::string& GetVersionString()	{ return mVersionString; }	// Get API Version string

	static WONAPICore* GetInstance() { return mInstance; }
	static bool IsStarted();

protected:
	WONAPICore(const char*); // work around: need other constructor for WONAPICoreEx to call
	void Init();

	friend class AsyncOp;
	static void QueueInContainer(AsyncOp *theOp);
	static void AddToTimerThread(AsyncOp *theOp, DWORD theTimeout);
	static void RemoveFromTimerThread(AsyncOp *theOp);

	friend class SocketOp;
	static void AddToSocketThread(SocketOp *theOp);
	static void RemoveFromSocketThread(SocketOp *theOp);

	static WONAPICore *mInstance;
	AsyncOpContainer mOpContainer;
	TimerThread mTimerThread;
	SocketThread *mSocketThread;

	bool	mIsStarted;
	bool	mDoSocketAndTimerThreads;
	bool	mDoPumpThread;

	static bool mHaveWrittenVersion;
	std::string mVersionString;
	void WriteVersion();

	std::string mFileDirectory;

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	class PumpThread : public Thread
	{
	private:
		virtual void ThreadFunc();	
	};
	PumpThread mPumpThread;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Note that you must call
// the Startup() method when using WONAPICoreEx.
class WONAPICoreEx : public WONAPICore
{
public:
	// Set methods only work if the API is not currently started.
	// So call your set methods and then call WONAPICore::Startup.
	void SetDefaultFileDirectory(const char *theDirectory);	// Set dir for API files
	void SetDoPumpThread(bool doPumpThread);
	void SetDoSocketAndTimerThreads(bool doSocketAndTimerThreads);
	void SetSocketThread(SocketThread *theThread);			// takes ownership

	// Constructor
public:
	WONAPICoreEx();
};


}; // namespace WONAPI

#endif
