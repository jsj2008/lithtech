// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_PLAYER_H_
#define _SCREEN_PLAYER_H_

#include "BaseScreen.h"

class CScreenPlayer : public CBaseScreen
{
public:
	CScreenPlayer();
	virtual ~CScreenPlayer();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	CLTGUIColumnCtrl*	m_pName;

	std::string m_sPlayerName;


};

#endif // _SCREEN_PLAYER_H_