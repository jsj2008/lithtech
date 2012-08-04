// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostLevels.h
//
// PURPOSE : Interface screen for choosing levels for a hosted game
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_HOST_LEVELS_H_
#define _SCREEN_HOST_LEVELS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"


class CScreenHostLevels : public CBaseScreen
{
public:
	CScreenHostLevels();
	virtual ~CScreenHostLevels();

	// Build the screen
    bool  Build();

    void    OnFocus(bool bFocus);

	void	AddMissionToList(int nMissionId, bool bVerifySize, bool bForce);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    bool  FillAvailList();
	void	LoadMissionList();
	void	MakeDefaultMissionList();
	void	SaveMissionList();
	
	void	UpdateButtons();

	CLTGUITextCtrl*			m_pAdd;
	CLTGUITextCtrl*			m_pRemove;
	CLTGUITextCtrl*			m_pAddAll;
	CLTGUITextCtrl*			m_pRemoveAll;
	CLTGUIListCtrlEx*		m_pAvailMissions;
	CLTGUIScrollBar*		m_pAvailMissionsScrollBar;
	CLTGUIListCtrlEx*		m_pSelMissions;
	CLTGUIScrollBar*		m_pSelMissionsScrollBar;

	StringSet m_setRequiredMapFeatures;
};

#endif // _SCREEN_HOST_LEVELS_H_