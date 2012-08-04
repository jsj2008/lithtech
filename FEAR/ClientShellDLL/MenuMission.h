// ----------------------------------------------------------------------- //
//
// MODULE  : MenuMission.h
//
// PURPOSE : In-Game Menu to display mission status
//
// CREATED : 07/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MENUMISSION_H__
#define __MENUMISSION_H__

#include "BaseMenu.h"

class CMenuMission : public CBaseMenu
{
public:

	CMenuMission( );
	virtual ~CMenuMission() {}

	virtual bool Init( CMenuMgr& menuMgr );

	virtual void OnFocus(bool bFocus);

	// Handle a command
	virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);

	// Render the control
	virtual void Render ();

	virtual bool  OnUp ( ) {return false;}
	virtual bool  OnDown ( ) {return false;}

	virtual bool   HandleKeyDown(int key, int rep);
	virtual bool   OnLButtonDown(int x, int y);
	virtual bool   OnRButtonDown(int x, int y);

	virtual bool   HandleKeyUp(int key);
	virtual bool   OnLButtonUp(int x, int y);
	virtual bool   OnRButtonUp(int x, int y);




private:
	CLTGUITextCtrl*			m_pMissionCtrl;
	CLTGUIFrame*			m_pLine;
	CLTGUITextureButton*	m_pObjectiveCtrl;

	bool					m_bKeyDown;
	bool					m_bLBDown;
	bool					m_bRBDown;

	float					m_fWait;
	StopWatchTimer			m_KeyWaitTimer;


};


#endif  // __MENUMISSION_H__
