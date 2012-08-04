#include "AuthPubKeyBlock.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthPubKeyBlock::~AuthPubKeyBlock()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthPubKeyBlock::UnpackHook(ReadBuffer &theData)
{
	if(!AuthBase::UnpackHook(theData))
		return false;

	mKeyBlockId = theData.ReadShort();
	unsigned short aNumKeys = theData.ReadShort();
	if(aNumKeys==0)
		return false;
	
	mKeyBlock.clear();
		
	for(int i=0; i<aNumKeys; i++)
	{
		unsigned short aKeyLen = theData.ReadShort();
		ElGamal aKey;
		if(!aKey.SetPublicKey(theData.ReadBytes(aKeyLen),aKeyLen))
			return false;
								
		mKeyBlock.push_back(aKey);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool AuthPubKeyBlock::Verify(const AuthBase* theBase) const
{
	if(!IsValid())
		return false;

	if(!theBase->IsValid())
		return false;

	KeyBlock::const_iterator anItr = mKeyBlock.begin();
	while(anItr!=mKeyBlock.end())
	{
		if(theBase->Verify(*anItr))
			return true;

		++anItr;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthPubKeyBlock::IsValidAndVerified(const ElGamal &theVerifierKey) const
{
	return IsValid() && theVerifierKey.Verify(GetData(),GetDataLen(),GetSig(),GetSigLen());
}


