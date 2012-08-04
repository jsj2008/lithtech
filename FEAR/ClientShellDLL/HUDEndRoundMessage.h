// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEndRoundMessage.h
//
// PURPOSE : Definition of CHUDEndRoundMessage used to display messages
//			 at the end of multiplayer rounds
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ENDROUNDMESSAGE_H
#define __HUD_ENDROUNDMESSAGE_H

#include "HUDItem.h"
#include "HUDMessage.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDEndRoundMessage : public CHUDItem
{
public:
	CHUDEndRoundMessage();
	
	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();

	virtual	void	Show(const char* szMessageID);
	virtual	void	Show(const wchar_t *pszString);
	
	virtual void	Hide() { m_Msg.Show( false ); }
	const wchar_t*	GetText() { return m_Msg.GetText(); }

protected:
	CHUDMessage		m_Msg;

	MsgCreate		m_MsgFormat;
};

#endif // __HUD_ENDROUNDMESSAGE_H