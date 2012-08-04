// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformanceAdvanced.h
//
// PURPOSE : Declares the CScreenPerformanceAdvanced class.  This class
//           handles an advanced performance menu.  All advanced
//           performance menus inherit from this class.
//
// CREATED : 04/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#pragma once
#ifndef __SCREENPERFORMANCEADVANCED_H__
#define __SCREENPERFORMANCEADVANCED_H__

#include "BaseScreen.h"

class CScreenPerformanceAdvanced : public CBaseScreen
{
public:
	CScreenPerformanceAdvanced( const char* pszTitleStringID, uint32 nGlobalType );
	virtual ~CScreenPerformanceAdvanced();

	// Build the screen
	virtual bool	Build();

	// called to repair values
	virtual void	Repair(){}

	virtual void    OnFocus(bool bFocus);

	CLTGUIColumnCtrl* FindColumnControl( const wchar_t* wszName );

protected:
	CLTGUIListCtrl* m_pList[8]; // max of 8

	const char*		m_pszTitleStringID;
	uint32			m_nGlobalType;

	LTRect2n		m_ListRect;
	uint8			m_nListFontSize;
	int				m_nOptionWidth;
	int				m_nSettingWidth;
	uint32			m_nWarningColor;

	int				m_nType;

	virtual void	InitOptionList();
	virtual void	UpdateOptionList();
	virtual void	AdjustOptionFrame();
	virtual void	UpdateOption( CLTGUIColumnCtrl* pCtrl, int nGroup, int nIndex );
	virtual void	SetOption( uint32 nGroup, uint32 nOption, int32 nLevel, bool bUpdate = true );

	virtual void	SetCurrentType(int nType);

	virtual uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	//take the values from the game and set the controls
	virtual void	Load();

	//take the values from the controls and set them in game
	virtual void	Save();
};

#endif  // __SCREENPERFORMANCEADVANCED_H__
