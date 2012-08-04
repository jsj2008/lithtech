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

#include "cpp_client_de.h"
#include "..\Shared\NetDefs.h"


// Defines...

#define NET_JOIN			0
#define NET_HOST			1
#define NET_UNKNOWN			2


// Structures...

typedef struct NetStart_t
{
	DBOOL			m_bHost;
	DBOOL			m_bHaveTcpIp;
	NetSession*		m_pNetSession;
	NetHost			m_NetHost;
	char			m_sLevel[256];
	char			m_sPlayer[128];
	char			m_sAddress[128];

}	NetStart;


// Externs...

class CIpMgr;


// Prototypes...

DBOOL NetStart_DoWizard(CClientDE* pClientDE);
DBOOL NetStart_DoWizard(CClientDE* pClient, NetStart* pNetStart, int nJoinHost);
DBOOL NetStart_DoLobbyLaunchWizard(CClientDE* pClientDE);
DBOOL NetStart_DoConsoleConnect(CClientDE* pClientDE, char* sAddress);
DBOOL NetStart_MinimizeMainWnd(CClientDE* pClient);
DBOOL NetStart_RestoreMainWnd();
DBOOL NetStart_RunServerOptions(CClientDE* pClientDE, ServerOptions* pServerOptions);
DBOOL NetStart_RunServerOptions(CClientDE* pClientDE);
DBOOL NetStart_DoSettingsDialog();

NetSession* NetStart_GetSessionList(CClientDE* pClientDE, char* pInfo);
void        NetStart_FreeSessionList(CClientDE* pClientDE);

NetPlayer*		NetStart_GetPlayerStruct();
NetGame*		NetStart_GetGameStruct();
NetClientData*	NetStart_GetClientDataStruct();
void			NetStart_ClearGameStruct();

NetSession* NetStart_GetSessionQueryResults(CClientDE* pClientDE);
void        NetStart_UpdateSessionQuery(CClientDE* pClientDE);
void        NetStart_EndSessionQuery(CClientDE* pClientDE);
DBOOL       NetStart_StartSessionQuery(CClientDE* pClientDE, char* sInfo);
DBOOL       NetStart_StartSessionQuery(CClientDE* pClientDE, CIpMgr* pIpMgr);


// EOF...

#endif



