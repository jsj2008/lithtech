#include "WONAPI.h"
#include "AuthContext.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/MD5.h"
#include "GetCertOp.h"

using namespace std;
using namespace WONAPI;

static CriticalSection& GetVerifierCrit()
{
	static CriticalSection aVerifierCrit;
	return aVerifierCrit;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::Init()
{
	mSessionCleanItr = mSessionMap.begin();

	mPeerData = new AuthPeerData;
	mLoginSecretFile  = WONAPICore::GetDefaultFileDirectory() + "_wonlogin.ks";
	mServerContext = new ServerContext;

	mDoAutoRefresh = false;
	mRefreshStatus = WS_None;

	AutoCrit aCrit(GetVerifierCrit());
	if(mCheckedVerifierFile)
		return;

	LoadVerifierKey( WONAPICore::GetDefaultFileDirectory() + "_wonkver.pub");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthContext::AuthContext() 
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthContext::AuthContext(const std::wstring &theUserName, const std::wstring& theCommunity,
						 const std::wstring &thePassword) : 
	mUserName(theUserName),  
	mPassword(thePassword)
{
	Init();
	AddCommunity(theCommunity);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthContext::~AuthContext()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::StaticGetCertCallback(AsyncOpPtr theOp, RefCountPtr theParam)
{
	GetCertOp *anOp = (GetCertOp*)theOp.get();

	AuthContext *aContext = (AuthContext*)theParam.get();
	aContext->GetCertCallback(anOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::GetCertCallback(GetCertOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	if(theOp!=mGetCertOp.get())
		return;

	AuthPeerDataPtr aPeerData = theOp->GetPeerData();

	RefreshSet::iterator anItr = mRefreshSet.begin();
	while(anItr!=mRefreshSet.end())
	{
		LightGetCertOp *aGetCert = *anItr;
		aGetCert->SetPeerData(aPeerData);
		aGetCert->Finish(theOp->GetStatus());
		++anItr;
	}

	mRefreshSet.clear();
	mGetCertOp = NULL;
	mRefreshStatus = theOp->GetStatus();
	StartAutoRefresh(true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::StaticAutoRefreshCallback(AsyncOpPtr theOp, RefCountPtr theParam)
{
	AuthContext *aContext = (AuthContext*)theParam.get();
	aContext->AutoRefreshCallback(theOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AutoRefreshCallback(AsyncOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	if(theOp!=mAutoRefreshOp.get())
		return;

	mAutoRefreshOp = NULL;
	if(theOp->Killed())
		return;

	RefreshAsync();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::StopAutoRefresh()
{
	AutoCrit aCrit(mDataCrit);
	if(mAutoRefreshOp.get()!=NULL)
	{
		mAutoRefreshOp->Kill();
		mAutoRefreshOp = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::StartAutoRefresh(bool force)
{
	AutoCrit aCrit(mDataCrit);
	StopAutoRefresh();
	if(mDoAutoRefresh)
	{
		bool doRefreshTimer = force;

		time_t aRefreshWaitTime = 60*5; // try again in five minutes by default
		if(mPeerData->IsValid())
		{
			aRefreshWaitTime = mPeerData->GetExpireDelta();
			aRefreshWaitTime-=120;
			if(aRefreshWaitTime<120)
				aRefreshWaitTime=120;

			doRefreshTimer = true;
		}

		if(doRefreshTimer)
		{
			mAutoRefreshOp = new AsyncOp;
			mAutoRefreshOp->SetCompletion(new OpRefCompletion(StaticAutoRefreshCallback,this));
			mAutoRefreshOp->RunAsync(aRefreshWaitTime*1000);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetDoAutoRefresh(bool doAutoRefresh)
{
	AutoCrit aCrit(mDataCrit);
	mDoAutoRefresh = doAutoRefresh;
	StartAutoRefresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::RefreshAsync(LightGetCertOp *theOp, DWORD theTimeout)
{
	AutoCrit aCrit(mDataCrit);	
	StopAutoRefresh();
	if(theOp!=NULL) 
	{
		mRefreshSet.insert(theOp);
		theOp->RunAsync(theTimeout);
	}

	if(mGetCertOp.get()==NULL)
	{
		mGetCertOp = new GetCertOp(this);

		mGetCertOp->SetCompletion(new OpRefCompletion(StaticGetCertCallback,this));
		mRefreshStatus = mGetCertOp->Run(OP_MODE_ASYNC,60000);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus AuthContext::RefreshBlock(DWORD theTimeout)
{
	GetCertOpPtr anOp;
	anOp = new GetCertOp(this);
	mRefreshStatus = anOp->Run(OP_MODE_BLOCK,theTimeout);
	if(mRefreshStatus==WS_Success)
		StartAutoRefresh();

	return mRefreshStatus;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::PutSession(const IPAddr &theAddr, AuthSession *theSession)
{
	AutoCrit aCrit(mDataCrit);

	if(theSession==NULL)
		return;

	SessionStruct &aStruct = mSessionMap[theAddr];

	time_t aTime = time(NULL);
	aStruct.mLastUseTime = aTime;
	aStruct.mSessionList.push_back(theSession);

	// Clean the sessions a bit 
	for(int i=0; i<5; i++)
	{
		if(mSessionCleanItr==mSessionMap.end())
		{
			mSessionCleanItr = mSessionMap.begin();
			break;
		}

		if(aTime - mSessionCleanItr->second.mLastUseTime > 300)
			mSessionMap.erase(mSessionCleanItr++);
		else
			++mSessionCleanItr;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthSessionPtr AuthContext::GetSession(const IPAddr &theAddr)
{
	AutoCrit aCrit(mDataCrit);

	SessionMap::iterator aMapItr = mSessionMap.find(theAddr);
	if(aMapItr==mSessionMap.end())
		return NULL;


	time_t aTime = time(NULL);

	SessionStruct &aStruct = aMapItr->second;
	aStruct.mLastUseTime = aTime;
	SessionList &aList = aStruct.mSessionList;
	SessionList::iterator anItr = aList.begin();
	while(anItr!=aList.end())
	{
		AuthSessionPtr aSession = *anItr;
		aList.erase(anItr++);
		if(aTime - aSession->GetLastUseTime() < 300)
			return aSession;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::Kill()
{
	AutoCrit aCrit(mDataCrit);
	if(mGetCertOp.get()!=NULL)
		mGetCertOp->Kill();

	StopAutoRefresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////       Login Data                    ////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char VerifierKeyBuf[] = 
	"\x30\x82\x03\x2c\x02\x82\x01\x01\x00\x86\x18\x0e\x3d\xd4\x9e\xfa\x42\xb9\x4c\xb3\x3e"
	"\xf7\x2b\x76\xce\xe6\xb0\x27\x8c\x64\x07\x93\x3b\x24\x64\x6c\x47\xb6\x9e\x51\x7b\xbf"
	"\xc9\xd5\xa1\xfb\xcf\x04\xfa\x7b\x8e\xc3\x6e\xd1\x88\xf6\x2e\xbd\x2b\x29\xda\xb5\x76"
	"\x66\x12\xb9\x7a\x64\x73\x19\x53\xa9\x78\xd8\x1e\xe5\x11\x0a\x23\x1c\x08\x21\x33\xad"
	"\x65\x76\x48\xff\xc5\x0b\xa2\xaa\x10\x22\xcb\x63\xbb\x68\xfc\xf0\x15\xf3\xe9\x90\x5f"
	"\x9d\x17\x8e\x77\x67\xec\xde\xdf\x8b\xcc\xbb\xa4\xd6\x06\xfa\x0e\xd8\xc0\x67\xd4\x36"
	"\xc9\x3a\xeb\x3f\xa4\x93\x57\xad\x62\x29\x24\x08\x67\x91\x6a\x0b\xd4\xdc\xad\xa4\x86"
	"\x85\x9f\xed\xf8\xb8\xda\xe1\xac\xf3\xf4\x44\xaf\x85\x8d\x21\x95\x6a\xf4\x6e\xac\x36"
	"\x83\x8f\x70\x0a\xc6\x17\xc1\x90\xc4\x7f\xbb\xe8\x18\xed\x4d\x0b\x8c\xdc\x76\x7f\xa9"
	"\xb2\x45\x0f\xc6\xa8\xca\x0a\x9f\xec\x9f\xe5\xe0\xbd\x71\x63\x06\xef\x32\xba\xe0\x40"
	"\x47\xf1\x91\xa8\xbf\x5e\xad\xb8\x2d\x28\x3a\x94\x63\xfa\x85\x1d\x24\xe3\xd2\x78\x7f"
	"\x0e\x85\xb1\xbb\x3f\x2d\x27\x16\x34\xcf\x94\x7d\x71\xdd\xf3\xec\x41\xc1\x27\xe5\x6f"
	"\xe6\x88\x78\xc8\x65\x15\x45\x3c\x0c\x5f\x30\x60\xc7\x02\x1d\x02\xe1\x4f\x25\x13\x97"
	"\xee\xdb\x46\x94\xb2\x54\xfc\x54\x26\x20\xa2\x6d\x57\x72\xdd\x47\xd2\xf0\x92\x00\x38"
	"\x51\xad\x02\x82\x01\x00\x72\x44\x9d\x49\x40\xff\x84\x75\xd8\x73\x57\x1f\xa7\xff\x52"
	"\x08\x82\x63\xfa\xd1\xa7\x30\xb8\x63\xc5\x10\x45\x8e\x12\xca\x9c\x36\x4f\xc8\x3e\xe8"
	"\x9a\x95\xfc\x48\xae\xcf\x80\xea\x75\xf6\x11\x3b\x2a\xf0\xd9\x52\xfb\x66\xee\x4b\xdb"
	"\x6c\xc4\x06\x28\x12\x28\xd2\x2c\x53\x3f\x08\x86\x89\x69\x6a\x89\x57\x7e\xb0\xd7\x81"
	"\xac\x79\xae\xfb\x56\xc0\x24\x0d\x84\x5a\xe1\x22\xc8\x3f\xba\x3c\x77\xbc\x22\x56\xdf"
	"\xf1\x78\x55\xcc\xd5\x97\x88\x00\x71\x1a\xe8\x8e\xa7\xb6\x87\x62\x55\x4d\x53\x87\xa2"
	"\x49\x3e\xa2\x79\x99\x51\x94\xd7\xe2\xf5\xce\x37\x1e\xc7\x79\xbd\xea\x0a\x31\x56\x1d"
	"\x56\x1a\x8c\xb7\xf3\xa5\xdc\x2a\xcd\xdc\xb1\x5c\x3e\x47\x20\xc7\xc2\x30\x32\xdf\x64"
	"\x91\xd5\xc6\xab\x85\x21\x67\x59\xc6\x09\x7c\x61\xd5\x90\x9b\xff\x5c\x6b\x48\x5e\x41"
	"\x85\xdb\x12\xed\x63\x04\xd3\x35\xca\x86\xce\x64\xa6\x54\x5b\x0f\x77\x90\x7e\xad\xd9"
	"\x56\x5d\x2f\x8a\x52\xd9\xcc\x79\x41\x69\x73\xca\x15\x0a\xbc\x68\x93\xbe\x2f\x03\x7c"
	"\x60\x7b\xf2\xa3\x03\x98\x34\x36\x9f\x88\x8e\xfb\x2b\x8a\x40\xdd\x35\xa9\x21\x2d\x92"
	"\xba\x1b\xe8\x58\xe1\x1c\xc2\x47\x34\x9c\x02\x82\x01\x00\x7f\xb4\x68\x1d\xde\x12\xd3"
	"\x00\x3a\x26\xb6\x60\x7e\xb9\x62\x9f\x31\xcb\x6b\x3a\x4f\x0e\x18\x64\x4f\x65\xad\x79"
	"\x1d\xd7\xe8\xbf\x83\x68\x78\xc4\x9e\x2b\x90\xcd\xd8\xad\x36\x1a\x1d\x4f\x56\x4b\x8a"
	"\x6f\xc3\x77\x93\x5e\xf4\x7d\x61\xfd\x48\xa5\xa5\xbe\x27\xa3\x5d\xa9\x0a\xb0\x77\xc9"
	"\x6d\x8d\xae\x94\x5a\x90\x07\xb4\x96\x1b\xda\x93\x56\xc5\x50\x19\xdd\xa4\xb3\x40\x66"
	"\x0b\xb0\x55\xdd\xd7\x9a\x50\x7b\x9a\xe7\xbc\x63\x65\x9b\x45\x2d\x82\xf3\xe7\x49\x19"
	"\x71\x10\x9d\x46\x2e\x82\x8e\x15\x35\xf8\x53\x99\x3f\x4f\xec\x83\x48\xef\x0f\x27\x28"
	"\x75\x1f\x73\x97\x1e\x14\x16\x24\x16\xe6\x3f\x67\xdc\x50\x4a\xdc\x83\x89\x47\xf9\x41"
	"\xa5\xec\xe1\x86\x08\x46\xf4\xa9\x97\x14\x73\xaf\xe6\x01\x30\xa1\xad\x2f\xb0\x2a\x35"
	"\xdd\xad\x1d\x70\x25\x95\xbf\x07\xd2\x8c\x14\x46\xc4\xd2\x79\x21\xdc\xa9\x21\x6e\xff"
	"\x99\x69\x47\x8b\x67\x95\x98\x4d\x09\x75\xee\x97\x0b\xd9\x3d\x75\xd1\xa8\x82\x1f\xc8"
	"\x15\x2e\x39\x29\x6f\x95\x4b\xf4\x94\x74\xeb\xd8\xe3\x4a\xfb\x41\xa8\x4c\x5c\x3d\xb6"
	"\x42\xc0\x15\x35\x9e\x1a\xe2\x54\x2c\x0c\x0b\x34\x5f\xe2\x35\x8d\x30\x73";

ElGamal AuthContext::mVerifierKey(VerifierKeyBuf,sizeof(VerifierKeyBuf),false);

bool AuthContext::mCheckedVerifierFile = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetServerContext(ServerContext *theContext)
{
	AutoCrit aCrit(mDataCrit);
	mServerContext = theContext;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ServerContextPtr AuthContext::GetServerContext()
{
	AutoCrit aCrit(mDataCrit);
	return mServerContext;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::AddAddress(const IPAddr &theAddr)
{
	AutoCrit aCrit(mDataCrit);
	return mServerContext->AddAddress(theAddr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::RemoveAddress(const IPAddr &theAddr)
{
	AutoCrit aCrit(mDataCrit);
	return mServerContext->RemoveAddress(theAddr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::RemoveAllAddresses()
{
	AutoCrit aCrit(mDataCrit);
	mServerContext->RemoveAllAddresses();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::AddAddressesFromDir(const DirEntityList &theDir, const wchar_t *theNameFilter)
{
	AutoCrit aCrit(mDataCrit);
	return mServerContext->AddAddressesFromDir(theDir, theNameFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::AddAddressesFromDir(const DirEntityMap &theDir, const std::wstring &thePath, const wchar_t *theNameFilter)
{
	AutoCrit aCrit(mDataCrit);
	return mServerContext->AddAddressesFromDir(theDir, thePath, theNameFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::LoadVerifierKey(const std::string &theFile)
{
	mCheckedVerifierFile = true;

	FILE *aFile = fopen(theFile.c_str(),"rb");
	if(aFile==NULL)
		return false;

	unsigned char aBuf[1024];
	RawBuffer aKeyBuf;
	while(!feof(aFile))
	{
		int aNumRead = fread(aBuf,1,1024,aFile);
		if(aNumRead>0)
			aKeyBuf.append(aBuf,aNumRead);
	
	}

	fclose(aFile);
	
	AutoCrit aCrit(GetVerifierCrit());
	return mVerifierKey.SetPublicKey(aKeyBuf.data(),aKeyBuf.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetUserName(const std::wstring &theName)
{
	AutoCrit aCrit(mDataCrit);
	mUserName = theName;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetPassword(const std::wstring &thePassword)
{
	AutoCrit aCrit(mDataCrit);
	mPassword = thePassword;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AddCommunity(const std::wstring &theCommunity)
{
	AutoCrit aCrit(mDataCrit);
	mCommunityMap.insert(AuthLoginCommunityMap::value_type(theCommunity,AuthLoginCommunityData()));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetCommunity(const std::wstring &theCommunity)
{
	AutoCrit aCrit(mDataCrit);
	AuthLoginCommunityMap::iterator anItr = mCommunityMap.find(theCommunity);
	if(anItr!=mCommunityMap.end())
	{
		AuthLoginCommunityData aData = anItr->second;
		mCommunityMap.clear();
		mCommunityMap[theCommunity] = aData;
	}
	else
	{
		mCommunityMap.clear();
		AddCommunity(theCommunity);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::RemoveCommunity(const std::wstring &theCommunity)
{
	AutoCrit aCrit(mDataCrit);
	mCommunityMap.erase(theCommunity);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetCDKey(const std::wstring &theCommunity, const CDKey &theKey)
{
	AutoCrit aCrit(mDataCrit);
	mCommunityMap[theCommunity].mCDKey = theKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AddCDKeyCommunityJoin(const std::wstring &theCommunity, const CDKey &theKey)
{
	AutoCrit aCrit(mDataCrit);
	mCDKeyCommunityJoinMap[theCommunity] = theKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::ClearCDKeyCommunityJoins()
{
	AutoCrit aCrit(mDataCrit);
	mCDKeyCommunityJoinMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AddUserData(const std::wstring &theCommunity, const ByteBuffer *theData)
{
	AutoCrit aCrit(mDataCrit);
	mSetCommunityUserDataMap[theCommunity] = theData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::ClearUserData()
{
	AutoCrit aCrit(mDataCrit);
	mSetCommunityUserDataMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::ClearOneTimeData()
{
	ClearUserData();
	ClearCDKeyCommunityJoins();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::SetHashFile(const std::wstring &theCommunity)
{
#ifdef WIN32_NOT_XBOX
	char aBuf[MAX_PATH];
	if(GetModuleFileName(GetModuleHandle(NULL),aBuf,MAX_PATH)>0)
		return SetHashFile(theCommunity,aBuf);
#endif

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::SetHashFile(const std::wstring &theCommunity, const std::string &theFile)
{
	return SetHashFileList(theCommunity,HashFileList(1,theFile));
}

bool AuthContext::SetHashFileList(const std::wstring &theCommunity, const HashFileList &theList)
{
	AutoCrit aCrit(mDataCrit);
	AuthLoginCommunityData &aData = mCommunityMap[theCommunity];
	aData.mSimpleHash.erase();
	aData.mKeyedHashData.erase();

	MD5 aSimpleHash;
	MD5 aHashSection;

	int curHashSectionPos = 0;
	HashFileList::const_iterator anItr = theList.begin();
	while(anItr!=theList.end())
	{
		FILE *aFile = fopen(anItr->c_str(),"rb");
		if(aFile==NULL)
			return false;

		++anItr;

		const unsigned int HASH_CHUNK_SIZE = 16384; 
		char aFileBuf[HASH_CHUNK_SIZE];

		while(!feof(aFile))
		{
			int aNumRead = fread(aFileBuf,1,HASH_CHUNK_SIZE-curHashSectionPos,aFile);
			if(aNumRead>0)
			{
				curHashSectionPos+=aNumRead;
				aSimpleHash.Update(aFileBuf,aNumRead);
				aHashSection.Update(aFileBuf,aNumRead);
				unsigned char aHashSectionBuf[16];

				if(curHashSectionPos==HASH_CHUNK_SIZE || (feof(aFile) && anItr==theList.end()))
				{
					curHashSectionPos = 0;
					aHashSection.Digest(aHashSectionBuf);
					aData.mKeyedHashData.append(aHashSectionBuf,16);
					aHashSection.Reset();
				}
			}
		}
		fclose(aFile);
	}

	
	unsigned char aSimpleHashBuf[16];	
	aSimpleHash.Digest(aSimpleHashBuf);
	aData.mSimpleHash.assign(aSimpleHashBuf,16);
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AppendHashes(WriteBuffer &theBuf, const RawBuffer &theChallengeSeed)
{
	AutoCrit aCrit(mDataCrit);

	int aNumHashes = 0;
	int aNumHashPos = theBuf.length();
	theBuf.SkipBytes(1); // put num hashes here

	AuthLoginCommunityMap::iterator anItr = mCommunityMap.begin();
	while(anItr!=mCommunityMap.end())
	{
		AuthLoginCommunityData &aData = anItr->second;
		if(!aData.mSimpleHash.empty())
		{
			MD5Digest aKeyedHash;
			aKeyedHash.update(theChallengeSeed);
			aKeyedHash.update(aData.mKeyedHashData);
			RawBuffer aKeyedHashBuf = aKeyedHash.digest();		
	

			theBuf.AppendByte(1); // hash tag
			theBuf.AppendWString(anItr->first); // community
			theBuf.AppendBytes(aData.mSimpleHash.data(),aData.mSimpleHash.length());
			theBuf.AppendBytes(aKeyedHashBuf.data(),aKeyedHashBuf.length());
			aNumHashes++;
		}

		++anItr;
	}

	theBuf.SetByte(aNumHashPos,aNumHashes);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AppendCommunityData(WriteBuffer &theBuf)
{
	AutoCrit aCrit(mDataCrit);
	theBuf.AppendByte(0);									// 0 community ids
	theBuf.AppendByte(mCommunityMap.size());				// num community names
	AuthLoginCommunityMap::iterator anItr = mCommunityMap.begin(); 
	while(anItr!=mCommunityMap.end())
	{
		theBuf.AppendWString(anItr->first);	// community name
		++anItr;
	}


	int aNumCommnityElementsPos = theBuf.length();
	theBuf.SkipBytes(2); 
	int aNumCommunityElements = 0;

	anItr = mCommunityMap.begin();
	while(anItr!=mCommunityMap.end()) // Append CD Keys
	{
		AuthLoginCommunityData &aData = anItr->second;
		if(aData.mCDKey.IsValid())
		{
			ByteBufferPtr aKey = anItr->second.mCDKey.GetRaw();
			if(aKey.get()!=NULL)
			{
				theBuf.AppendByte(1);			// Type = CD Key
				theBuf.AppendShort(anItr->first.length()*2 + aKey->length() + 2); // length of community + data
				theBuf.AppendWString(anItr->first);
				theBuf.AppendBytes(aKey->data(),aKey->length());
				aNumCommunityElements++;
			}
		}
		++anItr;
	}

	CDKeyCommunityJoinMap::iterator aKeyJoinItr = mCDKeyCommunityJoinMap.begin(); // Append Community Join By CDKey Info
	while(aKeyJoinItr!=mCDKeyCommunityJoinMap.end())
	{
		ByteBufferPtr aKey = aKeyJoinItr->second.GetRaw();
		if(aKey.get()!=NULL)
		{
			theBuf.AppendByte(7);			// Type = Join Community with CD Key
			theBuf.AppendShort(aKeyJoinItr->first.length()*2+2 + 4 + aKey->length()); // community name + commnityseq + key
			theBuf.AppendWString(aKeyJoinItr->first);
			theBuf.AppendLong(0);
			theBuf.AppendBytes(aKey->data(),aKey->length());
			aNumCommunityElements++;
		}
		++aKeyJoinItr;
	}

	SetCommunityUserDataMap::iterator aUserDataItr = mSetCommunityUserDataMap.begin(); // Append User Data for communities
	while(aUserDataItr!=mSetCommunityUserDataMap.end())
	{
		const ByteBuffer *aData = aUserDataItr->second;
		if(aData!=NULL)
		{
			theBuf.AppendByte(8);			// Type = SetCommunityUserData
			theBuf.AppendShort(aUserDataItr->first.length()*2+2 + 4 + aData->length()); // community name + commnityseq + key
			theBuf.AppendWString(aUserDataItr->first);
			theBuf.AppendLong(0);
			theBuf.AppendBuffer(aData);
			aNumCommunityElements++;
		}

		++aUserDataItr;
	}

	if(mSecretList.size()>0) 				// CD Keys -> append login secret
	{
		theBuf.AppendByte(6);				// Type = LoginSecret
		unsigned long aPos = theBuf.length();
		theBuf.SkipBytes(2);

		AppendLoginSecrets(theBuf);
		theBuf.SetShort(aPos,theBuf.length()-aPos-2);
		aNumCommunityElements++;
	}

	NicknameMap::iterator aNickItr = mNicknameMap.begin();
	while(aNickItr!=mNicknameMap.end())
	{
		const wstring& aKey = aNickItr->first;
		const wstring& aVal = aNickItr->second;

		theBuf.AppendByte(4); // retrieve nickname
		unsigned long aPos = theBuf.length();
		theBuf.SkipBytes(2);
		theBuf.AppendWString(aKey);
		theBuf.SetShort(aPos,theBuf.length()-aPos-2);
		aNumCommunityElements++;

		if(!aVal.empty())
		{
			theBuf.AppendByte(3); // set nickname
			unsigned long aPos = theBuf.length();
			theBuf.SkipBytes(2);
			theBuf.AppendWString(aKey);
			theBuf.AppendWString(aVal);
			theBuf.SetShort(aPos,theBuf.length()-aPos-2);

			aNumCommunityElements++;
		}

		++aNickItr;
	}

	theBuf.SetShort(aNumCommnityElementsPos,aNumCommunityElements);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetLoginSecretFile(const std::string &theFile)
{
	AutoCrit aCrit(mDataCrit);
	mLoginSecretFile = theFile;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::LoadSecretList()
{
	AutoCrit aCrit(mDataCrit);
	mSecretList.clear();

	FILE *aFile = fopen(mLoginSecretFile.c_str(),"rb");
	if(aFile==NULL)
		return false;

	unsigned char aNumSecrets = fgetc(aFile);

	int i=0;
	while(!feof(aFile) && i<aNumSecrets)
	{
		time_t aTime = 0;
		fread(&aTime, sizeof(time_t), 1, aFile);

		char *aBuf = new char[SECRET_SIZE];
		fread(aBuf,1,SECRET_SIZE,aFile);
		mSecretList.push_back(SecretData(aTime,new ByteBuffer(aBuf,SECRET_SIZE,true)));

		i++;
	}

	fclose(aFile);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::SaveSecretList()
{
	AutoCrit aCrit(mDataCrit);
	FILE *aFile = fopen(mLoginSecretFile.c_str(),"wb");
	if(aFile==NULL)
		return false;

	unsigned char aNumSecrets = mSecretList.size();
	fwrite(&aNumSecrets,1,1,aFile);

	SecretList::iterator anItr = mSecretList.begin();
	while(anItr!=mSecretList.end())
	{
		fwrite(&(anItr->mExpireTime), sizeof(time_t), 1, aFile);
		fwrite(anItr->mSecret->data(),1,anItr->mSecret->length(),aFile);
		++anItr;
	}

	fclose(aFile);
	return true;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AppendLoginSecrets(WriteBuffer &theBuf) 
{
	AutoCrit aCrit(mDataCrit);
	SecretList::iterator anItr = mSecretList.begin();

	unsigned long aLenPos = theBuf.length();
	unsigned char aNumSecrets = 0;
	theBuf.SkipBytes(1);
	while(anItr!=mSecretList.end() && aNumSecrets<256)
	{
		theBuf.AppendBytes(anItr->mSecret->data(), anItr->mSecret->length());
		aNumSecrets++;
		++anItr;
	}

	theBuf.SetByte(aLenPos,aNumSecrets);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AddSecret(const void *theSecret)
{
	AutoCrit aCrit(mDataCrit);
	mSecretList.push_front(SecretData(0,new ByteBuffer(theSecret,SECRET_SIZE)));
	SaveSecretList();	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::RemoveSecret(const void *theSecret)
{
	AutoCrit aCrit(mDataCrit);

	SecretList::iterator anItr = mSecretList.begin();
	while(anItr!=mSecretList.end())
	{
		if(anItr->mExpireTime==0) // must be removing a non-committed secret due to auth failure status
		{
			const ByteBuffer* aBuf = anItr->mSecret;
			if(!memcmp(aBuf->data(),theSecret,SECRET_SIZE))
			{
				mSecretList.erase(anItr);
				SaveSecretList();
				return;
			}
		}

		++anItr;
	}		
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::CommitSecrets(time_t theAuthTime, time_t theDuration)
{
	AutoCrit aCrit(mDataCrit);
	if(mSecretList.size()==0)
		return;

	SecretList::iterator anItr = mSecretList.begin();
	while(anItr!=mSecretList.end())
	{
		if(anItr->mExpireTime==0) // commit this dude
		{
			anItr->mExpireTime = theAuthTime + theDuration + 120;
			++anItr;
		}
		else if(anItr->mExpireTime <= theAuthTime) // this secret has expired
			mSecretList.erase(anItr++);
		else
			++anItr;
	}

	while(mSecretList.size()>=256)
		mSecretList.pop_back();

	SaveSecretList();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AuthPeerDataPtr AuthContext::GetPeerData()
{
	AutoCrit aCrit(mDataCrit);
	return mPeerData;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AuthContext::IsExpired(time_t theExtraTime)
{
	AutoCrit aCrit(mDataCrit);
	return mPeerData->Certificate2Expired(theExtraTime);	
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetPeerData(const AuthPeerData *theData)
{
	AutoCrit aCrit(mDataCrit);
	mPeerData = theData;
	
	mSessionMap.clear();
	mSessionCleanItr = mSessionMap.begin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::AddNickname(const std::wstring &theKey)
{
	AutoCrit aCrit(mDataCrit);
	mNicknameMap[theKey] = L""; 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::SetNickname(const std::wstring &theKey, const std::wstring &theName)
{ 
	AutoCrit aCrit(mDataCrit);
	mNicknameMap[theKey] = theName; 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AuthContext::ClearNicknames()
{
	AutoCrit aCrit(mDataCrit);
	mNicknameMap.clear();
}

