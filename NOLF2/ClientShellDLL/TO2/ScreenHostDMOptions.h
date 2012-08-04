// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostDMOptions.h
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//3
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_HOST_DM_OPTIONS_H_
#define _SCREEN_HOST_DM_OPTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "NetDefs.h"

class CScreenHostDMOptions : public CBaseScreen
{
public:
	CScreenHostDMOptions();
	virtual ~CScreenHostDMOptions();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);


protected:


protected:
//    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	int			m_nMaxPlayers;
	int			m_nRunSpeed;
	int			m_nScoreLimit;
	int			m_nTimeLimit;
	int			m_nRounds;

	int			m_nFragScore;
	int			m_nTagScore;


	CLTGUISlider*	m_pMaxPlayers;

};

#endif // _SCREEN_HOST_DM_OPTIONS_H_