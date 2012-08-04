// ----------------------------------------------------------------------- //
//
// MODULE  : TO2InterfaceMgr.h
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 11/5/01
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TO2INTERFACE_MGR_H__
#define __TO2INTERFACE_MGR_H__

#include "InterfaceMgr.h"
#include "TO2HUDMgr.h"
#include "TO2ScreenMgr.h"
#include "TO2PlayerStats.h"
#include "TO2LayoutMgr.h"

class CTO2InterfaceMgr : public CInterfaceMgr
{
public:
	CTO2InterfaceMgr();
	~CTO2InterfaceMgr();

	// Accessors
	CHUDMgr*		GetHUDMgr()			{ return &m_HUD;}
	CScreenMgr*		GetScreenMgr()		{ return &m_ScreenMgr;}
	CPlayerStats*	GetPlayerStats()	{ return &m_stats; }

    LTBOOL			OnCommandOn(int command);
	LTBOOL			OnKeyDown(int key, int rep);

	void			OnExitWorld();

private:
	CTO2HUDMgr			m_HUD;					// Heads-Up Display
	CTO2ScreenMgr		m_ScreenMgr;
	CTO2PlayerStats		m_stats;				// Player statistics (health, ammo, armor, etc.)
	CTO2LayoutMgr		m_TO2LayoutMgr;			// same as g_pLayoutMgr
};


#endif // __TO2INTERFACE_MGR_H__