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

	virtual bool	Init();
	virtual void	Term();
	virtual void	OnExitWorld();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();		

	virtual void	UpdateLayout();

	// Handles a key press
    bool           HandleKeyDown(int key, int rep);
    bool           HandleChar(wchar_t c);


	void		Show(bool bShow, bool bTeam);
	bool		IsVisible() {return m_bVisible;}

private:
	void		Send();


private:

	CLTGUIEditCtrl	m_EditCtrl;

	bool			m_bVisible;

	uint32			m_nInputWidth;

	wchar_t			m_szChatStr[kMaxChatLength];
	wchar_t			m_szChatHistory[kMaxChatHistory][kMaxChatLength];

	int				m_nHistory;

	bool			m_bTeamMessage;

};

#endif