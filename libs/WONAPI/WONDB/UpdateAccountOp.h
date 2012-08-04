#ifndef __WON_UPDATEACCOUNTOP_H__
#define __WON_UPDATEACCOUNTOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "WONDB/ProfileData.h"
#include "WONCrypt/ElGamal.h"
#include "WONCrypt/Blowfish.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class UpdateAccountOp : public ServerRequestOp
{
private:
	ProfileDataPtr mProfileData;
	std::string mErrorString;
	
	std::wstring mPassword;
	std::string  mEmail;

	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

public:
	UpdateAccountOp(ServerContext *theProfileContext, AuthContext* theAuthContext);
	UpdateAccountOp(const IPAddr &theAddr, AuthContext* theAuthContext);

	void SetProfileData(ProfileData *theData) { mProfileData = theData; }
	void SetPassword(const std::wstring &thePassword) { mPassword = thePassword; }
	void SetEmail(const std::string &theEmail) { mEmail = theEmail; }

	const std::wstring& GetPassword() const { return mPassword; }
	const std::string& GetEmail() const { return mEmail; }
	ProfileData* GetProfileData() { return mProfileData; }

	const std::string& GetErrorString() const { return mErrorString; }

};

typedef SmartPtr<UpdateAccountOp> UpdateAccountOpPtr;


}; // namespace WONAPI

#endif
