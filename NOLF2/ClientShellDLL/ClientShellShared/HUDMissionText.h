// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMissionText.h
//
// PURPOSE : Definition of CHUDMissionText to display transmission messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_MISSIONTEXT_H
#define __HUD_MISSIONTEXT_H

#include "HUDItem.h"
#include "TimedText.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDMissionText : public CHUDItem
{
public:
	CHUDMissionText();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	void	Start(int nStringId);
	void	Start(char *pszString);

	void	Clear();

	void	Pause(LTBOOL bPause);

	void	SetScale(float fScale);

	LTBOOL	IsVisible() {return m_bVisible;}

protected:
	CTimedText				m_Text;

    LTIntPt					m_BasePos;
	CUIFormattedPolyString*	m_pText;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;
	float		m_fScale;
	LTBOOL		m_bVisible;
	uint16		m_nWidth;

	TIMED_TEXT_INIT_STRUCT	m_Format;

	LTBOOL		m_bPause;


};



#endif