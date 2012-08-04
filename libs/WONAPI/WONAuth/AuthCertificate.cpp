#include "AuthCertificate.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthCertificate::~AuthCertificate()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthCertificate::UnpackHook(ReadBuffer &theData)
{
	if(!AuthBase::UnpackHook(theData))
		return false;

	mUserId = theData.ReadLong();
	mCommunityId = theData.ReadLong();
	mTrustLevel = theData.ReadShort();

	unsigned short aKeyLen = theData.ReadShort();
	if(!mPubKey.SetPublicKey(theData.ReadBytes(aKeyLen),aKeyLen))
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthCertificate::VerifyPermissions(DWORD theUserId, DWORD theCommunity, unsigned short theMinTrust) const
{
	if(theUserId!=0 && mUserId!=theUserId)
		return false;

	if(theCommunity!=0 && mCommunityId!=theCommunity)
		return false;

	return mTrustLevel>=theMinTrust;
}
