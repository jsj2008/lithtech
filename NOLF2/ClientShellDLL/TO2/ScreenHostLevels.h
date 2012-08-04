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
    LTBOOL  Build();
	void	Escape();

    void    OnFocus(LTBOOL bFocus);


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL  FillAvailList();
	void	LoadMissionList();
	void	MakeDefaultMissionList();
	void	SaveMissionList();
	void	AddMissionToList(int nMissionId);
	void	UpdateButtons();

	CLTGUITextCtrl*			m_pAdd;
	CLTGUITextCtrl*			m_pRemove;
	CLTGUITextCtrl*			m_pAddAll;
	CLTGUITextCtrl*			m_pRemoveAll;
	CLTGUIListCtrl*			m_pAvailMissions;
	CLTGUIListCtrl*			m_pSelMissions;

	CLTGUIToggle*			m_pLoopToggle;


	std::string		m_sCampaignFile;
	LTBOOL			m_bLoopMissions;
};

#endif // _SCREEN_HOST_LEVELS_H_