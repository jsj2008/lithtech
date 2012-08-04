// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.cpp
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMulti.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "ClientConnectionMgr.h"
#include "msgids.h"
#include "MissionMgr.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "CustomCtrls.h"
#include "ltstrutils.h"
#include "ltfileoperations.h"
#include "ltprofileutils.h"
#include "iltfilemgr.h"


namespace
{
	CScreenMulti* g_pScreenMulti = NULL;
	CWinSync_CS	  g_csMOTD;
	int32 kServerListColWidth[ServerBrowserCtrl::eColumnCount] = {0};
	uint32 kServerListFontSize = 0;
	char const* kpszServerListFont = NULL;

	enum EListIndex
	{
		eListIndex_Browser = 0,
		eListIndex_Player = 1,
		eListIndex_Rules = 2,
		eListIndex_Filters = 3,
		eListIndex_Join = 4,
		eListIndex_Customizers = 5,
		eListIndex_FriendsTab = 6,
		eListIndex_FriendsList = 7,
		eListIndex_FriendsButtons = 8,
		eListIndex_FavoriteServersTab = 9,
		eListIndex_FavoriteServersButtons = 10,
	};

	// Constants for player list.
	int32 kPlayerListColWidth_Name = 0;
	int32 kPlayerListColWidth_Score = 0;
	uint32 kPlayerListFontSize = 0;
	char const* kpszPlayerListFont = NULL;

	uint32 kOptionsTabWidth = 100;

	enum EPlayerListCol
	{
		ePlayerListCol_Name,
		ePlayerListCol_Score,
	};

	// Constants for rule list.
	int32 kRuleListColWidth_Name = 0;
	int32 kRuleListColWidth_Value = 0;
	uint32 kRuleListFontSize = 0;
	char const* kpszRuleListFont = NULL;
	enum ERuleListCol
	{
		eRuleListCol_Name,
		eRuleListCol_Value,
	};

	// Constants for customizers list.
	int32 kCustomizersListColWidth_Name = 0;
	int32 kCustomizersListColWidth_Value = 0;
	uint32 kCustomizersListFontSize = 0;
	char const* kpszCustomizersListFont = NULL;
	enum ECustomizersListCol
	{
		eCustomizersListCol_Name,
		eCustomizersListCol_Value,
	};

	// Constants for filters list.
	uint32 kFiltersListFontSize = 0;
	char const* kpszFiltersListFont = NULL;
	enum EVersionFilterTypes
	{
		eVersionFilter_All,
		eVersionFilter_Version,
		eVersionFilter_Mod,
		eVersionFilter_VersionAndMod,
	};



	enum eLocalCommands 
	{
		CMD_SORT_NAME = CMD_CUSTOM+1,
		CMD_SORT_PING,
		CMD_SORT_PLAYER,
		CMD_SORT_LOCK,
		CMD_SORT_MISSION,
		CMD_SORT_TYPE,
		CMD_SORT_PLATFORM,
		CMD_SORT_PUNKBUSTER,
		CMD_SORT_CUSTOMIZED,
		CMD_SORT_REQUIRESDOWNLOAD,
		CMD_FILTER_VERSION,
		CMD_FILTER_PLAYERS,
		CMD_FILTER_PING,
		CMD_FILTER_MOD,
		CMD_FILTER_TYPE,
		CMD_FILTER_CUSTOMIZED,
		CMD_FILTER_REQUIRESSDOWNLOAD,
		CMD_FILTER_PUNKBUSTER,
		CMD_DETAILS,
		CMD_FILTER_FRIEND,
		CMD_CLIENTSETTINGS,
		CMD_MOTD_SHOW,
		CMD_MOTD_LINK,
		CMD_MOTD_CLOSE,
		CMD_HOST,
		CMD_JOIN,
		CMD_SOURCE,
		CMD_SEARCH,
		CMD_SORT_PLAYERNAME,
		CMD_SORT_PLAYERSCORE,
		CMD_SORT_RULENAME,
		CMD_SORT_RULEVALUE,
		CMD_SORT_CUSTOMIZERSNAME,
		CMD_SORT_CUSTOMIZERSVALUE,
		CMD_PLAYERS,
		CMD_RULES,
		CMD_CUSTOMIZERS,
		CMD_FILTERS,
		CMD_FRIENDS,
		CMD_FRIENDS_ADD,
		CMD_FRIENDS_REMOVE,
		CMD_FRIENDS_DESELECT,
		CMD_FRIENDS_REFRESH,
		CMD_FAVORITESERVERS,
		CMD_FAVORITESERVERS_ADD,
		CMD_FAVORITESERVERS_ADDIP,
		CMD_FAVORITESERVERS_REMOVE,
		CMD_FAVORITESERVERS_REMOVEALL,
		CMD_EDIT_FRIENDS_ADD,
		CMD_EDIT_FAVORITESERVERS_ADDIP,
		CMD_EDIT_PASS,
	};

	// Adds all the columns for a row.
	void AddAllColumns( CLTGUIColumnCtrlEx& columnCtrl, CLTGUIHeaderCtrl& headerCtrl )
	{
		columnCtrl.AddIconColumn( "Interface\\menu\\ServerIcon\\blank.dds", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Lock), LTVector2n(kServerListFontSize,kServerListFontSize) );
		columnCtrl.AddIconColumn( "Interface\\menu\\ServerIcon\\blank.dds", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Platform), LTVector2n(kServerListFontSize,kServerListFontSize) );
		columnCtrl.AddIconColumn( "Interface\\menu\\ServerIcon\\blank.dds", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Punkbuster), LTVector2n(kServerListFontSize,kServerListFontSize) );
		columnCtrl.AddIconColumn( "Interface\\menu\\ServerIcon\\blank.dds", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Customized), LTVector2n(kServerListFontSize,kServerListFontSize) );
		columnCtrl.AddIconColumn( "Interface\\menu\\ServerIcon\\blank.dds", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_RequiresDownload), LTVector2n(kServerListFontSize,kServerListFontSize) );

		columnCtrl.AddTextColumn( L"", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Name), true );
		columnCtrl.AddTextColumn( L"", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Ping), true, kLeft );
		columnCtrl.AddTextColumn( L"", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Player), true );
		columnCtrl.AddTextColumn( L"", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Type), true );

		columnCtrl.AddTextColumn( L"", headerCtrl.GetItemBaseWidth(ServerBrowserCtrl::eColumn_Mission), true );
	}


	// Functor to do sorting by name.
	class ServerSortName
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			int nRes = LTStrICmp( seX->m_sName.c_str(), seY->m_sName.c_str());
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortName( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by version.
	class ServerSortVersion
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );

			// If the versions are different, then just compare those.
			int nVersionCmp = LTStrICmp(seX->m_sVersion.c_str(), seY->m_sVersion.c_str());
			if( nVersionCmp != 0 )
				return (( nVersionCmp < 0 ) == m_bAscending );

			// Versions are different, so it's up to the mod name to differentiate them.
			int nRes = LTStrICmp( seX->m_sModName.c_str(), seY->m_sModName.c_str() );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortVersion( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by mapname.
	class ServerSortMapName
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );

			int nRes = LTStrICmp( seX->m_sMission.c_str(), seY->m_sMission.c_str());
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortMapName( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by ping.
	class ServerSortPing
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortPing( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by number of players.
	class ServerSortPlayers
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			int nRes = ( seX->m_nNumPlayers - seY->m_nNumPlayers );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortPlayers( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by password locked servers.
	class ServerSortLock
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			int nRes = (seX->m_bUsePassword ? 1 : 0 ) - ( seY->m_bUsePassword ? 1 : 0 );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortLock( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by server platform.
	class ServerSortPlatform
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			// Sort by dedicated as a primary category
			int nRes = (seX->m_bDedicated ? 1 : 0 ) - ( seY->m_bDedicated ? 1 : 0 );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			else
			{
				// Sort by platform as a secondary category.
				int nRes = (seX->m_bLinuxServer ? 1 : 0 ) - ( seY->m_bLinuxServer ? 1 : 0 );
				if( nRes != 0 )
					return (( nRes < 0 ) == m_bAscending );
				else
					// Sort by ping as a tertiary category.
					return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );

			}
		}

		ServerSortPlatform( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by punkbuster.
	class ServerSortPunkbuster
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			// Sort by punkbuster as a primary category
			int nRes = (seX->m_bUsePunkbuster ? 1 : 0 ) - ( seY->m_bUsePunkbuster ? 1 : 0 );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			else
			{
				// Sort by ping as a secondary category.
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
			}
		}

		ServerSortPunkbuster( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by customized.
	class ServerSortCustomized
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			int nRes = (seX->m_bHasOverrides ? 1 : 0 ) - ( seY->m_bHasOverrides ? 1 : 0 );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortCustomized( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by requires download
	class ServerSortRequiresDownload
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			int nRes = (seX->m_nRequiredDownloadSize > 0 ? 1 : 0 ) - ( seY->m_nRequiredDownloadSize > 0 ? 1 : 0 );
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortRequiresDownload( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	// Functor to do sorting by game type.
	class ServerSortType
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			ServerEntry* seX = ( ServerEntry* )pX->GetParam1( );
			ServerEntry* seY = ( ServerEntry* )pY->GetParam1( );
			int nRes = LTStrICmp( g_pLTDatabase->GetRecordName( seX->m_hGameModeRecord ), g_pLTDatabase->GetRecordName( seY->m_hGameModeRecord ));
			if( nRes != 0 )
				return (( nRes < 0 ) == m_bAscending );
			// Sort by ping as a secondary category.
			else
				return ((seX->m_nPing < seY->m_nPing ) == m_bAscending );
		}

		ServerSortType( bool bAscending )
		{
			m_bAscending = bAscending;
		}

	private:

		bool m_bAscending;
	};

	void ControlListSort( ControlArray::iterator begin, ControlArray::iterator end, ServerBrowserCtrl::EColumn eColumn, bool bAscending )
	{
		switch(eColumn)
		{
		case ServerBrowserCtrl::eColumn_Name:
			return std::sort( begin, end, ServerSortName( bAscending ));
			break;
		default:
		case ServerBrowserCtrl::eColumn_Ping:
			return std::sort( begin, end, ServerSortPing( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Player:
			return std::sort( begin, end, ServerSortPlayers( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Lock:
			return std::sort( begin, end, ServerSortLock( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Mission:
			return std::sort( begin, end, ServerSortMapName( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Type:
			return std::sort( begin, end, ServerSortType( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Platform:
			return std::sort( begin, end, ServerSortPlatform( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Customized:
			return std::sort( begin, end, ServerSortCustomized( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_RequiresDownload:
			return std::sort( begin, end, ServerSortRequiresDownload( bAscending ));
			break;
		case ServerBrowserCtrl::eColumn_Punkbuster:
			return std::sort( begin, end, ServerSortPunkbuster( bAscending ));
			break;
		};
	}

	// Functor to do playerlist sorting by name.
	class PlayerListSortName
	{
	public:

		bool operator()( CLTGUICtrl const* pX, CLTGUICtrl const* pY ) const
		{
			PlayerEntry* pPlayerEntryX = ( PlayerEntry* )pX->GetParam1();
			PlayerEntry* pPlayerEntryY = ( PlayerEntry* )pY->GetParam1();
			return (LTStrICmp( pPlayerEntryX->m_sName.c_str(), pPlayerEntryY->m_sName.c_str()) < 0 );
		}
	};

	enum EFilterPlayers
	{
		eFilterPlayers_All,
		eFilterPlayers_NotEmpty,
		eFilterPlayers_NotFull,
		eFilterPlayers_NotBoth,
	};

	enum EFilterTristate
	{
		eFilterTristate_All,
		eFilterTristate_Yes,
		eFilterTristate_No,
	};

	void FilterServer( ServerEntry& serverEntry,
		uint8 nVersionFilter, 
		HRECORD hGameTypeFilter, EFilterPlayers ePlayersFilter, 
		uint8 nPingFilter, EFilterTristate eCustimizedFilter, EFilterTristate eRequiredDownloadFilter,
		EFilterTristate ePunkbusterFilter,
		char const* pszFriendNickName )
	{

		bool bShow = true;

		// Filter nickname.  When filtering by nickname, we always show
		// a server that has a player matching the nickname, even if
		// other filters say don't show.  This is because it is assumed
		// you always want to see servers with friends.
		if( !LTStrEmpty( pszFriendNickName ))
		{
			// Assume the player isn't on the server.
			bShow = false;

			// Iterate through all the known players on this server and see if they match our friend.
			for( PlayerEntryList::iterator iter = serverEntry.m_lstPlayerEntry.begin( ); iter != serverEntry.m_lstPlayerEntry.end( ); iter++ )
			{
				PlayerEntry& playerEntry = *iter;
				if( LTStrIEquals( playerEntry.m_sName.c_str(), pszFriendNickName ))
				{
					// Found him, we can stop looking through this server.
					bShow = true;
					break;
				}
			}
		}
		else
		{

		//Version Filter
		switch(nVersionFilter)
		{
		case eVersionFilter_Version:
			bShow &= LTStrIEquals(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion());
		case eVersionFilter_Mod:
			bShow &= LTStrIEquals( serverEntry.m_sModName.c_str(), g_pClientConnectionMgr->GetModName());
		case eVersionFilter_VersionAndMod:
			bShow &= (LTStrIEquals(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()) &&
				(LTStrIEquals( serverEntry.m_sModName.c_str(), g_pClientConnectionMgr->GetModName())));
		case eVersionFilter_All:
		default:
			break;
		}

		// Customized Filter
		if( bShow && eCustimizedFilter != eFilterTristate_All )
			bShow &= ( eCustimizedFilter == eFilterTristate_Yes ) == serverEntry.m_bHasOverrides;

		// Requires download Filter
		if( bShow && eRequiredDownloadFilter != eFilterTristate_All )
			bShow &= ( eRequiredDownloadFilter == eFilterTristate_Yes ) == ( serverEntry.m_nRequiredDownloadSize > 0 );

		// Punkbuster Filter
		if( bShow && ePunkbusterFilter != eFilterTristate_All )
			bShow &= ( ePunkbusterFilter == eFilterTristate_Yes ) == ( serverEntry.m_bUsePunkbuster );

		//Game type filter.
		if (hGameTypeFilter)
		{
			bShow = bShow && ( hGameTypeFilter == serverEntry.m_hGameModeRecord );
		}

		if( bShow )
		{
			switch (ePlayersFilter)
			{
			case eFilterPlayers_All:
				break;
			case eFilterPlayers_NotEmpty:
				bShow &= (serverEntry.m_nNumPlayers > 0);
				break;
			case eFilterPlayers_NotFull:
				bShow &= (serverEntry.m_nNumPlayers < serverEntry.m_nMaxPlayers);
				break;
			case eFilterPlayers_NotBoth:
				bShow &= (serverEntry.m_nNumPlayers > 0);
				bShow &= (serverEntry.m_nNumPlayers < serverEntry.m_nMaxPlayers);
				break;
			}
		}

		if( bShow )
		{
			switch (nPingFilter)
			{
			case 0:
				break;
			case 1:
				bShow &= (serverEntry.m_nPing < 100);
				break;
			case 2:
				bShow &= (serverEntry.m_nPing < 300);
				break;
			case 3:
				bShow &= (serverEntry.m_nPing < 500);
				break;
			}
		}
		}

		serverEntry.m_pColumnCtrl->Show(bShow);
	}

	void EditPassCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenMulti *pThisScreen = (CScreenMulti *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PASS);
	};

	void EditFriendsAdd(bool bReturn, void *pData, void* pUserData)
	{
		CScreenMulti *pThisScreen = (CScreenMulti *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_FRIENDS_ADD);
	};

	void EditFavoriteServerssAddIP(bool bReturn, void *pData, void* pUserData)
	{
		CScreenMulti *pThisScreen = (CScreenMulti *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_FAVORITESERVERS_ADDIP);
	};


	// Maximum number of characters in an IP:Port
	const uint32 kMaxIPPortLength = 21;

	// Function to filter out invalid characters for IP:Port entry.
	wchar_t IPPortFilter(wchar_t c, uint32 nPos)
	{
		if( isdigit( c ))
			return c;
		else if( c == ':' )
			return c;
		else if( c == '.' )
			return c;
		return '\0';
	}

	// Splits a string with IP and Port into IP string and Port value.
	// If no port specified in string, DEFAULT_PORT will be used.
	void SplitIPandPort( char const* pszIPandPort, char* pszIP, uint32 nIPSize, uint16& nPort )
	{
		// Convert the address string into ip and port.
		char szAddress[kMaxIPPortLength+1];
		LTStrCpy( szAddress, pszIPandPort, LTARRAYSIZE( szAddress ));
		char* pszSourceIp = strtok( szAddress, ":" );
		char* pszSourcePort = strtok( NULL, ":" );
		LTStrCpy( pszIP, pszSourceIp, nIPSize );
		if( pszSourcePort )
			nPort = ( uint16 )LTMIN( atoi( pszSourcePort ), 65535 );
		else
			nPort = DEFAULT_PORT;
	}

	// Join IP string and port value into one IPandPort string.
	void JoinIPandPort( char const* pszIP, uint16 nPort, char* pszIPandPort, uint32 nIPandPortSize )
	{
		LTSNPrintF( pszIPandPort, nIPandPortSize, "%s:%d", pszIP, nPort );
	}

	void IniCallback(const char*	 pBuffer,const uint32 nBufferLen,void* pCallbackParam)
	{
		CWinSync_CSAuto cs( g_csMOTD );
		if (g_pScreenMulti)
		{
			DebugCPrint(0,"MOTD Callback: %s",__FUNCTION__);
			g_pScreenMulti->MOTDIniCallback(pBuffer,nBufferLen);
		}		
	}
	void ImageCallback(const char*	 pBuffer,const uint32 nBufferLen,void* pCallbackParam)
	{
		CWinSync_CSAuto cs( g_csMOTD );
		if (g_pScreenMulti)
		{
			DebugCPrint(0,"MOTD Callback: %s",__FUNCTION__);
			g_pScreenMulti->MOTDImageCallback(pBuffer,nBufferLen);
		}		
	}

	const char* s_pszIniName = "MOTD.ini";
	const char* s_pszTempName = "MOTD.tmp";
	const char* s_pszImageName = "MOTD.dds";
	MOTDData tmpMOTD;

}




// number of times to query the LAN to account for UDP packet loss
const uint32 knNumberOfLANRefreshes = 8;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMulti::CScreenMulti()
{
	m_pFindCtrl			= NULL;
	m_pStatus			= NULL;
	m_pMOTD				= NULL;
	m_pMOTDDownload		= NULL;
	m_pMOTDDlg			= NULL;
	m_pMOTDImage		= NULL;
	m_hMOTDImage		= NULL;
	m_pMOTDLink			= NULL;

	m_pInternetServerListCtrl	= NULL;
	m_pFavoriteServerListCtrl	= NULL;
	m_pLANServerListCtrl= NULL;
	m_pServerListCtrl	= NULL;
	m_pLANScrollBar		= NULL;
	m_pInternetScrollBar= NULL;
	m_pFavoriteServerScrollBar= NULL;
	m_pLANHeaderCtrl	= NULL;
	m_pInternetHeaderCtrl= NULL;
	m_pFavoriteServerHeaderCtrl= NULL;
	m_pPlayerListCtrl	= NULL;
	m_pOptions			= NULL;
	m_pMission			= NULL;
	m_pPort				= NULL;
	m_pTabCtrl			= NULL;
	m_pPlayerHeaderCtrl	= NULL;
	m_pPlayerScrollBar	= NULL;
	m_pRulesHeaderCtrl	= NULL;
	m_pRulesScrollBar	= NULL;
	m_pRulesListCtrl	= NULL;
	m_pCustomizersHeaderCtrl	= NULL;
	m_pCustomizersScrollBar	= NULL;
	m_pCustomizersListCtrl	= NULL;
	m_pFiltersScrollBar	= NULL;
	m_pFiltersListCtrl	= NULL;
	m_pFriendsTab		= NULL;
	m_pFriendsAddButton = NULL;
	m_pFriendsRemoveButton = NULL;
	m_pFriendsDeselectButton = NULL;
	m_pFriendsRefreshButton = NULL;
	m_pFriendsListCtrl	= NULL;
	m_pFavoriteServersTab = NULL;
	m_pFavoriteServersAddButton = NULL;
	m_pFavoriteServersAddIPButton = NULL;
	m_pFavoriteServersRemoveButton = NULL;
	m_pFavoriteServersRemoveAllButton = NULL;
	m_pFrameLowerLeft	= NULL;
	m_pFrameLowerRight	= NULL;
	m_pFrameLowerLeft	= NULL;
	m_eLastSort			= ServerBrowserCtrl::eColumn_Ping;
	m_nVersionFilter	= 0;
	m_nCustomizedFilter = 0;
	m_nRequiresDownloadFilter = 0;
	m_nPunkbusterFilter = 0;
	m_nPlayersFilter	= 0;
	m_nPingFilter		= 0;
	m_nGameTypeFilter	= 0;
	m_pszFriendFilter	= NULL;
	m_eCurState			= eState_Inactive;
	m_eUpdateDirState	= eUpdateDirState_Idle;
	m_nSourceIndex = eServerSearchSource_Internet;
	m_eServerSearchSource = eServerSearchSource_Internet;
	
	m_nLANRefreshCount  = 0;
	m_pSourceCtrl = NULL;
	m_eLastBrowserStatus = IGameSpyBrowser::eBrowserStatus_Idle;
	m_nTotalServerCount = 0;
	m_bAscending = false;
}

CScreenMulti::~CScreenMulti()
{
	g_pScreenMulti = NULL;
	if( m_bInit )
		Term( );
}

// Build the screen
bool CScreenMulti::Build()
{
	g_pScreenMulti = this;

	CreateTitle("IDS_TITLE_MULTI");

	m_MOTD.Read(s_pszIniName);
	m_bCheckForMOTD = true;
	m_bDisplayMOTD = false;
	m_bMOTDIniBuffer = false;
	m_bMOTDImageBuffer = false;
	m_bMOTDDownloadFailed = false;
	m_DownloadTimer.SetEngineTimer(RealTimeTimer::Instance());


	int32 kTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	int32 kScreenFontSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	LTVector2n v2DefaultPos = GetDefaultPos( );

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init( );
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n(kTabWidth*2,kScreenFontSize);
	cs.nCommandID = CMD_SEARCH;
	cs.szHelpID = "IDS_HELP_FIND_SERVERS";
	m_pFindCtrl = AddTextItem("IDS_FIND_SERVERS", cs );

	// Create the search source control.
	{
		CLTGUICycleCtrl_create ccs;
		ccs.rnBaseRect = cs.rnBaseRect;
		ccs.nCommandID = CMD_SOURCE;
		ccs.szHelpID = "SCREENMULTI_SEARCH_SOURCE_HELP";
		ccs.pnValue = &m_nSourceIndex;
		ccs.nHeaderWidth = kTabWidth;
		ccs.pCommandHandler = this;
		m_pSourceCtrl = CreateCycle( LoadString( "SCREENMULTI_SEARCH_SOURCE" ), ccs );
		m_pSourceCtrl->AddString( LoadString( "SCREENMULTI_SEARCH_SOURCE_INTERNET" ));
		m_pSourceCtrl->AddString( LoadString( "SCREENMULTI_SEARCH_SOURCE_LAN" ));
		m_pSourceCtrl->AddString( LoadString( "SCREENMULTI_SEARCH_SOURCE_FAVORITES" ));
		AddControl( m_pSourceCtrl );
	}

	LabeledEditCtrl::CreateStruct labeledEditCtrlCS;
	labeledEditCtrlCS.m_cs.rnBaseRect.m_vMin.Init();
	labeledEditCtrlCS.m_cs.rnBaseRect.m_vMax = labeledEditCtrlCS.m_cs.rnBaseRect.m_vMin + LTVector2n(kTabWidth*2,kScreenFontSize);
	labeledEditCtrlCS.m_cs.szHelpID = "SCREENMULTI_PORT_HELP";
	labeledEditCtrlCS.m_nLabelWidth = kTabWidth;
	labeledEditCtrlCS.m_nEditWidth = kTabWidth;
	labeledEditCtrlCS.m_MaxLength = 6;
	labeledEditCtrlCS.m_pValueChangingCB = PortValueChangingCB;
	labeledEditCtrlCS.m_pUserData = this;
	labeledEditCtrlCS.m_eInput = kInputNumberOnly;
	labeledEditCtrlCS.m_pwsValue = &m_sPort;
	m_pPort = new LabeledEditCtrl;
	m_pPort->Create( *this, labeledEditCtrlCS, LoadString("SCREENMULTI_PORT"), true );

	m_pPort->Enable( false );
	m_pPort->Show( false );

	cs.rnBaseRect.m_vMin = v2DefaultPos;
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n(kTabWidth*2,kScreenFontSize);
	cs.rnBaseRect.Left( ) += kTabWidth * 3;
	cs.rnBaseRect.Right( ) += kTabWidth * 3;
	cs.nCommandID = CMD_HOST;
	cs.szHelpID = "IDS_HELP_HOST";
	AddTextItem("IDS_HOST", cs );

	cs.rnBaseRect.m_vMin.Init( );
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n(kTabWidth*2,kScreenFontSize);
	cs.nCommandID = CMD_CLIENTSETTINGS;
	cs.szHelpID = "SCREEN_CLIENT_SETTINGS_HELP";
	AddTextItem("SCREEN_CLIENT_SETTINGS", cs );

	cs.rnBaseRect.m_vMin.Init( );
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n(kTabWidth*2,kScreenFontSize);
	cs.nCommandID = CMD_MOTD_SHOW;
	cs.szHelpID = "ScreenMulti_MOTD_HELP";
	m_pMOTD = AddTextItem("ScreenMulti_MOTD", cs );

	cs.rnBaseRect.m_vMin = m_pMOTD->GetBasePos();
	cs.rnBaseRect.m_vMax.x = cs.rnBaseRect.m_vMin.x + m_pMOTD->GetBaseWidth();
	cs.rnBaseRect.m_vMax.y = cs.rnBaseRect.m_vMin.y + m_pMOTD->GetBaseHeight();
	m_pMOTDDownload = AddTextItem("ScreenMulti_MOTD_Download", cs, true );

	SetMOTDDownloadUI(false);

	LTRect2n listRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Lock] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Platform] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,1);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Punkbuster] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,2);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Name] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,3);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Player] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,4);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Type] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,5);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Ping] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,6);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Customized] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,7);
	kServerListColWidth[ServerBrowserCtrl::eColumn_RequiresDownload] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,8);
	kServerListColWidth[ServerBrowserCtrl::eColumn_Mission] = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,9);
	kServerListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	kpszServerListFont = g_pLayoutDB->GetListFont( m_hLayout, 0 );

	cs.rnBaseRect.m_vMin = listRect.m_vMin;
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n(0,kServerListFontSize);

	listRect.Top() += kServerListFontSize;

	// create 3 scrollbar controls: internet, lan and favorites
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = listRect;
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Expand( 0, g_pLayoutDB->GetListFrameExpand(m_hLayout,0) );
		csb.rnBaseRect.Bottom() += g_pLayoutDB->GetHeaderCtrlSize()*2;

		LTVector2n vOffset = g_pLayoutDB->GetScrollBarOffset( m_hLayout, 0 );
		csb.rnBaseRect.Offset( vOffset.x + g_pLayoutDB->GetListFrameExpand(m_hLayout,0) - 1, vOffset.y - g_pLayoutDB->GetHeaderCtrlSize() );

		m_pLANScrollBar = CreateScrollBar( csb );
		m_pInternetScrollBar = CreateScrollBar( csb );
		m_pFavoriteServerScrollBar = CreateScrollBar( csb );

		if( m_pLANScrollBar )
		{
			m_pLANScrollBar->Show( false );
			m_pLANScrollBar->Enable( false );
			m_pLANScrollBar->SetFrameWidth( 1 );
		}

		if( m_pInternetScrollBar )
		{
			m_pInternetScrollBar->Show( false );
			m_pInternetScrollBar->Enable( false );
			m_pInternetScrollBar->SetFrameWidth( 1 );
		}

		if( m_pFavoriteServerScrollBar )
		{
			m_pFavoriteServerScrollBar->Show( false );
			m_pFavoriteServerScrollBar->Enable( false );
			m_pFavoriteServerScrollBar->SetFrameWidth( 1 );
		}
	}

	// create 3 header controls: internet, lan and favorites
	{
		CLTGUIHeaderCtrl_create chc;
		chc.rnBaseRect = listRect;
		chc.rnBaseRect.Bottom() = chc.rnBaseRect.Top();
		chc.rnBaseRect.Top() -= g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Expand( g_pLayoutDB->GetListFrameExpand(m_hLayout,0), 0 );
		
		LTVector2n vOffset = g_pLayoutDB->GetHeaderCtrlOffset( m_hLayout, 0 );
		chc.rnBaseRect.Offset( vOffset.x, vOffset.y - g_pLayoutDB->GetListFrameExpand(m_hLayout,0) );

		chc.nItemCount = ServerBrowserCtrl::eColumnCount;
		chc.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();

		chc.pScrollBar = m_pLANScrollBar;
		m_pLANHeaderCtrl = AddHeaderCtrl( chc );
		chc.pScrollBar = m_pInternetScrollBar;
		m_pInternetHeaderCtrl = AddHeaderCtrl( chc );
		chc.pScrollBar = m_pFavoriteServerScrollBar;
		m_pFavoriteServerHeaderCtrl = AddHeaderCtrl( chc );

		if( m_pLANHeaderCtrl )
		{
			m_pLANHeaderCtrl->SetFont( CFontInfo(kpszServerListFont,kServerListFontSize,CFontInfo::kStyle_Bold) );
			m_pLANHeaderCtrl->SetIconSize( LTVector2n(kServerListFontSize,kServerListFontSize) );
			m_pLANHeaderCtrl->SetFrameWidth( 1 );

			AddServerBrowserColumns( m_pLANHeaderCtrl );

			m_pLANHeaderCtrl->Show( false );
			m_pLANHeaderCtrl->Enable( false );
		}

		if( m_pInternetHeaderCtrl )
		{
			m_pInternetHeaderCtrl->SetFont( CFontInfo(kpszServerListFont,kServerListFontSize,CFontInfo::kStyle_Bold) );
			m_pInternetHeaderCtrl->SetIconSize( LTVector2n(kServerListFontSize,kServerListFontSize) );
			m_pInternetHeaderCtrl->SetFrameWidth( 1 );

			AddServerBrowserColumns( m_pInternetHeaderCtrl );

			m_pInternetHeaderCtrl->Show( false );
			m_pInternetHeaderCtrl->Enable( false );
		}

		if( m_pFavoriteServerHeaderCtrl )
		{
			m_pFavoriteServerHeaderCtrl->SetFont( CFontInfo(kpszServerListFont,kServerListFontSize,CFontInfo::kStyle_Bold) );
			m_pFavoriteServerHeaderCtrl->SetIconSize( LTVector2n(kServerListFontSize,kServerListFontSize) );
			m_pFavoriteServerHeaderCtrl->SetFrameWidth( 1 );

			AddServerBrowserColumns( m_pFavoriteServerHeaderCtrl );

			m_pFavoriteServerHeaderCtrl->Show( false );
			m_pFavoriteServerHeaderCtrl->Enable( false );
		}
	}

	LTRect2n rnServerListCtrl = listRect;

	// Make 3 server lists: internet, lan and favorites
	CLTGUIListCtrlEx_create slcs;
	slcs.rnBaseRect = listRect;
	slcs.bAutoSelect = false;
	slcs.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();
	slcs.pScrollBar = m_pLANScrollBar;
	slcs.pHeaderCtrl = m_pLANHeaderCtrl;
	m_pServerListCtrl = AddListEx(slcs);
	m_pServerListCtrl->SetScrollWrap(false);
	TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,eListIndex_Browser,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,eListIndex_Browser,1));
	m_pServerListCtrl->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Browser));
	m_pServerListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Browser));
	m_pServerListCtrl->SetFrameWidth( 1 );
	m_pLANServerListCtrl = m_pServerListCtrl;
	slcs.pScrollBar = m_pInternetScrollBar;
	slcs.pHeaderCtrl = m_pInternetHeaderCtrl;
	m_pServerListCtrl = AddListEx(slcs);
	m_pServerListCtrl->SetScrollWrap(false);
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,eListIndex_Browser,0));
	hSelFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,eListIndex_Browser,1));
	m_pServerListCtrl->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Browser));
	m_pServerListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Browser));
	m_pServerListCtrl->SetFrameWidth( 1 );
	m_pInternetServerListCtrl = m_pServerListCtrl;
	slcs.pScrollBar = m_pFavoriteServerScrollBar;
	slcs.pHeaderCtrl = m_pFavoriteServerHeaderCtrl;
	m_pServerListCtrl = AddListEx(slcs);
	m_pServerListCtrl->SetScrollWrap(false);
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,eListIndex_Browser,0));
	hSelFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,eListIndex_Browser,1));
	m_pServerListCtrl->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Browser));
	m_pServerListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Browser));
	m_pServerListCtrl->SetFrameWidth( 1 );
	m_pFavoriteServerListCtrl = m_pServerListCtrl;

	m_pLANServerListCtrl->Show( false );
	m_pLANServerListCtrl->Enable( false );
	m_pInternetServerListCtrl->Show( true );
	m_pInternetServerListCtrl->Enable( true );
	m_pFavoriteServerListCtrl->Show( false );
	m_pFavoriteServerListCtrl->Enable( false );

	CLTGUIFillFrame_create cff;
	cff.rnBaseRect = rnServerListCtrl;
	cff.rnBaseRect.Top() = cff.rnBaseRect.Bottom();
	cff.rnBaseRect.Bottom() += g_pLayoutDB->GetHeaderCtrlSize();
	cff.rnBaseRect.Right() = cff.rnBaseRect.Left() + g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,0);
	m_pFrameLowerLeft = AddFillFrame( cff );
	if( m_pFrameLowerLeft )
		m_pFrameLowerLeft->SetRenderBorder( CLTGUIFillFrame::eBorder_Left|CLTGUIFillFrame::eBorder_Right|CLTGUIFillFrame::eBorder_Bottom );

	cs.rnBaseRect = cff.rnBaseRect;
	cs.rnBaseRect.Left() += g_pLayoutDB->GetHeaderCtrlIndent();
	cs.rnBaseRect.Top() += (cff.rnBaseRect.GetHeight() - kServerListFontSize)/2;
	cs.nCommandID = CMD_NONE;
	cs.szHelpID = "";
	m_pStatus = AddTextItem( "SCREENMULTI_LIST_WAITING", cs, true, kpszServerListFont, kServerListFontSize );


	cff.rnBaseRect = rnServerListCtrl;
	cff.rnBaseRect.Top() = cff.rnBaseRect.Bottom();
	cff.rnBaseRect.Bottom() += g_pLayoutDB->GetHeaderCtrlSize();
	cff.rnBaseRect.Left() = cff.rnBaseRect.Right() - g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,0);
	m_pFrameLowerRight = AddFillFrame( cff );
	if( m_pFrameLowerRight )
		m_pFrameLowerRight->SetRenderBorder( CLTGUIFillFrame::eBorder_Left|CLTGUIFillFrame::eBorder_Right|CLTGUIFillFrame::eBorder_Bottom );

	cs.rnBaseRect = cff.rnBaseRect;
	cs.rnBaseRect.Right() -= g_pLayoutDB->GetHeaderCtrlIndent();
	cs.rnBaseRect.Top() += (cff.rnBaseRect.GetHeight() - kServerListFontSize)/2;
	cs.nCommandID = CMD_NONE;
	cs.szHelpID = "";
	m_pServerCount = AddTextItem( L"", cs, true, kpszServerListFont, kServerListFontSize );
	m_pServerCount->SetAlignment( kRight );

	// add the scrollbar now so that they get rendered after the list
	if( m_pInternetScrollBar )
		AddControl( m_pInternetScrollBar );
	if( m_pLANScrollBar )
		AddControl( m_pLANScrollBar );

	
	// Create a header for the tab list.
	{
		uint32 const kPlayersTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,3,kOptionsTabWidth);
		uint32 const kRulesTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,4,kOptionsTabWidth);
		uint32 const kCustomizersTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,5,kOptionsTabWidth);
		uint32 const kFiltersTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,6,kOptionsTabWidth);
		uint32 const kFriendsTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,7,kOptionsTabWidth);
		uint32 const kFavoriteServersTabWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,8,kOptionsTabWidth);
		CLTGUITabCtrl_create ctc;
		listRect = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_Player);
		ctc.rnBaseRect = listRect;
		ctc.rnBaseRect.Left() += g_pLayoutDB->GetHeaderCtrlIndent();
		ctc.rnBaseRect.Bottom() = ctc.rnBaseRect.Top() + g_pLayoutDB->GetHeaderCtrlSize();
		ctc.rnBaseRect.Right() = ctc.rnBaseRect.Left() + kPlayersTabWidth + kRulesTabWidth + kCustomizersTabWidth + 
			kFiltersTabWidth + kFriendsTabWidth + kFavoriteServersTabWidth;
		LTVector2n vOffset = g_pLayoutDB->GetHeaderCtrlOffset( m_hLayout, eListIndex_Player );
		ctc.rnBaseRect.Offset( vOffset.x, vOffset.y - g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Player) );

		ctc.nItemCount = 6;
		ctc.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();

		m_pTabCtrl = AddTabCtrl( ctc );
		if( m_pTabCtrl )
		{
			m_pTabCtrl->SetFont( CFontInfo(kpszServerListFont,kServerListFontSize,CFontInfo::kStyle_Bold) );
			m_pTabCtrl->SetIconSize( LTVector2n(kServerListFontSize,kServerListFontSize) );
			m_pTabCtrl->SetFrameWidth( 1 );

			m_pTabCtrl->InsertItem( 0, CMD_PLAYERS, LoadString("ScreenMulti_PlayerTab"), "ScreenMulti_PlayerTab_Help", kLeft, 
				kPlayersTabWidth,  NULL, NULL );
			m_pTabCtrl->InsertItem( 1, CMD_RULES, LoadString("ScreenMulti_RulesTab"), "ScreenMulti_RulesTab_Help", kLeft, 
				kRulesTabWidth,  NULL, NULL );
			m_pTabCtrl->InsertItem( 2, CMD_CUSTOMIZERS, LoadString("ScreenMulti_CustomizersTab"), "ScreenMulti_CustomizersTab_Help", kLeft, 
				kCustomizersTabWidth,  NULL, NULL );
			m_pTabCtrl->InsertItem( 3, CMD_FILTERS, LoadString("ScreenMulti_FiltersTab"), "ScreenMulti_FiltersTab_Help", kLeft, 
				kFiltersTabWidth,  NULL, NULL );
			m_pTabCtrl->InsertItem( 4, CMD_FRIENDS, LoadString("ScreenMulti_FriendsTab"), "ScreenMulti_FriendsTab_Help", kLeft, 
				kFriendsTabWidth,  NULL, NULL );
			m_pTabCtrl->InsertItem( 5, CMD_FAVORITESERVERS, LoadString("ScreenMulti_FavoriteServersTab"), "ScreenMulti_FavoriteServersTab_Help", kLeft, 
				kFavoriteServersTabWidth,  NULL, NULL );
			m_pTabCtrl->Rescale();
			m_pTabCtrl->SetCommandHandler(this);
			m_pTabCtrl->SelectItem(0);
		}
	}


	// Get the player list constants.
	kPlayerListColWidth_Name = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_Player,0);
	kPlayerListColWidth_Score = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_Player,1);
	kPlayerListFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_Player);
	kpszPlayerListFont = g_pLayoutDB->GetListFont(m_hLayout,eListIndex_Player);

	listRect = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_Player);
	cs.rnBaseRect.m_vMin = listRect.m_vMin;
	cs.rnBaseRect.m_vMax = listRect.m_vMin + LTVector2n(0,kPlayerListFontSize);


	// Create a scroll bar for the player list.
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = listRect;
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Expand( 0, g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Player) );
		csb.rnBaseRect.Top() += 2*g_pLayoutDB->GetHeaderCtrlSize();
		csb.rnBaseRect.Bottom() += g_pLayoutDB->GetHeaderCtrlSize();

		LTVector2n vOffset = g_pLayoutDB->GetScrollBarOffset( m_hLayout, eListIndex_Player );
		csb.rnBaseRect.Offset( vOffset.x + g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Player), vOffset.y - g_pLayoutDB->GetHeaderCtrlSize() );

		m_pPlayerScrollBar = AddScrollBar( csb );

		if( m_pPlayerScrollBar )
		{
			m_pPlayerScrollBar->SetFrameWidth( 1 );
		}
	}

	// Create a header for the player list.
	{
		CLTGUIHeaderCtrl_create chc;
		chc.rnBaseRect = listRect;
		chc.rnBaseRect.Top() += g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Bottom() = chc.rnBaseRect.Top() + g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Expand( g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Player), 0 );

		LTVector2n vOffset = g_pLayoutDB->GetHeaderCtrlOffset( m_hLayout, eListIndex_Player );
		chc.rnBaseRect.Offset( vOffset.x, vOffset.y - g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Player) );

		chc.nItemCount = 2;
		chc.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();

		chc.pScrollBar = m_pPlayerScrollBar;
		m_pPlayerHeaderCtrl = AddHeaderCtrl( chc );

		if( m_pPlayerHeaderCtrl )
		{
			m_pPlayerHeaderCtrl->SetFont( CFontInfo(kpszPlayerListFont,kPlayerListFontSize,CFontInfo::kStyle_Bold) );
			m_pPlayerHeaderCtrl->SetIconSize( LTVector2n(kPlayerListFontSize,kPlayerListFontSize) );
			m_pPlayerHeaderCtrl->SetFrameWidth( 1 );

			m_pPlayerHeaderCtrl->InsertItem( 0, CMD_SORT_PLAYERNAME, LoadString("SCREENMULTI_LIST_PLAYERNAME"), "", kLeft, kPlayerListColWidth_Name, true, NULL, NULL );
			m_pPlayerHeaderCtrl->InsertItem( 1, CMD_SORT_PLAYERSCORE, LoadString("SCREENMULTI_LIST_PLAYERSCORE"), "", kLeft, kPlayerListColWidth_Score, true, NULL, NULL );
			m_pPlayerHeaderCtrl->Rescale();
		}

	}

	// Make the player info list.
	{
		CLTGUIListCtrlEx_create lcs;
		lcs.rnBaseRect = listRect;
		lcs.rnBaseRect.Top() += 2* g_pLayoutDB->GetHeaderCtrlSize();
		lcs.pHeaderCtrl = m_pPlayerHeaderCtrl;
		lcs.pScrollBar = m_pPlayerScrollBar;
		lcs.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();
		m_pPlayerListCtrl = AddListEx(lcs);
		m_pPlayerListCtrl->SetScrollWrap(false);
		m_pPlayerListCtrl->Enable( true );
		m_pPlayerListCtrl->SetFrameWidth( 1 );
		m_pPlayerListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Player));
	}

	listRect = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_Rules);

	// Get the rule list constants.
	kRuleListColWidth_Name = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_Rules,0);
	kRuleListColWidth_Value = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_Rules,1);
	kRuleListFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_Rules);
	kpszRuleListFont = g_pLayoutDB->GetListFont(m_hLayout,eListIndex_Rules);

	// Create a header for the rule list.
	cs.rnBaseRect.m_vMin = listRect.m_vMin;
	cs.rnBaseRect.m_vMax = listRect.m_vMin + LTVector2n(0,kRuleListFontSize);

	// Create a scroll bar for the rule list.
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = listRect;
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Expand( 0, g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Rules) );
		csb.rnBaseRect.Top() += 2*g_pLayoutDB->GetHeaderCtrlSize();
		csb.rnBaseRect.Bottom() += g_pLayoutDB->GetHeaderCtrlSize();

		LTVector2n vOffset = g_pLayoutDB->GetScrollBarOffset( m_hLayout, eListIndex_Rules );
		csb.rnBaseRect.Offset( vOffset.x + g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Rules), vOffset.y - g_pLayoutDB->GetHeaderCtrlSize() );

		m_pRulesScrollBar = AddScrollBar( csb );

		if( m_pRulesScrollBar )
		{
			m_pRulesScrollBar->SetFrameWidth( 1 );
		}
	}

	// Create a header for the rule list.
	{
		CLTGUIHeaderCtrl_create chc;
		chc.rnBaseRect = listRect;
		chc.rnBaseRect.Top() += g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Bottom() = chc.rnBaseRect.Top() + g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Expand( g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Rules), 0 );

		LTVector2n vOffset = g_pLayoutDB->GetHeaderCtrlOffset( m_hLayout, eListIndex_Rules );
		chc.rnBaseRect.Offset( vOffset.x, vOffset.y - g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Rules) );

		chc.nItemCount = 2;
		chc.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();

		chc.pScrollBar = m_pRulesScrollBar;
		m_pRulesHeaderCtrl = AddHeaderCtrl( chc );

		if( m_pRulesHeaderCtrl )
		{
			m_pRulesHeaderCtrl->SetFont( CFontInfo(kpszRuleListFont,kRuleListFontSize,CFontInfo::kStyle_Bold) );
			m_pRulesHeaderCtrl->SetIconSize( LTVector2n(kRuleListFontSize,kRuleListFontSize) );
			m_pRulesHeaderCtrl->SetFrameWidth( 1 );

			m_pRulesHeaderCtrl->InsertItem( 0, CMD_SORT_RULENAME, LoadString("SCREENMULTI_LIST_RULENAME"), "", kLeft, kRuleListColWidth_Name, true, NULL, NULL );
			m_pRulesHeaderCtrl->InsertItem( 1, CMD_SORT_RULEVALUE, LoadString("SCREENMULTI_LIST_RULEVALUE"), "", kLeft, kRuleListColWidth_Value, true, NULL, NULL );
			m_pRulesHeaderCtrl->Rescale();
		}
	}

	// Make the rules list.
	{
		CLTGUIListCtrlEx_create lcs;
		lcs.rnBaseRect = listRect;
		lcs.rnBaseRect.Top() += 2* g_pLayoutDB->GetHeaderCtrlSize();
		lcs.pHeaderCtrl = m_pRulesHeaderCtrl;
		lcs.pScrollBar = m_pRulesScrollBar;
		lcs.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();
		m_pRulesListCtrl = AddListEx(lcs);
		m_pRulesListCtrl->SetScrollWrap(false);
		m_pRulesListCtrl->Enable( true );
		m_pRulesListCtrl->SetFrameWidth( 1 );
		m_pRulesListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Rules));
	}

	listRect = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_Customizers);

	// Get the customizers list constants.
	kCustomizersListColWidth_Name = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_Customizers,0);
	kCustomizersListColWidth_Value = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_Customizers,1);
	kCustomizersListFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_Customizers);
	kpszCustomizersListFont = g_pLayoutDB->GetListFont(m_hLayout,eListIndex_Customizers);

	// Create a header for the customizers list.
	cs.rnBaseRect.m_vMin = listRect.m_vMin;
	cs.rnBaseRect.m_vMax = listRect.m_vMin + LTVector2n(0,kCustomizersListFontSize);

	// Create a scroll bar for the customizers list.
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = listRect;
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Expand( 0, g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Customizers) );
		csb.rnBaseRect.Top() += 2*g_pLayoutDB->GetHeaderCtrlSize();
		csb.rnBaseRect.Bottom() += g_pLayoutDB->GetHeaderCtrlSize();

		LTVector2n vOffset = g_pLayoutDB->GetScrollBarOffset( m_hLayout, eListIndex_Customizers );
		csb.rnBaseRect.Offset( vOffset.x + g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Customizers), vOffset.y - g_pLayoutDB->GetHeaderCtrlSize() );

		m_pCustomizersScrollBar = AddScrollBar( csb );

		if( m_pCustomizersScrollBar )
		{
			m_pCustomizersScrollBar->SetFrameWidth( 1 );
		}
	}

	// Create a header for the customizers list.
	{
		CLTGUIHeaderCtrl_create chc;
		chc.rnBaseRect = listRect;
		chc.rnBaseRect.Top() += g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Bottom() = chc.rnBaseRect.Top() + g_pLayoutDB->GetHeaderCtrlSize();
		chc.rnBaseRect.Expand( g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Customizers), 0 );

		LTVector2n vOffset = g_pLayoutDB->GetHeaderCtrlOffset( m_hLayout, eListIndex_Customizers );
		chc.rnBaseRect.Offset( vOffset.x, vOffset.y - g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Customizers) );

		chc.nItemCount = 2;
		chc.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();

		chc.pScrollBar = m_pCustomizersScrollBar;
		m_pCustomizersHeaderCtrl = AddHeaderCtrl( chc );

		if( m_pCustomizersHeaderCtrl )
		{
			m_pCustomizersHeaderCtrl->SetFont( CFontInfo(kpszCustomizersListFont,kCustomizersListFontSize,CFontInfo::kStyle_Bold) );
			m_pCustomizersHeaderCtrl->SetIconSize( LTVector2n(kCustomizersListFontSize,kCustomizersListFontSize) );
			m_pCustomizersHeaderCtrl->SetFrameWidth( 1 );

			m_pCustomizersHeaderCtrl->InsertItem( 0, CMD_SORT_CUSTOMIZERSNAME, LoadString("SCREENMULTI_LIST_CUSTOMIZERSNAME"), "", kLeft, kCustomizersListColWidth_Name, true, NULL, NULL );
			m_pCustomizersHeaderCtrl->InsertItem( 1, CMD_SORT_CUSTOMIZERSVALUE, LoadString("SCREENMULTI_LIST_CUSTOMIZERSVALUE"), "", kLeft, kCustomizersListColWidth_Value, true, NULL, NULL );
			m_pCustomizersHeaderCtrl->Rescale();
		}
	}

	// Make the customizers list.
	{
		CLTGUIListCtrlEx_create lcs;
		lcs.rnBaseRect = listRect;
		lcs.rnBaseRect.Top() += 2* g_pLayoutDB->GetHeaderCtrlSize();
		lcs.pHeaderCtrl = m_pCustomizersHeaderCtrl;
		lcs.pScrollBar = m_pCustomizersScrollBar;
		lcs.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();
		m_pCustomizersListCtrl = AddListEx(lcs);
		m_pCustomizersListCtrl->SetScrollWrap(false);
		m_pCustomizersListCtrl->Enable( true );
		m_pCustomizersListCtrl->SetFrameWidth( 1 );
		m_pCustomizersListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Customizers));
	}


	listRect = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_Filters);

	// Get the filters list constants.
	kFiltersListFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_Filters);
	kpszFiltersListFont = g_pLayoutDB->GetListFont(m_hLayout,eListIndex_Filters);

	// Create a scroll bar for the filters list.
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = listRect;
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Expand( 0, g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Filters) );

		LTVector2n vOffset = g_pLayoutDB->GetScrollBarOffset( m_hLayout, eListIndex_Filters );
		csb.rnBaseRect.Offset( vOffset.x + g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_Filters), vOffset.y );

		m_pFiltersScrollBar = AddScrollBar( csb );

		if( m_pFiltersScrollBar )
		{
			m_pFiltersScrollBar->SetFrameWidth( 1 );
		}
	}

	// Make the Filters list.
	{
		CLTGUIListCtrlEx_create lcs;
		lcs.rnBaseRect = listRect;
		// Move down to fit under tab control.
		lcs.rnBaseRect.Top() += g_pLayoutDB->GetHeaderCtrlSize();
		lcs.pHeaderCtrl = NULL;
		lcs.pScrollBar = m_pFiltersScrollBar;
		lcs.nTextIdent = g_pLayoutDB->GetHeaderCtrlIndent();
		m_pFiltersListCtrl = AddListEx(lcs);
		m_pFiltersListCtrl->SetScrollWrap(false);
		m_pFiltersListCtrl->Enable( true );
		m_pFiltersListCtrl->SetFrameWidth( 1 );
		m_pFiltersListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_Filters));
	}


	// Change the vertical spacing for the filters.
	m_pFiltersListCtrl->SetItemSpacing( g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,1) );

	// Create player filter.
	CLTGUICycleCtrl_create ccs;
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = ccs.rnBaseRect.m_vMin + LTVector2n(listRect.GetWidth( ),kFiltersListFontSize);
	ccs.nCommandID = CMD_FILTER_PLAYERS;
	ccs.szHelpID = "SCREENMULTI_FILTER_PLAYERS_HELP";
	ccs.pnValue = &m_nPlayersFilter;
	ccs.nHeaderWidth = kTabWidth;
	ccs.pCommandHandler = this;
	CLTGUICycleCtrl* pCycle = CreateCycle( LoadString( "SCREENMULTI_FILTER_PLAYERS" ), ccs, false, kpszFiltersListFont, kFiltersListFontSize );
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_ALL" ));
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_PLAYERS_NOTEMPTY" ));
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_PLAYERS_NOTFULL" ));
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_PLAYERS_NOTBOTH" ));
	m_pFiltersListCtrl->AddControl( pCycle );

	// Create type filter.
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = ccs.rnBaseRect.m_vMin + LTVector2n(listRect.GetWidth( ),kFiltersListFontSize);
	ccs.nCommandID = CMD_FILTER_TYPE;
	ccs.szHelpID = "SCREENMULTI_FILTER_TYPE_HELP";
	ccs.pnValue = &m_nGameTypeFilter;
	ccs.nHeaderWidth = kTabWidth;
	ccs.pCommandHandler = this;
	pCycle = CreateCycle( LoadString( "SCREENMULTI_FILTER_TYPE" ), ccs, false, kpszFiltersListFont, kFiltersListFontSize );
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_ALL" ));
	uint32 nNumGameModes = DATABASE_CATEGORY( GameModes ).GetNumRecords( );
	m_lstGameTypes.clear();
	m_lstGameTypes.reserve( nNumGameModes );
	for( uint32 nGameMode = 0; nGameMode < nNumGameModes; nGameMode++ )
	{
		HRECORD hGameModeRecord = DATABASE_CATEGORY( GameModes ).GetRecordByIndex( nGameMode );

		// Check if this is a multiplayer mode.
		if( !DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Multiplayer ))
			continue;

		// Add the multiplayer mode label to the cycle.
		pCycle->AddString( LoadString( DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Label )));
		m_lstGameTypes.push_back( hGameModeRecord );
	}
	m_pFiltersListCtrl->AddControl( pCycle );

	// Create version filter.
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = ccs.rnBaseRect.m_vMin + LTVector2n(listRect.GetWidth( ),kFiltersListFontSize);
	ccs.nCommandID = CMD_FILTER_VERSION;
	ccs.szHelpID = "SCREENMULTI_FILTER_VERSION_HELP";
	ccs.pnValue = &m_nVersionFilter;
	ccs.nHeaderWidth = kTabWidth;
	ccs.pCommandHandler = this;
	pCycle = CreateCycle( LoadString( "SCREENMULTI_FILTER_VERSION" ), ccs, false, kpszFiltersListFont, kFiltersListFontSize );
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_ALL" ));
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_VERSION_CURRENT" ));
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_VERSION_MOD" ));
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_VERSION_CURRENT_AND_MOD" ));
	m_pFiltersListCtrl->AddControl( pCycle );

	// Create customized filter.
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = ccs.rnBaseRect.m_vMin + LTVector2n(listRect.GetWidth( ),kFiltersListFontSize);
	ccs.nCommandID = CMD_FILTER_CUSTOMIZED;
	ccs.szHelpID = "SCREENMULTI_FILTER_CUSTOMIZED_HELP";
	ccs.pnValue = &m_nCustomizedFilter;
	ccs.nHeaderWidth = kTabWidth;
	ccs.pCommandHandler = this;
	pCycle = CreateCycle( LoadString( "SCREENMULTI_FILTER_CUSTOMIZED" ), ccs, false, kpszFiltersListFont, kFiltersListFontSize );
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_ALL" ));
	pCycle->AddString( LoadString( "IDS_YES" ));
	pCycle->AddString( LoadString( "IDS_NO" ));
	m_pFiltersListCtrl->AddControl( pCycle );

	// Create requires download filter.
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = ccs.rnBaseRect.m_vMin + LTVector2n(listRect.GetWidth( ),kFiltersListFontSize);
	ccs.nCommandID = CMD_FILTER_REQUIRESSDOWNLOAD;
	ccs.szHelpID = "SCREENMULTI_FILTER_REQUIRESDOWNLOAD_HELP";
	ccs.pnValue = &m_nRequiresDownloadFilter;
	ccs.nHeaderWidth = kTabWidth;
	ccs.pCommandHandler = this;
	pCycle = CreateCycle( LoadString( "SCREENMULTI_FILTER_REQUIRESDOWNLOAD" ), ccs, false, kpszFiltersListFont, kFiltersListFontSize );
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_ALL" ));
	pCycle->AddString( LoadString( "IDS_YES" ));
	pCycle->AddString( LoadString( "IDS_NO" ));
	m_pFiltersListCtrl->AddControl( pCycle );

	// Create punkbuster filter.
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = ccs.rnBaseRect.m_vMin + LTVector2n(listRect.GetWidth( ),kFiltersListFontSize);
	ccs.nCommandID = CMD_FILTER_PUNKBUSTER;
	ccs.szHelpID = "SCREENMULTI_FILTER_PUNKBUSTER_HELP";
	ccs.pnValue = &m_nPunkbusterFilter;
	ccs.nHeaderWidth = kTabWidth;
	ccs.pCommandHandler = this;
	pCycle = CreateCycle( LoadString( "SCREENMULTI_FILTER_PUNKBUSTER" ), ccs, false, kpszFiltersListFont, kFiltersListFontSize );
	pCycle->AddString( LoadString( "SCREENMULTI_FILTER_ALL" ));
	pCycle->AddString( LoadString( "IDS_YES" ));
	pCycle->AddString( LoadString( "IDS_NO" ));
	m_pFiltersListCtrl->AddControl( pCycle );


	// Make the friends tab.
	{
		CLTGUICtrl_create dcs;
		LTRect2n rectFriendsTabScreen = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_FriendsTab);
		rectFriendsTabScreen.Top( ) += g_pLayoutDB->GetHeaderCtrlSize();
		dcs.rnBaseRect = rectFriendsTabScreen;
		m_pFriendsTab = debug_new(CLTGUIWindow);
		m_pFriendsTab->Create(TextureReference(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenDialogFrame)),dcs);
		m_pFriendsTab->Enable( false );
		// Add the control to the screen.
		AddControl( m_pFriendsTab );

		// Make the friends list elements
		{
			// Get the friends list constants.
			int32 nFriendsListColWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,eListIndex_FriendsList,0);
			int32 kFriendsListFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_FriendsList);
			char const* pszFriendsListFont = g_pLayoutDB->GetListFont(m_hLayout,eListIndex_FriendsList);

			// Get screen and client coordinates for friends list.  
			LTRect2n rectFriendsListScreen = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_FriendsList);

			// Create a scroll bar for the friends list.
			CLTGUIScrollBar* pFriendsListScrollBar = NULL;
			{
				CLTGUIScrollBar_create csb;
				csb.rnBaseRect.Top( ) = rectFriendsListScreen.Top( );
				csb.rnBaseRect.Bottom( ) = rectFriendsListScreen.Bottom( );
				csb.rnBaseRect.Left( ) = rectFriendsListScreen.Right( );
				csb.rnBaseRect.Right( ) = csb.rnBaseRect.Left( ) + g_pLayoutDB->GetScrollBarSize();
				LTVector2n vOffset = g_pLayoutDB->GetScrollBarOffset( m_hLayout, eListIndex_FriendsList );
				csb.rnBaseRect.Offset( vOffset.x, vOffset.y );
				csb.rnBaseRect.Expand( 0, g_pLayoutDB->GetListFrameExpand(m_hLayout,eListIndex_FriendsList) );

				pFriendsListScrollBar = CreateScrollBar( csb );
				pFriendsListScrollBar->SetFrameWidth( 1 );
				AddControl( pFriendsListScrollBar );
			}

			// Make the friends list.
			{
				CLTGUIListCtrlEx_create lcs;
				lcs.rnBaseRect = rectFriendsListScreen;
				// Shrink a little bit since we have to make room for the header.
				lcs.pHeaderCtrl = NULL;
				lcs.pScrollBar = pFriendsListScrollBar;
				lcs.bAutoSelect = false;
				lcs.szHelpID = "ScreenMulti_Friends_FilterFriend_Help";
				m_pFriendsListCtrl = CreateListEx(lcs);
				m_pFriendsListCtrl->SetScrollWrap(false);
				m_pFriendsListCtrl->Enable( true );
				m_pFriendsListCtrl->SetFrameWidth( 1 );
				m_pFriendsListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,eListIndex_FriendsList));
				
				// Add the list control to the tab.
				AddControl( m_pFriendsListCtrl );
			}

		}

		// Add friends list buttons
		{
			LTRect2n rectFriendsButtonScreen = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_FriendsButtons);
			char const* pszButtonFont = g_pLayoutDB->GetListFont( m_hLayout, eListIndex_FriendsButtons );
            int32 nButtonFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_FriendsButtons);

			CLTGUICtrl_create cs;
			cs.rnBaseRect = rectFriendsButtonScreen;
			cs.rnBaseRect.Bottom() = cs.rnBaseRect.Top() + nButtonFontSize;

			cs.nCommandID = CMD_FRIENDS_ADD;
			cs.szHelpID = "ScreenMulti_Friends_Add_Help";
			m_pFriendsAddButton = CreateTextItem("ScreenMulti_Friends_Add", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFriendsAddButton );
			
			cs.rnBaseRect.Offset( 0, nButtonFontSize );
			cs.nCommandID = CMD_FRIENDS_REMOVE;
			cs.szHelpID = "ScreenMulti_Friends_Remove_Help";
			m_pFriendsRemoveButton = CreateTextItem("ScreenMulti_Friends_Remove", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFriendsRemoveButton );

			cs.rnBaseRect.Offset( 0, nButtonFontSize );
			cs.nCommandID = CMD_FRIENDS_DESELECT;
			cs.szHelpID = "ScreenMulti_Friends_Deselect_Help";
			m_pFriendsDeselectButton = CreateTextItem("ScreenMulti_Friends_Deselect", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFriendsDeselectButton );

			cs.rnBaseRect.Offset( 0, nButtonFontSize );
			cs.nCommandID = CMD_FRIENDS_REFRESH;
			cs.szHelpID = "ScreenMulti_Friends_Refresh_Help";
			m_pFriendsRefreshButton = CreateTextItem("ScreenMulti_Friends_Refresh", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFriendsRefreshButton );
		}
	}

	// Make the favorite servers tab.
	{
		CLTGUICtrl_create dcs;
		LTRect2n rectFavoriteServersTabScreen = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_FavoriteServersTab);
		rectFavoriteServersTabScreen.Top( ) += g_pLayoutDB->GetHeaderCtrlSize();
		dcs.rnBaseRect = rectFavoriteServersTabScreen;
		m_pFavoriteServersTab = debug_new(CLTGUIWindow);
		m_pFavoriteServersTab->Create(TextureReference(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenDialogFrame)),dcs);
		m_pFavoriteServersTab->Enable( false );
		// Add the control to the screen.
		AddControl( m_pFavoriteServersTab );

		// Add favorite servers buttons
		{
			LTRect2n rectFavoriteServersButtonScreen = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_FavoriteServersButtons);
			char const* pszButtonFont = g_pLayoutDB->GetListFont( m_hLayout, eListIndex_FavoriteServersButtons );
			int32 nButtonFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_FavoriteServersButtons);

			CLTGUICtrl_create cs;
			cs.rnBaseRect = rectFavoriteServersButtonScreen;
			cs.rnBaseRect.Bottom() = cs.rnBaseRect.Top() + nButtonFontSize;

			cs.nCommandID = CMD_FAVORITESERVERS_ADD;
			cs.szHelpID = "ScreenMulti_FavoriteServers_Add_Help";
			m_pFavoriteServersAddButton = CreateTextItem("ScreenMulti_FavoriteServers_Add", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFavoriteServersAddButton );

			cs.rnBaseRect.Offset( 0, nButtonFontSize );
			cs.nCommandID = CMD_FAVORITESERVERS_ADDIP;
			cs.szHelpID = "ScreenMulti_FavoriteServers_AddIP_Help";
			m_pFavoriteServersAddIPButton = CreateTextItem("ScreenMulti_FavoriteServers_AddIP", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFavoriteServersAddIPButton );

			cs.rnBaseRect.Offset( 0, nButtonFontSize );
			cs.nCommandID = CMD_FAVORITESERVERS_REMOVE;
			cs.szHelpID = "ScreenMulti_FavoriteServers_Remove_Help";
			m_pFavoriteServersRemoveButton = CreateTextItem("ScreenMulti_FavoriteServers_Remove", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFavoriteServersRemoveButton );

			cs.rnBaseRect.Offset( 0, nButtonFontSize );
			cs.nCommandID = CMD_FAVORITESERVERS_REMOVEALL;
			cs.szHelpID = "ScreenMulti_FavoriteServers_RemoveAll_Help";
			m_pFavoriteServersRemoveAllButton = CreateTextItem("ScreenMulti_FavoriteServers_RemoveAll", cs, false, pszButtonFont, nButtonFontSize );
			AddControl( m_pFavoriteServersRemoveAllButton );
		}
	}

	// Create the join button.
	cs.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,eListIndex_Join);
	cs.nCommandID = CMD_JOIN;
	cs.szHelpID = "IDS_HELP_JOIN";
	AddTextItem("IDS_JOIN", cs, false, NULL, g_pLayoutDB->GetListSize(m_hLayout,eListIndex_Join) );

	m_sPort = L"27888";

	CLTGUICtrl_create frameCs;
	TextureReference hGS(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,0));
	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,0);

	CLTGUIFrame* pGameSpy = debug_new(CLTGUIFrame);
	pGameSpy->Create(hGS,frameCs,true);
	AddControl(pGameSpy);

	// Make the MOTD popup
	{
		LTRect2n dlgRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenDialogRect);
		std::string sDlgFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenDialogFont);
		uint8 nDlgFontSz = ( uint8 )g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenDialogSize);

		CLTGUICtrl_create dcs;
		dcs.rnBaseRect = dlgRect;
		m_pMOTDDlg = debug_new(CLTGUIWindow);
		m_pMOTDDlg->Create(TextureReference(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenDialogFrame)),dcs);

		CLTGUICtrl_create dlgCs;
		TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,1));
		dlgCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,1);

		m_pMOTDImage = debug_new(CLTGUIFrame);
		m_pMOTDImage->Create(hFrame,dlgCs,true);
		m_pMOTDDlg->AddControl(m_pMOTDImage,dlgCs.rnBaseRect.m_vMin);

		dlgCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,2);
		dlgCs.nCommandID = NULL;
		dlgCs.szHelpID = "";
		m_pMOTDText = CreateTextItem(L"", dlgCs, true, sDlgFont.c_str(), nDlgFontSz);
		m_pMOTDText->SetWordWrap(true);
		m_pMOTDText->SetClipping(true);
		m_pMOTDDlg->AddControl(m_pMOTDText, dlgCs.rnBaseRect.m_vMin);

		dlgCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,3);
		dlgCs.nCommandID = CMD_MOTD_LINK;
		dlgCs.szHelpID = "ScreenMulti_MOTD_Link_Help";
		m_pMOTDLink = CreateTextItem("ScreenMulti_MOTD_Link", dlgCs, false, sDlgFont.c_str(), nDlgFontSz);
		m_pMOTDDlg->AddControl(m_pMOTDLink, dlgCs.rnBaseRect.m_vMin);

		dlgCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,4);
		dlgCs.nCommandID = CMD_MOTD_CLOSE;
		dlgCs.szHelpID = "ScreenMulti_MOTD_Close_Help";
		CLTGUITextCtrl *pCtrl = CreateTextItem("ScreenMulti_MOTD_Close", dlgCs, false, sDlgFont.c_str(), nDlgFontSz);
		pCtrl->SetAlignment(kRight);
		m_pMOTDDlg->AddControl(pCtrl, dlgCs.rnBaseRect.m_vMin);

		m_pMOTDDlg->Show(false);

		AddControl(m_pMOTDDlg);

	}


	// Make sure to call the base class
	return CBaseScreen::Build();
}

// adds the server browser headers
bool CScreenMulti::AddServerBrowserColumns( CLTGUIHeaderCtrl* pHeaderCtrl )
{
	LTASSERT( pHeaderCtrl, "Invalid header control" );
	if( !pHeaderCtrl )
		return false;

	struct SColumnData
	{
		ServerBrowserCtrl::EColumn	m_Column;
		uint32						m_idHeader;
		const wchar_t*				m_wszName;
		const char*					m_szHelpId;
		const char*					m_szIconNormal;
		const char*					m_szIconHot;
		eTextAlign					m_Align;
		uint32						m_Width;
		bool						m_bSizeable;
	};

	SColumnData pColumnData[ServerBrowserCtrl::eColumnCount] = {
		{ ServerBrowserCtrl::eColumn_Lock, CMD_SORT_LOCK, L"", "SCREENMULTI_SORT_LOCK_HELP", g_pLayoutDB->GetHeaderIcon("Password"), g_pLayoutDB->GetHighlightIcon("Password"), kCenter, kServerListColWidth[ServerBrowserCtrl::eColumn_Lock], false },
		{ ServerBrowserCtrl::eColumn_Platform, CMD_SORT_PLATFORM, L"", "SCREENMULTI_SORT_PLATFORM_HELP", g_pLayoutDB->GetHeaderIcon("Platform"), g_pLayoutDB->GetHighlightIcon("LinuxServer"), kCenter, kServerListColWidth[ServerBrowserCtrl::eColumn_Platform], false },
		{ ServerBrowserCtrl::eColumn_Punkbuster, CMD_SORT_PUNKBUSTER, L"", "SCREENMULTI_SORT_PUNKBUSTER_HELP", g_pLayoutDB->GetHeaderIcon("Punkbuster"), g_pLayoutDB->GetHighlightIcon("Punkbuster"), kCenter, kServerListColWidth[ServerBrowserCtrl::eColumn_Punkbuster], false },
		{ ServerBrowserCtrl::eColumn_Customized, CMD_SORT_CUSTOMIZED, L"", "SCREENMULTI_SORT_CUSTOMIZED_HELP", g_pLayoutDB->GetHeaderIcon("Customized"), g_pLayoutDB->GetHighlightIcon("Customized"), kCenter, kServerListColWidth[ServerBrowserCtrl::eColumn_Customized], false },
		{ ServerBrowserCtrl::eColumn_RequiresDownload, CMD_SORT_REQUIRESDOWNLOAD, L"", "SCREENMULTI_SORT_REQUIRESDOWNLOAD_HELP", g_pLayoutDB->GetHeaderIcon("Download"), g_pLayoutDB->GetHighlightIcon("Download"), kCenter, kServerListColWidth[ServerBrowserCtrl::eColumn_RequiresDownload], false },
		{ ServerBrowserCtrl::eColumn_Name, CMD_SORT_NAME, LoadString("SCREENMULTI_LIST_SERVER"), "SCREENMULTI_SORT_NAME_HELP", NULL, NULL, kLeft, kServerListColWidth[ServerBrowserCtrl::eColumn_Name], true },
		{ ServerBrowserCtrl::eColumn_Ping, CMD_SORT_PING, LoadString("SCREENMULTI_LIST_PING"), "SCREENMULTI_SORT_PING_HELP", NULL, NULL, kLeft, kServerListColWidth[ServerBrowserCtrl::eColumn_Ping], true },
		{ ServerBrowserCtrl::eColumn_Player, CMD_SORT_PLAYER, LoadString("SCREENMULTI_LIST_PLAYERS"), "SCREENMULTI_SORT_PLAYERS_HELP", NULL, NULL, kLeft, kServerListColWidth[ServerBrowserCtrl::eColumn_Player], true },
		{ ServerBrowserCtrl::eColumn_Type, CMD_SORT_TYPE, LoadString("SCREENMULTI_LIST_TYPE"), "SCREENMULTI_SORT_TYPE_HELP", NULL, NULL, kLeft, kServerListColWidth[ServerBrowserCtrl::eColumn_Type], true },
		{ ServerBrowserCtrl::eColumn_Mission, CMD_SORT_MISSION, LoadString("SCREENMULTI_LIST_MISSION"), "SCREENMULTI_SORT_MISSION_HELP", NULL, NULL, kLeft, kServerListColWidth[ServerBrowserCtrl::eColumn_Mission], true },
	};

	for(uint32 nColumn=0;nColumn<LTARRAYSIZE(pColumnData);++nColumn)
	{
		pHeaderCtrl->InsertItem( nColumn, pColumnData[nColumn].m_idHeader, pColumnData[nColumn].m_wszName, pColumnData[nColumn].m_szHelpId, pColumnData[nColumn].m_Align, pColumnData[nColumn].m_Width, pColumnData[nColumn].m_bSizeable, pColumnData[nColumn].m_szIconNormal, pColumnData[nColumn].m_szIconHot );
	}

	pHeaderCtrl->Rescale();

	return true;
}

uint32 CScreenMulti::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SORT_NAME:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Name, true );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Name );
		break;
	case CMD_SORT_PING:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Ping, true  );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Ping );
		break;
	case CMD_SORT_PLAYER:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Player, true  );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Player );
		break;
	case CMD_SORT_LOCK:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Lock, true  );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Lock );
		break;
	case CMD_SORT_MISSION:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Mission, true  );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Mission );
		break;
	case CMD_SORT_TYPE:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Type, true );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Type );
		break;
	case CMD_SORT_PLATFORM:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Platform, true );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Platform );
		break;
	case CMD_SORT_CUSTOMIZED:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Customized, true );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Customized );
		break;
	case CMD_SORT_REQUIRESDOWNLOAD:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_RequiresDownload, true );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_RequiresDownload );
		break;
	case CMD_SORT_PUNKBUSTER:
		DisplayCurServerList( ServerBrowserCtrl::eColumn_Punkbuster, true );
		m_pServerListCtrl->SetStartIndex( 0 );
		m_pServerListCtrl->SetSelectedColumn( ServerBrowserCtrl::eColumn_Punkbuster );
		break;
	case CMD_PLAYERS:
		m_pPlayerListCtrl->Show(true);
		m_pRulesListCtrl->Show(false);
		m_pCustomizersListCtrl->Show(false);
		m_pFiltersListCtrl->Show(false);
		m_pFriendsTab->Show(false);
		m_pFriendsListCtrl->Show(false);
		m_pFriendsAddButton->Show(false);
		m_pFriendsRemoveButton->Show(false);
		m_pFriendsDeselectButton->Show(false);
		m_pFriendsRefreshButton->Show(false);
		m_pFavoriteServersTab->Show(false);
		m_pFavoriteServersAddButton->Show(false);
		m_pFavoriteServersAddIPButton->Show(false);
		m_pFavoriteServersRemoveButton->Show(false);
		m_pFavoriteServersRemoveAllButton->Show(false);
		break;
	case CMD_RULES:
		m_pPlayerListCtrl->Show(false);
		m_pRulesListCtrl->Show(true);
		m_pCustomizersListCtrl->Show(false);
		m_pFiltersListCtrl->Show(false);
		m_pFriendsTab->Show(false);
		m_pFriendsListCtrl->Show(false);
		m_pFriendsAddButton->Show(false);
		m_pFriendsRemoveButton->Show(false);
		m_pFriendsDeselectButton->Show(false);
		m_pFriendsRefreshButton->Show(false);
		m_pFavoriteServersTab->Show(false);
		m_pFavoriteServersAddButton->Show(false);
		m_pFavoriteServersAddIPButton->Show(false);
		m_pFavoriteServersRemoveButton->Show(false);
		m_pFavoriteServersRemoveAllButton->Show(false);
		break;
	case CMD_CUSTOMIZERS:
		m_pPlayerListCtrl->Show(false);
		m_pRulesListCtrl->Show(false);
		m_pCustomizersListCtrl->Show(true);
		m_pFiltersListCtrl->Show(false);
		m_pFriendsTab->Show(false);
		m_pFriendsListCtrl->Show(false);
		m_pFriendsAddButton->Show(false);
		m_pFriendsRemoveButton->Show(false);
		m_pFriendsDeselectButton->Show(false);
		m_pFriendsRefreshButton->Show(false);
		m_pFavoriteServersTab->Show(false);
		m_pFavoriteServersAddButton->Show(false);
		m_pFavoriteServersAddIPButton->Show(false);
		m_pFavoriteServersRemoveButton->Show(false);
		m_pFavoriteServersRemoveAllButton->Show(false);
		break;
	case CMD_FILTERS:
		m_pPlayerListCtrl->Show(false);
		m_pRulesListCtrl->Show(false);
		m_pCustomizersListCtrl->Show(false);
		m_pFiltersListCtrl->Show(true);
		m_pFriendsTab->Show(false);
		m_pFriendsListCtrl->Show(false);
		m_pFriendsAddButton->Show(false);
		m_pFriendsRemoveButton->Show(false);
		m_pFriendsDeselectButton->Show(false);
		m_pFriendsRefreshButton->Show(false);
		m_pFavoriteServersTab->Show(false);
		m_pFavoriteServersAddButton->Show(false);
		m_pFavoriteServersAddIPButton->Show(false);
		m_pFavoriteServersRemoveButton->Show(false);
		m_pFavoriteServersRemoveAllButton->Show(false);
		break;
	case CMD_FRIENDS:
		m_pPlayerListCtrl->Show(false);
		m_pRulesListCtrl->Show(false);
		m_pCustomizersListCtrl->Show(false);
		m_pFiltersListCtrl->Show(false);
		m_pFriendsTab->Show(true);
		m_pFriendsListCtrl->Show(true);
		m_pFriendsAddButton->Show(true);
		m_pFriendsRemoveButton->Show(true);
		m_pFriendsDeselectButton->Show(true);
		m_pFriendsRefreshButton->Show(true);
		m_pFavoriteServersTab->Show(false);
		m_pFavoriteServersAddButton->Show(false);
		m_pFavoriteServersAddIPButton->Show(false);
		m_pFavoriteServersRemoveButton->Show(false);
		m_pFavoriteServersRemoveAllButton->Show(false);
		break;

	case CMD_FAVORITESERVERS:
		m_pPlayerListCtrl->Show(false);
		m_pRulesListCtrl->Show(false);
		m_pCustomizersListCtrl->Show(false);
		m_pFiltersListCtrl->Show(false);
		m_pFriendsTab->Show(false);
		m_pFriendsListCtrl->Show(false);
		m_pFriendsAddButton->Show(false);
		m_pFriendsRemoveButton->Show(false);
		m_pFriendsDeselectButton->Show(false);
		m_pFriendsRefreshButton->Show(false);
		m_pFavoriteServersTab->Show(true);
		m_pFavoriteServersAddButton->Show(true);
		m_pFavoriteServersAddIPButton->Show(true);
		m_pFavoriteServersRemoveButton->Show(true);
		m_pFavoriteServersRemoveAllButton->Show(true);
		break;

	case CMD_CANCEL :
	{
		SetCapture(NULL);
		ChangeState(eState_Waiting);
		break;
	}

	case CMD_DETAILS :
	{
		char szPublicAddress[64] = "";
		bool bDirectConnect = false;

		// Clear out the current details.
		{
			// Lock the list while we get some data out of it.
			CWinSync_CSAuto cs( m_csAddServer );

			m_pPlayerListCtrl->RemoveAll( true );
			m_pRulesListCtrl->RemoveAll( true );
			m_pCustomizersListCtrl->RemoveAll( true );

			ServerEntry* pServerEntry = reinterpret_cast< ServerEntry* >( dwParam1 );
			if( !pServerEntry )
				return 0;

			m_sSelectedServerAddress = pServerEntry->m_sPublicAddress;

			LTStrCpy( szPublicAddress, pServerEntry->m_sPublicAddress.c_str(), LTARRAYSIZE( szPublicAddress ));
			bDirectConnect = pServerEntry->m_bDirectConnect;
		}

		RequestServerDetails( szPublicAddress, bDirectConnect, true );
		break;
	}

	case CMD_SEARCH:
		FindServers();
		break;

	case CMD_HOST:
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST);
		break;

	case CMD_OK:
		HandleCallback(dwParam1,dwParam2);
		break;

	case CMD_JOIN:
		{
			CLTGUICtrl* pSelectedCtrl = m_pServerListCtrl->GetSelectedControl( );
			if( !pSelectedCtrl )
				return 0;
			ServerEntry* pServerEntry = reinterpret_cast< ServerEntry* >( pSelectedCtrl->GetParam1( ));
			if( !pServerEntry )
				return 0;

			if( !JoinServer( *pServerEntry ))
				return 0;

			return 1;
		} 
		break;

	case CMD_CLIENTSETTINGS:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PLAYER);
		}
		break;
	case CMD_MOTD_SHOW:
		{
			m_pMOTDDlg->Show(true);
			SetCapture(m_pMOTDDlg);
		}
		break;
	case CMD_MOTD_CLOSE:
		{
			m_pMOTDDlg->Show(false);
			if (GetCapture() == m_pMOTDDlg)
			{
				SetCapture(NULL);
			}
		}
		break;
	case CMD_MOTD_LINK:
		{
			LaunchApplication::LaunchMOTDLink(MPW2A(m_MOTD.m_wsLinkURL.c_str()).c_str());
		}
		break;
		
	case CMD_SOURCE:
		{
			HandleCmd_Source();
		} 
		break;

	case CMD_FILTER_FRIEND:
	case CMD_FILTER_VERSION:
	case CMD_FILTER_TYPE:
	case CMD_FILTER_PLAYERS:
	case CMD_FILTER_CUSTOMIZED:
	case CMD_FILTER_REQUIRESSDOWNLOAD:
	case CMD_FILTER_PUNKBUSTER:
		{
			bool bFound = false;
			char szPublicAddress[64] = "";
			bool bDirectConnect = false;

			{
				CWinSync_CSAuto cs( m_csAddServer );

				m_pPlayerListCtrl->RemoveAll( true );
				m_pRulesListCtrl->RemoveAll( true );
				m_pCustomizersListCtrl->RemoveAll( true );

				UpdateData( true );

				// Update the friend filter manually.
				CLTGUITextCtrl* pSelectedFriendCtrl = ( CLTGUITextCtrl* )m_pFriendsListCtrl->GetSelectedControl();
				if( pSelectedFriendCtrl )
				{
					SetFriendFilter( MPW2A( pSelectedFriendCtrl->GetString()));
				}

				// Setup game type filter.  If 0, then that means all, otherwise it's an index into the map to gamemode
				// records.
				HRECORD hGameTypeFilter = (m_nGameTypeFilter && m_nGameTypeFilter <= m_lstGameTypes.size()) ? m_lstGameTypes[m_nGameTypeFilter-1] : NULL;

				// Filter the servers.  Don't ever filter the favorites list, though.  This 
				// should always be seen since they are our favorite.
				if( m_eServerSearchSource != eServerSearchSource_Favorites )
				{
					ControlArray& lst = m_pServerListCtrl->GetControlArray();
					for( ControlArray::iterator iter = lst.begin( ); iter != lst.end( ); iter++ )
					{
						CLTGUICtrl* pControl = *iter;
						ServerEntry* pServerEntry = ( ServerEntry* )pControl->GetParam1();
						FilterServer( *pServerEntry, m_nVersionFilter, hGameTypeFilter, 
							( EFilterPlayers )m_nPlayersFilter, m_nPingFilter, 
							( EFilterTristate )m_nCustomizedFilter, ( EFilterTristate )m_nRequiresDownloadFilter, 
							( EFilterTristate )m_nPunkbusterFilter, 
							GetFriendFilter());
					}
				}
				m_pServerListCtrl->RecalcLayout();

				CLTGUICtrl* pSelectedCtrl = m_pServerListCtrl->GetSelectedControl( );
				if( pSelectedCtrl && pSelectedCtrl->IsVisible())
				{
					ServerEntry* pServerEntry = ( ServerEntry* )pSelectedCtrl->GetParam1();
					if( pServerEntry )
					{
						m_sSelectedServerAddress = pServerEntry->m_sPublicAddress;
						LTStrCpy( szPublicAddress, pServerEntry->m_sPublicAddress.c_str(), LTARRAYSIZE( szPublicAddress ));
						bDirectConnect = pServerEntry->m_bDirectConnect;
						bFound = true;
					}
				}
			}

			if( bFound )
			{
				RequestServerDetails( szPublicAddress, bDirectConnect, true );
			}
		}
		break;


	case CMD_FRIENDS_ADD:
		{
			// Ask user for the nickname of the new friend.
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditFriendsAdd;
			mb.pString = L"";
			mb.nMaxChars = MAX_PLAYER_NAME;
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox( LoadString( "ScreenMulti_Friends_Add_Edit" ), &mb );
		}
		break;

	case CMD_FRIENDS_REMOVE:
		{
			// Need to have a profile to remove the friend from.
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			if( pProfile )
			{
				// Get the friend they have selected.
				uint32 nSelectedControlIndex = m_pFriendsListCtrl->GetSelectedIndex( );
				if( nSelectedControlIndex != CLTGUIListCtrlEx::kNoSelection )
				{
					// Remove the nickname from the profile.
					pProfile->RemoveFriend( nSelectedControlIndex );

					// Remove the control from the list control.
					m_pFriendsListCtrl->RemoveControl( nSelectedControlIndex, true );

					// Allow Add button if there's room in the list.
					m_pFriendsAddButton->Enable( pProfile->GetNumFriends() < CUserProfile::eFriendsInfo_MaxCount );

					// Clear any selection.
					m_pFriendsListCtrl->SetSelection( CLTGUIListCtrlEx::kNoSelection );

					// Clear the friend filter, since we just deleted it.
					SetFriendFilter( NULL );
					DisplayCurServerList( m_eLastSort, false );
				}
			}
		}
		break;

	case CMD_FRIENDS_DESELECT:
		{
			// Stop filtering by friend.
			if( m_pFriendsListCtrl )
			{
				m_pFriendsListCtrl->SetSelection( CLTGUIListCtrlEx::kNoSelection );
				SetFriendFilter( NULL );
				DisplayCurServerList( m_eLastSort, false );
			}
		}
		break;

	case CMD_FRIENDS_REFRESH:
		{
			RefreshDetailsOnServers( );
		}
		break;

	case CMD_FAVORITESERVERS_ADD:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			if( pProfile )
			{
				// Allow player to add servers from Internet or LAN sources.
				if( m_eServerSearchSource != eServerSearchSource_Favorites )
				{
					// Get the selected server.
					ServerEntry* pServerEntry = NULL;
					CLTGUIColumnCtrl* pColumnCtrl = ( CLTGUIColumnCtrl* )m_pServerListCtrl->GetSelectedControl( );
					if( pColumnCtrl )
					{
						pServerEntry = reinterpret_cast< ServerEntry* >( pColumnCtrl->GetParam1( ));
					}

					if( pServerEntry )
					{
						AddServerEntryToFavorites( *pServerEntry );

						// Save the profile.
						pProfile->Save();
					}
				}
			}
		}
		break;

	case CMD_FAVORITESERVERS_ADDIP:
		{
			// Ask user for the IP:Port
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditFavoriteServerssAddIP;
			mb.pString = L"";
			mb.nMaxChars = kMaxIPPortLength + 1;
			mb.pUserData = this;
			mb.pFilterFn = IPPortFilter;
			g_pInterfaceMgr->ShowMessageBox( LoadString( "ScreenMulti_FavoriteServers_AddIP_Edit" ), &mb );
		}
		break;

	case CMD_FAVORITESERVERS_REMOVE:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			if( pProfile )
			{
				// Can only remove a favorite when on the favorites source.
				if( m_eServerSearchSource == eServerSearchSource_Favorites )
				{
					// Get the server control selected.
					CLTGUIColumnCtrl* pSelectedCtrl = ( CLTGUIColumnCtrl* )m_pServerListCtrl->GetSelectedControl( );
					if( pSelectedCtrl )
					{
						// Get the serverentry out of the control.
						ServerEntry* pServerEntry = ( ServerEntry* )pSelectedCtrl->GetParam1();
						if( pSelectedCtrl )
						{
							// Remove the server.
							char szServerIPandPort[kMaxIPPortLength+1];
							LTStrCpy( szServerIPandPort, pServerEntry->m_sPublicAddress.c_str( ), LTARRAYSIZE( szServerIPandPort ));
							RemoveServerIPFromFavorites( szServerIPandPort );

							// Save the profile.
							pProfile->Save();
						}
					}
				}
			}
		}
		break;


	case CMD_FAVORITESERVERS_REMOVEALL:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			if( pProfile )
			{
				// Can only remove a favorite when on the favorites source.
				if( m_eServerSearchSource == eServerSearchSource_Favorites && pProfile->GetNumFavoriteServers( ))
				{
					// Iterate through all the servers and remove them.
					while( pProfile->GetNumFavoriteServers())
					{
						FavoriteServer* pFavoriteServer = pProfile->GetFavoriteServer( 0 );
						if( pFavoriteServer )
						{
							// Remove the server.
							char szServerIPandPort[kMaxIPPortLength+1];
							LTStrCpy( szServerIPandPort, pFavoriteServer->GetServerIPandPort(), LTARRAYSIZE( szServerIPandPort ));
							RemoveServerIPFromFavorites( szServerIPandPort );
						}
					}

					// Save the profile.
					pProfile->Save();
				}
			}
		}
		break;


	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void CScreenMulti::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (bFocus)
	{
		// Update our values from our profile.
		if( pProfile )
		{
			m_nVersionFilter = pProfile->m_nVersionFilter;
			m_nPlayersFilter = pProfile->m_nPlayersFilter;
			m_nPingFilter = pProfile->m_nPingFilter;
			m_nGameTypeFilter = pProfile->m_nGameTypeFilter;
			m_eServerSearchSource = pProfile->m_eServerSearchSource;
			m_nSourceIndex = m_eServerSearchSource;
			m_nCustomizedFilter = pProfile->m_nCustomizedFilter;
			m_nRequiresDownloadFilter = pProfile->m_nRequiresDownloadFilter;
			m_nPunkbusterFilter = pProfile->m_nPunkbusterFilter;
		}


		char szPath[MAX_PATH*2];
		LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
		LTStrCat( szPath, MDB_MP_File, LTARRAYSIZE( szPath ));
		g_pMissionDB->Init( szPath );

		bool bCreatedBrowser = false;

		if( !g_pClientConnectionMgr->GetServerBrowser( ))
		{
			g_pClientConnectionMgr->CreateServerBrowser( );
			bCreatedBrowser = true;
		}

		// Always set the callback data...
		if( g_pClientConnectionMgr->GetServerBrowser( ))
			g_pClientConnectionMgr->GetServerBrowser( )->SetServerInfoCB( ServerInfoCallback, this );

		// Be sure our server list is showing the correct list.
		UpdateServerListControls();

		// Wait for user to do something.
		ChangeState(eState_Waiting);

		// we may have gotten here after failing a join, so rebuild our history
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
		}

		SetSelection(GetIndex(m_pFindCtrl));

		m_pPlayerListCtrl->Show(true);
		m_pRulesListCtrl->Show(false);
		m_pCustomizersListCtrl->Show(false);
		m_pFiltersListCtrl->Show(false);
		m_pFriendsTab->Show(false);
		m_pFriendsListCtrl->Show(false);
		m_pFriendsAddButton->Show(false);
		m_pFriendsRemoveButton->Show(false);
		m_pFriendsDeselectButton->Show(false);
		m_pFriendsRefreshButton->Show(false);
		m_pFavoriteServersTab->Show(false);
		m_pFavoriteServersAddButton->Show(false);
		m_pFavoriteServersAddIPButton->Show(false);
		m_pFavoriteServersRemoveButton->Show(false);
		m_pFavoriteServersRemoveAllButton->Show(false);

		// Populate the friends list control from the profile.
		PopulateFriendsListControl( );

		// Clear any friends filter.
		if( m_pFriendsListCtrl )
		{
			m_pFriendsListCtrl->SetSelection( CLTGUIListCtrlEx::kNoSelection );
			SetFriendFilter( NULL );
		}

		// Populate the favorite server lists.
		PopulateFavoriteServers( );

		UpdateMOTD();
		if (m_bCheckForMOTD)
		{
			CheckForMOTD();
		}

        UpdateData(LTFALSE);

		m_eLastBrowserStatus = IGameSpyBrowser::eBrowserStatus_Idle;
	}
	else
	{
		// Don't go!
		ChangeState(eState_Inactive);

		UpdateData();

		// Store all our values in the profile.
		if( pProfile )
		{
			pProfile->m_nVersionFilter = m_nVersionFilter;
			pProfile->m_nPlayersFilter = m_nPlayersFilter;
			pProfile->m_nPingFilter = m_nPingFilter;
			pProfile->m_nGameTypeFilter = m_nGameTypeFilter;
			pProfile->m_eServerSearchSource = m_eServerSearchSource;
			pProfile->m_nCustomizedFilter = m_nCustomizedFilter;
			pProfile->m_nRequiresDownloadFilter = m_nRequiresDownloadFilter;
			pProfile->m_nPunkbusterFilter = m_nPunkbusterFilter;

			pProfile->Save();
		}
	}

	CBaseScreen::OnFocus(bFocus);
}





bool CScreenMulti::Render()
{
	if( m_pFrameLowerRight )
	{
		if( m_pServerListCtrl == m_pInternetServerListCtrl )
		{
			if( m_pInternetScrollBar )
				m_pFrameLowerRight->SetRenderBorder( CLTGUIFillFrame::eBorder_Left|CLTGUIFillFrame::eBorder_Bottom|((m_pInternetScrollBar->IsVisible())?0:CLTGUIFillFrame::eBorder_Right) );
		}
		else if( m_pServerListCtrl == m_pLANServerListCtrl )
		{
			if( m_pLANScrollBar )
				m_pFrameLowerRight->SetRenderBorder( CLTGUIFillFrame::eBorder_Left|CLTGUIFillFrame::eBorder_Bottom|((m_pLANScrollBar->IsVisible())?0:CLTGUIFillFrame::eBorder_Right) );
		}
	}

	Update();

	// lock while rendering
	CWinSync_CSAuto cs( m_csAddServer );


	bool bRet = CBaseScreen::Render();

	if (bRet && m_pMOTDDlg->IsVisible() && m_hMOTDImage)
	{
		

		//setup the draw prim for rendering
		g_pLTClient->GetDrawPrim()->SetDynamicTexture(m_hMOTDImage);

		//and render away
		g_pLTClient->GetDrawPrim()->DrawPrim(&m_MOTDQuad, 1);

		g_pLTClient->GetDrawPrim()->SetDynamicTexture(NULL);
	}

	return bRet;
}


void CScreenMulti::FindServers()
{
	{
		// Need to lock access to the controls while updating data.
		CWinSync_CSAuto cs( m_csAddServer );

		UpdateData( true );

		if( m_pServerListCtrl )
			m_pServerListCtrl->SetSelectedColumn( -1 );

		if( !g_pClientConnectionMgr->GetServerBrowser( ))
			return;

		// Clear the friend filter since we're going to lose all that information.
		if( m_pFriendsListCtrl )
		{
			m_pFriendsListCtrl->SetSelection( CLTGUIListCtrlEx::kNoSelection );
			SetFriendFilter( NULL );
		}

		// Don't allow a new find while the browsers are working.
		if( g_pClientConnectionMgr->GetServerBrowser( )->GetBrowserStatus( ) == 
			IGameSpyBrowser::eBrowserStatus_Processing )
			return;
	}

	ChangeState(eState_UpdateDir);
}


void CScreenMulti::Escape()
{

	if (m_pMOTDDlg->IsVisible())
	{
		m_pMOTDDlg->Show(false);
		SetCapture(NULL);
	}
	CBaseScreen::Escape();
}

void CScreenMulti::Term( )
{
	// Stop any requests going.  This function may be called even if the 
	// ClientConnectionManager wasn't created (failure during client 
	// initialization), so verify the pointer before using it.
	if( g_pClientConnectionMgr && g_pClientConnectionMgr->GetServerBrowser( ))
		g_pClientConnectionMgr->GetServerBrowser( )->HaltRequest( );

	// Get rid of server lists.
	TermServerList( m_cInternetServerMap, m_pInternetServerListCtrl );
	TermServerList( m_cLANServerMap, m_pLANServerListCtrl );
	TermServerList( m_cFavoriteServerMap, m_pFavoriteServerListCtrl );

	if (m_hMOTDImage)
	{
		g_pILTTextureMgr->ReleaseDynamicTexture(m_hMOTDImage);
	}


	CBaseScreen::Term( );
}


// ----------------------------------------------------------------------- //
// Function name   : CScreenMulti::TermServerList
// Description     : Deletes the server list objects.  Good idea
//						to delete them when you are done with them,
//						since they can store lots of data in the server lists.
// ----------------------------------------------------------------------- //
void CScreenMulti::TermServerList( TServerEntryMap& serverMap, CLTGUIListCtrlEx* pServerListCtrl )
{
	// Clear out our current server list.  We need
	// to check m_bInit here because ScreenMgr
	// could have deleted our controls from under us.
	if( m_bInit && pServerListCtrl )
	{
		pServerListCtrl->SetStartIndex( 0 );
		pServerListCtrl->RemoveAll();
	}

	// Clear out the server entry map.
	serverMap.clear( );

	if( m_pPlayerListCtrl )
		m_pPlayerListCtrl->RemoveAll();
	if( m_pRulesListCtrl )
		m_pRulesListCtrl->RemoveAll();
	if( m_pCustomizersListCtrl )
		m_pCustomizersListCtrl->RemoveAll();

	// No total information anymore.
	m_nTotalServerCount = 0;

	// Clear the server callback response list, just in case	
	{
		CWinSync_CSAuto cs( m_csAddServer );
		ltstd::reset_vector(m_aServerCallbackResponses);
	}
}

// ----------------------------------------------------------------------- //
// Function name   : CScreenMulti::DisplayCurServerList
// Description     : Refresh the display of the server list.
// Return type     : void 
// Argument        : ESortBy eSortBy - sort by this criteria.
// ----------------------------------------------------------------------- //
void CScreenMulti::DisplayCurServerList( ServerBrowserCtrl::EColumn eSortBy, bool bToggleDirection )
{
	CLTGUICtrl* pSelectedCtrl = m_pServerListCtrl->GetSelectedControl( );
	m_pServerListCtrl->ClearSelection();

	// Sort the servers.
	m_eLastSort = eSortBy;

	// Toggle the ascending flag.
	if( bToggleDirection )
	m_bAscending = !m_bAscending;

	ControlArray& lst = m_pServerListCtrl->GetControlArray();
	ControlListSort(lst.begin(),lst.end(), m_eLastSort, m_bAscending );

	// If we had a selection before, reselect it.
	if( pSelectedCtrl )
	{
		uint32 nSelectedIndex = m_pServerListCtrl->GetIndex( pSelectedCtrl );
		m_pServerListCtrl->SetSelection( nSelectedIndex );
	}

	// Filter the servers.  Don't ever filter the favorites list, though.  This 
	// should always be seen since they are our favorite.
	if( m_eServerSearchSource != eServerSearchSource_Favorites )
	{
		// Filter the servers.
		HRECORD hGameTypeFilter = (m_nGameTypeFilter && m_nGameTypeFilter <= m_lstGameTypes.size()) ? m_lstGameTypes[m_nGameTypeFilter-1] : NULL;
		for( ControlArray::iterator iter = lst.begin( ); iter != lst.end( ); iter++ )
		{
			CLTGUICtrl* pControl = *iter;
			ServerEntry* pServerEntry = ( ServerEntry* )pControl->GetParam1();
			FilterServer( *pServerEntry, m_nVersionFilter, hGameTypeFilter, 
				( EFilterPlayers )m_nPlayersFilter, m_nPingFilter, 
				( EFilterTristate )m_nCustomizedFilter, ( EFilterTristate )m_nRequiresDownloadFilter,
				( EFilterTristate )m_nPunkbusterFilter,
				GetFriendFilter( ));
		}
	}

	m_pServerListCtrl->RecalcLayout();
}

uint32 CScreenMulti::HandleCallback( uint32 dwParam1, uint32 dwParam2 )
{
	switch( dwParam2 )
	{
		case CMD_EDIT_PASS :
		{
			m_sPassword = (wchar_t *)dwParam1;

			CLTGUICtrl* pSelectedCtrl = m_pServerListCtrl->GetSelectedControl( );
			if( !pSelectedCtrl )
				return 0;
			ServerEntry* pServerEntry = ( ServerEntry* )pSelectedCtrl->GetParam1();
			if( !pServerEntry )
				return 0;
			
			JoinCurGame( *pServerEntry );
			break;
		}

		case CMD_EDIT_FRIENDS_ADD:
			{
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				if( pProfile )
				{
					// Get the new nickname from the messagebox.
					wchar_t const* pwszNewFriedNickName = (wchar_t *)dwParam1;

					// Add it to our profile.
					uint32 nNewFriendIndex = pProfile->AddFriend( pwszNewFriedNickName );
					pProfile->Save();

					// Repopulate the friends list control.
					PopulateFriendsListControl( );

					// Select the new friend.
					if( nNewFriendIndex != CUserProfile::eFriendsInfo_Invalid )
					{
						m_pFriendsListCtrl->SetSelection( nNewFriendIndex );
						SetFriendFilter( MPW2A( pwszNewFriedNickName ).c_str( ));
					}
					else
					{
						m_pFriendsListCtrl->SetSelection( CLTGUIListCtrlEx::kNoSelection );
						SetFriendFilter( NULL );
					}

					// Refresh the display.
					DisplayCurServerList( m_eLastSort, false );
				}
			}
			break;

		case CMD_EDIT_FAVORITESERVERS_ADDIP:
			{
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				if( pProfile )
				{
					// Add the new ip to the favorites.
					char szEntry[kMaxIPPortLength+1];
					LTStrCpy( szEntry, MPW2A((wchar_t *)dwParam1).c_str(), LTARRAYSIZE( szEntry ));
					// Split and rejoin the IP and port to ensure they have the port specified.  Will
					// use default if not.
					char szIP[kMaxIPPortLength+1];
					uint16 nPort;
					SplitIPandPort( szEntry, szIP, LTARRAYSIZE( szIP ), nPort );
					char szIPandPort[kMaxIPPortLength+1];
					JoinIPandPort( szIP, nPort, szIPandPort, LTARRAYSIZE( szIPandPort ));
					AddServerIPToFavorites( szIPandPort );

					// Save the profile.
					pProfile->Save( );
				}
			}
			break;
	}

	ChangeState(eState_Waiting);
	return 1;
}

// ----------------------------------------------------------------------- //
// Function name   : CScreenMulti::JoinCurGame
// Description     : Completes the joining of server already setup in JoinServer.
// Return type     : void 
// ----------------------------------------------------------------------- //
void CScreenMulti::JoinCurGame( ServerEntry& serverEntry )
{
	// Setup our client to join this server.
	if( !g_pClientConnectionMgr->SetupClient( MPA2W( serverEntry.m_sName.c_str()).c_str( ), m_sPassword.c_str(), 
		!serverEntry.m_bDirectConnect, serverEntry.m_bConnectViaPublic, 
		serverEntry.m_sPublicAddress.c_str( ), serverEntry.m_sPrivateAddress.c_str( )))
	{
		ChangeState(eState_Inactive);
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_MULTI);
		g_pInterfaceMgr->ConnectionFailed(g_pClientConnectionMgr->GetLastConnectionResult());
		return;
	}

	if( !g_pMissionMgr->StartGameAsClient( ))
	{
		ChangeState(eState_Inactive);
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_MULTI);
		g_pInterfaceMgr->ConnectionFailed(g_pClientConnectionMgr->GetLastConnectionResult());
		return;
	}
}



bool CScreenMulti::PreState(EState eNewState)
{
	switch (eNewState)
	{
		case eState_Inactive :
			return PreState_Inactive();
		case eState_Startup :
			return PreState_Startup();
		case eState_UpdateDir :
			return PreState_UpdateDir();
		case eState_Waiting :
			return PreState_Waiting();
		default :
			return true;
	}
}

bool CScreenMulti::PostState(EState eNewState)
{
	switch (m_eCurState)
	{
		case eState_UpdateDir :
			return PostState_UpdateDir(eNewState);
		case eState_Waiting :
			return PostState_Waiting(eNewState);
		default :
			return true;
	}
}

bool CScreenMulti::ChangeState(EState eNewState)
{
	if (!PostState(m_eCurState))
	{
		return false;
	}

	m_eCurState = eState_Transition;

	if (!PreState(eNewState))
	{
		m_pFindCtrl->Enable(true);
		m_eCurState = eState_Waiting;
		return false;
	}

	m_eCurState = eNewState;

	return true;
}

static uint32 CountPlayersOnServers( TServerEntryMap& serverEntryMap )
{
	uint32 nPlayerCount = 0;
	for( TServerEntryMap::iterator iter = serverEntryMap.begin(); iter != serverEntryMap.end( ); iter++ )
	{
		ServerEntry& serverEntry = iter->second;
		nPlayerCount += serverEntry.m_nNumPlayers;
	}

	return nPlayerCount;
}

void CScreenMulti::Update()
{

	ReadCallbackResponses();
	ReadMOTDCallbackResponses();
	
	switch (m_eCurState)
	{
		case eState_Startup :
			Update_State_Startup();
			break;
		case eState_UpdateDir :
			Update_State_UpdateDir();
			break;
		default :
			break;
	}

	// Update the server list status text if it changed.
	if (g_pClientConnectionMgr->GetServerBrowser())
	{
		IGameSpyBrowser::EBrowserStatus eBrowserStatus = g_pClientConnectionMgr->GetServerBrowser( )->GetBrowserStatus( );
		switch( eBrowserStatus )
		{
		default:
		case IGameSpyBrowser::eBrowserStatus_Complete:
		case IGameSpyBrowser::eBrowserStatus_Idle:
			{
				if( eBrowserStatus != m_eLastBrowserStatus )
				{
					m_pStatus->SetString( LoadString( "SCREENMULTI_LIST_WAITING" ));

					uint32 nPlayerCount = 0;
					{
						CWinSync_CSAuto cs( m_csAddServer );
						m_nTotalServerCount = m_pcServerMap->size( );
						nPlayerCount = CountPlayersOnServers( *m_pcServerMap );
					}
					if( m_nTotalServerCount > 0 )
					{
						wchar_t wszOut[1024];
						FormatString( "SCREENMULTI_SERVERCOUNT_FINAL", wszOut, LTARRAYSIZE( wszOut ), m_nTotalServerCount, 
							m_nTotalServerCount, nPlayerCount );
						m_pServerCount->SetString( wszOut );
						m_pServerCount->Show( true );
					}
					else
					{
						m_pServerCount->Show( false );
					}
				}
			}
			break;
		case IGameSpyBrowser::eBrowserStatus_Error:
			{
				if( eBrowserStatus != m_eLastBrowserStatus )
				{
					m_pStatus->SetString( LoadString( "SCREENMULTI_LIST_ERROR" ));

					uint32 nPlayerCount = 0;
					{
						CWinSync_CSAuto cs( m_csAddServer );
						m_nTotalServerCount = m_pcServerMap->size( );
						nPlayerCount = CountPlayersOnServers( *m_pcServerMap );
					}
					if( m_nTotalServerCount > 0 )
					{
						wchar_t wszOut[1024];
						FormatString( "SCREENMULTI_SERVERCOUNT_FINAL", wszOut, LTARRAYSIZE( wszOut ), m_nTotalServerCount, 
							m_nTotalServerCount, nPlayerCount );
						m_pServerCount->SetString( wszOut );
						m_pServerCount->Show( true );
					}
					else
					{
						m_pServerCount->Show( false );
					}
				}
			}
			break;
		case IGameSpyBrowser::eBrowserStatus_Processing:
			{
				if( eBrowserStatus != m_eLastBrowserStatus )
				{
					m_pStatus->SetString( LoadString( "SCREENMULTI_QUERYING" ));
				}

				// Say we're waiting for servers.  Use the gamespy number if this is the first time we're updating the value.
				// Otherwise just leave it alone.  This keeps it from slewing back and forth.  We don't really
				// know how many of the pending queries are servers we already know about.
				uint32 nServersRecieved = 0;
				{
					CWinSync_CSAuto cs( m_csAddServer );
					nServersRecieved = m_pcServerMap->size( );
				}
				uint32 nTotalServers = g_pClientConnectionMgr->GetServerBrowser( )->GetNumPendingServers( ) + nServersRecieved;
				if( m_nTotalServerCount == 0 )
					m_nTotalServerCount = nTotalServers;
				m_nTotalServerCount = LTMAX( m_nTotalServerCount, nServersRecieved );
				if( m_nTotalServerCount > 0 )
				{
					wchar_t wszOut[1024];
					FormatString( "SCREENMULTI_SERVERCOUNT_PROCESSING", wszOut, LTARRAYSIZE( wszOut ), nServersRecieved, 
						m_nTotalServerCount );
					m_pServerCount->SetString( wszOut );
					m_pServerCount->Show( true );
				}
				else
				{
					m_pServerCount->Show( false );
				}
			}
			break;
		}
		m_eLastBrowserStatus = eBrowserStatus;
	}

	if (m_bDisplayMOTD)
	{
		m_pMOTDDlg->Show(true);
		SetCapture(m_pMOTDDlg);

		m_bDisplayMOTD = false;
	}
	
}

void CScreenMulti::UpdateServerListControls()
{
	// Lock access since we're going to mess around with the map and list control.
	CWinSync_CSAuto cs( m_csAddServer );

	switch( m_eServerSearchSource )
	{
	case eServerSearchSource_Internet:

		m_pcServerMap = &m_cInternetServerMap;
		m_pServerListCtrl = m_pInternetServerListCtrl;

		m_pInternetServerListCtrl->Show( true );
		m_pInternetServerListCtrl->Enable( true );
		m_pInternetScrollBar->Show( true );
		m_pInternetScrollBar->Enable( true );
		m_pInternetHeaderCtrl->Show( true );
		m_pInternetHeaderCtrl->Enable( true );

		m_pLANServerListCtrl->Show( false );
		m_pLANServerListCtrl->Enable( false );
		m_pLANScrollBar->Show( false );
		m_pLANScrollBar->Enable( false );
		m_pLANHeaderCtrl->Show( false );
		m_pLANHeaderCtrl->Enable( false );
		m_pPort->Enable( false );
		m_pPort->Show( false );

		m_pFavoriteServerListCtrl->Show( false );
		m_pFavoriteServerListCtrl->Enable( false );
		m_pFavoriteServerScrollBar->Show( false );
		m_pFavoriteServerScrollBar->Enable( false );
		m_pFavoriteServerHeaderCtrl->Show( false );
		m_pFavoriteServerHeaderCtrl->Enable( false );

		break;
	case eServerSearchSource_LAN:

		m_pcServerMap = &m_cLANServerMap;
		m_pServerListCtrl = m_pLANServerListCtrl;

		m_pInternetServerListCtrl->Show( false );
		m_pInternetServerListCtrl->Enable( false );
		m_pInternetScrollBar->Show( false );
		m_pInternetScrollBar->Enable( false );
		m_pInternetHeaderCtrl->Show( false );
		m_pInternetHeaderCtrl->Enable( false );

		m_pLANServerListCtrl->Show( true );
		m_pLANServerListCtrl->Enable( true );
		m_pLANScrollBar->Show( true );
		m_pLANScrollBar->Enable( true );
		m_pLANHeaderCtrl->Show( true );
		m_pLANHeaderCtrl->Enable( true );
		m_pPort->Enable( true );
		m_pPort->Show( true );

		m_pFavoriteServerListCtrl->Show( false );
		m_pFavoriteServerListCtrl->Enable( false );
		m_pFavoriteServerScrollBar->Show( false );
		m_pFavoriteServerScrollBar->Enable( false );
		m_pFavoriteServerHeaderCtrl->Show( false );
		m_pFavoriteServerHeaderCtrl->Enable( false );

		break;
	case eServerSearchSource_Favorites:

		m_pcServerMap = &m_cFavoriteServerMap;
		m_pServerListCtrl = m_pFavoriteServerListCtrl;

		m_pInternetServerListCtrl->Show( false );
		m_pInternetServerListCtrl->Enable( false );
		m_pInternetScrollBar->Show( false );
		m_pInternetScrollBar->Enable( false );
		m_pInternetHeaderCtrl->Show( false );
		m_pInternetHeaderCtrl->Enable( false );

		m_pLANServerListCtrl->Show( false );
		m_pLANServerListCtrl->Enable( false );
		m_pLANScrollBar->Show( false );
		m_pLANScrollBar->Enable( false );
		m_pLANHeaderCtrl->Show( false );
		m_pLANHeaderCtrl->Enable( false );
		m_pPort->Enable( false );
		m_pPort->Show( false );

		m_pFavoriteServerListCtrl->Show( true );
		m_pFavoriteServerListCtrl->Enable( true );
		m_pFavoriteServerScrollBar->Show( true );
		m_pFavoriteServerScrollBar->Enable( true );
		m_pFavoriteServerHeaderCtrl->Show( true );
		m_pFavoriteServerHeaderCtrl->Enable( true );

		break;
	}
}

bool CScreenMulti::PreState_Inactive()
{
	// Make sure we're not doing anything...
	if( g_pClientConnectionMgr->GetServerBrowser( ))
		g_pClientConnectionMgr->GetServerBrowser( )->HaltRequest( );

	return true;
}

bool CScreenMulti::PreState_Startup()
{
	return true;
}

bool CScreenMulti::PreState_UpdateDir()
{
	// Disable the find control, and make sure it's not selected, otherwise the
	// select highlight will overshadow the disabled look.
	CLTGUICtrl* pCtrl = GetControl( GetSelection( ));
	if( pCtrl == m_pFindCtrl )
	{
		SetSelection( kNoSelection, false );
	}
	m_pFindCtrl->Enable( false );

	// Remember our selected address.
	CLTGUIColumnCtrl* pColumnCtrl = ( CLTGUIColumnCtrl* )m_pServerListCtrl->GetSelectedControl( );
	if( pColumnCtrl )
	{
		ServerEntry* pServerEntry = reinterpret_cast< ServerEntry* >( pColumnCtrl->GetParam1( ));
		if( pServerEntry )
		{
			m_sSelectedServerAddress = pServerEntry->m_sPublicAddress;
		}
	}

	// Stop any requests going.
	if( g_pClientConnectionMgr->GetServerBrowser( ))
		g_pClientConnectionMgr->GetServerBrowser( )->HaltRequest( );

	// Throw out old list.
	switch( m_eServerSearchSource )
	{
	case eServerSearchSource_Internet:
		TermServerList( m_cInternetServerMap, m_pInternetServerListCtrl );
		break;
	case eServerSearchSource_LAN:
		TermServerList( m_cLANServerMap, m_pLANServerListCtrl );
		break;
	case eServerSearchSource_Favorites:
		// Don't throw out favorites, since they are specially handled.
		break;
	}

	// Reset the lan refresh count.
	m_nLANRefreshCount = 0;

	// Start by sending the request.
	m_eUpdateDirState = eUpdateDirState_Request;

	return true;
}

bool CScreenMulti::RequestServerDetails( char const* pszPublicAddress, bool bDirectConnect, bool bFullDetails )
{
	if( LTStrEmpty( pszPublicAddress ))
	{
		return false;
	}

	// Convert the address string into ip and port.
	char szIP[kMaxIPPortLength+1];
	uint16 nPort;
	SplitIPandPort( pszPublicAddress, szIP, LTARRAYSIZE( szIP ), nPort );

	// Request the details for this server.
	IGameSpyBrowser* pGameSpyBrowser = g_pClientConnectionMgr->GetServerBrowser( );
	if( !pGameSpyBrowser || !pGameSpyBrowser->RequestServerDetails( szIP, nPort, bDirectConnect, bFullDetails ))
	{
		return false;
	}

	return true;
}


bool CScreenMulti::PreState_Waiting()
{
	m_pFindCtrl->Enable( true );

	return true;
}

bool CScreenMulti::PostState_UpdateDir(EState eNewState)
{
	// Consider us idle.
	m_eUpdateDirState = eUpdateDirState_Idle;

	return true;
}

bool CScreenMulti::PostState_Waiting(EState eNewState)
{
	m_pFindCtrl->Enable(LTFALSE);

	return true;
}

void CScreenMulti::Update_State_Startup()
{
	// Update the directory list
	ChangeState(eState_UpdateDir);
}

void CScreenMulti::Update_State_UpdateDir()
{
	if( !g_pClientConnectionMgr->GetServerBrowser( ))
	{
		ChangeState(eState_Waiting);
		return;
	}

	// Check if we should make the request.
	switch( m_eUpdateDirState )
	{
	case eUpdateDirState_Idle:
	case eUpdateDirState_Finished:
		{
			// Done with the updatedir state.
			ChangeState( eState_Waiting );
			return;
		}
		break;
	case eUpdateDirState_Request:
		{
			UpdateDir_Request();
			return;
		}
		break;
	case eUpdateDirState_Processing:
		{
			switch( g_pClientConnectionMgr->GetServerBrowser( )->GetBrowserStatus( ))
			{
				// Still waiting for the results.
			case IGameSpyBrowser::eBrowserStatus_Processing:
				return;
			case IGameSpyBrowser::eBrowserStatus_Idle:
			case IGameSpyBrowser::eBrowserStatus_Complete:
			case IGameSpyBrowser::eBrowserStatus_Error:
				{
					// Check if we're searching LAN.
					if( m_eServerSearchSource == eServerSearchSource_LAN )
					{
						// Check if we need to give LAN searches a little more time.
						if( m_nLANRefreshCount < knNumberOfLANRefreshes )
						{
							// Go back to making a request.
							m_eUpdateDirState = eUpdateDirState_Request;
							return;
						}						
					}

					// Switch to the next state for when we are complete with this request.
					m_eUpdateDirState = eUpdateDirState_Finished;
					break;
				}
			default :
				{
					LTERROR("Unknown directory status encountered");
					ChangeState(eState_Waiting);
					break;
				}
			}

			return;
		}
		break;
	}
}

// Handles making the request when doing the updatedir state.
void CScreenMulti::UpdateDir_Request( )
{
	// Handle requests for the favorites servers one by one.
	if( m_eServerSearchSource == eServerSearchSource_Favorites )
	{
		// Get the summary information for each server in the favorite map.
		// We don't need to lock the critical section here.  Normally, access to the servermap is locked,
		// but the only way nodes of the favorite server map can be added or removed is from the main thread.
		// We only need to lock access to the serverentry as we pull data from it, since the data of the
		// serverentry can change.
		for( TServerEntryMap::iterator iter = m_cFavoriteServerMap.begin( ); iter != m_cFavoriteServerMap.end( ); iter++ )
		{
			ServerEntry& serverEntry = iter->second;
			
			char szServerIPandPort[kMaxIPPortLength+1];
			bool bDirectConnect;
			{
				// Lock the access to the servermap.
				CWinSync_CSAuto cs( m_csAddServer );
				LTStrCpy( szServerIPandPort, serverEntry.m_sPublicAddress.c_str(), LTARRAYSIZE( szServerIPandPort ));
				bDirectConnect = serverEntry.m_bDirectConnect;
			}

			RequestServerDetails( szServerIPandPort, bDirectConnect, false );
		}
	}
	// Handle requests for internet/lan servers.
	else
	{
		uint16 nPort = ( uint16 )LTCLAMP( LTStrToLong( m_sPort.c_str( )), 0, 65535 );
		bool bSearchInternet = ( m_eServerSearchSource == eServerSearchSource_Internet );

		// Request the serverlist.
		g_pClientConnectionMgr->GetServerBrowser( )->RequestServerList( bSearchInternet, nPort, NULL );

		// clear the LAN refresh count if we just did an Internet query, otherwise
		// increment the count 
		if (bSearchInternet)
		{
			m_nLANRefreshCount = 0;
		}
		else
		{
			m_nLANRefreshCount++;
		}
	}

	// Switch to processing request.
	m_eUpdateDirState = eUpdateDirState_Processing;
}

// ----------------------------------------------------------------------- //
// Function name   : ServerInfoCallback
// Description     : Callback function called by Gamespy for
//						each server that is found.
// Return type     : static void 
// Argument        : IGameSpyBrowser& gameSpyBrowser - browser supplying info.
// Argument        : IGameSpyBrowser::HGAMESERVER hGameServer - server handle.
// Argument        : void* pUserData - CScreenMulti* object.
// ----------------------------------------------------------------------- //
void CScreenMulti::ServerInfoCallback( IGameSpyBrowser& gameSpyBrowser, 
									  IGameSpyBrowser::HGAMESERVER hGameServer, void* pUserData, 
									  IGameSpyBrowser::EServerInfoCallbackReason eReason,
									  uint16 nPing )
{
	CScreenMulti* pScreenMulti = reinterpret_cast< CScreenMulti* >( pUserData );

	CWinSync_CSAuto cs( pScreenMulti->m_csAddServer );

	pScreenMulti->m_aServerCallbackResponses.push_back(ServerEntry());
	ServerEntry& serverEntry = pScreenMulti->m_aServerCallbackResponses.back();

	// Read the information on the server into the serverEntry.
	if( !ServerBrowserCtrl::ReadServerEntry( gameSpyBrowser, hGameServer, serverEntry ))
	{
		pScreenMulti->m_aServerCallbackResponses.pop_back();
		return;
	}

	// Record what we searching for.
	serverEntry.m_bLan = ( pScreenMulti->m_eServerSearchSource == eServerSearchSource_LAN );

	// Record the ping.  If the ping is bad, then try ICMP.  If this is a ping callback, then we're stuck
	// with the bad ping.
	serverEntry.m_nPing = nPing;
	if( nPing == ( uint16 )IGameSpyBrowser::kPingError && eReason != IGameSpyBrowser::eServerinfoCallbackReason_Ping )
	{
		g_pClientConnectionMgr->GetServerBrowser( )->RequestServerICMPPing( hGameServer );
	}
}

bool CScreenMulti::HandleKeyDown(int key, int rep)
{

	if( key == VK_F5 )
	{
		FindServers();
		return true;
	}


	// If the selected control is the server list, we need to trap if they change
	// the server selection to do the details request.
	if( GetSelectedControl() == m_pServerListCtrl )
	{
		bool bDoDetailsCmd = false;
		uint32 nParam1 = 0;
		{
			CWinSync_CSAuto cs( m_csAddServer );

			// Get the current selected control and compare it to after the key processing.
			CLTGUICtrl* pOldSelectedServer = m_pServerListCtrl->GetSelectedControl();
			if( !CBaseScreen::HandleKeyDown(key,rep))
				return false;
			CLTGUICtrl* pNewSelectedServer = m_pServerListCtrl->GetSelectedControl();

			// If the selection changed, then do a details command.
			if( pOldSelectedServer != pNewSelectedServer )
			{
				bDoDetailsCmd = true;
				nParam1 = ( uint32 )pNewSelectedServer->GetParam1( );
			}
		}

		// If the selection changed, then do a details command.
		if( bDoDetailsCmd )
		{
			SendCommand( CMD_DETAILS, nParam1, 0 );
		}

		return true;
	}
	else if( GetSelectedControl() == m_pFriendsListCtrl )
	{
		// Get the current selected control and compare it to after the key processing.
		CLTGUITextCtrl* pOldSelected = ( CLTGUITextCtrl* )m_pFriendsListCtrl->GetSelectedControl();
		if( !CBaseScreen::HandleKeyDown(key,rep))
			return false;
		CLTGUITextCtrl* pNewSelected = ( CLTGUITextCtrl* )m_pFriendsListCtrl->GetSelectedControl();

		// If the selection changed, then change our filter.
		if( pOldSelected != pNewSelected )
		{
			SetFriendFilter( MPW2A( pNewSelected->GetString( )).c_str( ));
			DisplayCurServerList( m_eLastSort, false );
		}

		return true;
	}
	else
	{
		return CBaseScreen::HandleKeyDown(key,rep);
	}
}

bool CScreenMulti::HandleKeyUp(int key) 
{
	CWinSync_CSAuto cs( m_csAddServer );
	return CBaseScreen::HandleKeyUp(key);
}


bool CScreenMulti::OnLButtonDblClick(int x, int y)
{
	ServerEntry serverEntry;
	bool bFoundServer = false;

	// Find the server entry clicked on with a lock on the addserver CS.
	// Then make a copy of the serverentry so we can release the CS before joining.
	// Joining will eventually call HaltRequests, which needs the CS.
	{
		CWinSync_CSAuto cs( m_csAddServer );

		// Check if we clicked somewhere on the list control.
		uint16 nIndex = 0;
		if( GetControlUnderPoint( x, y, &nIndex ))
		{
			if( GetControl( nIndex ) == m_pServerListCtrl )
			{
				uint32 nListIndex = 0;
				CLTGUICtrl* pSelectedCtrl = m_pServerListCtrl->GetControlUnderPoint( x, y, &nListIndex );
				if( pSelectedCtrl && nListIndex != CLTGUIListCtrl::kNoSelection )
				{
					ServerEntry* pServerEntry = ( ServerEntry* )pSelectedCtrl->GetParam1();
					if( pServerEntry )
					{
						bFoundServer = true;
						serverEntry = *pServerEntry;
					}
				}
			}
		}
	}

	if( bFoundServer )
	{
		JoinServer( serverEntry );
	}

	return CBaseScreen::OnLButtonDblClick( x, y );
}

bool CScreenMulti::JoinServer( ServerEntry& serverEntry )
{
	g_pClientConnectionMgr->GetServerBrowser( )->HaltRequest( );

	m_sSelectedServerAddress = serverEntry.m_sPublicAddress.c_str();
	SetCapture(NULL);

	if (!LTStrEquals(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion( )))
	{
		wchar_t wsMsg[256];
		wsMsg[0] = L'\0';
		MBCreate mb;
		FormatString("ScreenMulti_WrongVersion",wsMsg,LTARRAYSIZE(wsMsg),MPA2W(serverEntry.m_sVersion.c_str()).c_str());
		g_pInterfaceMgr->ShowMessageBox(wsMsg,&mb);
		return true;
	}

	// Don't allow joining servers that require pb if we don't have it on.
	bool bUsePunkBuster = false;
	IPunkBusterClient* pPunkBusterClient = g_pGameClientShell->GetPunkBusterClient();
	if( pPunkBusterClient )
	{
		bUsePunkBuster = pPunkBusterClient->IsEnabled();
	}
	if( serverEntry.m_bUsePunkbuster && !bUsePunkBuster )
	{
		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox("IDS_SERVER_PUNKBUSTERNOTENABLED",&mb);
		return true;
	}

	if (serverEntry.m_nNumPlayers >= serverEntry.m_nMaxPlayers)
	{
		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox("IDS_SERVERFULL",&mb);
		return true;
	}

	if( !LTStrIEquals( serverEntry.m_sModName.c_str(), g_pClientConnectionMgr->GetModName( )))
	{
		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox( "IDS_SERVER_WRONGMOD", &mb );
		return true;
	}

	g_pClientConnectionMgr->SetupClientBandwidth(serverEntry.m_bLan);

	m_sPassword = L"";
	if( serverEntry.m_bUsePassword)
	{	
		//show edit box here	
		MBCreate mb;
		mb.eType = LTMB_EDIT;
		mb.pFn = EditPassCallBack;
		mb.pString = L"";
		mb.nMaxChars = MAX_PASSWORD-1;
		mb.pUserData = this;
		g_pInterfaceMgr->ShowMessageBox( LoadString( "IDS_PASSWORD" ),&mb);
	}
	else
	{
		JoinCurGame( serverEntry );
	}

	return true;
} 

static char const* ParseCustomizers( char const* pszCustomizerInstance, CLTGUIListCtrlEx& customizersListCtrl,
							 uint32 nSelectedColor, uint32 nNonSelectedColor, uint32 nDisabledColor )
{
	char const* pszBegin = pszCustomizerInstance;
	char const* pszEquals = strchr( pszBegin, '=' );
	if( !pszEquals )
		return NULL;
	char const* pszSemiColon = strchr( pszEquals + 1, ';' );
	if( !pszSemiColon )
		return NULL;
	char const* pszNextCustomizer = pszSemiColon + 1;
	char szCompressedKey[64] = "";
	LTSubStrCpy( szCompressedKey, pszBegin, LTARRAYSIZE( szCompressedKey ), pszEquals - pszBegin );
	uint32 nCategoryIndex = -1;
	uint32 nRecordIndex = -1;
	uint32 nAttributeIndex = -1;
	sscanf( szCompressedKey, "%d.%d.%d", &nCategoryIndex, &nRecordIndex, &nAttributeIndex );
	if( nCategoryIndex == ( uint32 )-1 || nRecordIndex == ( uint32 )-1 || nAttributeIndex == ( uint32 )-1 )
	{
		return pszNextCustomizer;
	}

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(0,kCustomizersListFontSize);
	CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
	if( !pColumnCtrl )
	{
		LTERROR( "Couldn't create column control for customizer." );
		return pszNextCustomizer;
	}
	pColumnCtrl->Create(cs);
	pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	pColumnCtrl->SetColors(nSelectedColor,nNonSelectedColor,nDisabledColor);
	pColumnCtrl->SetFont( CFontInfo(kpszCustomizersListFont,kCustomizersListFontSize) );

	// Get the main db.  Gamemodes has it.
	HDATABASE hMainDatabase = DATABASE_CATEGORY( GameModes ).GetDatabase();
	if( !hMainDatabase )
		return pszNextCustomizer;

	// get the name of the category from the overrides database
	HCATEGORY hCategory = g_pLTDatabase->GetCategoryByIndex(hMainDatabase, nCategoryIndex);
	const char* pszCategoryName = g_pLTDatabase->GetCategoryName(hCategory);
	if( LTStrEmpty( pszCategoryName ))
		return pszNextCustomizer;

	// get the name of the record from the overrides database
	HRECORD hRecord = g_pLTDatabase->GetRecordByIndex(hCategory, nRecordIndex);
	const char* pszRecordName = g_pLTDatabase->GetRecordName(hRecord);
	if( LTStrEmpty( pszRecordName ))
		return pszNextCustomizer;

	// get the name of the attribute from the overrides database
	HATTRIBUTE hAttribute = g_pLTDatabase->GetAttributeByIndex(hRecord, nAttributeIndex);
	const char* pszAttributeName = g_pLTDatabase->GetAttributeName(hAttribute);
	if( LTStrEmpty( pszAttributeName ))
		return pszNextCustomizer;

	char szKey[1024] = "";
	LTSNPrintF( szKey, LTARRAYSIZE( szKey ), "%s/%s/%s", pszCategoryName, pszRecordName, pszAttributeName );

	// Get the value.
	char szValue[1024] = "";
	LTSubStrCpy( szValue, pszEquals + 1, LTARRAYSIZE( szValue ), pszSemiColon - ( pszEquals + 1 ));

	// Add the key and value.
	pColumnCtrl->AddTextColumn( MPA2W( szKey ).c_str(), kCustomizersListColWidth_Name, true );
	pColumnCtrl->AddTextColumn( MPA2W( szValue ).c_str(), kCustomizersListColWidth_Value, true );
	pColumnCtrl->Enable( true );
	customizersListCtrl.AddControl( pColumnCtrl );

	// Return to the next customizer.
	return pszNextCustomizer;
}

static void SetupCustomizersDetailInfo( ServerEntry const& serverEntry, CLTGUIListCtrlEx& customizersListCtrl,
							 uint32 nSelectedColor, uint32 nNonSelectedColor, uint32 nDisabledColor )
{
	// Clear the list of customizers first.
	customizersListCtrl.RemoveAll( true );

	char const* pszCustomizersData = serverEntry.m_sOverridesData.c_str();

	// Check if empty.
	if( LTStrEmpty( pszCustomizersData ))
		return;

	// Can only parse customizers if we're using the same version.
	bool bSameVersion = true;
	if( !LTStrIEquals(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()))
	{
		bSameVersion = false;
	}
	if( bSameVersion && !LTStrIEquals( serverEntry.m_sModName.c_str( ), g_pClientConnectionMgr->GetModName( )))
	{
		bSameVersion = false;
	}
	if( !bSameVersion )
		return;

	// Iterate through each of the customizers in the packed string.
	char const* pszCustomizerInstance = pszCustomizersData;
	while( pszCustomizerInstance )
	{
		pszCustomizerInstance = ParseCustomizers( pszCustomizerInstance, customizersListCtrl,
			nSelectedColor, nNonSelectedColor, nDisabledColor );
	}
}

static void SetupRulesDetailInfo( ServerEntry const& serverEntry, CLTGUIListCtrlEx& rulesListCtrl,
								 uint32 nSelectedColor, uint32 nNonSelectedColor, uint32 nDisabledColor )
{
	// Clear out the details list.
	rulesListCtrl.RemoveAll( true );

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(0,kRuleListFontSize);

	// First add the version row.
	CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
	if( !pColumnCtrl )
	{
		LTERROR( "Couldn't create column control for rule list." );
		return;
	}
	pColumnCtrl->Create(cs);
	pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	pColumnCtrl->SetColors(nSelectedColor, nNonSelectedColor, nDisabledColor);
	pColumnCtrl->SetFont( CFontInfo(kpszRuleListFont,kRuleListFontSize) );
	pColumnCtrl->AddTextColumn( LoadString( "SCREENMULTI_LIST_VERSION" ), kRuleListColWidth_Name, true );

	pColumnCtrl->AddTextColumn( MPA2W( serverEntry.m_sVersion.c_str()).c_str(), kRuleListColWidth_Value, true );
	pColumnCtrl->Enable( true );
	rulesListCtrl.AddControl( pColumnCtrl );

	// Add the mod row.
	pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
	if( !pColumnCtrl )
	{
		LTERROR( "Couldn't create column control for rule list." );
		return;
	}
	pColumnCtrl->Create(cs);
	pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	pColumnCtrl->SetColors(nSelectedColor,nNonSelectedColor,nDisabledColor);
	pColumnCtrl->SetFont( CFontInfo(kpszRuleListFont,kRuleListFontSize) );
	pColumnCtrl->AddTextColumn( LoadString( "SCREENMULTI_LIST_MOD" ), kRuleListColWidth_Name, true );

	if (LTStrIEquals( serverEntry.m_sModName.c_str(), RETAIL_MOD_NAME ))
	{
		pColumnCtrl->AddTextColumn( LoadString("Version_Retail"), kRuleListColWidth_Value, true );
	}
	else if (LTStrIEquals( serverEntry.m_sModName.c_str(), RETAILXP_MOD_NAME ))
	{
		pColumnCtrl->AddTextColumn( LoadString("Version_Retail_Exp"), kRuleListColWidth_Value, true );
	}
	else
	{
		char szMod[128];
		ModNameToModDisplayName( serverEntry.m_sModName.c_str(), szMod, LTARRAYSIZE( szMod ));
		pColumnCtrl->AddTextColumn( MPA2W( szMod ).c_str(), kRuleListColWidth_Value, true );
	}

	pColumnCtrl->Enable( true );
	rulesListCtrl.AddControl( pColumnCtrl );

	// Add the required download size value.
	pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
	if( !pColumnCtrl )
	{
		LTERROR( "Couldn't create column control for rule list." );
		return;
	}
	pColumnCtrl->Create(cs);
	pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	pColumnCtrl->SetColors(nSelectedColor,nNonSelectedColor,nDisabledColor);
	pColumnCtrl->SetFont( CFontInfo(kpszRuleListFont,kRuleListFontSize) );
	pColumnCtrl->AddTextColumn( LoadString( "SCREENMULTI_REQUIREDDOWNLOADSIZE" ), kRuleListColWidth_Name, true );
	wchar_t wszDownloadSize[32];
	// Write out bytes.
	if( serverEntry.m_nRequiredDownloadSize < 1024 )
	{
		LTSNPrintF( wszDownloadSize, LTARRAYSIZE( wszDownloadSize ), L"%d b", serverEntry.m_nRequiredDownloadSize );
	}
	// Write out k
	else if( serverEntry.m_nRequiredDownloadSize < 1024000 )
	{
		LTSNPrintF( wszDownloadSize, LTARRAYSIZE( wszDownloadSize ), L"%.1f kb", (( float )serverEntry.m_nRequiredDownloadSize / 1024.0f ));
	}
	// Write out Megs.
	else
	{
		LTSNPrintF( wszDownloadSize, LTARRAYSIZE( wszDownloadSize ), L"%.1f Mb", (( float )serverEntry.m_nRequiredDownloadSize / 1024000.0f ));
	}

	pColumnCtrl->AddTextColumn( wszDownloadSize, kRuleListColWidth_Value, true );
	pColumnCtrl->Enable( true );
	rulesListCtrl.AddControl( pColumnCtrl );

	bool bSameVersion = true;
	if( !LTStrIEquals(serverEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()))
	{
		bSameVersion = false;
	}
	if( bSameVersion && !LTStrIEquals( serverEntry.m_sModName.c_str( ), g_pClientConnectionMgr->GetModName( )))
	{
		bSameVersion = false;
	}

	if (bSameVersion)
	{
		// Switch to the mode so we can read the meta data about the rules.
		if( GameModeMgr::Instance().ResetToMode( serverEntry.m_hGameModeRecord ))
		{
			// Parse the options and put them on the list.
			wchar_t wszValue[256];
			ConParse parse;
			parse.Init( serverEntry.m_sOptions.c_str( ));
			while (g_pCommonLT->Parse(&parse) == LT_OK)
			{
				// Need both rule name and value.
				if( parse.m_nArgs < 2 )
					continue;

				// Get the rule index.
				uint32 nGameRuleIndex = LTStrToLong( parse.m_Args[0] );
				if( nGameRuleIndex < GameRule::GetGameRuleList().size())
				{
					GameRule* pGameRule = GameRule::GetGameRuleList()[nGameRuleIndex];

					// Skip rules not shown on the host options list.
					if( !pGameRule->IsCanModify( ) || !pGameRule->IsShowInOptions( ))
						continue;

					pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
					if( !pColumnCtrl )
					{
						LTERROR( "Couldn't create column control for rule list." );
						return;
					}
					pColumnCtrl->Create(cs);
					pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
					pColumnCtrl->SetColors(nSelectedColor,nNonSelectedColor,nDisabledColor);
					pColumnCtrl->SetFont( CFontInfo(kpszRuleListFont,kRuleListFontSize) );

					// Add the localized rule label and value.
					char const* pszLabelId = g_pLTDatabase->GetString( 
						CGameDatabaseReader::GetStructAttribute( pGameRule->GetStruct(), 0, "Label" ), 0, "" );
					pColumnCtrl->AddTextColumn( LoadString( pszLabelId ), kRuleListColWidth_Name, true );
					pGameRule->FromString( MPA2W( parse.m_Args[1] ).c_str(), false );
					pGameRule->ToString( wszValue, LTARRAYSIZE( wszValue ), true );
					pColumnCtrl->AddTextColumn( wszValue, kRuleListColWidth_Value, true );
					pColumnCtrl->Enable( true );
					rulesListCtrl.AddControl( pColumnCtrl );
				}
			}
		}
	}
}

static void SetupPlayersDetailInfo( ServerEntry const& serverEntry, CLTGUIListCtrlEx& playersListCtrl,
								 uint32 nSelectedColor, uint32 nNonSelectedColor, uint32 nDisabledColor )
{
	// First clear out the player list.
	playersListCtrl.RemoveAll( true );

	// Check if we have any players.
	if( !serverEntry.m_lstPlayerEntry.empty())
	{
		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(0,kPlayerListFontSize);

		// Iterate through the players.
		for( PlayerEntryList::const_iterator iter = serverEntry.m_lstPlayerEntry.begin( ); iter != serverEntry.m_lstPlayerEntry.end( ); iter++ )
		{
			// Create a column control to show the player info.
			CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
			pColumnCtrl->Create(cs);
			pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
			pColumnCtrl->SetColors(nSelectedColor,nNonSelectedColor,nDisabledColor);
			pColumnCtrl->SetFont( CFontInfo(kpszPlayerListFont,kPlayerListFontSize) );

			// Create columns for the name and score.
			PlayerEntry const& playerEntry = *iter;
			pColumnCtrl->AddTextColumn( MPA2W( playerEntry.m_sName.c_str( )).c_str(), kPlayerListColWidth_Name, true );
			wchar_t wszScore[16];
			LTSNPrintF( wszScore, LTARRAYSIZE( wszScore ), L"%i", playerEntry.m_nScore );
			pColumnCtrl->AddTextColumn( wszScore, kPlayerListColWidth_Score, true );

			pColumnCtrl->Enable( true );

			// Store the back pointer for the sorting routine.
			pColumnCtrl->SetParam1(( uint32 )&playerEntry );

			// Add the columncontrol to the player list.
			playersListCtrl.AddControl( pColumnCtrl );
		}

		// Sort the players by name.
		std::sort( playersListCtrl.GetControlArray().begin(), playersListCtrl.GetControlArray().end( ), PlayerListSortName( ));
		playersListCtrl.SetStartIndex( 0 );
	}
}

bool CScreenMulti::SetDetailInfo( ServerEntry const& serverEntry )
{
	if( !m_pPlayerListCtrl || !m_pRulesListCtrl || !m_pCustomizersListCtrl )
		return false;

	CWinSync_CSAuto csAuto( m_csAddServer );

	// Players
	SetupPlayersDetailInfo( serverEntry, *m_pPlayerListCtrl, m_SelectedColor, m_NonSelectedColor, m_DisabledColor );

	// Rules.
	SetupRulesDetailInfo( serverEntry, *m_pRulesListCtrl, m_SelectedColor, m_NonSelectedColor, m_DisabledColor );

	// Customizers
	SetupCustomizersDetailInfo( serverEntry, *m_pCustomizersListCtrl, m_SelectedColor,m_NonSelectedColor,m_DisabledColor );

	return true;
}

void CScreenMulti::PortValueChangingCB( std::wstring& wsValue, void* pUserData )
{
	CScreenMulti *pThisScreen = (CScreenMulti *)pUserData;

	// Make sure the input is in range.
	uint16 nPort = (uint16)LTCLAMP( LTStrToLong(wsValue.c_str( )), 0, 65535 );
	wchar_t wszClampedPort[10];
	LTSNPrintF( wszClampedPort, LTARRAYSIZE( wszClampedPort ), L"%d", nPort );
	wsValue = wszClampedPort;
};

void CScreenMulti::ReadCallbackResponses()
{
	CWinSync_CSAuto cs( m_csAddServer );

	// Check if there's nothing to do.
	if( m_aServerCallbackResponses.empty( ))
		return;

	// Record the currently selected control.
	CLTGUICtrl* pSelectedControl = m_pServerListCtrl->GetSelectedControl();
	uint32 nSelectedIndex = m_pServerListCtrl->GetSelectedIndex();
	uint32 nFirstShowIndex = m_pServerListCtrl->GetStartIndex( );
	uint32 nSelOffset = nSelectedIndex - nFirstShowIndex;

	bool bIsLan = false;
	bool bFavoritesUpdated = false;
	for (TServerEntryList::iterator iCurEntry = m_aServerCallbackResponses.begin(); iCurEntry != m_aServerCallbackResponses.end(); ++iCurEntry)
	{
		ServerEntry& serverEntry = *iCurEntry;

		// Consider this a LAN search if the pre-ready favorite said it came from LAN.
		if( m_eServerSearchSource == eServerSearchSource_Favorites )
		{
			// Get the lan information from the existing entry in the favorite server.
			TServerEntryMap::iterator iter = m_cFavoriteServerMap.find( serverEntry.m_sPublicAddress.c_str( ));
			ServerEntry& existingServerEntry = iter->second;
			bIsLan = existingServerEntry.m_bLan;
		}
		// Otherwise, if not on the favorites list, then use the value set when the
		// request came in.
		else
		{
			bIsLan = serverEntry.m_bLan;
		}

		// If LAN search, then update the LAN lists.
		if( bIsLan )
		{
			bool bWasUpdated;
			UpdateServerMapAndList( serverEntry, m_cLANServerMap, *m_pLANServerListCtrl, *m_pLANHeaderCtrl, true, true, true, bWasUpdated );
		}
		// Internet search, so update the internet lists.
		else
		{
			bool bWasUpdated;
			UpdateServerMapAndList( serverEntry, m_cInternetServerMap, *m_pInternetServerListCtrl, *m_pInternetHeaderCtrl, false, true, true, bWasUpdated );
		}

		// Update the favorite lists too. Don't apply filters since we always want to see favorite servers.
		bool bWasUpdated;
		UpdateServerMapAndList( serverEntry, m_cFavoriteServerMap, *m_pFavoriteServerListCtrl, *m_pFavoriteServerHeaderCtrl, bIsLan, false, false, bWasUpdated );
		// Track if we updated any favorites server info.
		if( bWasUpdated )
		{
			bFavoritesUpdated = true;
		}
	}

	m_pServerListCtrl->RecalcLayout();

		// Check if this was the server we had selected before.
		if( pSelectedControl )
		{
			// Try to preserve the same offset from the top of the list control.  
			uint32 nSelectedIndex = m_pServerListCtrl->GetIndex( pSelectedControl );
			m_pServerListCtrl->SetSelection( nSelectedIndex );
		if( nSelectedIndex - nSelOffset >= 0 )
			{
				m_pServerListCtrl->SetStartIndex( nSelectedIndex - nSelOffset );
			}
			else
			{
				m_pServerListCtrl->SetStartIndex( 0 );
			}
		}

	// If we updated any favorites information, then we need to update our profile.
	if( bFavoritesUpdated )
	{
		SaveFavoritesToProfile( );
	}

	// Clear the response list
	ltstd::reset_vector(m_aServerCallbackResponses);
}

void CScreenMulti::PopulateFriendsListControl( )
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return;

	// Clear the list out for the new initialization.
	m_pFriendsListCtrl->RemoveAll( true );

	int32 kFriendsListFontSize = g_pLayoutDB->GetListSize(m_hLayout,eListIndex_FriendsList);
	char const* pszFriendsListFont = g_pLayoutDB->GetListFont(m_hLayout,eListIndex_FriendsList);
	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_pFriendsListCtrl->GetBaseWidth(), kFriendsListFontSize );
	cs.nCommandID = CMD_FILTER_FRIEND;
	cs.szHelpID = "";
	cs.pCommandHandler = this;

	// Iterate through all the friends and add them to the list control.
	for( uint32 nFriendIndex = 0; nFriendIndex < pProfile->GetNumFriends(); nFriendIndex++ )
	{
		wchar_t const* pwszFriendNickName = pProfile->GetFriendNickName( nFriendIndex );
		CLTGUITextCtrl* pTextControl = CreateTextItem( pwszFriendNickName, cs, false, pszFriendsListFont, kFriendsListFontSize );
		pTextControl->Enable( true );
		m_pFriendsListCtrl->AddControl( pTextControl );
	}

	// Allow Add button if there's room in the list.
	m_pFriendsAddButton->Enable( pProfile->GetNumFriends() < CUserProfile::eFriendsInfo_MaxCount );
}

// Refresh the details info on all the servers we know about.
void CScreenMulti::RefreshDetailsOnServers( )
{
	char szPublicAddress[64];
	bool bDirectConnect = false;

	// Stop any requests we may already have out.  This is to prevent new servers from getting added to 
	// the map while we're using it.
	if( g_pClientConnectionMgr->GetServerBrowser( ))
		g_pClientConnectionMgr->GetServerBrowser( )->HaltRequest( );

	// Clear out the current details.
	{
		// Lock the list while we get some data out of it.
		CWinSync_CSAuto cs( m_csAddServer );

		m_pPlayerListCtrl->RemoveAll( true );
		m_pRulesListCtrl->RemoveAll( true );
		m_pCustomizersListCtrl->RemoveAll( true );
	}

	// This operation will loop through the existing map.  Servers won't be added or removed from the map
	// from another thread, since this is only done when we request a new server list, exiting the screen, etc.  
	// Since the request is only done in the main thread, same as the thread calling this function, we 
	// can be assured the map will not get modified.  The map elements data may, but the map nodes will
	// stay constant.
	for( TServerEntryMap::iterator iter = m_pcServerMap->begin(); iter != m_pcServerMap->end( ); iter++ )
	{
		// Copy the necessary data out of the server entry.
		{
			ServerEntry& serverEntry = iter->second;
			CWinSync_CSAuto cs( m_csAddServer );
			LTStrCpy( szPublicAddress, serverEntry.m_sPublicAddress.c_str(), LTARRAYSIZE( szPublicAddress ));
			bDirectConnect = serverEntry.m_bDirectConnect;
		}

		RequestServerDetails( szPublicAddress, bDirectConnect, true );
	}
	
	// Refresh the display.
	DisplayCurServerList( m_eLastSort, false );
}

// Add the server to the servermap and list control.  Specify if this is a lan search or not.
void CScreenMulti::UpdateServerMapAndList( ServerEntry const& serverEntry, TServerEntryMap& serverMap, 
											CLTGUIListCtrlEx& serverListControl, CLTGUIHeaderCtrl& headerControl,
											bool bIsLan, bool bApplyFilters, bool bAllowAdds, bool& bWasUpdated )
{
	// Lock access since we're going to mess around with the map and list control.
	CWinSync_CSAuto cs( m_csAddServer );

	// Assume we're not currently in list.
	bool bInLinst = false;

	// Assume we don't update a server in this list.
	bWasUpdated = false;

	// Check if this server isn't in our map yet.  
	TServerEntryMap::iterator iter = serverMap.find( serverEntry.m_sPublicAddress.c_str( ));

	// Check if the server is new to the list.
	if( iter == serverMap.end( ))
	{
		// Check if adds are allowed.
		if( !bAllowAdds )
			return;

		// Add the server to the map.  We use insert, because we need to get the iterator to the
		// newly added item.
		typedef std::pair< std::string, ServerEntry > TServerMapInsertValuePair;
		typedef std::pair< TServerEntryMap::iterator, bool > TServerMapInsertResultsPair;
		TServerMapInsertResultsPair mapInsert = serverMap.insert( 
			TServerMapInsertValuePair( serverEntry.m_sPublicAddress.c_str(), serverEntry ));
		iter = mapInsert.first;

		ServerEntry& newEntry = iter->second;

		// Give our ServerEntry a columnctrl.
		CreateServerEntryColumnControl( newEntry );

		// set the LAN flag on the entry - Note that we only do this on new entries so that a public 
		// response callback for an existing LAN server entry will not overwrite the status of this 
		// flag (so it will stay marked as LAN).
		newEntry.m_bLan = bIsLan;

	}
	// We have this server already.
	else
	{
		// We're in list, we'll need to relocate ourselves.
		bInLinst = true;

		ServerEntry& existingEntry = iter->second;

		// Preserve previous column control and lan info.
		CLTGUIColumnCtrlEx* pColumnCtrl = existingEntry.m_pColumnCtrl;
		bool bLan = existingEntry.m_bLan;

		// Copy in the new serverentry data.
		existingEntry = serverEntry;
		existingEntry.m_pColumnCtrl = pColumnCtrl;
		existingEntry.m_bLan = bLan;
	}

	// Get the serverentry that exists in the map, rather than the local copy, so we can edit the permanent one.
	ServerEntry& insertedServerEntry = iter->second;

	// Make sure we have all the columns.
	if( insertedServerEntry.m_pColumnCtrl->GetNumColumns( ) == 0 )
	{
		AddAllColumns( *insertedServerEntry.m_pColumnCtrl, headerControl );
	}

	// Set our backpointer.
	insertedServerEntry.m_pColumnCtrl->SetParam1(( uint32 )&insertedServerEntry );

	// Deselected the server for now.
	serverListControl.SetSelection( CLTGUIListCtrlEx::kNoSelection );

	// Set our control data.
	ServerBrowserCtrl::SetSummaryInfo( *insertedServerEntry.m_pColumnCtrl, insertedServerEntry );

	// If their version/mod is different than our version, gray it out.  If the
	// server doesn't have version/mod information, then don't gray it out, since
	// it just may be having problems reporting itself.
	if( !LTStrEmpty( insertedServerEntry.m_sVersion.c_str( )))
	{
		bool bSameVersion = true;
		if( !LTStrIEquals(insertedServerEntry.m_sVersion.c_str(), g_pVersionMgr->GetNetVersion()))
		{
			bSameVersion = false;
		}
		else if( !LTStrEmpty( insertedServerEntry.m_sModName.c_str()) && !LTStrIEquals( insertedServerEntry.m_sModName.c_str( ), g_pClientConnectionMgr->GetModName( )))
		{
			bSameVersion = false;
		}
		if( !bSameVersion )
		{
			uint32 a, r, g, b;
			GET_ARGB( m_DisabledColor, a, r, g, b );
			r = LTMIN( 255, r + 40 );
			g = LTMIN( 255, g + 40 );
			b = LTMIN( 255, b + 40 );
			insertedServerEntry.m_pColumnCtrl->SetColors(SET_ARGB( a, r, g, b ),m_DisabledColor,m_DisabledColor );
		}
	}

	// Get the place to insert this control.
	if( !bInLinst )
	{
		serverListControl.AddControl( insertedServerEntry.m_pColumnCtrl );
	}

	// See if we should apply filter.
	if( bApplyFilters )
	{
		// Update filter status.
		HRECORD hGameTypeFilter = (m_nGameTypeFilter && m_nGameTypeFilter <= m_lstGameTypes.size()) ? m_lstGameTypes[m_nGameTypeFilter-1] : NULL;
		FilterServer( insertedServerEntry, m_nVersionFilter, hGameTypeFilter, 
			( EFilterPlayers )m_nPlayersFilter, m_nPingFilter, 
			( EFilterTristate )m_nCustomizedFilter, ( EFilterTristate )m_nRequiresDownloadFilter,
			( EFilterTristate )m_nPunkbusterFilter,
			GetFriendFilter( ));
	}

	// Fill in the player list and details if we have the server detail info.
	if( insertedServerEntry.m_bHasDetails && m_sSelectedServerAddress == insertedServerEntry.m_sPublicAddress )
	{
		SetDetailInfo( insertedServerEntry );
	}

	// We updated the server info.
	bWasUpdated = true;
}

// Adds the serverentry to the favorites list and map.
bool CScreenMulti::AddServerEntryToFavorites( ServerEntry& serverEntry )
{
	// Get the current user profile that we're going to add to.
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return false;

	// Lock access since we're going to mess around with the map and list control.
	CWinSync_CSAuto cs( m_csAddServer );

	// Add the server to the favorites list in the profile.
	bool bWasAdded;
	pProfile->AddFavoriteServer( serverEntry.m_sPublicAddress.c_str(), MPA2W( serverEntry.m_sName.c_str( )).c_str(), 
		serverEntry.m_bLan, bWasAdded );

	// If the server wasn't added, then there is nothing else to do.
	if( !bWasAdded )
		return true;

	// Add the server to the favorites map and control.  Use the serverentry lan information, since that should already be available. 
	// Normally we check what kind of search we're doing, but adding a server to the favorites doesn't require a search.
	// Don't allow filters, since we aleays want to see favorites in our favorites list.
	bool bWasUpdated;
	UpdateServerMapAndList( serverEntry, m_cFavoriteServerMap, *m_pFavoriteServerListCtrl, *m_pFavoriteServerHeaderCtrl,
		serverEntry.m_bLan, false, true, bWasUpdated );

	return true;
}

// Adds the server IP to the favorites list and map.
bool CScreenMulti::AddServerIPToFavorites( char const* pszServerIPandPort )
{
	// Get the current user profile that we're going to add to.
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return false;

	char szServerIPandPort[kMaxIPPortLength+1];
	szServerIPandPort[0] = '\0';
	{
		// Lock access since we're going to mess around with the map and list control.
		CWinSync_CSAuto cs( m_csAddServer );

		// We may need to use this temporary serverentry if we can't find the new ip in the internet or lan lists.
		ServerEntry serverEntry;
		ServerEntry* pServerEntry = NULL;

		// See if this IP is already in the internet list.
		TServerEntryMap::iterator iter = m_cInternetServerMap.find( pszServerIPandPort );
		if( iter != m_cInternetServerMap.end( ))
		{
			// Use the internet serverentry.
			ServerEntry& internetServerEntry = iter->second;
			pServerEntry = &internetServerEntry;
		}

		// See if this IP is already in the LAN list.
		if( !pServerEntry )
		{
			iter = m_cLANServerMap.find( pszServerIPandPort );
			if( iter != m_cLANServerMap.end( ))
			{
				// Use the lan serverentry
				ServerEntry& lanServerEntry = iter->second;
				pServerEntry = &lanServerEntry;
			}
		}

		// Check if still no serverentry
		if( !pServerEntry )
		{
			// Use the temporary serverentry.
			pServerEntry = &serverEntry;

			// Fill in parts of it.
			serverEntry.m_sPublicAddress = pszServerIPandPort;
			serverEntry.m_sPrivateAddress = pszServerIPandPort;

			// Use the ip as the name for now until we can get more info on it.
			serverEntry.m_sName = pszServerIPandPort;

			// We don't know anything about this server, so make some assumptions.  
			// Assume it's an internet server.
			serverEntry.m_bLan = false;
			// Assume we will have to do NAT negotiations.
			serverEntry.m_bDirectConnect = false;
		}

		// Add the server to the favorites list in the profile.
		bool bWasAdded;
		pProfile->AddFavoriteServer( pServerEntry->m_sPublicAddress.c_str(), MPA2W( pServerEntry->m_sName.c_str()).c_str(),
			pServerEntry->m_bLan, bWasAdded );

		// If the server wasn't added, then there is nothing else to do.
		if( !bWasAdded )
			return true;

		// Add the server to the favorites map and control.  Use the serverentry lan information, since that should already be available. 
		// Normally we check what kind of search we're doing, but adding a server to the favorites doesn't require a search.
		// Don't allow filters, since we aleays want to see favorites in our favorites list.
		UpdateServerMapAndList( *pServerEntry, m_cFavoriteServerMap, *m_pFavoriteServerListCtrl, *m_pFavoriteServerHeaderCtrl,
			pServerEntry->m_bLan, false, true, bWasAdded );

		LTStrCpy( szServerIPandPort, pServerEntry->m_sPublicAddress.c_str(), LTARRAYSIZE( szServerIPandPort ));
	}

	// Get the summary info on the server.
	if( !LTStrEmpty( szServerIPandPort ))
		RequestServerDetails( szServerIPandPort, false, false );

	return true;
}

// Remove server from favorites.
bool CScreenMulti::RemoveServerIPFromFavorites( char const* pszServerIPandPort )
{
	// Check for empty string.
	if( LTStrEmpty( pszServerIPandPort ))
		return false;

	// Get the current user profile that we're going to add to.
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return false;

	// Lock access since we're going to mess around with the map and list control.
	CWinSync_CSAuto cs( m_csAddServer );

	// Remove it from the profile.
	uint32 nFavoriteServerIndex = pProfile->GetFavoriteServerIndex( pszServerIPandPort );
	if( nFavoriteServerIndex != ( uint32 )CUserProfile::eFavoriteServerInfo_Invalid )
	{
		pProfile->RemoveFavoriteServer( nFavoriteServerIndex );
	}

	// Find the serverentry in the map.
	TServerEntryMap::iterator iter = m_cFavoriteServerMap.find( pszServerIPandPort );
	if( iter == m_cFavoriteServerMap.end( ))
		return false;
	ServerEntry& serverEntry = iter->second;
	
	// Remove the control.
	if( serverEntry.m_pColumnCtrl )
	{
		m_pFavoriteServerListCtrl->RemoveControl( serverEntry.m_pColumnCtrl, true );
		serverEntry.m_pColumnCtrl = NULL;
		m_pFavoriteServerListCtrl->RecalcLayout( );
	}

	// Remove the serverentry from the map.
	m_cFavoriteServerMap.erase( iter );

	return true;
}

bool CScreenMulti::PopulateFavoriteServers( )
{
	// Get the current user profile that we're going to add to.
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return false;

	// Lock access since we're going to mess around with the map and list control.
	CWinSync_CSAuto cs( m_csAddServer );

	// Start fresh.
	TermServerList( m_cFavoriteServerMap, m_pFavoriteServerListCtrl );

	ServerEntry serverEntry;
	for( uint32 nFavoriteIndex = 0; nFavoriteIndex < pProfile->GetNumFavoriteServers(); nFavoriteIndex++ )
	{
		FavoriteServer* pFavoriteServer = pProfile->GetFavoriteServer( nFavoriteIndex );
		if( !pFavoriteServer )
			continue;

		serverEntry.m_sPublicAddress = pFavoriteServer->GetServerIPandPort();
		serverEntry.m_sPrivateAddress = pFavoriteServer->GetServerIPandPort();
		serverEntry.m_sName = MPW2A( pFavoriteServer->GetServerSessionName()).c_str();
		serverEntry.m_bLan = pFavoriteServer->GetLAN();
		// Assume we'll have to do NAT negotiations until we get some data from the server that
		// indicates otherwise.
		serverEntry.m_bDirectConnect = false;

		// Add the server to the favorites map and control.  Use the serverentry lan information, since that should already be available. 
		// Normally we check what kind of search we're doing, but adding a server to the favorites doesn't require a search.
		// Don't allow filters, since we aleays want to see favorites in our favorites list.
		bool bWasUpdated;
		UpdateServerMapAndList( serverEntry, m_cFavoriteServerMap, *m_pFavoriteServerListCtrl, *m_pFavoriteServerHeaderCtrl,
			serverEntry.m_bLan, false, true, bWasUpdated );
	}

	return true;
}


void CScreenMulti::SaveFavoritesToProfile( )
{
	// Get the current user profile that we're going to add to.
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return;

	// Lock access since we're going to mess around with the map and list control.
	CWinSync_CSAuto cs( m_csAddServer );

	// Iterate through each of the servers in the map and update the profile entry for that server.
	for( TServerEntryMap::iterator iter = m_cFavoriteServerMap.begin( ); iter != m_cFavoriteServerMap.end( ); iter++ )
	{
		ServerEntry& serverEntry = iter->second;

		// Get the existing profile entry for this server.
		uint32 nFavoriteServerIndex = pProfile->GetFavoriteServerIndex( serverEntry.m_sPublicAddress.c_str( ));
		FavoriteServer* pFavoriteServer = pProfile->GetFavoriteServer( nFavoriteServerIndex );
		if( !pFavoriteServer )
			continue;

		// Update the tracked information.
		pFavoriteServer->SetServerSessionName( MPA2W( serverEntry.m_sName.c_str( )).c_str( ));
		pFavoriteServer->SetLAN( serverEntry.m_bLan );
	}

	// Save the profile.
	pProfile->Save();
}

// Create a column control for a ServerEntry.
CLTGUIColumnCtrlEx* CScreenMulti::CreateServerEntryColumnControl( ServerEntry& serverEntry )
{
	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(0,kServerListFontSize);
	cs.nCommandID = CMD_DETAILS;
	cs.szHelpID = "";
	cs.pCommandHandler = this;
	serverEntry.m_pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
	serverEntry.m_pColumnCtrl->Create(cs);
	serverEntry.m_pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
	serverEntry.m_pColumnCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	serverEntry.m_pColumnCtrl->SetBasePos(GetDefaultPos());
	serverEntry.m_pColumnCtrl->SetFont( CFontInfo(kpszServerListFont,kServerListFontSize) );

	return serverEntry.m_pColumnCtrl;
}

// Handle the ui cmd CMD_SOURCE.
void CScreenMulti::HandleCmd_Source( )
{
	// Cancel any existing requests.
	if( g_pClientConnectionMgr->GetServerBrowser( ))
		g_pClientConnectionMgr->GetServerBrowser( )->HaltRequest( );

	CWinSync_CSAuto cs( m_csAddServer );

	m_pSourceCtrl->UpdateData( true );

	// Now that the requests have been halted, we can officially switch to the new source.
	m_eServerSearchSource = ( ServerSearchSource )m_nSourceIndex;

	// Throw away any previous responses.
	ltstd::reset_vector( m_aServerCallbackResponses );

	// Clear out the server selected.
	CLTGUICtrl* pSelectedCtrl = m_pServerListCtrl->GetSelectedControl( );
	m_pServerListCtrl->ClearSelection();
	m_pPlayerListCtrl->RemoveAll( true );
	m_pRulesListCtrl->RemoveAll( true );
	m_pCustomizersListCtrl->RemoveAll( true );

	// Switch active list.
	UpdateServerListControls();

	// If we had a selection before, reselect it.
	if( !m_sSelectedServerAddress.empty())
	{
		// Get the serverentry in the new map.
		TServerEntryMap::iterator iter = m_pcServerMap->find( m_sSelectedServerAddress.c_str());
		if( iter != m_pcServerMap->end( ))
		{
			ServerEntry& serverEntry = iter->second;
			if( serverEntry.m_pColumnCtrl )
			{
				// Set the selection.
				uint32 nSelectedIndex = m_pServerListCtrl->GetIndex( serverEntry.m_pColumnCtrl );
				m_pServerListCtrl->SetSelection( nSelectedIndex );

				// Update the detail info controls.
				SetDetailInfo( serverEntry );
			}
		}
	}

	// Update the total count to the size of the new list.
	m_nTotalServerCount = m_pcServerMap->size();

	// Clear the last browser status so we get a fresh update.
	m_eLastBrowserStatus = IGameSpyBrowser::eBrowserStatus_Error;
}


void CScreenMulti::CheckForMOTD()
{
	const char* pszURL = MPW2A(LoadString("ScreenMulti_MOTD_URL")).c_str();

	g_pClientConnectionMgr->GetServerBrowser( )->RequestURLData(pszURL,IniCallback,this);
	m_bCheckForMOTD = false;
	DebugCPrint(0,"%s : starting ini download",__FUNCTION__);
	SetMOTDDownloadUI(true);
	m_DownloadTimer.Start(60.0f);

}

void CScreenMulti::MOTDIniCallback(const char* pBuffer,const uint32 nBufferLen)
{
	if (!pBuffer || !nBufferLen)
	{
		DebugCPrint(0,"%s : error downloading ini",__FUNCTION__);
		m_bMOTDDownloadFailed = true;
		return;
	}

	//write the buffer into the temp file
	ILTOutStream* pOut = g_pLTClient->FileMgr()->OpenUserFileForWriting(s_pszTempName);
	if (pOut)
	{
		pOut->Write(pBuffer,nBufferLen);
		LTSafeRelease(pOut);
	}

	m_bMOTDIniBuffer = true;
	m_bMOTDDownloadFailed = false;

}

//returns true if an image should be downloaded, false otherwise
// this is structured so that we avoid the possibility of critical section deadlocks
bool CScreenMulti::UpdateMOTDIni()
{
	char szPath[MAX_PATH*2];
	SetMOTDDownloadUI(false);
	m_DownloadTimer.Stop();

	//if we have a temp file...
	if (tmpMOTD.Read(s_pszTempName))
	{
		//if the date or the image URL has changed, we have to download the image again...
		if (LTStrEmpty(tmpMOTD.m_wsImageURL.c_str()))
		{
			//delete the image file
			LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
			LTStrCat( szPath, s_pszImageName, LTARRAYSIZE( szPath ));
			if (LTFileOperations::FileExists(szPath))
			{
				LTFileOperations::DeleteFile(szPath);
			}
		}
		else if (!LTStrIEquals(m_MOTD.m_wsDate.c_str(),tmpMOTD.m_wsDate.c_str()) ||
				!LTStrIEquals(m_MOTD.m_wsImageURL.c_str(),tmpMOTD.m_wsImageURL.c_str()))
		{
			//we need to download the image, so return true
			return true;
		}

		if ( !LTStrIEquals(m_MOTD.m_wsText.c_str(),tmpMOTD.m_wsText.c_str()) ||
			!LTStrIEquals(m_MOTD.m_wsLinkURL.c_str(),tmpMOTD.m_wsLinkURL.c_str())
			)
		{
			m_bDisplayMOTD = true;
		}

		//save the new ini file
		m_MOTD = tmpMOTD;
		m_MOTD.Write(s_pszIniName);

		
		UpdateMOTD();

	}

	//delete the temp file
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, s_pszTempName, LTARRAYSIZE( szPath ));
	if (LTFileOperations::FileExists(szPath))
	{
		LTFileOperations::DeleteFile(szPath);
	}

	//no need to download an image
	return false;


}

void CScreenMulti::MOTDImageCallback(const char* pBuffer,const uint32 nBufferLen)
{
	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, s_pszImageName, LTARRAYSIZE( szPath ));

	//remove any old image file
	if (LTFileOperations::FileExists(szPath))
	{
		LTFileOperations::DeleteFile(szPath);
	}
	if (!pBuffer || !nBufferLen)
	{
		DebugCPrint(0,"%s : error downloading image",__FUNCTION__);
		m_bMOTDDownloadFailed = true;
		return;
	}


	//write the buffer into the temp file
	ILTOutStream* pOut = g_pLTClient->FileMgr()->OpenUserFileForWriting(s_pszImageName);
	if (pOut)
	{
		pOut->Write(pBuffer,nBufferLen);
		LTSafeRelease(pOut);
	}

	m_bMOTDImageBuffer = true;
	m_bMOTDDownloadFailed = false;
}



void CScreenMulti::UpdateMOTDImage()
{
	SetMOTDDownloadUI(false);
	m_DownloadTimer.Stop();
	
	m_bDisplayMOTD = true;

	//save the new ini file
	m_MOTD = tmpMOTD;
	m_MOTD.Write(s_pszIniName);

	UpdateMOTD();

	//delete the temp file
	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, s_pszTempName, LTARRAYSIZE( szPath ));
	if (LTFileOperations::FileExists(szPath))
	{
		LTFileOperations::DeleteFile(szPath);
	}

}

void CScreenMulti::SetMOTDDownloadUI(bool bDownload)
{
	if (bDownload)
	{
		if (m_pMOTD)
			m_pMOTD->Show(false);
		if (m_pMOTDDownload)
			m_pMOTDDownload->Show(true);
	}
	else
	{
		if (m_pMOTD)
			m_pMOTD->Show(true);
		if (m_pMOTDDownload)
			m_pMOTDDownload->Show(false);
	}
}

void CScreenMulti::UpdateMOTD()
{
	ILTInStream* pStream = g_pLTBase->FileMgr()->OpenUserFileForReading( s_pszImageName );
	if( pStream )
	{
		g_pILTTextureMgr->CreateDynamicTexture(m_hMOTDImage,pStream);
		LTSafeRelease( pStream );

		if	(m_hMOTDImage)
		{

			LTRect2f baseRect = m_pMOTDImage->GetRect();
			float fAspect = baseRect.GetWidth() / baseRect.GetHeight();

			uint32 nImgWidth, nImgHeight;
			g_pILTTextureMgr->GetTextureDims(m_hMOTDImage,nImgWidth,nImgHeight); 
			float fTexAspect = (float)nImgWidth / (float)nImgHeight;

			//if the image is wider than the target rect
			if (fTexAspect > fAspect)
			{
				float fNewHeight = baseRect.GetWidth()/fTexAspect;
				float fYOffset = (baseRect.GetHeight() - fNewHeight) /2.0f;
				baseRect.Top() += fYOffset;
				baseRect.Bottom() -= fYOffset;
			}
			else if (fTexAspect < fAspect)
			{
				//the image is taller than the target
				float fNewWidth = baseRect.GetHeight()*fTexAspect;
				float fXOffset = (baseRect.GetWidth() - fNewWidth) /2.0f;
				baseRect.Left() += fXOffset;
				baseRect.Right() -= fXOffset;
			}

			//setup the quad to be in the correct position
			DrawPrimSetRGBA(m_MOTDQuad, 0xFF, 0xFF, 0xFF, 0xFF);
			DrawPrimSetXYWH(m_MOTDQuad,baseRect.Left(),baseRect.Top(),baseRect.GetWidth(),baseRect.GetHeight());
			DrawPrimSetUVWH(m_MOTDQuad, 0.0f, 0.0f, 1.0f, 1.0f);

		}


	}

	m_pMOTDText->SetString(m_MOTD.m_wsText.c_str(),false);

	//only show the default image if there is no downloaded image
	m_pMOTDImage->Show(m_hMOTDImage == NULL);

	m_pMOTDLink->Enable(!LTStrEmpty(m_MOTD.m_wsLinkURL.c_str()));
}


bool MOTDData::Read(const char* pszFile)
{
	m_wsDate = L"";
	m_wsImageURL = L"";
	m_wsLinkURL = L"";
	m_wsText = L"";

	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, pszFile, LTARRAYSIZE( szPath ));

	if (!LTFileOperations::FileExists(szPath))
	{
		return false;
	}

	
	wchar_t wsBuffer[512];
	wsBuffer[0] = L'\0';
	LTProfileUtils::ReadString( L"MOTD", L"Date", L"INVALID", wsBuffer, LTARRAYSIZE(wsBuffer), MPA2W(szPath).c_str() );
	//file is incorrectly formatted, bail out
	if (LTStrIEquals(wsBuffer,L"INVALID"))
	{
		return false;
	}
	m_wsDate = wsBuffer;

	wsBuffer[0] = L'\0';
	LTProfileUtils::ReadString( L"MOTD", L"Image", L"", wsBuffer, LTARRAYSIZE(wsBuffer), MPA2W(szPath).c_str() );
	m_wsImageURL = wsBuffer;

	wsBuffer[0] = L'\0';
	LTProfileUtils::ReadString( L"MOTD", L"Link", L"", wsBuffer, LTARRAYSIZE(wsBuffer), MPA2W(szPath).c_str() );
	m_wsLinkURL = wsBuffer;

	wsBuffer[0] = L'\0';
	LTProfileUtils::ReadString( L"MOTD", L"Text", L"", wsBuffer, LTARRAYSIZE(wsBuffer), MPA2W(szPath).c_str() );
	m_wsText = wsBuffer;

	return true;
}

bool MOTDData::Write(const char* pszFile) const
{
	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, pszFile, LTARRAYSIZE( szPath ));

	if (LTFileOperations::FileExists(szPath))
	{
		LTFileOperations::DeleteFile(szPath);
	}

	ILTOutStream* pOutStream = g_pLTBase->FileMgr()->OpenUserFileForWriting( pszFile );
	if( !pOutStream )
		return false;
	// Write out the Unicode BOM to make the new blank file Unicode.
	uint8 aUnicodeBOM[2] = { 0xFF, 0xFE };
	pOutStream->Write( aUnicodeBOM, LTARRAYSIZE( aUnicodeBOM ));
	LTSafeRelease(pOutStream);

	LTProfileUtils::WriteString( L"MOTD", L"Date", m_wsDate.c_str(), MPA2W(szPath).c_str() );

	LTProfileUtils::WriteString( L"MOTD", L"Image", m_wsImageURL.c_str(), MPA2W(szPath).c_str() );

	LTProfileUtils::WriteString( L"MOTD", L"Link", m_wsLinkURL.c_str(), MPA2W(szPath).c_str() );

	LTProfileUtils::WriteString( L"MOTD", L"Text", m_wsText.c_str(), MPA2W(szPath).c_str() );

	return true;
}

void CScreenMulti::ReadMOTDCallbackResponses()
{
	bool bStartImageDownload = false;
	if (m_DownloadTimer.IsStarted() && m_DownloadTimer.IsTimedOut())
	{
		m_DownloadTimer.Stop();
		m_bMOTDDownloadFailed = true;
	}

	{	//scoped for auto critical section

		CWinSync_CSAuto cs( g_csMOTD );
		if (m_bMOTDIniBuffer)
		{
			bStartImageDownload = UpdateMOTDIni();
			m_bMOTDIniBuffer = false;
		}
		if (m_bMOTDImageBuffer)
		{
			UpdateMOTDImage();
			m_bMOTDImageBuffer = false;
		}
		if (m_bMOTDDownloadFailed)
		{
			SetMOTDDownloadUI(false);
			m_DownloadTimer.Stop();
			m_bMOTDDownloadFailed = false;
		}
	}

	//if we need to download the image, make the call outside of the critical section to avoid deadlocks
	if (bStartImageDownload)
	{
		g_pClientConnectionMgr->GetServerBrowser( )->RequestURLData( MPW2A(tmpMOTD.m_wsImageURL.c_str()).c_str(),ImageCallback,this);
		DebugCPrint(0,"%s : starting image download",__FUNCTION__);
		SetMOTDDownloadUI(true);
		m_DownloadTimer.Start(60.0f);

	}

}