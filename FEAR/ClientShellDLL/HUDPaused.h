// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPaused.h
//
// PURPOSE : Definition of CHUDPaused to display a paused message
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PAUSED_H
#define __HUD_PAUSED_H

#include "HUDItem.h"
#include "HUDMessage.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDPaused : public CHUDItem
{
public:
	CHUDPaused();
	

    virtual bool		Init();
	virtual void		Term();

    virtual void        Render();
	virtual	void        Update();
	virtual	void        ScaleChanged();

	virtual void        UpdateLayout();

	virtual	void		Show(bool bShow);

protected:
	CHUDMessage		m_Msg;

	MsgCreate		m_MsgFormat;


};



#endif