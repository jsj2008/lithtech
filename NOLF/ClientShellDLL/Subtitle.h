// Subtitle.h: interface for the CSubtitle class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SUBTITLE_INCLUDED_
#define _SUBTITLE_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSubtitle
{
public:
	CSubtitle();
	virtual ~CSubtitle();

	void	Init();


	void	Show(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius=0.0f, LTFLOAT fDuration=-1.0f);
	void	Clear();

	void	Draw();
	void	Update();

	void	ScreenDimsChanged();

private:
	void	ClearSurfaces();

	LTIntPt			m_CinematicPos;
	uint32			m_nCinematicWidth;
	LTIntPt			m_FullScreenPos;
	uint32			m_nFullScreenWidth;
	uint32			m_nMaxLines;
	uint32			m_nLineHeight;

	LTIntPt			m_pos;
	uint32			m_dwWidth;
	uint32			m_dwHeight;
	LTVector		m_vSpeakerPos;

	int				m_nCursorPos;
	HSURFACE		m_hForeSurf;
	CLTGUIFont		*m_pForeFont;
	LTBOOL			m_bVisible;
	LTBOOL			m_bOverflow;
	LTFLOAT			m_fRadius;
	LTFLOAT			m_fDuration;

	HLTCOLOR		m_hSubtitleColor;
	LTIntPt			m_txtSize;


	LTRect			m_rcBaseRect;
	LTRect			m_rcSrcRect;
	LTFLOAT			m_fScrollStartTime;
	LTFLOAT			m_fScrollSpeed;
	LTFLOAT			m_fOffset;
	LTFLOAT			m_fMaxOffset;

};

#endif // !defined(AFX_Subtitle_H__FFB1A7C1_ACB4_11D3_B2DB_006097097C7B__INCLUDED_)