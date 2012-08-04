//////////////////////////////////////////////////////////////////////////////
// CServerDir_Titan implementation

#include "stdafx.h"

#include "cserverdir_titan.h"

#include <algorithm>

#include "titantest.h"

#include "iltcsbase.h"
#include "iltclient.h"
#include "iltcommon.h"

#include "iserverdir_titan.h"

#include <malloc.h>

ILTCSBase* g_pLTCSBase = NULL;
ILTClient* g_pLTClient = NULL;
HMODULE g_hResourceModule = NULL;


// Defines...

#define DEFAULT_TITAN_PRODUCT_STRING ("NOLF2")
#define TITAN_AUTHSERVER_PATH (L"/TitanServers/Auth")
#define TITAN_GAMESERVER_TIMEOUT 300
#define TITAN_GAMESERVER_DEFAULTPORT 27888
#define TITAN_SUMMARY_DATATYPE ("s")
#define TITAN_TIMEOUT (30000)
#define GAMESPYSUPPORT_AUTOPUMPTIME		30 // ms

// Functions which are handy for dealing with Titan...
static std::wstring Make_wstring(const std::string &cString)
{
	uint32 nStrLen = cString.length();
	wchar_t *pTempBuffer = (wchar_t*)alloca(sizeof(wchar_t) * (nStrLen + 1));

	// Convert it into our temporary buffer
	const char *pInFinger = cString.c_str();
	wchar_t *pOutFinger = pTempBuffer;
	wchar_t *pOutEnd = pTempBuffer + nStrLen;
	while (pOutFinger != pOutEnd)
	{
		*pOutFinger = btowc(*pInFinger);
		++pOutFinger;
		++pInFinger;
	}

	*pOutFinger = 0;

	// Save it
	std::wstring sResult = pTempBuffer;
	return sResult;
}

static void Make_string(const std::wstring &cString, std::string& cOutString )
{
	cOutString.erase( );
	uint32 nStrLen = cString.length();
	char *pTempBuffer = (char*)alloca(sizeof(char) * (nStrLen + 1));

	// Convert it into our temporary buffer
	const wchar_t *pInFinger = cString.c_str();
	char *pOutFinger = pTempBuffer;
	char *pOutEnd = pTempBuffer + nStrLen;
	while (pOutFinger != pOutEnd)
	{
		*pOutFinger = wctob(*pInFinger);
		++pOutFinger;
		++pInFinger;
	}

	*pOutFinger = 0;

	// Save it
	cOutString = pTempBuffer;
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry implementation

CSDTitan_RequestEntry::CSDTitan_RequestEntry(CServerDirectory_Titan *pDir) :
	m_pDir(pDir)
{
}

void CSDTitan_RequestEntry::Finish(IServerDirectory::ERequestResult eResult, const std::string &sResultString)
{
	GetDir()->FinishRequest(this, eResult, sResultString);
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Noop

CSDTitan_RequestEntry_Noop::CSDTitan_RequestEntry_Noop(CServerDirectory_Titan *pDir, IServerDirectory::ERequest eRequest) :
	CSDTitan_RequestEntry(pDir),
	m_eRequest(eRequest)
{
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_TitanOp implementation

CSDTitan_RequestEntry_TitanOp::CSDTitan_RequestEntry_TitanOp(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry(pDir)
{
}

void CSDTitan_RequestEntry_TitanOp::Start()
{
	if (GetAsyncOp()->Killed())
		Reset();
	GetDir()->SetCurStatusString(GetStatusString());
	GetAsyncOp()->SetCompletion(GetCompletionFn());
	GetAsyncOp()->Run(OP_MODE_ASYNC, TITAN_TIMEOUT);
}

void CSDTitan_RequestEntry_TitanOp::Cancel()
{
	CWinSync_CSAuto cNotDuringCompletion(m_cCompletionCS);
	// Die!  (Note : The false parameter is to avoid deadlock situations)
	GetAsyncOp()->Kill(false);
	// Clear out the completion function
	GetAsyncOp()->SetCompletion(0);
	// Tell our parent
	CSDTitan_RequestEntry::Cancel();
}

OpCompletionBase *CSDTitan_RequestEntry_TitanOp::GetCompletionFn()
{
	return new ParamCompletion<AsyncOpPtr, CSDTitan_RequestEntry_TitanOp *>(Completion_Bootstrap, this);
}

void CSDTitan_RequestEntry_TitanOp::OnCompletion()
{
	WONStatus nOpResult = GetAsyncOp()->GetStatus();

	IServerDirectory::ERequestResult eResult = IServerDirectory::eRequestResult_Success;
	std::string sResultStr;
	if (nOpResult != WS_Success)
	{
		eResult = IServerDirectory::eRequestResult_Failed;
		sResultStr = std::string(GetFailureString()) + std::string(" (") + std::string(WONStatusToString(nOpResult)) + std::string(")");
	}
	else
		sResultStr = GetSuccessString();

	// Entered in the bootstrap
	m_cCompletionCS.Leave();

	Finish(eResult, sResultStr);
}

void CSDTitan_RequestEntry_TitanOp::Completion_Bootstrap(AsyncOpPtr pAsyncOp, CSDTitan_RequestEntry_TitanOp *pRequestEntry)
{
	AsyncOpPtr pRequestOp = pRequestEntry->GetAsyncOp();
	if (pRequestOp != pAsyncOp)
		return;

	if (pRequestOp->Killed())
		return;

	// Left in the final OnCompletion or CancelCompletion
	pRequestEntry->m_cCompletionCS.Enter();

	pRequestEntry->OnCompletion();
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_MOTD

CSDTitan_RequestEntry_MOTD::CSDTitan_RequestEntry_MOTD(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_TitanOp(pDir)
{
	std::string sGameName = pDir->GetGameName();
	m_pMOTDOp = new GetMOTDOp( sGameName );
}

void CSDTitan_RequestEntry_MOTD::Reset()
{
	std::string sGameName = GetDir()->GetGameName();
	m_pMOTDOp = new GetMOTDOp( sGameName );
}

void CSDTitan_RequestEntry_MOTD::OnCompletion()
{
	if (m_pMOTDOp->GetStatus() == WS_Success)
	{
		GetDir()->SetMOTD(IServerDirectory::eMOTD_Game, m_pMOTDOp->GetGameMOTD()->data(), m_pMOTDOp->GameMOTDIsNew());
		GetDir()->SetMOTD(IServerDirectory::eMOTD_System, m_pMOTDOp->GetSysMOTD()->data(), m_pMOTDOp->SysMOTDIsNew());
	}
	CSDTitan_RequestEntry_TitanOp::OnCompletion();
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Validate_Version

CSDTitan_RequestEntry_Validate_Version::CSDTitan_RequestEntry_Validate_Version(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_TitanOp(pDir),
	m_pGetDirOp(new GetDirOp(pDir->GetDirServerList())),
	m_pVersionOp(0)
{
	m_pGetDirOp->SetFlags(DIR_GF_DECOMPSERVICES | DIR_GF_SERVADDNETADDR | DIR_GF_SERVADDPATH);
	m_pGetDirOp->SetPath(L"/" + Make_wstring(GetDir()->GetGameName()) + L"/Patch");

	m_pActiveOp = m_pGetDirOp;
}

void CSDTitan_RequestEntry_Validate_Version::Reset()
{
	m_pActiveOp = m_pGetDirOp;
	m_pVersionOp = 0;
}

void CSDTitan_RequestEntry_Validate_Version::OnCompletion()
{
	if (m_pActiveOp == m_pGetDirOp)
	{
		if (m_pActiveOp->GetStatus() == WS_Success)
		{
			AuthContextPtr pAuthContext = GetDir()->GetAuthContext();
			ServerContextPtr pServerContext = new ServerContext;
			pServerContext->AddAddressesFromDir(m_pGetDirOp->GetDirEntityList());
			m_pVersionOp = new CheckValidVersionOp(GetDir()->GetGameName(), pServerContext);
			m_pVersionOp->SetVersion(GetDir()->GetVersion());
			m_pVersionOp->SetConfigName(GetDir()->GetRegion());
			m_pVersionOp->SetGetPatchList(false);
			m_pActiveOp = m_pVersionOp;
			// Start it back up again
			Start();
			// We're not complete yet
			CancelCompletion();
		}
		else
		{
			CSDTitan_RequestEntry_TitanOp::OnCompletion();
		}
	}
	else
	{
		// Oo, what did we get?
		switch (m_pActiveOp->GetStatus())
		{
			// You're A-OK
			case WS_Success :
			case WS_DBProxyServ_ValidNotLatest :
				GetDir()->SetVersionState(CServerDirectory_Titan::eVersion_Latest);
				m_pActiveOp = m_pGetDirOp;
				break;
			// You could be better..
			case WS_DBProxyServ_OutOfDate :
				GetDir()->SetVersionState(CServerDirectory_Titan::eVersion_Patchable);
				// Go back to the dir op, since that's got a status of WS_Success, and this was a successful result
				m_pActiveOp = m_pGetDirOp;
				break;
			// You stink.
			case WS_DBProxyServ_OutOfDateNoUpdate :
				GetDir()->SetVersionState(CServerDirectory_Titan::eVersion_Old);
				// Go back to the dir op, since that's got a status of WS_Success, and this was a successful result
				m_pActiveOp = m_pGetDirOp;
				break;
			// Aw, nothin'.. Shucks
			default :
				GetDir()->SetVersionState(CServerDirectory_Titan::eVersion_Unknown);
				break;
		}
		CSDTitan_RequestEntry_TitanOp::OnCompletion();
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Validate_CDKey

CSDTitan_RequestEntry_Validate_CDKey::CSDTitan_RequestEntry_Validate_CDKey(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_TitanOp(pDir),
	m_pGetDirOp(new GetDirOp(pDir->GetDirServerList()))
{
	m_pGetDirOp->SetFlags(DIR_GF_DECOMPSERVICES | DIR_GF_SERVADDNETADDR | DIR_GF_SERVADDPATH);
	m_pGetDirOp->SetPath(TITAN_AUTHSERVER_PATH);
	AuthContextPtr pAuthContext = GetDir()->GetAuthContext();

	// Set the CD key
	pAuthContext->SetCDKey(Make_wstring(GetDir()->GetGameName()), GetDir()->GetTitanCDKey());

	if (pAuthContext->GetServerContext()->GetNumAddresses() != 0)
	{
		m_pGetCertOp = new GetCertOp(pAuthContext);
		m_pActiveOp = m_pGetCertOp;
	}
	else
		m_pActiveOp = m_pGetDirOp;
}

void CSDTitan_RequestEntry_Validate_CDKey::Reset()
{
	AuthContextPtr pAuthContext = GetDir()->GetAuthContext();
	if (pAuthContext->GetServerContext()->GetNumAddresses() == 0)
	{
		GetDirOpPtr pNewOp = new GetDirOp(GetDir()->GetDirServerList());
		pNewOp->SetFlags(m_pGetDirOp->GetFlags());
		pNewOp->SetPath(m_pGetDirOp->GetPath());
		m_pGetDirOp = pNewOp;
		m_pActiveOp = m_pGetDirOp;
		m_pGetCertOp = 0;
	}
	else
	{
		m_pGetCertOp = new GetCertOp(pAuthContext);
		m_pActiveOp = m_pGetCertOp;
	}
}

void CSDTitan_RequestEntry_Validate_CDKey::OnCompletion()
{
	if (m_pActiveOp->GetStatus() == WS_Success)
	{
		if (m_pActiveOp == m_pGetDirOp)
		{
			AuthContextPtr pAuthContext = GetDir()->GetAuthContext();
			pAuthContext->AddAddressesFromDir(m_pGetDirOp->GetDirEntityList());
			m_pGetCertOp = new GetCertOp(pAuthContext);
			m_pActiveOp = m_pGetCertOp;
			// Start it back up again
			Start();
			// We're not complete yet
			CancelCompletion();
		}
		else
		{
			// Tell the directory we're valid
			GetDir()->SetAuthContextValid(true);
			CSDTitan_RequestEntry_TitanOp::OnCompletion();
		}
	}
	else
	{
		// Tell the directory we're invalid
		GetDir()->SetAuthContextValid(false);
		CSDTitan_RequestEntry_TitanOp::OnCompletion();
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Publish_Server

CSDTitan_RequestEntry_Publish_Server::CSDTitan_RequestEntry_Publish_Server(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_TitanOp(pDir),
	m_pAddServiceOp(new AddServiceOp(pDir->GetDirServerList()))
{
	m_pActiveOp = m_pAddServiceOp;

	CWinSync_CSAuto cProtection(pDir->GetPeerInfoCS());

	const CServerDirectory_Titan::SPeerData sCurInfo = pDir->GetLocalPeerInfo();
	m_pAddServiceOp->SetFlags(DIR_UF_SERVGENNETADDR | DIR_UF_DIRNOTUNIQUE);
	m_pAddServiceOp->SetNetAddr(sCurInfo.m_cAddr);
	m_pAddServiceOp->SetNetAddrPort(sCurInfo.m_cAddr.GetPort());
	m_pAddServiceOp->SetName(Make_wstring(pDir->GetGameName()));
	m_pAddServiceOp->SetDisplayName(Make_wstring(sCurInfo.m_sName.c_str()));
	m_pAddServiceOp->AddDataObject(TITAN_SUMMARY_DATATYPE, new ByteBuffer(&(*(sCurInfo.m_cSummary.begin())), sCurInfo.m_cSummary.size()));
	std::string sPath = "/";
	sPath += pDir->GetGameName();
	m_pAddServiceOp->SetPath(Make_wstring( sPath ));
	m_pAddServiceOp->SetLifespan(TITAN_GAMESERVER_TIMEOUT);
}

void CSDTitan_RequestEntry_Publish_Server::Reset()
{
	m_pRemoveServiceOp = 0;
	m_pAddServiceOp = new AddServiceOp(GetDir()->GetDirServerList());

	CWinSync_CSAuto cProtection(GetDir()->GetPeerInfoCS());

	const CServerDirectory_Titan::SPeerData sCurInfo = GetDir()->GetLocalPeerInfo();
	m_pAddServiceOp->SetFlags(DIR_UF_SERVGENNETADDR | DIR_UF_DIRNOTUNIQUE);
	m_pAddServiceOp->SetNetAddr(sCurInfo.m_cAddr);
	m_pAddServiceOp->SetNetAddrPort(sCurInfo.m_cAddr.GetPort());
	m_pAddServiceOp->SetName(Make_wstring(GetDir()->GetGameName()));
	m_pAddServiceOp->SetDisplayName(Make_wstring(sCurInfo.m_sName.c_str()));
	m_pAddServiceOp->AddDataObject(TITAN_SUMMARY_DATATYPE, new ByteBuffer(&(*(sCurInfo.m_cSummary.begin())), sCurInfo.m_cSummary.size()));
	std::string sPath = "/";
	sPath += GetDir()->GetGameName();
	m_pAddServiceOp->SetPath(Make_wstring( sPath ));
	m_pAddServiceOp->SetLifespan(TITAN_GAMESERVER_TIMEOUT);

	m_pActiveOp = m_pAddServiceOp;
}

void CSDTitan_RequestEntry_Publish_Server::OnCompletion()
{
	if ((m_pActiveOp->GetStatus() != WS_Success) && (m_pActiveOp == m_pAddServiceOp))
	{
		// Remove the entry if it's already in there
		m_pRemoveServiceOp = new RemoveServiceOp(GetDir()->GetDirServerList());
		m_pRemoveServiceOp->SetNetAddr(GetDir()->GetLocalPeerInfo().m_cAddr);
		m_pRemoveServiceOp->SetName(Make_wstring(GetDir()->GetGameName()));
		std::string sPath = "/";
		sPath += GetDir()->GetGameName();
		m_pRemoveServiceOp->SetPath(Make_wstring( sPath ));
		m_pActiveOp = m_pRemoveServiceOp;
		Start();
		// We're not complete yet
		CancelCompletion();
		return;
	}
	else if ((m_pActiveOp->GetStatus() == WS_Success) && (m_pActiveOp == m_pRemoveServiceOp))
	{
		// Re-add the entry
		m_pActiveOp = m_pAddServiceOp;
		// And make sure we don't try to remove it again if this fails
		m_pAddServiceOp = 0;
		Start();
		// We're not complete yet
		CancelCompletion();
		return;
	}
	else
	{
		// Make the gamespysupport if we haven't already.
		if( !GetDir( )->IsGameSupportCreated( ))
			GetDir( )->CreateGameSpySupport( );

		CSDTitan_RequestEntry_TitanOp::OnCompletion();
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Remove_Server

CSDTitan_RequestEntry_Remove_Server::CSDTitan_RequestEntry_Remove_Server(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_TitanOp(pDir),
	m_pRemoveServiceOp(new RemoveServiceOp(pDir->GetDirServerList()))
{
	CWinSync_CSAuto cProtection(pDir->GetPeerInfoCS());

	const CServerDirectory_Titan::SPeerData sCurInfo = pDir->GetLocalPeerInfo();
	m_pRemoveServiceOp->SetNetAddr(sCurInfo.m_cAddr);
	m_pRemoveServiceOp->SetName(Make_wstring(GetDir()->GetGameName()));
	std::string sPath = "/";
	sPath += pDir->GetGameName();
	m_pRemoveServiceOp->SetPath(Make_wstring( sPath ));
}

void CSDTitan_RequestEntry_Remove_Server::Reset()
{
	RemoveServiceOpPtr pNewRemoveOp = new RemoveServiceOp(GetDir()->GetDirServerList());

	pNewRemoveOp->SetNetAddr(m_pRemoveServiceOp->GetNetAddr());
	pNewRemoveOp->SetName(m_pRemoveServiceOp->GetName());
	pNewRemoveOp->SetPath(m_pRemoveServiceOp->GetPath());

	m_pRemoveServiceOp = pNewRemoveOp;
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_UpdateList

CSDTitan_RequestEntry_UpdateList::CSDTitan_RequestEntry_UpdateList(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_TitanOp(pDir),
	m_pGetDirOp(new GetDirOp(pDir->GetDirServerList()))
{
	m_pGetDirOp->SetFlags(
		DIR_GF_DECOMPSERVICES |
		DIR_GF_ADDDISPLAYNAME |
		DIR_GF_ADDDOTYPE |
		DIR_GF_ADDDODATA |
		DIR_GF_SERVADDNETADDR);
	std::string sPath = "/";
	sPath += pDir->GetGameName();
	m_pGetDirOp->SetPath(Make_wstring( sPath ));
	m_pGetDirOp->AddDataType(TITAN_SUMMARY_DATATYPE);
}

void CSDTitan_RequestEntry_UpdateList::Reset()
{
	GetDirOpPtr pNewDirOp = new GetDirOp(GetDir()->GetDirServerList());

	pNewDirOp->SetFlags(m_pGetDirOp->GetFlags());
	pNewDirOp->SetPath(m_pGetDirOp->GetPath());
	pNewDirOp->AddDataType(TITAN_SUMMARY_DATATYPE);

	m_pGetDirOp = pNewDirOp;
}

void CSDTitan_RequestEntry_UpdateList::OnCompletion()
{
	if (m_pGetDirOp->GetStatus() == WS_Success)
	{
		CWinSync_CSAuto cProtection(GetDir()->GetPeerInfoCS());

		// Get the source and destination
		const DirEntityList &cDir = m_pGetDirOp->GetDirEntityList();
		CServerDirectory_Titan::TPeerDataList &cPeerInfo = GetDir()->GetPeerInfo();

		// Remember where we were
		uint32 nOldActivePeerIndex = GetDir()->GetActivePeerIndex();

		// Make sure we have at least enough room
		cPeerInfo.reserve(cDir.size() + 1);
		DirEntityList::const_iterator iCurSrc = cDir.begin();
		for (; iCurSrc != cDir.end(); ++iCurSrc)
		{
			// Find that peer
			GetDir()->SetActivePeer((*iCurSrc)->GetNetAddrAsIP().GetHostAndPortString().c_str());
			CServerDirectory_Titan::SPeerData &sCurDest = GetDir()->GetActivePeerInfo();

			// Read the name
			Make_string((*iCurSrc)->mDisplayName, sCurDest.m_sName );
			sCurDest.m_bHasName = true;
			// Read the summary
			sCurDest.m_bHasSummary = (!(*iCurSrc)->mDataObjects.empty());
			if (sCurDest.m_bHasSummary)
			{
				const DirDataObject &cDataObject = (*iCurSrc)->mDataObjects.front();
				ASSERT(cDataObject.mDataType == std::string(TITAN_SUMMARY_DATATYPE));
				sCurDest.m_cSummary.resize(cDataObject.mData->length());
				memcpy(&(*(sCurDest.m_cSummary.begin())), cDataObject.mData->data(), cDataObject.mData->length());
			}
		}

		// Restore the active peer
		GetDir()->SetActivePeerIndex(nOldActivePeerIndex);
	}
	CSDTitan_RequestEntry_TitanOp::OnCompletion();
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_UpdatePings

CSDTitan_RequestEntry_UpdatePings::CSDTitan_RequestEntry_UpdatePings(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry(pDir)
{
	m_aPingQueue.reserve(k_nPingCount);

	Reset();
}

CSDTitan_RequestEntry_UpdatePings::~CSDTitan_RequestEntry_UpdatePings()
{
	Reset();
}

void CSDTitan_RequestEntry_UpdatePings::Start()
{
	m_cNotBusyEvent.Clear();
	m_cBusyEvent.Set();

	{
		// Queue up the address list
		CWinSync_CSAuto cProtection(GetDir()->GetPeerInfoCS());

		// Get the peer list
		CServerDirectory_Titan::TPeerDataList &cPeerInfo = GetDir()->GetPeerInfo();

		// Give us some room
		m_aAddressQueue.reserve(cPeerInfo.size());

		// Copy the list
		CServerDirectory_Titan::TPeerDataList::const_iterator iCurPeer = cPeerInfo.begin();
		for (; iCurPeer != cPeerInfo.end(); ++iCurPeer)
		{
			m_aAddressQueue.push_back(iCurPeer->m_cAddr);
		}
	}

	while (!m_aPingQueue.empty() || !m_aAddressQueue.empty())
	{
		QueueAddressesForPing();
		if (m_cCancelEvent.Block(k_nPingDelay))
			break;
		RetireFinishedPings();
	}

	bool bWasCancelled = m_cCancelEvent.IsSet();
	if (bWasCancelled)
		FlushPingQueue();

	m_cBusyEvent.Clear();
	m_cNotBusyEvent.Set();

	if (!bWasCancelled)
		Finish(IServerDirectory::eRequestResult_Success, std::string(GetTempString(IDS_WON_PING_SUCCESS)));
}

void CSDTitan_RequestEntry_UpdatePings::Cancel()
{
	m_cCancelEvent.Set();
	m_cNotBusyEvent.Block();
}

void CSDTitan_RequestEntry_UpdatePings::Reset()
{
	if (m_cBusyEvent.IsSet())
		Cancel();
	m_cCancelEvent.Clear();
	m_cBusyEvent.Clear();
	m_cNotBusyEvent.Set();

	m_aAddressQueue.clear();
	ASSERT(m_aPingQueue.empty());
	if (!m_aPingQueue.empty()) // This shouldn't ever happen..  But in case it does, don't leak ping ID's
		FlushPingQueue();
}

void CSDTitan_RequestEntry_UpdatePings::QueueAddressesForPing()
{
	while (!m_aAddressQueue.empty() && (m_aPingQueue.size() < k_nPingCount))
	{
		// Start a ping request..
		SPingReq sReq;
		sReq.m_cAddr = m_aAddressQueue.back();
		// Tell the engine to start a ping
		if (g_pLTCSBase->StartPing(sReq.m_cAddr.GetHostString().c_str(), sReq.m_cAddr.GetPort(), &sReq.m_nID) == LT_OK)
			m_aPingQueue.push_back(sReq);
		m_aAddressQueue.pop_back();
	}
}

void CSDTitan_RequestEntry_UpdatePings::RetireFinishedPings()
{
	TPingList::iterator iCurPing = m_aPingQueue.begin();
	while (iCurPing != m_aPingQueue.end())
	{
		// If we've gotten a response on this address...
		uint32 nPingStatus;
		uint32 nPingTime = 0;
		LTRESULT nQueryResult = g_pLTCSBase->GetPingStatus(iCurPing->m_nID, &nPingStatus, &nPingTime);
		bool bGotResponse = true;
		bool bFinishedPing = false;
		if ((nQueryResult == LT_NOTFOUND) || (nPingStatus == PING_STATUS_TIMEOUT))
		{
			bGotResponse = false;
			bFinishedPing = true;
		}
		else
		{
			bGotResponse = (nPingStatus == PING_STATUS_SUCCESS);
			bFinishedPing = bGotResponse;
		}
		if (!bFinishedPing)
		{
			++iCurPing;
			continue;
		}

		// Save the results
		if (bGotResponse)
			SavePingResult(iCurPing->m_cAddr, nPingTime);

		// Tell the engine we're done with it
		g_pLTCSBase->RemovePing(iCurPing->m_nID);

		// Remove it from the list
		if (m_aPingQueue.size() > 0)
			std::swap(*iCurPing, m_aPingQueue.back());
		m_aPingQueue.pop_back();
	}
}

void CSDTitan_RequestEntry_UpdatePings::FlushPingQueue()
{
	// Keep whatever happened to sneak in under the wire
	RetireFinishedPings();
	// Cancel the remaining pings
	while (!m_aPingQueue.empty())
	{
		g_pLTCSBase->RemovePing(m_aPingQueue.back().m_nID);
		m_aPingQueue.pop_back();
	}
}

void CSDTitan_RequestEntry_UpdatePings::SavePingResult(const IPAddr &cAddr, uint32 nPingTime)
{
	CWinSync_CSAuto cProtection(GetDir()->GetPeerInfoCS());

	// Get the peer list
	CServerDirectory_Titan::TPeerDataList &cPeerInfo = GetDir()->GetPeerInfo();

	// Find the IP addr
	CServerDirectory_Titan::TPeerDataList::iterator iCurPeer = cPeerInfo.begin();
	for (; iCurPeer != cPeerInfo.end(); ++iCurPeer)
	{
		if (iCurPeer->m_cAddr == cAddr)
		{
			// Save it..
			iCurPeer->m_bHasPing = true;
			iCurPeer->m_nPing = nPingTime;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_MsgOp

CSDTitan_RequestEntry_MsgOp::CSDTitan_RequestEntry_MsgOp(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry(pDir),
	m_cDestination(pDir->GetActivePeerInfo().m_cAddr)
{
}

bool CSDTitan_RequestEntry_MsgOp::OnMessage(ILTMessage_Read &cMsg, const IPAddr &cSource)
{
	// If we're not waiting for a message, jump out
	if (!m_cIsWaiting.IsSet())
		return false;

	// If we're not waiting for a message from this address, jump out
	if (cSource != m_cDestination)
		return false;

	// Take care of it
	if (!HandleMessage(cMsg))
		return false;

	// Ok, we're done waiting now
	m_cContinueEvent.Set();

	return true;
}

void CSDTitan_RequestEntry_MsgOp::Cancel()
{
	m_cCancelEvent.Set();
	if (m_cIsWaiting.IsSet())
	{
		WaitForSingleObject(m_cCancelReadyEvent.GetEvent(), INFINITE);
	}
}

void CSDTitan_RequestEntry_MsgOp::Reset()
{
	if (m_cIsWaiting.IsSet())
		Cancel();
	m_cCancelEvent.Clear();
	m_cContinueEvent.Clear();
	m_cIsWaiting.Clear();
	m_cCancelReadyEvent.Clear();
}

CSDTitan_RequestEntry_MsgOp::EWaitResult CSDTitan_RequestEntry_MsgOp::WaitForMessage(uint32 nTimeout)
{
	HANDLE aWaitArray[2];
	aWaitArray[0] = m_cContinueEvent.GetEvent();
	aWaitArray[1] = m_cCancelEvent.GetEvent();

	m_cContinueEvent.Clear();
	m_cCancelEvent.Clear();
	m_cCancelReadyEvent.Clear();

	m_cIsWaiting.Set();

	uint32 nResult = WaitForMultipleObjects(2, aWaitArray, FALSE, nTimeout);

	m_cIsWaiting.Clear();

	if (nResult == (WAIT_OBJECT_0 + 1))
		m_cCancelReadyEvent.Set();

	if (nResult == WAIT_OBJECT_0)
		return eWait_Success;
	else if (nResult == WAIT_TIMEOUT)
		return eWait_TimeOut;
	else
		return eWait_Cancel;
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Peer_Details

CSDTitan_RequestEntry_Peer_Details::CSDTitan_RequestEntry_Peer_Details(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_MsgOp(pDir)
{
}

void CSDTitan_RequestEntry_Peer_Details::Start()
{
	uint32 nNumRetries = k_nNumRetries;
	do
	{
		// Send the detail query
		CLTMsgRef_Write cMsg(GetDir()->CreateMessage());
		cMsg->Writeuint8(eSDTitan_Msg_DetailInfo_Query);
		g_pLTCSBase->SendTo(cMsg->Read(), m_cDestination.GetHostString().c_str(), m_cDestination.GetPort());

		// Wait for a response
		EWaitResult eResult = WaitForMessage(k_nResponseTimeout);
		if (eResult == eWait_Success)
			break;
		else if (eResult == eWait_Cancel)
			return;
	} while (--nNumRetries != 0);

	// We're done
	if (nNumRetries != 0)
		Finish(IServerDirectory::eRequestResult_Success, GetTempString(IDS_WON_DETAIL_SUCCESS));
	else
		Finish(IServerDirectory::eRequestResult_Failed, GetTempString(IDS_WON_DETAIL_FAIL));
}

bool CSDTitan_RequestEntry_Peer_Details::HandleMessage(ILTMessage_Read &cMsg)
{
	// Make sure we're getting what we expect
	if (cMsg.Readuint8() != eSDTitan_Msg_DetailInfo_Response)
	{
		cMsg.Seek(-8);
		return false;
	}

	GetDir()->m_cPeerCS.Enter();

		uint32 nOldActivePeerIndex = GetDir()->GetActivePeerIndex();

		// Switch to the right peer
		GetDir()->SetActivePeer(m_cDestination.GetHostString().c_str());

		// Stuff the information in there
		CServerDirectory_Titan::SPeerData &sActivePeer = GetDir()->GetActivePeerInfo();
		sActivePeer.m_bHasDetails = true;
		uint32 nLength = cMsg.Readuint32();
		sActivePeer.m_cDetails.resize(nLength);
		cMsg.ReadData(&(*(sActivePeer.m_cDetails.begin())), nLength * 8);

		// Restore the active peer
		GetDir()->SetActivePeerIndex(nOldActivePeerIndex);

	GetDir()->m_cPeerCS.Leave();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Validate_Client

CSDTitan_RequestEntry_Validate_Client::CSDTitan_RequestEntry_Validate_Client(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry_MsgOp(pDir)
{
}

void CSDTitan_RequestEntry_Validate_Client::Start()
{
	// Start up the peer-to-peer authentication process
	PeerAuthClient &cPeerAuth = GetDir()->GetPeerAuthClient();
	ByteBufferPtr aMessageBuf;
	cPeerAuth.SetUseAuth2(true);
	aMessageBuf = cPeerAuth.Start(GetDir()->GetAuthContext()->GetPeerData(), AUTH_TYPE_PERSISTENT, 4);

	// Send off the initial message
	CLTMsgRef_Write pMsg(GetDir()->CreateMessage());
	pMsg->Writeuint8(eSDTitan_Msg_PeerAuth_Client);
	pMsg->WriteData(aMessageBuf->data(), aMessageBuf->length() * 8);
	g_pLTCSBase->SendTo(pMsg->Read(), m_cDestination.GetHostString().c_str(), m_cDestination.GetPort());

	// It is now up to fate
}

bool CSDTitan_RequestEntry_Validate_Client::HandleMessage(ILTMessage_Read &cMsg)
{
	// Make sure we're getting what we expect
	if (cMsg.Readuint8() != eSDTitan_Msg_PeerAuth_Server)
	{
		cMsg.Seek(-8);
		return false;
	}

	// Dump the message into a buffer
	char aTempBuff[1024];
	uint32 nLength = LTMIN(sizeof(aTempBuff), (cMsg.TellEnd() + 7) / 8);
	cMsg.ReadData(aTempBuff, nLength * 8);

	// Pass the buffer through the authentication stuff
	ByteBufferPtr aResultBuffer;
	GetDir()->GetPeerAuthCS().Enter();
	WONStatus nResult = GetDir()->GetPeerAuthClient().HandleRecvMsg(aTempBuff + 4, nLength - 4, aResultBuffer);
	GetDir()->GetPeerAuthCS().Leave();

	// Handle a successful result
	if (nResult == WS_Success)
	{
		CWinSync_CSAuto cProtection(GetDir()->GetPeerInfoCS());

		uint32 nOldActivePeer = GetDir()->GetActivePeerIndex();

		// Figure out whom we are dealing with
		GetDir()->SetActivePeer(m_cDestination.GetHostAndPortString().c_str());

		// Servers always get immediate validation..
		GetDir()->GetActivePeerInfo().m_eValidation = CServerDirectory_Titan::SPeerData::eValidationState_Complete;

		// Go back to the previous active peer
		GetDir()->SetActivePeerIndex(nOldActivePeer);
	}

	// Send a response if necessary
	if (aResultBuffer.get())
	{
		CLTMsgRef_Write pResponseMsg(GetDir()->CreateMessage());
		pResponseMsg->WriteData(aResultBuffer->data(), aResultBuffer->length() * 8);
		g_pLTCSBase->SendTo(pResponseMsg->Read(), m_cDestination.GetHostString().c_str(), m_cDestination.GetPort());
	}
	else if (nResult == WS_Success)
	{
		// OK, we're done
		Finish(IServerDirectory::eRequestResult_Success, GetTempString(IDS_WON_VAL_SUCCESS));
	}

	if (nResult != WS_Success)
	{
		// Something went wrong...
		Finish(IServerDirectory::eRequestResult_Failed, std::string(GetTempString(IDS_WON_VAL_SUCCESS)) + std::string(" (") + std::string(WONStatusToString(nResult)) + std::string(")"));
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CSDTitan_RequestEntry_Validate_Server

CSDTitan_RequestEntry_Validate_Server::CSDTitan_RequestEntry_Validate_Server(CServerDirectory_Titan *pDir) :
	CSDTitan_RequestEntry(pDir)
{
}

void CSDTitan_RequestEntry_Validate_Server::Start()
{
	{
		CWinSync_CSAuto cProtection(GetDir()->GetPeerAuthCS());

		// All we need to do for this request is start the PeerAuthServer object
		PeerAuthServer &cPeerAuth = GetDir()->GetPeerAuthServer();
		if (cPeerAuth.GetState() == PeerAuthServer::STATE_NOT_STARTED)
		{
			cPeerAuth.SetUseAuth2(true);
			cPeerAuth.Start(GetDir()->GetAuthContext()->GetPeerData(), 4);
		}
	}

	// Remember that the server's been validated
	GetDir()->GetPeerInfoCS().Enter();
		GetDir()->GetLocalPeerInfo().m_eValidation = CServerDirectory_Titan::SPeerData::eValidationState_Complete;
	GetDir()->GetPeerInfoCS().Leave();

	// We're done here.
	Finish(IServerDirectory::eRequestResult_Success, GetTempString(IDS_WON_SERV_VAL));
}

//////////////////////////////////////////////////////////////////////////////
// CServerDirectory_Titan IServerDirectory implementation


CServerDirectory_Titan::CServerDirectory_Titan() :
	m_pDirServerList(new ServerContext),
	m_pAuthContext(new AuthContext),
	m_bIsAuthContextValid(false),
	m_sGameName(DEFAULT_TITAN_PRODUCT_STRING),
	m_cWONAPICore(true),
	m_cCDKey(DEFAULT_TITAN_PRODUCT_STRING),
	m_nActivePeerIndex(0),
	m_eCurStatus(eStatus_Waiting),
	m_sCurStatusString(GetTempString(IDS_WAITING)),
	m_eCompletionAction(eCompletionBlock_None),
	m_eLastSuccessfulRequest(eRequest_Nothing),
	m_eLastErrorRequest(eRequest_Nothing),
	m_eLastRequest(eRequest_Nothing),
	m_eLastRequestResult(eRequestResult_Success),
	m_eVersionState(eVersion_Unknown)
{
	TitanTest_Init();

	LoadDirServerList();

	m_pAuthContext->SetCommunity(Make_wstring(GetGameName()));

	m_cCDKey.LoadFromRegistry();

	// Make sure we have a local peer
	m_aPeerInfo.resize(1);
	m_aPeerInfo.front().m_cAddr = IPAddr::GetLocalAddr();
	m_aPeerInfo.front().m_cAddr.SetThePort(TITAN_GAMESERVER_DEFAULTPORT);

	// Create the start request thread
	uint32 nThreadID;
	m_hStartRequestThread = CreateThread(NULL, 0, StartRenderThread_Bootstrap, this, 0, (unsigned long *)&nThreadID);
	m_cSRTReadyEvent.Block();

	m_pGameSpySupport = NULL;
}

CServerDirectory_Titan::~CServerDirectory_Titan()
{
	DestroyGameSpySupport( );

	// Make sure we're not doing anything
	PauseRequestList();

	// Get rid of the waiting requests
	ClearRequestList();

	// Tell everything to shut down
	m_cShutdownEvent.Set();

	// Wait for the threads
	WaitForSingleObject(m_hStartRequestThread, INFINITE);

	// Delete the threads
	CloseHandle(m_hStartRequestThread);

	TitanTest_Term();
}


bool CServerDirectory_Titan::QueueRequest(ERequest eNewRequest)
{
	if (!ValidateRequest(eNewRequest))
		return false;

	CSDTitan_RequestEntry *pRequestEntry = MakeRequestEntry(eNewRequest);
	if( !pRequestEntry )
		return false;

	StartRequest(pRequestEntry);

	return true;
}

bool CServerDirectory_Titan::QueueRequestList(const TRequestList &cNewRequests)
{
	TRequestList::const_iterator iCurRequest = cNewRequests.begin();
	for (; iCurRequest != cNewRequests.end(); ++iCurRequest)
	{
		if (!ValidateRequest(*iCurRequest))
			return false;
	}

	for (; iCurRequest != cNewRequests.end(); ++iCurRequest)
	{
		CSDTitan_RequestEntry *pRequestEntry = MakeRequestEntry(*iCurRequest);
		if( !pRequestEntry )
			return false;

		StartRequest(pRequestEntry);
	}

	return true;
}

IServerDirectory::TRequestList CServerDirectory_Titan::GetWaitingRequestList() const
{
	TRequestList cResult;

	CWinSync_CSAuto cProtection(m_cStateCS);

	// Give me some room
	cResult.reserve(m_cRequestEntryList.size());

	// Bring home the bacon
	TRequestEntryList::const_iterator iCurRequestEntry = m_cRequestEntryList.begin();
	for (; iCurRequestEntry != m_cRequestEntryList.end(); ++iCurRequestEntry)
		cResult.push_back((*iCurRequestEntry)->GetRequest());

	return cResult;
}

bool CServerDirectory_Titan::ClearRequestList()
{
	// Stop right there, mister!
	EStatus eOldStatus;
	std::string sOldStatusStr;
	TemporaryPause(GetTempString(IDS_WON_CLEAR_REQ_LIST), &eOldStatus, &sOldStatusStr);

	m_cStateCS.Enter();

		// Delete all the requests
		TRequestEntryList::const_iterator iCurRequestEntry = m_cRequestEntryList.begin();
		for (; iCurRequestEntry != m_cRequestEntryList.end(); ++iCurRequestEntry)
			delete *iCurRequestEntry;

		// Clear the list
		m_cRequestEntryList.clear();

	m_cStateCS.Leave();

	// As you were
	TemporaryResume(eOldStatus, sOldStatusStr.c_str());

	return true;
}

IServerDirectory::ERequestResult CServerDirectory_Titan::ProcessRequest(ERequest eNewRequest, uint32 nTimeout)
{
	if (!ValidateRequest(eNewRequest))
		return eRequestResult_Failed;

	// Stop right there, mister!
	EStatus eOldStatus;
	std::string sOldStatusStr;
	TemporaryPause(GetTempString(IDS_WON_PROCESS_REQ), &eOldStatus, &sOldStatusStr);

	// Push the request on the front of the list
	m_cStateCS.Enter();
		TRequestEntryList cOldEntryList;
		cOldEntryList.swap(m_cRequestEntryList);
		m_cRequestEntryList.push_front(MakeRequestEntry(eRequest_Pause));
		m_cRequestEntryList.push_front(MakeRequestEntry(eNewRequest));
	m_cStateCS.Leave();

	// Wait for it to finish
	ERequestResult eResult = BlockOnRequest(eRequest_Pause, nTimeout);

	if (eResult == eRequestResult_InProgress)
	{
		CancelActiveRequest();
	}
	m_cStateCS.Enter();
		cOldEntryList.swap(m_cRequestEntryList);
	m_cStateCS.Leave();

	// As you were
	TemporaryResume(eOldStatus, sOldStatusStr.c_str());

	return eResult;
}


bool CServerDirectory_Titan::PauseRequestList()
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	switch (m_eCurStatus)
	{
		case eStatus_Error :
		{
			// If we're in an error state, we're already paused..
			return false;
		}
		case eStatus_Processing :
		{
			// Cancel the active request
			CancelActiveRequest();
			break;
		}
		default :
		{
			break;
		}
	}

	m_eCurStatus = eStatus_Paused;
	return true;
}

bool CServerDirectory_Titan::ProcessRequestList()
{
	// Don't do anything if we're already processing
	EStatus eCurStatus = GetCurStatus();
	if ((eCurStatus == eStatus_Processing) || (eCurStatus == eStatus_Waiting))
		return true;

	// Is a request list empty?
	if (m_cRequestEntryList.empty())
	{
		// Ok, we're going to wait for you to do something then
		m_eCurStatus = eStatus_Waiting;
		m_sCurStatusString = GetTempString(IDS_WAITING);
		return true;
	}

	// Say we're processing
	m_eCurStatus = eStatus_Processing;

	// Start processing the active request
	StartActiveRequest();

	return true;
}


IServerDirectory::ERequestResult CServerDirectory_Titan::BlockOnActiveRequest(uint32 nTimeout)
{
	// Set up the completion blocking state
	{
		CWinSync_CSAuto cProtection(m_cStateCS);

		if (m_cRequestEntryList.empty())
			return eRequestResult_Aborted;

		m_eCompletionAction = eCompletionBlock_Active;
	}

	return BlockOnCompletion(nTimeout);
}

IServerDirectory::ERequestResult CServerDirectory_Titan::BlockOnRequest(ERequest eBlockRequest, uint32 nTimeout)
{
	// This is just a convenience function for BlockOnRequestList
	TRequestList cTempList;
	cTempList.push_back(eBlockRequest);
	return BlockOnRequestList(cTempList, nTimeout);
}

IServerDirectory::ERequestResult CServerDirectory_Titan::BlockOnRequestList(const TRequestList &cBlockRequestList, uint32 nTimeout)
{
	// Set up the completion blocking state
	{
		CWinSync_CSAuto cProtection(m_cStateCS);

		if (m_cRequestEntryList.empty())
			return eRequestResult_Aborted;

		m_eCompletionAction = eCompletionBlock_List;
		m_cCompletionList = cBlockRequestList;
	}

	return BlockOnCompletion(nTimeout);
}

IServerDirectory::ERequestResult CServerDirectory_Titan::BlockOnProcessing(uint32 nTimeout)
{
	// Set up the completion blocking state
	{
		CWinSync_CSAuto cProtection(m_cStateCS);

		if (m_cRequestEntryList.empty())
			return eRequestResult_Success;

		m_eCompletionAction = eCompletionBlock_Processing;
	}

	return BlockOnCompletion(nTimeout);
}

bool CServerDirectory_Titan::IsRequestPending(ERequest ePendingRequest) const
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	// Search the active request list for this request
	TRequestEntryList::const_iterator iCurRequest = m_cRequestEntryList.begin();
	for (; iCurRequest != m_cRequestEntryList.end(); ++iCurRequest)
	{
		if ((*iCurRequest)->GetRequest() == ePendingRequest)
			return true;
	}

	return false;
}

IServerDirectory::ERequest CServerDirectory_Titan::GetLastSuccessfulRequest() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_eLastSuccessfulRequest;
}

IServerDirectory::ERequest CServerDirectory_Titan::GetLastErrorRequest() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_eLastErrorRequest;
}

IServerDirectory::ERequest CServerDirectory_Titan::GetActiveRequest() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	if (m_cRequestEntryList.empty())
		return eRequest_Nothing;
	else
		return m_cRequestEntryList.front()->GetRequest();
}


IServerDirectory::ERequest CServerDirectory_Titan::GetLastRequest() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_eLastRequest;
}

IServerDirectory::ERequestResult CServerDirectory_Titan::GetLastRequestResult() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_eLastRequestResult;
}

char const* CServerDirectory_Titan::GetLastRequestResultString() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_sLastRequestResultString.c_str( );
}



IServerDirectory::EStatus CServerDirectory_Titan::GetCurStatus() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_eCurStatus;
}

char const* CServerDirectory_Titan::GetCurStatusString() const
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	return m_sCurStatusString.c_str( );
}

void CServerDirectory_Titan::SetGameName(const char *pName)
{
	CWinSync_CSAuto cProtection(m_cGameNameCS);
	m_sGameName = pName;

	// Set all the other stuff that's going to use the name
	m_cCDKey.SetProductString(pName);
	m_cAuthContextCS.Enter();
		m_pAuthContext->SetCommunity(Make_wstring(pName));
	m_cAuthContextCS.Leave();
}

char const* CServerDirectory_Titan::GetGameName() const
{
	CWinSync_CSAuto cProtection(m_cGameNameCS);
	return m_sGameName.c_str( );
}


void CServerDirectory_Titan::SetStartupInfo( ILTMessage_Read& cMsg )
{
	m_StartupInfo.m_sGameSpyName = "";
	m_StartupInfo.m_sGameSpySecretKey = "";

	StartupInfo_Titan const* pStartupInfo = ( StartupInfo_Titan* )cMsg.Readuint32( );
	if( !pStartupInfo )
		return;

	m_StartupInfo = *pStartupInfo;
}


void CServerDirectory_Titan::GetStartupInfo( ILTMessage_Write& cMsg )
{
	cMsg.Writeuint32(( uint32 )&m_StartupInfo );
}

bool CServerDirectory_Titan::SetCDKey(const char *pKey)
{
	// Test the CD key first
	CDKey cTestCDKey(GetGameName());
	cTestCDKey.Init(pKey);
	if (!cTestCDKey.IsValid())
		return false;

	// Save it
	m_cCDKey.Init(pKey);
	m_cCDKey.SaveToRegistry();

	return true;
}

bool CServerDirectory_Titan::GetCDKey(std::string *pKey)
{
	if (!pKey)
		return false;

	*pKey = m_cCDKey.GetString();

	return true;
}

bool CServerDirectory_Titan::IsCDKeyValid() const
{
	return IsAuthContextValid();
}



void CServerDirectory_Titan::SetVersion(const char *pVersion)
{
	CWinSync_CSAuto cProtection(m_cVersionCS);
	m_sCurVersion = pVersion;
	m_eVersionState = eVersion_Unknown;
}

void CServerDirectory_Titan::SetRegion(const char *pRegion)
{
	CWinSync_CSAuto cProtection(m_cVersionCS);
	m_sCurRegion = pRegion;
}

bool CServerDirectory_Titan::IsVersionValid() const
{
	CWinSync_CSAuto cProtection(m_cVersionCS);
	return m_eVersionState != eVersion_Unknown;
}

bool CServerDirectory_Titan::IsVersionNewest() const
{
	CWinSync_CSAuto cProtection(m_cVersionCS);
	return m_eVersionState == eVersion_Latest;
}

bool CServerDirectory_Titan::IsPatchAvailable() const
{
	CWinSync_CSAuto cProtection(m_cVersionCS);
	return m_eVersionState == eVersion_Patchable;
}


bool CServerDirectory_Titan::IsMOTDNew(EMOTD eMOTD) const
{
	CWinSync_CSAuto cProtection(m_cMOTDCS);
	return m_aMOTD[eMOTD].m_bNew;
}

char const* CServerDirectory_Titan::GetMOTD(EMOTD eMOTD) const
{
	CWinSync_CSAuto cProtection(m_cMOTDCS);
	return m_aMOTD[eMOTD].m_sMessage.c_str( );
}



bool CServerDirectory_Titan::SetActivePeer(const char *pAddr)
{
	CWinSync_CSAuto cProtection(m_cPeerCS);

	// Handle setting to the local peer
	if (!pAddr)
	{
		m_nActivePeerIndex = 0;
		return true;
	}

	// Get the IP address
	IPAddr nActiveIP(pAddr);
	// Fail an invalid IP
	if (!nActiveIP.IsValid())
		return false;

	// Go searching..
	for (uint32 nCurPeer = 1; nCurPeer < m_aPeerInfo.size(); ++nCurPeer)
	{
		if (m_aPeerInfo[nCurPeer].m_cAddr == nActiveIP)
		{
			m_nActivePeerIndex = nCurPeer;
			return true;
		}
	}

	// Didn't find it..  Add a new one
	SPeerData spd;
	m_aPeerInfo.push_back(spd);
	m_aPeerInfo.back().m_cAddr.Set(pAddr);
	m_nActivePeerIndex = m_aPeerInfo.size() - 1;

	return true;
}

bool CServerDirectory_Titan::GetActivePeer(std::string *pAddr, bool *pLocal) const
{
	// Check the parameters
	if (!pAddr || !pLocal)
		return false;

	CWinSync_CSAuto cProtection(m_cPeerCS);

	// Handle the local peer..
	if (m_nActivePeerIndex == 0)
	{
		pAddr->clear();
		*pLocal = true;
		return true;
	}

	// Handle a non-local peer
	const SPeerData &cActivePeer = m_aPeerInfo[m_nActivePeerIndex];
	*pAddr = cActivePeer.m_cAddr.GetHostAndPortString();
	*pLocal = false;

	return true;
}

bool CServerDirectory_Titan::RemoveActivePeer()
{
	CWinSync_CSAuto cProtection(m_cPeerCS);

	// Can't remove the local peer
	if (m_nActivePeerIndex == 0)
		return false;

	// Remove it.
	m_aPeerInfo.erase(m_aPeerInfo.begin() + m_nActivePeerIndex);

	// Point back at the local peer
	m_nActivePeerIndex = 0;

	return true;
}


bool CServerDirectory_Titan::SetActivePeerInfo(EPeerInfo eInfoType, ILTMessage_Read &cMsg)
{
	CLTMsgRef_Read cMsgRef(&cMsg);

	cMsg.SeekTo(0);

	CWinSync_CSAuto cProtection(m_cPeerCS);

	SPeerData &cActivePeer = m_aPeerInfo[m_nActivePeerIndex];

	switch (eInfoType)
	{
		case ePeerInfo_Port :
		{
			uint16 nNewPort = cMsg.Readuint16();
			cActivePeer.m_cAddr.SetThePort((unsigned short)nNewPort);
			break;
		}
		case ePeerInfo_Name :
		{
			char aNameBuff[MAX_PACKET_LEN];
			if (cMsg.ReadString(aNameBuff, MAX_PACKET_LEN) > MAX_PACKET_LEN)
				return false;
			cActivePeer.m_sName = aNameBuff;
			cActivePeer.m_bHasName = true;
			break;
		}
		case ePeerInfo_Summary :
		{
			uint32 nLength = cMsg.Size();
			cActivePeer.m_cSummary.resize((nLength + 7) / 8);
			cMsg.ReadData(&(*(cActivePeer.m_cSummary.begin())), nLength);
			cActivePeer.m_bHasSummary = true;
			break;
		}
		case ePeerInfo_Service :
		{
			PeerInfo_Service_Titan* pInfo = ( PeerInfo_Service_Titan* )cMsg.Readuint32( );
			if( !pInfo )
				return false;
			if( !SetPeerInfoService( cActivePeer, *pInfo ))
				return false;

			break;
		}
		case ePeerInfo_Details :
		{
			uint32 nLength = cMsg.Size();
			cActivePeer.m_cDetails.resize((nLength + 7) / 8);
			cMsg.ReadData(&(*(cActivePeer.m_cDetails.begin())), nLength);
			cActivePeer.m_bHasDetails = true;
			break;
		}
		case ePeerInfo_Validated :
		{
			uint8 nIsValidated = cMsg.Readuint8();
			if (nIsValidated)
				cActivePeer.m_eValidation = SPeerData::eValidationState_Complete;
			else
				cActivePeer.m_eValidation = SPeerData::eValidationState_None;
			break;
		}
		case ePeerInfo_Age :
		{
			float fNewAge = cMsg.Readfloat();
				return false;
			cActivePeer.m_fCreationTime = g_pLTCSBase->GetTime() - fNewAge;
			break;
		}
		case ePeerInfo_Ping :
		{
			cActivePeer.m_nPing = cMsg.Readuint16();
			cActivePeer.m_bHasPing = true;
			break;
		}
		default :
			ASSERT(!"Unknown peer info request encountered");
			return false;
	}

	cMsg.SeekTo(0);

	return true;
}

bool CServerDirectory_Titan::HasActivePeerInfo(EPeerInfo eInfoType) const
{
	CWinSync_CSAuto cProtection(m_cPeerCS);

	const SPeerData &cActivePeer = m_aPeerInfo[m_nActivePeerIndex];

	switch (eInfoType)
	{
		case ePeerInfo_Port :
		case ePeerInfo_Validated :
		case ePeerInfo_Age :
			return true;
		case ePeerInfo_Name :
			return cActivePeer.m_bHasName;
		case ePeerInfo_Summary :
			return cActivePeer.m_bHasSummary;
		case ePeerInfo_Details :
			return cActivePeer.m_bHasDetails;
		case ePeerInfo_Ping :
			return cActivePeer.m_bHasPing;
	}

	return false;
}

bool CServerDirectory_Titan::GetActivePeerInfo(EPeerInfo eInfoType, ILTMessage_Write *pMsg) const
{
	if (!pMsg)
		return false;

	pMsg->Reset();

	CWinSync_CSAuto cProtection(m_cPeerCS);

	const SPeerData &cActivePeer = m_aPeerInfo[m_nActivePeerIndex];

	switch (eInfoType)
	{
		case ePeerInfo_Port :
		{
			pMsg->Writeuint16((uint16)cActivePeer.m_cAddr.GetPort());
			break;
		}
		case ePeerInfo_Name :
		{
			pMsg->WriteString(cActivePeer.m_sName.c_str());
			break;
		}
		case ePeerInfo_Summary :
		{
			pMsg->WriteData(&(*(cActivePeer.m_cSummary.begin())), cActivePeer.m_cSummary.size() * 8);
			break;
		}
		case ePeerInfo_Details :
		{
			pMsg->WriteData(&(*(cActivePeer.m_cDetails.begin())), cActivePeer.m_cDetails.size() * 8);
			break;
		}
		case ePeerInfo_Service :
		{
			pMsg->Writeuint32(( uint32 )&cActivePeer.m_PeerInfoService );
			break;
		}
		case ePeerInfo_Validated :
		{
			pMsg->Writeuint8((cActivePeer.m_eValidation == SPeerData::eValidationState_Complete) ? 1 : 0);
			break;
		}
		case ePeerInfo_Age :
		{
			pMsg->Writefloat(g_pLTCSBase->GetTime() - cActivePeer.m_fCreationTime);
			break;
		}
		case ePeerInfo_Ping :
		{
			if (!cActivePeer.m_bHasPing)
				return false;
			pMsg->Writeuint16(cActivePeer.m_nPing);
			break;
		}
		default :
			return false;
	}

	return true;
}



IServerDirectory::TPeerList CServerDirectory_Titan::GetPeerList() const
{
	CWinSync_CSAuto cProtection(m_cPeerCS);

	TPeerList cResult;
	cResult.reserve(m_aPeerInfo.size() - 1);

	TPeerDataList::const_iterator iCurServer = m_aPeerInfo.begin() + 1;
	for (; iCurServer != m_aPeerInfo.end(); ++iCurServer)
	{
		cResult.push_back(iCurServer->m_cAddr.GetHostAndPortString());
	}

	return cResult;
}

void CServerDirectory_Titan::ClearPeerList()
{
	CWinSync_CSAuto cProtection(m_cPeerCS);

	// Don't do anything if it's empty
	if (m_aPeerInfo.size() == 1)
		return;

	// Kill them all dead
	m_aPeerInfo.erase(m_aPeerInfo.begin() + 1, m_aPeerInfo.end());
}


bool CServerDirectory_Titan::HandleNetMessage(ILTMessage_Read &cStr, const char *pSender, uint16 nPort)
{
	// Who sent you?
	IPAddr cSenderAddr(pSender, nPort);

	// Find out the message ID
	uint8 nMsgID = cStr.Peekuint8();

	// Handle messages we're going to handle by ourselves
	switch (nMsgID)
	{
		case eSDTitan_Msg_DetailInfo_Query :
			return HandleNetMsg_DetailInfo_Query(cStr, cSenderAddr);
		case eSDTitan_Msg_PeerAuth_Client :
			return HandleNetMsg_PeerAuth_Client(cStr, cSenderAddr);
	}

	// We have to be actively processing to handle any other messages...
	if (GetCurStatus() != eStatus_Processing)
		return false;

	// Hand it off to the active request
	m_cStateCS.Enter();
		CSDTitan_RequestEntry *pActiveRequest = m_cRequestEntryList.front();
	m_cStateCS.Leave();

	return pActiveRequest->OnMessage(cStr, cSenderAddr);
}

bool CServerDirectory_Titan::SetNetHeader(ILTMessage_Read &cStr)
{
	CLTMsgRef_Read cMsgRef(&cStr);

	// Copy the header
	CWinSync_CSAuto cProtection(m_cMsgHeaderCS);
	m_pMsgHeader = cStr.Clone();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CServerDirectory_Titan private function implementations

void CServerDirectory_Titan::FinishRequest(CSDTitan_RequestEntry *pRequest, ERequestResult eResult, const std::string &sResultStr)
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	// Make sure nothing wonky is afoot
	if (m_cRequestEntryList.empty())
	{
		ASSERT(!"Finished request encountered from empty list");
		return;
	}

	// Make sure we're actually processing....
	if (m_eCurStatus != eStatus_Processing)
	{
		ASSERT(!"Finished request encountered while not processing");
		return;
	}

	// Remove it from the list
	if (pRequest == m_cRequestEntryList.front())
		m_cRequestEntryList.pop_front();
	else
	{
		ASSERT(!"Out of order request completion encountered");
		TRequestEntryList::iterator iRemoveList = std::remove(m_cRequestEntryList.begin(), m_cRequestEntryList.end(), pRequest);
		m_cRequestEntryList.erase(iRemoveList, m_cRequestEntryList.end());
	}

	// Who were you again?
	ERequest eCurRequest = pRequest->GetRequest();

	// Ok, we're done with the request now
	delete pRequest;
	pRequest = 0;

	// Handle a pause request
	if (eCurRequest == eRequest_Pause)
	{
		m_eCurStatus = eStatus_Paused;
	}
	// Otherwise set up the last result variables
	else
	{
		m_eLastRequest = eCurRequest;
		m_eLastRequestResult = eResult;
		m_sLastRequestResultString = sResultStr;
		if (eResult == eRequestResult_Success)
			m_eLastSuccessfulRequest = m_eLastRequest;
		else
		{
			m_eLastErrorRequest = m_eLastRequest;
			m_eCurStatus = eStatus_Error;
			m_sCurStatusString = sResultStr;
		}
	}

	// Unblock any functions waiting on completion of an action
	bool bUnblockCompletion;
	switch (m_eCompletionAction)
	{
		case eCompletionBlock_None :
		{
			bUnblockCompletion = false;
			break;
		}
		case eCompletionBlock_Active :
		{
			bUnblockCompletion = true;
			break;
		}
		case eCompletionBlock_List :
		{
			bUnblockCompletion = eResult != eRequestResult_Success;
			TRequestList::const_iterator iCurRequest = m_cCompletionList.begin();
			for (; (iCurRequest != m_cCompletionList.end()) && (!bUnblockCompletion); ++iCurRequest)
				bUnblockCompletion = (*iCurRequest == eCurRequest);
			break;
		}
		case eCompletionBlock_Processing :
		{
			bUnblockCompletion = m_cRequestEntryList.empty();
			break;
		}
	}

	// Unblock..
	if (bUnblockCompletion)
	{
		m_cCompletionEvent_Continue.Clear();
		m_cCompletionEvent.Set();
		m_cStateCS.Leave();
			m_cCompletionEvent_Continue.Block();
		m_cStateCS.Enter();
	}

	// If we're supposed to stop processing, we're done here...
	if ((m_eCurStatus == eStatus_Paused) || (m_eCurStatus == eStatus_Error))
		return;

	// Check for being empty
	if (m_cRequestEntryList.empty())
	{
		m_eCurStatus = eStatus_Waiting;
		m_sCurStatusString = GetTempString(IDS_WAITING);
		return;
	}

	// Start up the next request
	StartActiveRequest();
}

void CServerDirectory_Titan::SetMOTD(EMOTD eMOTD, const char *pMessage, bool bIsNew)
{
	CWinSync_CSAuto cProtection(m_cMOTDCS);

	m_aMOTD[eMOTD].m_bNew = bIsNew;
	m_aMOTD[eMOTD].m_sMessage = pMessage;
}

void CServerDirectory_Titan::SetVersionState(EVersionState eState)
{
	CWinSync_CSAuto cProtection(m_cVersionCS);

	m_eVersionState = eState;
}

char const* CServerDirectory_Titan::GetVersion() const
{
	CWinSync_CSAuto cProtection(m_cVersionCS);

	return m_sCurVersion.c_str( );
}

char const* CServerDirectory_Titan::GetRegion() const
{
	CWinSync_CSAuto cProtection(m_cVersionCS);

	return m_sCurRegion.c_str( );
}

void CServerDirectory_Titan::SetAuthContextValid(bool bValue)
{
	CWinSync_CSAuto cProtection(m_cAuthContextCS);
	m_bIsAuthContextValid = bValue;
	m_pAuthContext->SetDoAutoRefresh(bValue);
}

void CServerDirectory_Titan::SetCurStatusString(const char *pMessage)
{
	CWinSync_CSAuto cProtection(m_cStateCS);
	m_sCurStatusString = pMessage;
}

ILTMessage_Write *CServerDirectory_Titan::CreateMessage()
{
	ILTMessage_Write *pResult;
	g_pLTCSBase->Common( )->CreateMessage(pResult);

	m_cMsgHeaderCS.Enter();
		pResult->WriteMessageRaw(m_pMsgHeader);
	m_cMsgHeaderCS.Leave();

	return pResult;
}

bool CServerDirectory_Titan::ValidateRequest(ERequest eNewRequest) const
{
	switch (eNewRequest)
	{
		case eRequest_Validate_CDKey :
		{
			// Don't validate unless we think it's valid locally
			return m_cCDKey.IsValid();
		}
		case eRequest_MOTD :
		case eRequest_Update_List :
		case eRequest_Update_Pings :
		{
			return IsCDKeyValid( );
		}
		case eRequest_Validate_Client :
		case eRequest_Validate_Server :
		{
			// Make sure we're not trying to do this request with the local peer
			std::string sTempAddr;
			bool bLocal;
			GetActivePeer(&sTempAddr, &bLocal);
			return !bLocal;
		}
		case eRequest_Peer_Name :
		case eRequest_Peer_Summary :
		case eRequest_Peer_Details :
		{
			// Make sure we're not trying to do this request with the local peer
			std::string sTempAddr;
			bool bLocal;
			GetActivePeer(&sTempAddr, &bLocal);
			if( bLocal )
				return false;

			return IsCDKeyValid( );
		}
		case eRequest_Remove_Server :
		{
			// Make sure we are trying to do this request with the local peer
			std::string sTempAddr;
			bool bLocal;
			GetActivePeer(&sTempAddr, &bLocal);
			return bLocal;
		}
		case eRequest_Publish_Server :
		{
			// Make sure we're trying to publish the local peer
			std::string sTempAddr;
			bool bLocal;
			GetActivePeer(&sTempAddr, &bLocal);
			if (!bLocal)
				return false;

			// Make sure we've got all of the information we'll need to publish
			if (!HasActivePeerInfo(ePeerInfo_Name))
				return false;
			if (!HasActivePeerInfo(ePeerInfo_Summary))
				return false;
			if (!HasActivePeerInfo(ePeerInfo_Details))
				return false;

			return true;
		}

		default : return true;
	}
}

void CServerDirectory_Titan::LoadDirServerList()
{
	FILE *pFile;
	char szBuffer[256];
	char szServer[256];

	// Read the directory servers out of the SierraUp.cfg file
	pFile = fopen("sierraup.cfg", "r");
	if (!pFile)
		return;

	while(fgets(szBuffer, sizeof(szBuffer), pFile))
	{
		if(!strnicmp(szBuffer, "DirectoryServer", 15))
		{
			char *pStart = strstr(szBuffer, "\"");
			pStart++;

			char *pEnd = strstr(pStart, "\"");

			strncpy(szServer, pStart, (pEnd - pStart));
			szServer[pEnd - pStart] = 0;

			m_pDirServerList->AddAddress(szServer);
		}
	}
}

bool CServerDirectory_Titan::IsAuthContextValid() const
{
	CWinSync_CSAuto cProtection(m_cAuthContextCS);
	return m_bIsAuthContextValid;
}

CSDTitan_RequestEntry *CServerDirectory_Titan::MakeRequestEntry(ERequest eNewRequest)
{
	switch (eNewRequest)
	{
		case eRequest_Nothing : return new CSDTitan_RequestEntry(this);
		case eRequest_Pause : return new CSDTitan_RequestEntry_Pause(this);
		case eRequest_MOTD : return new CSDTitan_RequestEntry_MOTD(this);
		case eRequest_Validate_Version : return new CSDTitan_RequestEntry_Validate_Version(this);
		case eRequest_Validate_CDKey : return new CSDTitan_RequestEntry_Validate_CDKey(this);
		case eRequest_Publish_Server : return new CSDTitan_RequestEntry_Publish_Server(this);
		case eRequest_Remove_Server : return new CSDTitan_RequestEntry_Remove_Server(this);
		case eRequest_Update_List : return new CSDTitan_RequestEntry_UpdateList(this);
		case eRequest_Update_Pings : return new CSDTitan_RequestEntry_UpdatePings(this);
		case eRequest_Peer_Details : return new CSDTitan_RequestEntry_Peer_Details(this);
		case eRequest_Validate_Client : return new CSDTitan_RequestEntry_Validate_Client(this);
		case eRequest_Validate_Server : return new CSDTitan_RequestEntry_Validate_Server(this);
		// Non-detail peer requests are a noop in this system
		case eRequest_Peer_Name :
		case eRequest_Peer_Summary : return new CSDTitan_RequestEntry_Noop(this, eNewRequest);
		default : ASSERT(!"Unimplemented request entry encountered");
	}

	return 0;
}

void CServerDirectory_Titan::StartRequest(CSDTitan_RequestEntry *pRequest)
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	// Add to the queue of entries
	m_cRequestEntryList.push_back(pRequest);

	// If we're waiting, start it up
	if (m_eCurStatus == eStatus_Waiting)
	{
		StartActiveRequest();
	}
}

void CServerDirectory_Titan::TemporaryPause(const char *pNewStatusStr, EStatus *pOldStatus, std::string *pOldStatusStr)
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	*pOldStatus = m_eCurStatus;
	*pOldStatusStr = m_sCurStatusString;

	PauseRequestList();

	m_sCurStatusString = pNewStatusStr;
}

void CServerDirectory_Titan::TemporaryResume(EStatus eOldStatus, const char *pOldStatusStr)
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	if (eOldStatus == eStatus_Processing)
		ProcessRequestList();

	m_eCurStatus = eOldStatus;
	m_sCurStatusString = pOldStatusStr;
}

void CServerDirectory_Titan::StartActiveRequest()
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	if (m_cRequestEntryList.empty())
		return;

	m_eCurStatus = eStatus_Processing;
	m_cSRTStartEvent.Set();
}

void CServerDirectory_Titan::CancelActiveRequest()
{
	CWinSync_CSAuto cProtection(m_cStateCS);

	if (m_cRequestEntryList.empty())
		return;

	m_cRequestEntryList.front()->Cancel();
}

IServerDirectory::ERequestResult CServerDirectory_Titan::BlockOnCompletion(uint32 nTimeout)
{
	ERequestResult eResult = eRequestResult_InProgress;

	// Make sure we're running..
	if (GetCurStatus() != eStatus_Processing)
		StartActiveRequest();

	// Wait until we're done
	if (m_cCompletionEvent.Block(nTimeout))
	{
		eResult = GetLastRequestResult();
	}

	// Turn off the completion blocking
	m_cStateCS.Enter();
		m_eCompletionAction = eCompletionBlock_None;
	m_cStateCS.Leave();

	// Set the continue event since we don't want to block any more
	m_cCompletionEvent_Continue.Set();

	return eResult;
}

unsigned long _stdcall CServerDirectory_Titan::StartRenderThread_Bootstrap(void *pParam)
{
	CServerDirectory_Titan *pDirectory = reinterpret_cast<CServerDirectory_Titan *>(pParam);
	return (unsigned long)pDirectory->StartRenderThread_Run();
}

uint32 CServerDirectory_Titan::StartRenderThread_Run()
{
	HANDLE aWaitingList[2];
	aWaitingList[0] = m_cShutdownEvent.GetEvent();
	aWaitingList[1] = m_cSRTStartEvent.GetEvent();

	m_cSRTReadyEvent.Set();
	while (WaitForMultipleObjects(2, aWaitingList, FALSE, INFINITE) != WAIT_OBJECT_0)
	{
		CSDTitan_RequestEntry *pRequest;

		{
			CWinSync_CSAuto cProtection(m_cStateCS);

			if (m_cRequestEntryList.empty())
				continue;

			pRequest = m_cRequestEntryList.front();
		}

		pRequest->Start();
	}

	return 0;
}

bool CServerDirectory_Titan::HandleNetMsg_DetailInfo_Query(ILTMessage_Read &cMsg, IPAddr &cAddr)
{
	// Fill out the response
	CLTMsgRef_Write pResponse = CreateMessage();
	pResponse->Writeuint8(eSDTitan_Msg_DetailInfo_Response);
	m_cPeerCS.Enter();
	SPeerData::TRawData &cDetails = m_aPeerInfo.front().m_cDetails;
	pResponse->Writeuint32(cDetails.size());
	pResponse->WriteData(&(*(cDetails.begin())), cDetails.size() * 8);
	m_cPeerCS.Leave();

	// Send it off
	g_pLTCSBase->SendTo(pResponse->Read(), cAddr.GetHostString().c_str(), cAddr.GetPort());

	return true;
}

bool CServerDirectory_Titan::HandleNetMsg_PeerAuth_Client(ILTMessage_Read &cMsg, IPAddr &cAddr)
{
	// Dump the message into a buffer
	char aTempBuff[1024];
	uint32 nLength = LTMIN(sizeof(aTempBuff), (cMsg.TellEnd() + 7) / 8);
	cMsg.ReadData(aTempBuff, nLength);

	// Pass the buffer through the authentication stuff
	ByteBufferPtr aResultBuffer;
	m_cValidationCS.Enter();
	WONStatus nResult = m_cPeerAuthServer.HandleRecvMsg(aTempBuff + 4, nLength - 4, aResultBuffer);
	m_cValidationCS.Leave();

	// Handle a successful result
	if (nResult == WS_Success)
	{
		CWinSync_CSAuto cProtection(m_cPeerCS);

		uint32 nOldActivePeer = GetActivePeerIndex();

		// Figure out whom we are dealing with
		SetActivePeer(cAddr.GetHostAndPortString().c_str());

		// Bump the validation state
		SPeerData &cClient = GetActivePeerInfo();
		switch (cClient.m_eValidation)
		{
			case SPeerData::eValidationState_None :
				cClient.m_eValidation = SPeerData::eValidationState_Partial;
				break;
			case SPeerData::eValidationState_Partial :
				cClient.m_eValidation = SPeerData::eValidationState_Complete;
				break;
			case SPeerData::eValidationState_Complete :
				break;
		}

		// Go back to the previous active peer
		SetActivePeerIndex(nOldActivePeer);
	}

	// Send a response if necessary
	if (aResultBuffer.get())
	{
		CLTMsgRef_Write pResponseMsg = CreateMessage();
		pResponseMsg->Writeuint8(eSDTitan_Msg_PeerAuth_Server);
		pResponseMsg->WriteData(aResultBuffer->data(), aResultBuffer->length() * 8);
		g_pLTCSBase->SendTo(pResponseMsg->Read(), cAddr.GetHostString().c_str(), cAddr.GetPort());
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// CServerDirectory_Titan::SPeerData implementation

CServerDirectory_Titan::SPeerData::SPeerData() :
	m_eValidation(eValidationState_None),
	m_bHasName(false),
	m_bHasSummary(false),
	m_bHasDetails(false),
	m_bHasPing(false),
	m_fCreationTime(g_pLTCSBase->GetTime())
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerDirectory_Titan::SetPeerInfoService
//
//	PURPOSE:	Sets the service peer info.
//
// ----------------------------------------------------------------------- //
bool CServerDirectory_Titan::SetPeerInfoService( SPeerData& activePeer, PeerInfo_Service_Titan const& info )
{
	CWinSync_CSAuto cGSSProtection(m_cGameSpySupportCS);

	if( m_pGameSpySupport == NULL )
		return false;

	CWinSync_CSAuto cPeerProtection(m_cPeerCS);

	// Copy it in for reference.
	activePeer.m_PeerInfoService = info;

	m_pGameSpySupport->LockKeyValues( );
	{
		char szValue[256] = "";
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "hostname", info.m_sHostName.c_str( ));
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "hostip", activePeer.m_cAddr.GetHostString( ).c_str( ));
		sprintf( szValue, "%d", activePeer.m_cAddr.GetPort( ));
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "hostport", szValue );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "mapname", info.m_sCurWorld.c_str( ));
		sprintf( szValue, "%d", info.m_nCurNumPlayers );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "numplayers", szValue );
		sprintf( szValue, "%d", info.m_nMaxNumPlayers );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "maxplayers", szValue );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "gametype", info.m_sGameType.c_str( ));
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Info, "gamemode", "openplaying" );
		sprintf( szValue, "%d", info.m_bUsePassword ? 1 : 0 );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Rules, "password", szValue );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Rules, "options", info.m_sServerOptions.c_str( ));
		sprintf( szValue, "%d", info.m_nScoreLimit );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Rules, "fraglimit", szValue );
		sprintf( szValue, "%d", info.m_nTimeLimit );
		m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Rules, "timelimit", szValue );

		int nPlayer = 0;
		char szTag[256] = "";
		PeerInfo_Service_Titan::PlayerList::const_iterator iter = info.m_PlayerList.begin( );
		while( iter != info.m_PlayerList.end( ))
		{
			PeerInfo_Service_Titan::Player const& player = *iter;

			sprintf( szTag, "player_%d", nPlayer );
			m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Players, szTag, player.m_sName.c_str( ));
			sprintf( szTag, "frags_%d", nPlayer );
			sprintf( szValue, "%i", player.m_nScore );
			m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Players, szTag, szValue );
			sprintf( szTag, "ping_%d", nPlayer );
			sprintf( szValue, "%d", player.m_nPing  );
			m_pGameSpySupport->SetKeyValue( GameSpyQueryType_Players, szTag, szValue );

			nPlayer++;
			iter++;
		}
	}
	m_pGameSpySupport->UnlockKeyValues( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerDirectory_Titan::CreateGameSpySupport
//
//	PURPOSE:	Creates the gamespysupport object
//
// ----------------------------------------------------------------------- //
bool CServerDirectory_Titan::CreateGameSpySupport( )
{
	CWinSync_CSAuto cProtection(m_cGameSpySupportCS);

	// Start clean.
	if( m_pGameSpySupport != NULL )
	{
		DestroyGameSpySupport( );
	}

	// Create a new gamespysupport.
	m_pGameSpySupport = new GameSpySupport( m_StartupInfo.m_sGameSpyName.c_str( ),
		GetVersion( ), m_StartupInfo.m_sGameSpySecretKey.c_str( ), GetDirServerList( ));
	if( m_pGameSpySupport == NULL )
		return false;

	// Start it up with our local IP.
	IPAddr gamespyaddress = IPAddr::GetLocalAddr();
	SPeerData &cActivePeer = m_aPeerInfo[m_nActivePeerIndex];
	gamespyaddress.SetThePort( cActivePeer.m_cAddr.GetPort( ) + 1 );
	WONStatus nStatus = m_pGameSpySupport->Startup( gamespyaddress );
	if( nStatus != WS_Success)
	{
		DestroyGameSpySupport( );
		return false;
	}

	// Have Titan handle the updating.
	m_pGameSpySupport->SetAutoPumpTime( GAMESPYSUPPORT_AUTOPUMPTIME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerDirectory_Titan::DestroyGameSpySupport
//
//	PURPOSE:	Destroys the gamespysupport object
//
// ----------------------------------------------------------------------- //
void CServerDirectory_Titan::DestroyGameSpySupport( )
{
	CWinSync_CSAuto cProtection(m_cGameSpySupportCS);

	if( m_pGameSpySupport != NULL )
	{
		m_pGameSpySupport->Shutdown( );
		m_pGameSpySupport = NULL;
	}
}



// Factory for CServerDirectory_Titan objects

SERVERDIR_API IServerDirectory *Factory_Create_IServerDirectory_Titan( bool bClientSide, ILTCSBase& ltCSBase, HMODULE hResourceModule )
{
	g_pLTCSBase = &ltCSBase;
	if( bClientSide )
	{
		g_pLTClient = ( ILTClient* )g_pLTCSBase;
	}

	g_hResourceModule = hResourceModule;
	return new CServerDirectory_Titan;
}

const char* GetTempString(int messageCode)
{
	static char s_szStringBuffer[256];

	s_szStringBuffer[0] = '\0';

	if( !g_hResourceModule )
		return s_szStringBuffer;


	uint32 nBytes = LoadString(g_hResourceModule, messageCode, s_szStringBuffer, sizeof(s_szStringBuffer));

	return s_szStringBuffer;
}
