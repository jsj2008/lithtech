// MissionText.h: interface for the CMissionText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MISSIONTEXT_H__FFB1A7C1_ACB4_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_MISSIONTEXT_H__FFB1A7C1_ACB4_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LithFontMgr.h"

class CMissionText
{
public:
	CMissionText();
	virtual ~CMissionText();

	void	Init();

//	Note: The caller of Start(hString) is responsible for freeing hString
	void	Start(HSTRING hString);

	void	Start(int nStringId);
	void	Start(char *pszString);

	void	Clear();
	void	Draw();
	void	Update();

	void	Pause(LTBOOL bPause);

private:
	void	ClearSurface();

	int					m_nLineHeight;
	int					m_nCursorPos;
	HSURFACE			m_hForeSurf;
    uint32              m_dwWidth;
    uint32              m_dwHeight;

	HSTRING				m_hText;

    LTFLOAT              m_fDelay;
    LTFLOAT              m_fFadeTime;
    LTFLOAT              m_fTimeRemaining;
    LTFLOAT              m_fAlpha;

    HLTSOUND            m_hSound;
	LITHFONTDRAWDATA	m_lfDrawData;
	LITHFONTSAVEDATA	m_lfSaveData;

    LTIntPt              m_pos;

    LTBOOL               m_bScrolling;
    LTBOOL               m_bPause;

	char				m_TypeSound[128];
	char				m_ScrollSound[128];

};

#endif // !defined(AFX_MISSIONTEXT_H__FFB1A7C1_ACB4_11D3_B2DB_006097097C7B__INCLUDED_)