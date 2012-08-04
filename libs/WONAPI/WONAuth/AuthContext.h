#ifndef __WON_AUTHCONTEXT_H__
#define __WON_AUTHCONTEXT_H__
#include "WONShared.h"

#include "WONCommon/AsyncOp.h"
#include "WONCommon/BiMap.h"
#include "WONCrypt/ElGamal.h"
#include "WONCrypt/Blowfish.h"
#include "WONServer/ServerContext.h"
#include "AuthSession.h"
#include "AuthPeerData.h"
#include "CDKey.h"

#include <list>
#include <string>
#include <set>


namespace WONAPI
{


class GetCertOp;
typedef SmartPtr<GetCertOp> GetCertOpPtr;

class LightGetCertOp;
typedef SmartPtr<LightGetCertOp> LightGetCertOpPtr;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AuthLoginCommunityData
{
public:
	AuthLoginCommunityData() {}

	CDKey mCDKey;
	RawBuffer mSimpleHash;
	RawBuffer mKeyedHashData;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::map<std::wstring,AuthLoginCommunityData> AuthLoginCommunityMap;
typedef std::list<std::string> HashFileList;
typedef std::map<std::wstring,CDKey> CDKeyCommunityJoinMap;
typedef std::map<std::wstring,ByteBufferPtr> SetCommunityUserDataMap;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AuthContext : public RefCount
{
private:
	CriticalSection mDataCrit;

	// VerifierKey
	static ElGamal mVerifierKey;
	static bool mCheckedVerifierFile;

	// Refreshing certificate
	typedef std::set<LightGetCertOpPtr,LightGetCertOpPtr::Comp> RefreshSet;
	RefreshSet mRefreshSet;
	GetCertOpPtr mGetCertOp;
	AsyncOpPtr mAutoRefreshOp;
	WONStatus mRefreshStatus;
	bool mDoAutoRefresh;
	void StartAutoRefresh(bool force = false);
	void StopAutoRefresh();

	// Peer Sessions
	typedef std::list<AuthSessionPtr> SessionList;
	struct SessionStruct
	{
		time_t mLastUseTime;
		SessionList mSessionList;
	};	
	typedef std::map<IPAddr,SessionStruct> SessionMap;
	SessionMap mSessionMap;
	SessionMap::iterator mSessionCleanItr;

	// LoginData
	AuthPeerDataPtr mPeerData;
	std::wstring mUserName;
	std::wstring mPassword;
	std::string mLoginSecretFile;
	AuthLoginCommunityMap mCommunityMap;

	struct SecretData
	{
		time_t mExpireTime;
		ByteBufferPtr mSecret;

		SecretData() : mExpireTime(0) {}
		SecretData(time_t theExpireTime, const ByteBuffer *theSecret) : mExpireTime(theExpireTime), mSecret(theSecret) {}
	};

	typedef std::list<SecretData> SecretList;
	SecretList mSecretList;
	enum { SECRET_SIZE = 8 };

	CDKeyCommunityJoinMap mCDKeyCommunityJoinMap;
	SetCommunityUserDataMap mSetCommunityUserDataMap;

	typedef std::map<std::wstring, std::wstring> NicknameMap;
	NicknameMap mNicknameMap;

	ServerContextPtr mServerContext;

	void Init();


	friend class GetCertOp;
	void AppendHashes(WriteBuffer &theBuf, const RawBuffer &theChallengeSeed);
	void AppendCommunityData(WriteBuffer &theBuf);

	bool LoadSecretList();
	bool SaveSecretList();
	void AddSecret(const void *theSecret);
	void RemoveSecret(const void *theSecret);
	void CommitSecrets(time_t theAuthTime, time_t theDuration); 
	void AppendLoginSecrets(WriteBuffer &theBuf);

private:
	static void StaticGetCertCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	void GetCertCallback(GetCertOp *theOp);

	static void StaticAutoRefreshCallback(AsyncOpPtr theOp, RefCountPtr theParam);
	void AutoRefreshCallback(AsyncOp *theOp);

public:
	AuthContext();
	AuthContext(const std::wstring &theUserName, const std::wstring& theCommunity, const std::wstring &thePassword);

	// Which auth server network are we using?
	void SetServerContext(ServerContext *theContext);
	ServerContextPtr GetServerContext();
	bool AddAddress(const IPAddr &theAddr);
	bool RemoveAddress(const IPAddr &theAddr);
	void RemoveAllAddresses();
	bool AddAddressesFromDir(const DirEntityList &theDir, const wchar_t *theNameFilter = NULL);
	bool AddAddressesFromDir(const DirEntityMap &theDir, const std::wstring &thePath, const wchar_t *theNameFilter = NULL);

	// Verifier Key
	static bool LoadVerifierKey(const std::string &theFileName);
	static const ElGamal& GetVerifierKey() { return mVerifierKey; }

	// Username/Password
	const std::wstring& GetUserName() const { return mUserName; } 
	const std::wstring& GetPassword() const { return mPassword; }
	void SetUserName(const std::wstring &theName);
	void SetPassword(const std::wstring &thePassword);

	// Nicknames
	void AddNickname(const std::wstring &theKey);
	void SetNickname(const std::wstring &theKey, const std::wstring &theName);
	void ClearNicknames();

	// Community (CDKey and Hashfiles are per community)
	void SetCommunity(const std::wstring &theCommunity);
	void AddCommunity(const std::wstring &theCommunity);
	void RemoveCommunity(const std::wstring &theCommunity);
	void SetCDKey(const std::wstring &theCommunity, const CDKey &theKey);
	bool SetHashFile(const std::wstring &theCommunity); // use GetModuleFileName for theFile
	bool SetHashFile(const std::wstring &theCommunity, const std::string &theFile);
	bool SetHashFileList(const std::wstring &theCommunity, const HashFileList &theList);

	// Add to Invite Only Community With CDKey
	void AddCDKeyCommunityJoin(const std::wstring &theCommunity, const CDKey &theKey);
	void ClearCDKeyCommunityJoins();

	// Set User Data for community
	void AddUserData(const std::wstring &theCommunity, const ByteBuffer *theData);
	void ClearUserData();

	// Clear CDKeyCommunityJoin and UserData.  Automatically called by GetCertOp when successful.
	void ClearOneTimeData();

	// Getting/Refreshing the certificate
	void RefreshAsync(LightGetCertOp *theOp = NULL, DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus RefreshBlock(DWORD theTimeout = OP_TIMEOUT_INFINITE);
	WONStatus GetRefreshStatus() const { return mRefreshStatus; } 
	void SetDoAutoRefresh(bool doAutoRefresh);

	// Login secrets (stored in a file)
	void SetLoginSecretFile(const std::string &theFile);
	
	// AuthPeerData (PubKeyBlock,Certificate,PrivateKey -> Result of getting certificate from authserver)
	AuthPeerDataPtr GetPeerData();
	void SetPeerData(const AuthPeerData *theData);
	bool IsExpired(time_t theExtraTime = 0);

	// Sessions established with servers
	void PutSession(const IPAddr &theAddr, AuthSession *theSession);
	AuthSessionPtr GetSession(const IPAddr &theAddr);

	// Kills the circular reference with the GetCertOp that exists when in the middle of 
	// asyncronously refreshing. 
	void Kill();

protected:
	~AuthContext();

};

typedef SmartPtr<AuthContext> AuthContextPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
