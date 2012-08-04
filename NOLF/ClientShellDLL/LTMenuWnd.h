/****************************************************************************
;
;	 MODULE:		CLTMenuWnd (.h)
;
;	PURPOSE:		Class for a menu window
;
;	HISTORY:		03/20/00 [jrg] This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTMENUWND_H_
#define _LTMENUWND_H_

// Includes
#include "LTMaskedWnd.h"
#include "Timer.h"
#include "LTTextWnd.h"
#include "BaseMenu.h"

#define MAX_ITEMS			20
#define MENUWND_MAX_WIDTH	540


class MENUWNDCREATESTRUCT : public LTWNDCREATESTRUCT
{
public:
    MENUWNDCREATESTRUCT();

    LTFLOAT          fAlpha;
	BOOL			bFrame;
	const char		*szFrame;
	CLTGUIFont		*pFont;
};

inline MENUWNDCREATESTRUCT::MENUWNDCREATESTRUCT()
{
	memset(this, 0, sizeof(MENUWNDCREATESTRUCT));
	szWndName = "MenuWnd";
}

class CLTGUIFont;

// Classes
class CLTMenuWnd : public CLTMaskedWnd
{
public:

	CLTMenuWnd();
	virtual ~CLTMenuWnd() { Term(); }
	virtual void Term();
	virtual BOOL Init(MENUWNDCREATESTRUCT *pcs);



	virtual BOOL DrawToSurface(HSURFACE hSurfDest);

	virtual BOOL Update(float fTimeDelta);
	virtual BOOL ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);
	virtual BOOL OnLButtonDown(int xPos, int yPos);
	virtual BOOL OnRButtonDown(int xPos, int yPos);
	virtual BOOL OnKeyDown(int nKey, int nRep);
	virtual BOOL SendMessage(CLTWnd* pSender, int nMsg, int nParam1 = 0,int nParam2 = 0);

	BYTE	GetActivatedSelection() { return m_byActivatedSelection; }
	void	Reset() { m_bClosing = FALSE; m_bOpening = FALSE; }
	BOOL	InitFrame(const char *szFrame);

	void	SetMenu( CBaseMenu *pMenu) {m_pMenu = pMenu;}

    void    AddItem(uint32 nTextID, uint8 byCommand);
    void    AddItem(HSTRING hText, uint8 byCommand);
    void    AddItem(const char *szText, uint8 byCommand);

	void	ClearItems();
	BOOL	Open();
	void	Close();

	void	SetCurSelection(int nSelection);

protected:
    void    SetAlpha(LTFLOAT fAlpha);

	CLTGUIFont		*m_pFont;
	BOOL			m_bOpening;
	BOOL			m_bClosing;
	CTimer			m_tmrClose;
	float			m_fStartPos;
	CRect			m_rcFrame;
	CRect			m_rcTotal;
	CLTTextWnd		m_ItemWnds[MAX_ITEMS];
    uint8           m_ItemCmds[MAX_ITEMS];
	BYTE			m_byActivatedSelection;
	CLTSurfaceArray m_collFrames;
	int				m_nNumItems;
	BOOL			m_bFrame;
	CBaseMenu		*m_pMenu;

    LTIntPt         m_curSz;
	int				m_nCurSelection;

};

// Inlines
inline CLTMenuWnd::CLTMenuWnd()
{
	m_rcFrame.SetRect(0,0,0,0);
	m_rcTotal.SetRect(0,0,0,0);
	m_pFont = NULL;
	m_nNumItems = 0;
	m_bFrame = FALSE;
    m_pMenu = LTNULL;
	m_byActivatedSelection = 0;
}

#endif