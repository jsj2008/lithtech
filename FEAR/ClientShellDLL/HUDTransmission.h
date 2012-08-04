// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTransmission.h
//
// PURPOSE : Definition of CHUDTransmission to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_TRANSMISSION_H
#define __HUD_TRANSMISSION_H

#include "HUDItem.h"
#include "HUDMessage.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDTransmission : public CHUDItem
{
public:
	CHUDTransmission();
	
	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();

	virtual	void		Show(const char* szMessageID);
	virtual	void		Show(const wchar_t *pszString);
	
	virtual void		Hide() { m_Msg.Show( false ); }

protected:
	CHUDMessage		m_Msg;

	MsgCreate	m_MsgFormat;


};



#endif