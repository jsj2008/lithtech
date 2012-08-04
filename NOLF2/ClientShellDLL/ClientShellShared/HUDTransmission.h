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
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	virtual	void		Show(int nMessageId);
	virtual	void		Show(const char *pszString);
	
	virtual void		Hide() { m_Msg.Show( LTFALSE ); }

protected:
	CHUDMessage		m_Msg;

    LTIntPt		m_BasePos;
	MsgCreate	m_MsgFormat;


};



#endif