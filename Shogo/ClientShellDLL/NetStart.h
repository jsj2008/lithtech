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

#include "clientheaders.h"
#include "..\Shared\NetDefs.h"


// Structures...

typedef struct NetStart_t
{
	LTBOOL			m_bHost;
	LTBOOL			m_bHaveTcpIp;
	NetSession*		m_pNetSession;
	NetHost			m_NetHost;
	char			m_sLevel[256];
	char			m_sPlayer[128];
	char			m_sAddress[128];

}	NetStart;


// Externs...

class CIpMgr;


// Prototypes...

LTBOOL NetStart_DoWizard(ILTClient* pClientDE);
LTBOOL NetStart_DoWizard(ILTClient* pClient, NetStart* pNetStart);
LTBOOL NetStart_DoLobbyLaunchWizard(ILTClient* pClientDE);
LTBOOL NetStart_DoConsoleConnect(ILTClient* pClientDE, char* sAddress);
LTBOOL NetStart_MinimizeMainWnd(ILTClient* pClient);
LTBOOL NetStart_RestoreMainWnd();
LTBOOL NetStart_RunServerOptions(ILTClient* pClientDE, ServerOptions* pServerOptions);
LTBOOL NetStart_RunServerOptions(ILTClient* pClientDE);
LTBOOL NetStart_DoSettingsDialog();

NetSession* NetStart_GetSessionList(ILTClient* pClientDE, char* pInfo);
void        NetStart_FreeSessionList(ILTClient* pClientDE);

NetPlayer*	NetStart_GetPlayerStruct();
NetGame*	NetStart_GetGameStruct();
void		NetStart_ClearGameStruct();

NetSession* NetStart_GetSessionQueryResults(ILTClient* pClientDE);
void        NetStart_UpdateSessionQuery(ILTClient* pClientDE);
void        NetStart_EndSessionQuery(ILTClient* pClientDE);
LTBOOL       NetStart_StartSessionQuery(ILTClient* pClientDE, char* sInfo);
LTBOOL       NetStart_StartSessionQuery(ILTClient* pClientDE, CIpMgr* pIpMgr);


// EOF...

#endif



