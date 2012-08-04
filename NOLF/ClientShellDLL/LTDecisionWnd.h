/****************************************************************************
;
;	 MODULE:		CLTDecisionWnd (.h)
;
;	PURPOSE:		Class for a decision window
;
;	HISTORY:		04/16/99 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTDECISIONWND_H_
#define _LTDECISIONWND_H_

// Includes
#include "LTMaskedWnd.h"
#include "Timer.h"
#include "LTTextWnd.h"

#define MAX_DECISIONS			5
#define DECISIONWND_MAX_WIDTH	540


class DECISIONWNDCREATESTRUCT : public LTWNDCREATESTRUCT
{
public:
    DECISIONWNDCREATESTRUCT();

    LTFLOAT          fAlpha;
	BOOL			bFrame;
	CLTSurfaceArray *pcollFrames;
	CLTGUIFont		*pFont;
	CRect*			prcFrame;
	CRect*			prcTotal;
};

inline DECISIONWNDCREATESTRUCT::DECISIONWNDCREATESTRUCT()
{
	memset(this, 0, sizeof(DECISIONWNDCREATESTRUCT));
	szWndName = "DecisionWnd";
}

class CLTGUIFont;

// Classes
class CLTDecisionWnd : public CLTMaskedWnd
{
public:

	CLTDecisionWnd();
	virtual ~CLTDecisionWnd() { Term(); }
	virtual void Term();
	virtual BOOL Init(DECISIONWNDCREATESTRUCT *pcs);



	virtual BOOL DrawToSurface(HSURFACE hSurfDest);

	virtual BOOL Update(float fTimeDelta);
	virtual BOOL ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);
	virtual BOOL OnLButtonDown(int xPos, int yPos);
	virtual BOOL OnRButtonDown(int xPos, int yPos);
	virtual BOOL OnKeyDown(int nKey, int nRep);
	virtual BOOL SendMessage(CLTWnd* pSender, int nMsg, int nParam1 = 0,int nParam2 = 0);
	BOOL	DisplayText(CStringArray *pcollDecisions, CLTWnd* pDialogueWnd, BOOL bBelow = TRUE);
	uint8	GetActivatedSelection() { return m_byActivatedSelection; }
	void	Reset() { m_bClosing = FALSE; m_bOpening = FALSE; }
	//BOOL	InitFrame(CLTSurfaceArray* pcollSurfs);
	void	SetCurSelection(int nSelection);

protected:
    void    SetAlpha(LTFLOAT fAlpha);

	CLTGUIFont		*m_pFont;
	BOOL			m_bOpening;
	BOOL			m_bClosing;
	CTimer			m_tmrClose;
	float			m_fStartPos;
	CRect			*m_prcFrame;
	CRect			*m_prcTotal;
	CLTTextWnd		m_DecisionWnds[MAX_DECISIONS];
	uint8			m_byActivatedSelection;
	CLTSurfaceArray *m_pcollFrames;
	int				m_nNumDecisions;
	int				m_nCurSelection;
	BOOL			m_bFrame;
};

// Inlines
inline CLTDecisionWnd::CLTDecisionWnd()
{
	m_prcFrame = NULL;
	m_prcTotal = NULL;
	m_pFont = NULL;
	m_nNumDecisions = 0;
	m_pcollFrames = NULL;
	m_bFrame = FALSE;
}

#endif