
#ifndef __REQUESTACCOUNTINFOEMAILOP_H__
#define __REQUESTACCOUNTINFOEMAILOP_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "WONDB/DBProxyOp.h"					// Base Class
#include "WONDB/ProfileData.h"

namespace WONAPI 
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// This op is called when a user is requesting his/her AccountInfo to be emailed to them.
class RequestAccountInfoEmailOp: public DBProxyOp
{
	// Attributes
protected:
	std::string mEmail;
	ProfileDataPtr mProfileData;

	// Operations
public:
	void SetEmailAddress(const std::string& theEmailAddr) { mEmail = theEmailAddr;	}
	const std::string& GetEmailAddr()					  { return mEmail;			}
	void SetProfileData(ProfileData *theData)			  { mProfileData = theData;	}
	ProfileData* GetProfileData()						  { return mProfileData;	}

protected:
	void Init();

	// Overrides
public:
	const unsigned short mMsgType;				// DO NOT MODIFY (MUST MATCH SERVER)
	//enum MSGTYPE { MSGTYPE = 14 };				// DO NOT MODIFY (MUST MATCH SERVER)
	virtual WONStatus CheckResponse();			// Recv - For Extended Unpack
	virtual void	  RunHook();				// Send - For Extended Pack

	// Constructor
public:
	RequestAccountInfoEmailOp(ServerContext* theContext);			// Requires List of DBProxy servers
	RequestAccountInfoEmailOp(const IPAddr &theAddr);				// or addr of a single server

	// Destructor
protected:
	virtual ~RequestAccountInfoEmailOp() {}			// Safety Lock -- Enforces Smart Ptr Use

};
typedef SmartPtr<RequestAccountInfoEmailOp> RequestAccountInfoEmailOpPtr;		// Smart Ptr Template


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
} // namespace WONAPI


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // #ifndef __REQUESTACCOUNTINFOEMAILOP_H__
