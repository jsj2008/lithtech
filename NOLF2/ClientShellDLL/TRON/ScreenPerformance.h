// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformance.h
//
// PURPOSE : Interface screen for setting performance options
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_PERFORMANCE_H_
#define _SCREEN_PERFORMANCE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "PerformanceMgr.h"


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

	
	CLTGUICycleCtrl*	m_pPerformance;
	CLTGUIListCtrl*		m_pDisplay;
	CLTGUIListCtrl*		m_pSFX;
	CLTGUIListCtrl*		m_pTexture;
	CLTGUITextCtrl*		m_pDisplayLabel;
	CLTGUITextCtrl*		m_pSFXLabel;
	CLTGUITextCtrl*		m_pTextureLabel;
	CLTGUIFrame*		m_pDisplayFrame;
	CLTGUIFrame*		m_pSFXFrame;
	CLTGUIFrame*		m_pTextureFrame;

	CLTGUICycleCtrl*	m_pDetailLevel; 
	CLTGUISlider*		m_pTexGroup[kNumDetailGroups];
	int					m_nSliderVal[kNumDetailGroups];

	sPerformCfg	m_sCfg;
	uint8	m_nOverall;


};

#endif // _SCREEN_PERFORMANCE_H_