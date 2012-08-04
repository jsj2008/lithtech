#ifndef __WON_CREATEACCOUNTOP_H__
#define __WON_CREATEACCOUNTOP_H__
#include "WONShared.h"

#include "WONServer/ServerRequestOp.h"
#include "ProfileData.h"
#include "WONCrypt/ElGamal.h"
#include "WONCrypt/Blowfish.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CreateAccountOp : public ServerRequestOp
{
private:
	ProfileDataPtr mProfileData;
	std::string mErrorString;
	ElGamal mPubKey;
	Blowfish mSymKey;

	enum
	{
		State_GettingPubKey,
		State_SettingSessionKey,
		State_CreatingAccount
	} mState;

	// User account fields (BirthDate is part of ProfileData)
	std::wstring mUserName;
	std::wstring mPassword;
	std::string mEmail;

	void Init();


protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

public:
	CreateAccountOp(ServerContext *theProfileContext);
	CreateAccountOp(const IPAddr &theAddr);

	// Main operations
	void SetProfileData(ProfileData *theData) { mProfileData = theData; }
	void SetUserName(const std::wstring &theUserName) { mUserName = theUserName; }
	void SetPassword(const std::wstring &thePassword) { mPassword = thePassword; }
	void SetEmail(const std::string &theEmail) { mEmail = theEmail; }

	// Accessors
	const std::wstring& GetUserName() const { return mUserName; }
	const std::wstring& GetPassword() const { return mPassword; }
	const std::string& GetEmail() const { return mEmail; }
	ProfileData* GetProfileData() { return mProfileData; }

	const std::string& GetErrorString() const { return mErrorString; }

};

typedef SmartPtr<CreateAccountOp> CreateAccountOpPtr;


}; // namespace WONAPI

#endif
