// ----------------------------------------------------------------------- //
//
// MODULE  : ServerBrowserCtrl.h
//
// PURPOSE : Control to browse multiplayer servers.
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "serverbrowserctrl.h"
#include <algorithm>
#include "sys/win/mpstrconv.h"
#include "ClientConnectionMgr.h"
#include "GameModeMgr.h"

// the ILTClientContentTransfer interface
#include "iltclientcontenttransfer.h"
static ILTClientContentTransfer* g_pLTClientContentTransfer;
define_holder(ILTClientContentTransfer, g_pLTClientContentTransfer);



// ----------------------------------------------------------------------- //
// Function name   : SetSummaryInfo
// Description     : Sets the summary info on the column control.
// Return type     : static bool - true on success.
// Argument        : CLTGUIColumnCtrlExEx& columnCtrl - column control to setup.
// Argument        : char const* pszAddress - address of new server.
// Argument        : ServerEntry const& serverEntry - serverentry to use.
// ----------------------------------------------------------------------- //
bool ServerBrowserCtrl::SetSummaryInfo( CLTGUIColumnCtrlEx& columnCtrl, ServerEntry const& serverEntry )
{
	// Make sure the columns were made first.
	if( columnCtrl.GetNumColumns( ) == 0 )
		return false;

	wchar_t wszTempBuffer[256];

	// Start the column index on the first column.
	uint8 nColumnIndex = 0;
	CLTGUITextCtrl* pTextColumn = NULL;
	CLTGUITextureButton* pIconColumn = NULL;

	wchar_t wszListTrue[10];
	LTStrCpy( wszListTrue, LoadString( "SCREENMULTI_LIST_TRUE" ), LTARRAYSIZE( wszListTrue ));

	// Setup the password locked column.
	pIconColumn = (CLTGUITextureButton* )columnCtrl.GetColumn( eColumn_Lock );
	pIconColumn->SetTexture( (serverEntry.m_bUsePassword ? TextureReference(g_pLayoutDB->GetServerIcon("Password",1)) : TextureReference(g_pLayoutDB->GetServerIcon("Password",0)) ));

	// Setup the platform column.
	pIconColumn = (CLTGUITextureButton* )columnCtrl.GetColumn( eColumn_Platform );
	if (serverEntry.m_bDedicated)
	{
		pIconColumn->SetTexture( (serverEntry.m_bLinuxServer ? TextureReference(g_pLayoutDB->GetServerIcon("Platform",2)) : TextureReference(g_pLayoutDB->GetServerIcon("Platform",1)) ));
	}
	else
	{
		pIconColumn->SetTexture( TextureReference(g_pLayoutDB->GetServerIcon("Platform",0)) );
	}
	
	// Setup the punkbuster column.
	pIconColumn = (CLTGUITextureButton* )columnCtrl.GetColumn( eColumn_Punkbuster );
	pIconColumn->SetTexture( (serverEntry.m_bUsePunkbuster ? TextureReference(g_pLayoutDB->GetServerIcon("Punkbuster",1)) : TextureReference(g_pLayoutDB->GetServerIcon("Punkbuster",0)) ));


	// Setup the customized column.
	pIconColumn = (CLTGUITextureButton* )columnCtrl.GetColumn( eColumn_Customized );
	pIconColumn->SetTexture( (serverEntry.m_bHasOverrides ? TextureReference(g_pLayoutDB->GetServerIcon("Customized",1)) : TextureReference(g_pLayoutDB->GetServerIcon("Customized",0)) ));

	// Setup the requires download column.
	pIconColumn = (CLTGUITextureButton* )columnCtrl.GetColumn( eColumn_RequiresDownload );
	pIconColumn->SetTexture( (serverEntry.m_nRequiredDownloadSize > 0 ? TextureReference(g_pLayoutDB->GetServerIcon("Download",1)) : TextureReference(g_pLayoutDB->GetServerIcon("Download",0)) ));

	// Do the name
	pTextColumn = (CLTGUITextCtrl* )columnCtrl.GetColumn( eColumn_Name );
	pTextColumn->SetString( MPA2W( serverEntry.m_sName.c_str( )).c_str( ), true);
	
	// Do the game type...
	const char* pszStringId = NULL;
	pszStringId = DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( serverEntry.m_hGameModeRecord, Label );
	if( !pszStringId || !pszStringId[0] )
		pszStringId = "IDS_UNKNOWN";
	pTextColumn = (CLTGUITextCtrl* )columnCtrl.GetColumn( eColumn_Type );
	pTextColumn->SetString( LoadString( pszStringId ), true);

	// Do the ping.  If the ping is unknown, leave it blank.
	if( serverEntry.m_nPing != ( uint16 )IGameSpyBrowser::kPingError )
		LTSNPrintF( wszTempBuffer, LTARRAYSIZE( wszTempBuffer ), L"%d", serverEntry.m_nPing);
	else
		LTStrCpy( wszTempBuffer, L"", LTARRAYSIZE( wszTempBuffer ));
	pTextColumn = (CLTGUITextCtrl* )columnCtrl.GetColumn( eColumn_Ping );
	pTextColumn->SetString( wszTempBuffer, true );

	// Do the number of players
	LTSNPrintF( wszTempBuffer, LTARRAYSIZE( wszTempBuffer ), L"%d/%d", serverEntry.m_nNumPlayers, serverEntry.m_nMaxPlayers);
	pTextColumn = (CLTGUITextCtrl* )columnCtrl.GetColumn( eColumn_Player );
	pTextColumn->SetString( wszTempBuffer, true );

	// Do the mission
	pTextColumn = (CLTGUITextCtrl* )columnCtrl.GetColumn( eColumn_Mission );
	pTextColumn->SetString( serverEntry.m_sMission.c_str( ), true);

	return true;
}



// ----------------------------------------------------------------------- //
// Function name   : ReadServerEntry
// Description     : Reads the server entry from gamespy and fills
//						in the serverentry object.
// Return type     : static bool - true on success.
// Argument        : IGameSpyBrowser& gameSpyBrowser - browser supplying info.
// Argument        : IGameSpyBrowser::HGAMESERVER hGameServer - gameserver handle.
// Argument        : std::string& sPrivateAddress - private address of server to fill in.
// Argument        : ServerEntry& serverEntry - serverentry object to fill in.
// ----------------------------------------------------------------------- //
bool ServerBrowserCtrl::ReadServerEntry( IGameSpyBrowser& gameSpyBrowser, 
							IGameSpyBrowser::HGAMESERVER hGameServer, 
							ServerEntry& serverEntry )
{
	char szString[1024];
	char szPath[256];

	// Get the connection information.
	IGameSpyBrowser::ServerConnectionInfo serverConnectionInfo;
	if( !gameSpyBrowser.GetServerConnectInfo( hGameServer, serverConnectionInfo ))
		return false;

	serverEntry.m_bConnectViaPublic = serverConnectionInfo.bConnectViaPublic;
	serverEntry.m_bDirectConnect = serverConnectionInfo.bDirectConnect;

	// Create a combined address string. 
	LTSNPrintF( szString, LTARRAYSIZE( szString ), "%s:%d", serverConnectionInfo.szPrivateAddress, serverConnectionInfo.nPrivatePort );
	serverEntry.m_sPrivateAddress = szString;

	// Create a combined address string. 
	LTSNPrintF( szString, LTARRAYSIZE( szString ), "%s:%d", serverConnectionInfo.szPublicAddress, serverConnectionInfo.nPublicPort );
	serverEntry.m_sPublicAddress = szString;

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "hostname", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_sName = szString;

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "gamever", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_sVersion = szString;

	// Gamevariant 
	if( !gameSpyBrowser.GetServerProperty( hGameServer, "gamevariant", szString, ARRAY_LEN( szString )))
		return false;
	char szModName[MAX_PATH];
	GameVariantToModName( szString, szModName, LTARRAYSIZE( szModName ));
	serverEntry.m_sModName = szModName;

	serverEntry.m_nNumPlayers = gameSpyBrowser.GetNumPlayers( hGameServer );

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "maxplayers", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_nMaxPlayers = atoi( szString );

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "password", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_bUsePassword = !!( atoi( szString ));

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "dedicated", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_bDedicated = !!( atoi( szString ));

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "linux", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_bLinuxServer = !!( atoi( szString ));

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "punkbuster", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_bUsePunkbuster = !!( atoi( szString ));

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "gametype", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_hGameModeRecord = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( GameModes ).GetCategory( ), szString );
	if( !serverEntry.m_hGameModeRecord )
		return false;
		
	if( !gameSpyBrowser.GetServerProperty( hGameServer, "hasoverrides", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_bHasOverrides = !!( atoi( szString ));

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "downloadablefiles", szString, ARRAY_LEN( szString )))
		return false;
	serverEntry.m_sDownloadableFiles = szString;

	if (!serverEntry.m_sDownloadableFiles.empty())
	{
		// determine the required download size from the non-retail files string
		serverEntry.m_nRequiredDownloadSize = DetermineRequiredDownloadSize(serverEntry.m_sDownloadableFiles);
	}

	if( !gameSpyBrowser.GetServerProperty( hGameServer, "mappath", szPath, ARRAY_LEN( szPath	)))
	{
		szPath[0] = '\0';
	}
	if( gameSpyBrowser.GetServerProperty( hGameServer, "mapname", szString, ARRAY_LEN( szString )))
	{
		wchar_t wszTmp[128];
		if (LTStrLen(szPath))
		{
			LTStrCat(szPath,szString,LTARRAYSIZE(szPath));
		}
		else
		{
			LTStrCpy(szPath,szString,LTARRAYSIZE(szPath));
		}
		g_pMissionDB->GetMissionDisplayName(szPath,wszTmp,LTARRAYSIZE(wszTmp));
		serverEntry.m_sMission = wszTmp;
	}

	// Read details if available.
	if( gameSpyBrowser.HasServerDetails( hGameServer ))
	{
		serverEntry.m_bHasDetails = true;

		if( gameSpyBrowser.GetServerProperty( hGameServer, "options", szString, ARRAY_LEN( szString )))
			serverEntry.m_sOptions = szString;

		//read player list
		uint32 nNumPlayers = gameSpyBrowser.GetNumPlayers( hGameServer );
		serverEntry.m_lstPlayerEntry.resize( 0 );
		serverEntry.m_lstPlayerEntry.reserve( nNumPlayers );
		char szKey[256];
		PlayerEntry playerEntry;
		for( uint32 i = 0; i < nNumPlayers; i++ )
		{
			LTSNPrintF( szKey, LTARRAYSIZE( szKey ), "player_%d", i );
			if( !gameSpyBrowser.GetServerProperty( hGameServer, szKey, szString, ARRAY_LEN( szString )))
				break;
			playerEntry.m_sName = szString;

			LTSNPrintF( szKey, LTARRAYSIZE( szKey ), "ping_%d", i );
			if( gameSpyBrowser.GetServerProperty( hGameServer, szKey, szString, ARRAY_LEN( szString )))
				playerEntry.m_nPing = min( atoi( szString ), 65535 );
			else
				playerEntry.m_nPing = 65535;

			LTSNPrintF( szKey, LTARRAYSIZE( szKey ), "score_%d", i );
			if( gameSpyBrowser.GetServerProperty( hGameServer, szKey, szString, ARRAY_LEN( szString )))
				playerEntry.m_nScore = atoi( szString );

			serverEntry.m_lstPlayerEntry.push_back( playerEntry );
		}

		serverEntry.m_nNumPlayers = serverEntry.m_lstPlayerEntry.size( );

		if( gameSpyBrowser.GetServerProperty( hGameServer, "overridesdata", szString, ARRAY_LEN( szString )))
			serverEntry.m_sOverridesData = szString;
	}

	return true;
}

uint32 ServerBrowserCtrl::DetermineRequiredDownloadSize(const std::string& strDownloadableFilesString)
{
	// parse the string into a list of archive file information
	size_t nNumberOfArchiveFilesSeparator = strDownloadableFilesString.find(g_pszDownloadableFileCountSeparator);
	long nNumberOfArchiveFiles = LTStrToLong(strDownloadableFilesString.substr(0, nNumberOfArchiveFilesSeparator).c_str());

	SClientContentTransferArchiveFileInfo* pArchiveFileInfoList = NULL;
	LT_MEM_TRACK_ALLOC(pArchiveFileInfoList = new SClientContentTransferArchiveFileInfo[nNumberOfArchiveFiles], LT_MEM_TYPE_GAMECODE);

	size_t nOffset = 0;
	size_t nLastEntryPos = 0;
	uint32 nIndex = 0;

	std::string strEntryList = strDownloadableFilesString.substr(nNumberOfArchiveFilesSeparator + 1);
	while ((nOffset = strEntryList.find(g_pszDownloadableFileEntrySeparator, nLastEntryPos)) != strDownloadableFilesString.npos)
	{
		std::string strEntry = strEntryList.substr(nLastEntryPos, nOffset - nLastEntryPos);

		// convert the string to a GUID
		size_t nGUIDSeparatorPos = strEntry.find(g_pszDownloadableFileSizeSeparator);
		StringToLTGUID(strEntry.substr(0, nGUIDSeparatorPos).c_str(), pArchiveFileInfoList[nIndex].m_cGUID);
	
		// parse the file size
		pArchiveFileInfoList[nIndex].m_nSize = LTStrToLong(strEntry.substr(nGUIDSeparatorPos + 1).c_str());

		// move to the next entry
		nIndex++;
		nLastEntryPos = nOffset + 1;
	}

	// call ILTClientContentTransfer to determine the required download size
	bool bIsMissingFiles = false;
	uint32 nRequiredDownloadSize = 0;
	g_pLTClientContentTransfer->CheckForMissingArchiveFiles(pArchiveFileInfoList,
				 										    nNumberOfArchiveFiles,
															bIsMissingFiles,
															nRequiredDownloadSize);

	// clean up the list
	delete [] pArchiveFileInfoList;

	// success
	return nRequiredDownloadSize;
}

