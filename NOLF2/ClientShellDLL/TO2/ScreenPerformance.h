// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformance.h
//
// PURPOSE : Interface screen for setting performance options
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_PERFORMANCE_H_
#define _SCREEN_PERFORMANCE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "PerformanceMgr.h"


// #define _SHOW_PERFORMACE_FRAMERATE_

class CScreenPerformance : public CBaseScreen
{
public:
	CScreenPerformance();
	virtual ~CScreenPerformance();

	// Build the screen
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

	LTBOOL	OnLeft();
	LTBOOL	OnRight();
	LTBOOL	OnLButtonUp(int x, int y);
	LTBOOL	OnRButtonUp(int x, int y);

	virtual void Escape();


	// Calls UpdateData on each control in the screen
    virtual void            UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL   Render(HSURFACE hDestSurf);
	
	CLTGUICycleCtrl*	m_pPerformance;
	CLTGUITextCtrl*		m_pResolution;

	CLTGUIListCtrl*		m_pDisplay;
	CLTGUITextCtrl*		m_pDisplayLabel;
	CLTGUIFrame*		m_pDisplayFrame;

	
	CLTGUIListCtrl*		m_pSFX;
	CLTGUITextCtrl*		m_pSFXLabel;
	CLTGUIFrame*		m_pSFXFrame;

	CLTGUITextCtrl*		m_pPerformanceTest;
	
	CLTGUICycleCtrl*	m_pDetailLevel; 
	CLTGUICycleCtrl*	m_pPrecache; 

	CLTGUIWindow*		m_pDlg;

#ifdef _SHOW_PERFORMACE_FRAMERATE_
	CLTGUITextCtrl*		m_pMinFPS;
	CLTGUITextCtrl*		m_pMaxFPS;
	CLTGUITextCtrl*		m_pAveFPS;
	CLTGUITextCtrl*		m_pBelowMinFPS;
	CLTGUITextCtrl*		m_pMintoMaxFPS;
	CLTGUITextCtrl*		m_pAboveMaxFPS;
#else
	CLTGUITextCtrl*		m_pRecommendation;
#endif // _SHOW_PERFORMACE_FRAMERATE_

	sPerformCfg	m_sCfg;
	uint8	m_nOverall;
	uint8	m_nTripleBuffer;


};

#endif // _SCREEN_PERFORMANCE_H_