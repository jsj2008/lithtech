// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSubtitles.h
//
// PURPOSE : Definition of CHUDSubtitles to display subtitles
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SUBTITLES_H
#define __HUD_SUBTITLES_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDSubtitles : public CHUDItem
{
public:
	CHUDSubtitles();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	LTBOOL	Show(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius=0.0f, LTFLOAT fDuration=-1.0f, bool bSubtitlePriority = false);
	void	Clear();

	void	SetScale(float fScale);

	LTBOOL	IsVisible() {return m_bVisible;}

protected:

    LTIntPt					m_BasePos;
	CUIFormattedPolyString*	m_pText;

	CUIRECT		m_DisplayRect;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;
	float		m_fScale;
	float		m_fElapsedTime;			// The amount of time that has elapsed while this subtitle is displayed
	LTBOOL		m_bVisible;

	LTIntPt			m_CinematicPos;
	uint16			m_nCinematicWidth;
	LTIntPt			m_FullScreenPos;
	uint16			m_nFullScreenWidth;
	uint8			m_nMaxLines;

	LTVector		m_vSpeakerPos;
	bool			m_bSubtitlePriority;

	int				m_nCursorPos;
	LTBOOL			m_bOverflow;
	LTFLOAT			m_fRadius;
	LTFLOAT			m_fDuration;

	LTFLOAT			m_fScrollStartTime;
	LTFLOAT			m_fScrollSpeed;
	LTFLOAT			m_fOffset;
	LTFLOAT			m_fMaxOffset;

	LTFLOAT			m_fEndTime;


};



#endif