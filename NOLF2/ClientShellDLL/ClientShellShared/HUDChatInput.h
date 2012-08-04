// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChatInput.h
//
// PURPOSE : HUDItem to display chat input
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_CHAT_H
#define __HUD_CHAT_H

#include "HUDItem.h"
#include "LTGUIMgr.h"

const int kMaxChatLength = 64;
const int kMaxChatHistory = 25;

//******************************************************************************************
//** HUD Chat display
//******************************************************************************************
class CHUDChatInput : public CHUDItem
{
public:
	CHUDChatInput();

    LTBOOL      Init();
	void		Term();

	void		OnExitWorld();

    void        Render();
    void        Update();

    void        UpdateLayout();

	// Handles a key press
    LTBOOL           HandleKeyDown(int key, int rep);
    LTBOOL           HandleChar(unsigned char c);


	void		Show(bool bShow, bool bTeam);
	LTBOOL		IsVisible() {return m_bVisible;}

private:
	void		Send();


private:

	CLTGUIEditCtrl	m_EditCtrl;

	LTBOOL			m_bVisible;

	LTIntPt			m_BasePos;
	uint8			m_nFontSize;

	CUIPolyString*	m_pStr;


	char			m_szChatStr[kMaxChatLength];
	char			m_szChatHistory[kMaxChatHistory][kMaxChatLength];

	int				m_nHistory;

	bool			m_bTeamMessage;

};

#endif