// ----------------------------------------------------------------------- //
//
// MODULE  : TO2GameServerShell.cpp
//
// PURPOSE : Game's Server Shell - Definition
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __TO2_GAME_SERVER_SHELL_H__
#define __TO2_GAME_SERVER_SHELL_H__

#include "GameServerShell.h"
#include "TO2VersionMgr.h"
#include "TO2MissionButeMgr.h"

class CTO2GameServerShell : public CGameServerShell
{
	public :

		declare_interface(CTO2GameServerShell);

        CTO2GameServerShell();
		virtual ~CTO2GameServerShell();

		// Not used in TO2
		virtual bool DropInventoryObject(Body* pBody) { return false; }
	
		// Cap the number of bodies in a radius.
		virtual bool	IsCapNumberOfBodies( );
		
		// Are we able to use the radar functionality
		virtual bool	ShouldUseRadar( ) { return true; }

	protected:
	    virtual LTRESULT OnServerInitialized();
		virtual void OnServerTerm();
        virtual void Update(LTFLOAT timeElapsed);

	private:

		virtual CPlayerObj* CreatePlayer(HCLIENT hClient, ModelId ePlayerModelId );
		CTO2VersionMgr	m_VersionMgr; // Same as g_pVersionMgr
		CTO2MissionButeMgr	m_MissionButeMgr;	// Same as g_pMissionButeMgr

		uint32	m_nLastPublishTime;
};


#endif  // __TO2_GAME_SERVER_SHELL_H__