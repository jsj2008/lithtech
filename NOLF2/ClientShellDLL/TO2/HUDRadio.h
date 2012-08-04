// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRadio.h
//
// PURPOSE : Definition of CHUDRadio to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_RADIO_H
#define __HUD_RADIO_H

#include "HUDItem.h"
#include "ClientServerShared.h"

#define MAX_RADIO_CHOICES 6
//******************************************************************************************
//** HUD Radio dialog
//******************************************************************************************
class CHUDRadio : public CHUDItem
{
public:
	CHUDRadio();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	//hide or show
	void	Show(bool bShow);

	void	SetScale(float fScale);

	bool	IsVisible() {return m_bVisible;}

	void	Choose(uint8 nChoice);

protected:
	uint8		m_nNumChoices;

    LTIntPt		m_BasePos;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;
	float		m_fScale;
	bool		m_bVisible;
	uint16		m_nWidth;
	LTIntPt		m_Offset;

	CLTGUIWindow	    m_Dlg;
	CLTGUIColumnCtrl*	m_pText[MAX_RADIO_CHOICES];

};



#endif