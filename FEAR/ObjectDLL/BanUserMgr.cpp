// ----------------------------------------------------------------------- //
//
// MODULE  : BanUserMgr.h
//
// PURPOSE : Handles banned users.
//
// CREATED : 08/04/2005
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "BanUserMgr.h"
#include "ltprofileutils.h"
#include "ltfileoperations.h"
#include "igamespy.h"
#include "ServerConnectionMgr.h"

// the name of the user ban file
const char* const g_szUserBanFilename = "BanUserList.txt";

// section name
const char* const g_szUserBansSectionName = "UserBans";

// character used to separate the ban key and player name in each entry
const char g_cEntrySeparator = ';';

BanUserMgr::BanUserMgr()
{
}

BanUserMgr::~BanUserMgr()
{
}

BanUserMgr& BanUserMgr::Instance()
{
	static BanUserMgr sSingleton;
	return sSingleton;
}

void BanUserMgr::Init()
{
	LoadBanFile();
}

bool BanUserMgr::IsClientBanned(const char* pszUserKey)
{
	// Check the permanent bans.
	for (TBanList::iterator itBan = m_BanList.begin(); itBan != m_BanList.end(); ++itBan)
	{
		SUserBanEntry& sUserBanEntry = (SUserBanEntry&)*itBan;

		// Check if this isn't the right user.
		if (sUserBanEntry.strUserKey != pszUserKey)
			continue;

		// this user is in the ban list
		return true;
	}

	// Check the temporary bans.
	double fCurTime = RealTimeTimer::Instance().GetTimerAccumulatedS();

	TTempBanList::iterator itTempBan = m_TempBanList.begin();
	while (itTempBan != m_TempBanList.end())
	{
		SUserTempBanEntry& sUserTempBanEntry = (SUserTempBanEntry&)*itTempBan;	

		// Check if this user's ban time limit has expired and clean it out.
		if( sUserTempBanEntry.m_fBanTimeLimit <= fCurTime )
		{
			// Remove it from the list.
			m_TempBanList.erase( itTempBan++ );
			continue;
		}
		else
		{
			++itTempBan;
		}

		// Check if this isn't the right user.
		if (sUserTempBanEntry.strUserKey != pszUserKey)
		{
			continue;
		}

		// This user is on the temp ban list.
		return true;
	}

	// not banned
	return false;
}

// Kicks a client based on a userkey.
static void KickClientByUserKey( char const* pszUserKey )
{
	// find the client and kick them from the server
	HCLIENT hCurrentClient = g_pLTServer->GetNextClient(NULL);
	while (hCurrentClient)
	{
		HCLIENT hNextClient = g_pLTServer->GetNextClient(hCurrentClient);

		// get the user's key hash
		uint16 nClientID = (uint16)g_pLTServer->GetClientID(hCurrentClient);
		const char* pszUserCDKeyHash = NULL;
		g_pGameServerShell->GetGameSpyServer()->GetUserCDKeyHash(nClientID, pszUserCDKeyHash);

		// check to see if the user is banned
		if( LTStrEquals( pszUserKey, pszUserCDKeyHash ))
		{
			GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( hCurrentClient );
			if( pGameClientData )
			{
				ServerConnectionMgr::Instance( ).BootWithReason( *pGameClientData, eClientConnectionError_Banned, NULL );
			}
			else
			{
				g_pLTServer->KickClient(hCurrentClient);
			}
			break;
		}

		hCurrentClient = hNextClient;
	}
}

void BanUserMgr::AddBan(char const* pszUserKey, const char* pszPlayerName)
{
	// build the ban entry
	SUserBanEntry sUserBanEntry;
	sUserBanEntry.strPlayerName = pszPlayerName;
	sUserBanEntry.strUserKey    = pszUserKey;

	// add the ban to the list
	m_BanList.insert(sUserBanEntry);

	// save the bans
	SaveBanFile();

	// Kick the client.
	KickClientByUserKey( pszUserKey );
}

void BanUserMgr::RemoveBan(const SUserBanEntry& SUserBanEntry)
{
	// remove the ban from the list
	m_BanList.erase(SUserBanEntry);

	// save the bans
	SaveBanFile();
}

void BanUserMgr::AddTempBan(char const* pszUserKey, const char* pszPlayerName, float fDuration)
{
	// build the ban entry
	SUserTempBanEntry sUserBanEntry;
	sUserBanEntry.strPlayerName = pszPlayerName;
	sUserBanEntry.strUserKey    = pszUserKey;
	sUserBanEntry.m_fBanTimeLimit = RealTimeTimer::Instance().GetTimerAccumulatedS() + fDuration;

	// Check if user already banned.  If so, update the timeout.
	TTempBanList::iterator iter = m_TempBanList.find( sUserBanEntry );
	if( iter != m_TempBanList.end( ))
	{
		SUserTempBanEntry& sExistingBanEntry = (SUserTempBanEntry&)*iter;
		sExistingBanEntry.m_fBanTimeLimit = sUserBanEntry.m_fBanTimeLimit;
	}
	// add the ban to the list
	else
	{
		m_TempBanList.insert(sUserBanEntry);
	}

	// Kick the client.
	KickClientByUserKey( pszUserKey );
}

void BanUserMgr::LoadBanFile()
{
	char   szBanKey[128] = "";
	char   szBanEntry[255] = "";
	uint32 nBanIndex = 0;

	// build the file name by prepending the user directory
	char szFilename[MAX_PATH];
	LTFileOperations::GetUserDirectory(szFilename, LTARRAYSIZE(szFilename));
	LTStrCat(szFilename, g_szUserBanFilename, LTARRAYSIZE(szFilename));

	// read the file until we run out of entries
	while (1)
	{
		// format the key name 
		LTSNPrintF(szBanKey, LTARRAYSIZE( szBanKey ), "Ban%d", nBanIndex );

		// read the value
		LTProfileUtils::ReadString(g_szUserBansSectionName, szBanKey, NULL, szBanEntry, LTARRAYSIZE(szBanEntry), szFilename);

		if (LTStrEmpty(szBanEntry))
		{
			// end of file
			break;
		}
	
		// find the separator
		std::string strBanEntry = szBanEntry;
		size_t nSeparatorPos = strBanEntry.find(g_cEntrySeparator);
		if (nSeparatorPos == strBanEntry.npos)
		{
			// improperly formatted entry - skip it
			continue;
		}

		// build the new entry structure
		SUserBanEntry sUserBanEntry;
		sUserBanEntry.strUserKey	= strBanEntry.substr(0, nSeparatorPos);
		sUserBanEntry.strPlayerName = strBanEntry.substr(nSeparatorPos + 1);

		// add the ban to the list
		m_BanList.insert(sUserBanEntry);
		nBanIndex++;
	}	
}

void BanUserMgr::SaveBanFile()
{	
	char   szBanKey[128] = "";
	char   szBanEntry[255] = "";
	uint32 nBanIndex = 0;

	// build the file name by prepending the user directory
	char szFilename[MAX_PATH];
	LTFileOperations::GetUserDirectory(szFilename, LTARRAYSIZE(szFilename));
	LTStrCat(szFilename, g_szUserBanFilename, LTARRAYSIZE(szFilename));

	// remove the existing file
	LTFileOperations::DeleteFile(szFilename);

	// write out all the ban entries
	for (TBanList::iterator itBan = m_BanList.begin(); itBan != m_BanList.end(); ++itBan)
	{
		SUserBanEntry& sUserBanEntry = (SUserBanEntry&)*itBan;

		// build the key and value
		LTSNPrintF(szBanKey, LTARRAYSIZE(szBanKey), "Ban%d", nBanIndex);
		LTSNPrintF(szBanEntry, LTARRAYSIZE(szBanEntry), "%s%c%s", 
				   sUserBanEntry.strUserKey.c_str(), 
				   g_cEntrySeparator, 
				   sUserBanEntry.strPlayerName.c_str());

		// write the entry
		LTProfileUtils::WriteString(g_szUserBansSectionName, szBanKey, szBanEntry, szFilename);
		nBanIndex++;
	}

	// flush the file
	LTProfileUtils::WriteString(NULL, NULL, NULL, szFilename);
}
