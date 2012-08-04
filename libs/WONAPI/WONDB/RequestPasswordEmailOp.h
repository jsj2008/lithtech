
#ifndef __REQUESTPASSWORDEMAILOP_H__
#define __REQUESTPASSWORDEMAILOP_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "WONDB/DBProxyOp.h"					// Base Class
#include "WONDB/ProfileData.h"

namespace WONAPI 
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// This op is called when a user is requesting his/her password to be emailed to them.
class RequestPasswordEmailOp: public DBProxyOp
{
	// Attributes
protected:
	std::wstring mUserName;
	ProfileDataPtr mProfileData;

	// Operations
public:
	void SetUserName(const std::wstring& theUserName)	{ mUserName = theUserName;  }
	const std::wstring& GetUserName()					{ return mUserName;			}
	void SetProfileData(ProfileData *theData)			{ mProfileData = theData;	}
	ProfileData* GetProfileData()						{ return mProfileData;		}

protected:
	void Init();

	// Overrides
public:
	const unsigned short mMsgType;				// DO NOT MODIFY (MUST MATCH SERVER)
	//enum MSGTYPE { MSGTYPE = 8 };				// DO NOT MODIFY (MUST MATCH SERVER)
	virtual WONStatus CheckResponse();			// Recv - For Extended Unpack
	virtual void	  RunHook();				// Send - For Extended Pack

	// Constructor
public:
	RequestPasswordEmailOp(ServerContext* theContext);			// Requires List of DBProxy servers
	RequestPasswordEmailOp(const IPAddr &theAddr);				// or addr of a single server

	// Destructor
protected:
	virtual ~RequestPasswordEmailOp() {}			// Safety Lock -- Enforces Smart Ptr Use

};
typedef SmartPtr<RequestPasswordEmailOp> RequestPasswordEmailOpPtr;		// Smart Ptr Template


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
} // namespace WONAPI


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // #ifndef __REQUESTPASSWORDEMAILOP_H__
