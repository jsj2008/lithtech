#ifndef __WON_GETMOTDOP_H__
#define __WON_GETMOTDOP_H__
#include "WONShared.h"

#include "WONCommon/AsyncOpTracker.h"
#include "WONCommon/ByteBuffer.h"
#include "HTTPGetOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetMOTDOp : public AsyncOpWithTracker
{
private:
	std::string mProduct;
	std::string mExtraConfig;

	bool mMOTDIsNew[2];
	bool mTriedDefault[2];
	WONStatus mMOTDStatus[2];
	int mHTTPStatus[2];
	std::string mMOTDPath[2];

	enum MOTDType
	{
		MOTD_Sys = 0,
		MOTD_Game = 1
	};

	virtual void RunHook();
	virtual bool CallbackHook(AsyncOp *theOp, int theParam);
	void DoFinish();

	ByteBufferPtr GetMOTD(MOTDType theType) const;
	void OpFinished(HTTPGetOp *theOp, int theMOTDType);
	void TryHTTPOp(MOTDType theType, bool forceDefault = false);

private:
	static std::string mHost;
	static std::string mPath;
	static void CheckHostAndPath();

public:
	GetMOTDOp(const std::string &theProduct);

	void SetExtraConfig(const std::string &theExtraConfig) { mExtraConfig = theExtraConfig; }
	void SetSysMOTDPath(const std::string &thePath) { mMOTDPath[MOTD_Sys] = thePath; }
	void SetGameMOTDPath(const std::string &thePath) { mMOTDPath[MOTD_Game] = thePath; }

	const std::string& GetSysMOTDPath() const { return mMOTDPath[MOTD_Sys]; }
	const std::string& GetGameMOTDPath() const { return mMOTDPath[MOTD_Game]; }

	bool SysMOTDIsNew() const { return mMOTDIsNew[MOTD_Sys]; }
	bool GameMOTDIsNew() const { return mMOTDIsNew[MOTD_Game]; }

	ByteBufferPtr GetSysMOTD() const { return GetMOTD(MOTD_Sys); }
	ByteBufferPtr GetGameMOTD() const { return GetMOTD(MOTD_Game); }	
};

typedef SmartPtr<GetMOTDOp> GetMOTDOpPtr;

}; // namespace WONAPI


#endif
