// ----------------------------------------------------------------------- //
//
// MODULE  : TronPlayerObj.h
//
// PURPOSE : Player Object
//
// CREATED : 01/28/01
//
// ----------------------------------------------------------------------- //

#ifndef __TRON_GAME_SERVER_SHELL_H__
#define __TRON_GAME_SERVER_SHELL_H__

#include "GameServerShell.h"
#include "TronVersionMgr.h"
#include "TronMissionButeMgr.h"
#include "LightCycleMgr.h"

class Body;

class CTronGameServerShell : public CGameServerShell
{
	public :

		declare_interface(CTronGameServerShell);

        CTronGameServerShell();
		~CTronGameServerShell();

		virtual bool DropInventoryObject(Body* pBody);

	protected:
		virtual LTRESULT OnServerInitialized();
		virtual void OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg);

		virtual void Update(LTFLOAT timeElapsed);

		// Tron specific MID_ messages
		void HandlePlayerCompile(HCLIENT hSender, ILTMessage_Read *pMsg);

	private:

		virtual CPlayerObj* CreatePlayer(HCLIENT hClient);
		CTronVersionMgr		m_VersionMgr; // Same as g_pVersionMgr
		CTronMissionButeMgr	m_MissionButeMgr;	// Same as g_pMissionButeMgr
		CLightCycleMgr m_LightCycleMgr; // Same as g_pLightCycleMgr
};


#endif  // __TRON_GAME_SERVER_SHELL_H__