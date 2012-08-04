// ----------------------------------------------------------------------- //
//
// MODULE  : TronGameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 11/5/01
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONGAME_CLIENT_SHELL_H__
#define __TRONGAME_CLIENT_SHELL_H__

#include "GameClientShell.h"
#include "TronInterfaceMgr.h"
#include "TronVersionMgr.h"
#include "TronPlayerMgr.h"
#include "TronClientWeaponAllocator.h"
#include "TronMissionButeMgr.h"

class CTronGameClientShell : public CGameClientShell
{
	public:
		declare_interface(CTronGameClientShell);

		CTronGameClientShell();
		~CTronGameClientShell();

		virtual CInterfaceMgr * GetInterfaceMgr() { return &m_InterfaceMgr; }
		virtual CPlayerMgr*		GetPlayerMgr() { return &m_PlayerMgr; }
		virtual CClientWeaponAllocator const *GetClientWeaponAllocator() const
				{ return &m_ClientWeaponAllocator; }

		virtual uint32 OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid);
	    virtual LTRESULT ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort);
		virtual void OnMessage(ILTMessage_Read *pMsg);

		virtual void PauseGame(bool bPause, bool bPauseSound = false);

	private:
		CTronInterfaceMgr			m_InterfaceMgr;		// Interface manager
		CTronVersionMgr				m_VersionMgr;		// Same as g_pVersionMgr
		CTronPlayerMgr				m_PlayerMgr;		// Player manager
		CTronMissionButeMgr			m_MissionButeMgr;	// Same as g_pMissionButeMgr

		// allocator for client weapons
		CTronClientWeaponAllocator	m_ClientWeaponAllocator;
};

#endif  // __TRON_GAME_CLIENT_SHELL_H__
