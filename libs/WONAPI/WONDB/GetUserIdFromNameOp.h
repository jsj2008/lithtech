#ifndef __WON_GETUSERIDFROMNAMEOP_H__
#define __WON_GETUSERIDFROMNAMEOP_H__
#include "WONShared.h"

#include "WONDB/DBProxyOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetUserIdFromNameOp : public DBProxyOp
{
	// Attributes
private:
	std::wstring    mUserName;	// [in]	user name to query for
	unsigned long	mUserId;	// [out]	users wonuserseq

	// Overrides
protected:
	virtual WONStatus CheckResponse();
	virtual void RunHook();
	virtual void Reset();

	// Operations
public:
	GetUserIdFromNameOp(ServerContext *theDBProxyContext);
	GetUserIdFromNameOp(const IPAddr &theAddr);


	void SetUserName(const std::wstring& theUserName) { mUserName = theUserName; }

	std::wstring  GetUserName() { return mUserName;	}
	unsigned long GetUserId()   { return mUserId;	}


	// Destructor enforces smartPtr use
protected:
	~GetUserIdFromNameOp();

};

typedef SmartPtr<GetUserIdFromNameOp> GetUserIdFromNameOpPtr;


}; // namespace WONAPI

#endif // __WON_GETUSERNAMEFROMIDOP_H__
