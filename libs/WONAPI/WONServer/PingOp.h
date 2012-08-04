#ifndef __WON_PINGOP_H__
#define __WON_PINGOP_H__
#include "WONShared.h"

#include "ServerRequestOp.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PingOp : public ServerRequestOp
{
private:
	bool mExtended;

public:
	DWORD mLag;
	std::string mAppName;
	std::string mLogicalName;
	std::string mImage;
	std::string mPorts;
	std::string mRegData;
	std::string mServData;
	time_t mVersion;
	long mPID;
	bool mIsAuth;

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

public:
	PingOp(const IPAddr &theAddr, unsigned char theLengthFieldSize, bool extended = false);

	void SetExtended(bool isExtended) { mExtended = isExtended; }
	bool GetExtended() { return mExtended; }

};

typedef SmartPtr<PingOp> PingOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
