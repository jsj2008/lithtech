// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostOptions.h
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//3
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_HOST_OPTIONS_H_
#define _SCREEN_HOST_OPTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "NetDefs.h"

class CScreenHostOptions : public CBaseScreen
{
public:
	CScreenHostOptions();
	virtual ~CScreenHostOptions();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);


protected:


protected:
//    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	LTBOOL		m_bUseSkills;
	int			m_nMaxPlayers;
	LTBOOL		m_bFriendlyFire;
	uint8		m_nDifficulty;
	int			m_nPlayerDiff;

	CLTGUISlider*	m_pMaxPlayers;
	CLTGUIToggle*	m_pSkillToggle;

};

#endif // _SCREEN_HOST_OPTIONS_H_