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
	

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
    virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();
	virtual void	OnExitWorld() {Clear(); }

	bool	Show(const char* szStringId, LTVector vSpeakerPos, float fRadius=0.0f, float fDuration=-1.0f, bool bSubtitlePriority = false);
	void	Clear();



	bool	IsVisible() {return m_bVisible;}

protected:

	LTRect2n	m_Rect;

	float		m_fElapsedTime;			// The amount of time that has elapsed while this subtitle is displayed
	bool		m_bVisible;

//	LTVector2n		m_CinematicPos;
//	uint16			m_nCinematicWidth;
//	LTVector2n		m_FullScreenPos;
//	uint16			m_nFullScreenWidth;
	uint32			m_nWidth;
	uint8			m_nMaxLines;

	LTVector		m_vSpeakerPos;
	bool			m_bSubtitlePriority;

	int				m_nCursorPos;
	bool			m_bOverflow;
	float			m_fRadius;
	float			m_fDuration;

	float			m_fScrollStartTime;
	float			m_fScrollSpeed;
	float			m_fOffset;
	float			m_fMaxOffset;

	float			m_fEndTime;


};



#endif