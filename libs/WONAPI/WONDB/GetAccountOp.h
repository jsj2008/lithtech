#ifndef __WON_GETACCOUNTOP_H__
#define __WON_GETACCOUNTOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "WONDB/ProfileData.h"
#include "WONCrypt/ElGamal.h"
#include "WONCrypt/Blowfish.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetAccountOp : public ServerRequestOp
{
	// Attributes
private:
	std::string    mEmail;
	std::string    mErrorString;
	ProfileDataPtr mProfileData;

	// Overrides
protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

	// Operations
public:
	void Init();

	GetAccountOp(ServerContext *theProfileContext, AuthContext* theAuthContext);
	GetAccountOp(const IPAddr &theAddr, AuthContext* theAuthContext);

	void SetProfileData(ProfileData *theData) { mProfileData = theData; }

	const std::string& GetEmail() const { return mEmail; }
	ProfileData* GetProfileData() { return mProfileData; }
	const std::string& GetErrorString() const { return mErrorString; }
};

typedef SmartPtr<GetAccountOp> GetAccountOpPtr;


}; // namespace WONAPI

#endif
