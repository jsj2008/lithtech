/****************************************************************************
;
;	 MODULE:		NetStart (.H)
;
;	PURPOSE:		Network game start/join/host dialog code
;
;	HISTORY:		06/28/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _NETSTART_H_
#define _NETSTART_H_


// Includes...

#include "netdefs.h"
#include "server_interface.h"


// Defines...

#define NML_LEVEL			128				// NML: net max len

#define NGT_SINGLE			0				// NGT: net game type
#define	NGT_DEATHMATCH		1
#define NGT_CAPTUREFLAG		2
#define NGT_COOPERATIVE		3

#define NST_GAMENAME		"NAME"			// NST: net string token
#define NST_GAMEHOST		"HOST"
#define NST_GAMETYPE		"TYPE"
#define NST_GAMELEVEL		"LEVL"
#define NST_GAMETIME		"TIME"
#define NST_PLRCOUNT		"PLRS"
#define NST_PLRNAME_BASE	"PLRN"
#define NST_PLRFRAG_BASE	"PLRF"
#define NST_CURLEVEL		"CLEV"
#define NST_NEXTLEVEL		"NLEV"
#define NST_GENERICMESSAGE	"GMSG"
#define NST_CONSOLEMSG		"CMSG"

#define NGE_FRAGS			1				// NGE: net game end
#define NGE_TIME			2

#define NGM_STANDARDUPDATE	0				// NGM: net generic message
#define NGM_LEVELCHANGE		1
#define NGM_CONSOLEMSG		2


// Structures...

typedef struct ServerInfo_t
{
	char	m_sName[128];
	char	m_sService[128];
	char	m_sAddress[128];
	DWORD	m_dwMaxPlayers;
	BOOL	m_bUpdateGameInfo;
	BOOL	m_bRegisterServer;
	BOOL	m_bUseGameSpy;

} ServerInfo;


// Prototypes...
void NetStart_LoadConsoleVars();
void NetStart_SaveConsoleVars();
BOOL NS_GetGameVar(char *pName, int &val);
DRESULT NetStart_SaveConfigFile(char *pFilename);

BOOL NetStart_DoWizard(HINSTANCE hInst, ServerInfo* pServerInfo, ServerOptions* pServerOptions, NetGame* pNetGame, BOOL bNoDlgs, char *pConfig);
BOOL NetStart_GoNoDialogs(ServerInfo* pServerInfo, NetGame* pNetGame);
BOOL NetStart_RunServerOptions(ServerInterface* pServerMgr, ServerOptions* pServerOptions);
BOOL NetStart_DoOptionsDialog(HWND hWnd, ServerInterface* pServerMgr, ServerOptions* pServerOptions);

int  NetStart_GetPort();


// EOF...

#endif



