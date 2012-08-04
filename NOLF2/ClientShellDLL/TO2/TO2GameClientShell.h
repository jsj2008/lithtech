// ----------------------------------------------------------------------- //
//
// MODULE  : TO2GameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 11/5/01
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TO2GAME_CLIENT_SHELL_H__
#define __TO2GAME_CLIENT_SHELL_H__

#include "GameClientShell.h"
#include "TO2InterfaceMgr.h"
#include "TO2PlayerMgr.h"
#include "TO2VersionMgr.h"
#include "TO2ClientWeaponAllocator.h"
#include "TO2MissionButeMgr.h"

class CTO2GameClientShell : public CGameClientShell
{
	public:
		declare_interface(CTO2GameClientShell);

		CTO2GameClientShell();
		~CTO2GameClientShell();

		virtual CInterfaceMgr * GetInterfaceMgr() { return &m_InterfaceMgr;}
		virtual CPlayerMgr * GetPlayerMgr() { return &m_PlayerMgr;}
		virtual CClientWeaponAllocator const *GetClientWeaponAllocator() const
				{ return &m_ClientWeaponAllocator; }

		virtual uint32 OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid);
	    virtual LTRESULT ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort);
	    virtual void OnMessage(ILTMessage_Read *pMsg);
		
		// Are we able to use the radar functionality
		virtual bool	ShouldUseRadar( ) { return true; }

		virtual void OnEnterWorld();


	private:
		CTO2InterfaceMgr			m_InterfaceMgr;		// Interface manager
		CTO2VersionMgr				m_VersionMgr;		// Same as g_pVersionMgr
		CTO2PlayerMgr				m_PlayerMgr;		// Interface manager
		CTO2MissionButeMgr			m_MissionButeMgr;	// Same as g_pMissionButeMgr

		// allocator for client weapons
		CTO2ClientWeaponAllocator	m_ClientWeaponAllocator;
};

#endif  // __TRON_GAME_CLIENT_SHELL_H__
