// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDecision.h
//
// PURPOSE : Definition of CHUDDecision to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DECISION_H
#define __HUD_DECISION_H

#include "HUDItem.h"
#include "ClientServerShared.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDDecision : public CHUDItem
{
public:
	CHUDDecision();
	

    virtual bool		Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();
	virtual void        ScaleChanged();

	virtual void        UpdateLayout();

	void	OnObjectRemove(HLOCALOBJ hObj);

	//hide or show based on messages from the server
	void	Show(ILTMessage_Read *pMsg);

	//close window and tell server
	void	Hide();

	bool	IsVisible() {return m_bVisible;}

	void	Choose(uint8 nChoice);

	HOBJECT GetObject() { return m_hObject; }
	float	GetRadius() { return m_fRadius; }

protected:

	bool		m_bVisible;
	uint16		m_nWidth;

	LTObjRef	m_hObject;
	LTVector	m_vObjPos;
	float		m_fRadius;

	CLTGUIWindow	    m_Dlg;
	CLTGUIColumnCtrl*	m_pText[MAX_DECISION_CHOICES];

};



#endif