// PopupText.h: interface for the CPopupText class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _POPUPTEXT_INCLUDED_
#define _POPUPTEXT_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPopupText
{
public:
	CPopupText();
	virtual ~CPopupText();

	void	Init();
	void	Term();


	void	Show(HMESSAGEREAD hMessage);
	void	Update();
	void	Clear();

	LTBOOL OnKeyDown(int key, int rep);

	void	Draw();

	LTBOOL  IsVisible() { return m_bVisible;}

private:

	void	ClearSurfaces();
	void	ShowText(int nStringId);
	void	CreateScaleFX(char *szFXName);

	LTRect			m_rcRect;
	LTIntPt			m_pos;
	uint32			m_dwWidth;
	uint32			m_dwHeight;

	int				m_nLineHeight;
	int				m_nKey;

	HSURFACE		m_hForeSurf;
	CLTGUIFont		*m_pForeFont;
	LTBOOL			m_bVisible;

	CMoArray<CSpecialFX *>	m_SFXArray;
};

#endif // _POPUPTEXT_INCLUDED_