// ----------------------------------------------------------------------- //
//
// MODULE  : TronInterfaceMgr.h
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 11/5/01
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRONINTERFACE_MGR_H__
#define __TRONINTERFACE_MGR_H__

#include "InterfaceMgr.h"
#include "TRONHUDMgr.h"
#include "TRONScreenMgr.h"
#include "TRONPlayerStats.h"
#include "SubroutineMgr.h"
#include "TronLayoutMgr.h"

class CTronInterfaceMgr : public CInterfaceMgr
{
public:
	CTronInterfaceMgr();
	~CTronInterfaceMgr();

    LTBOOL  Init();
	void	Term();

	// Accessors
	CHUDMgr*		GetHUDMgr()			{return &m_HUD;}
	CScreenMgr*		GetScreenMgr()			{return &m_ScreenMgr;}
	CPlayerStats*	GetPlayerStats()	{ return &m_stats; }

    LTBOOL			OnCommandOn(int command);
	void			OnExitWorld();

	virtual void    OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);

	virtual void	UpdatePlayingState();
	virtual void	UpdatePlayerStats(uint8 nThing, uint8 nType1, uint8 nType2, LTFLOAT fAmount);

	bool			IsDisplayingProgress() {return m_bDisplayProgress;}

protected:
	// Message function
	virtual LTBOOL OnMessage(uint8 messageID, ILTMessage_Read *pMsg);

private:
	CTronHUDMgr			m_HUD;					// Heads-Up Display
	CTronScreenMgr		m_ScreenMgr;
	CTronPlayerStats	m_stats;				// Player statistics (health, ammo, armor, etc.)
	CSubroutineMgr		m_SubroutineMgr;
	CTronLayoutMgr		m_TronLayoutMgr;		// same as g_pLayoutMgr
	bool				m_bDisplayProgress;		// flag for displaying objectives and procedurals
};
#endif // __TRONINTERFACE_MGR_H__