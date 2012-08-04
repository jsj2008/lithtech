// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenDisplay.h
//
// PURPOSE : Interface screen for setting display options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_DISPLAY_H_
#define _SCREEN_DISPLAY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "dynarray.h"

typedef struct ScreenDisplayResolution_t
{
    uint32 m_dwWidth;       // Screen width
    uint32 m_dwHeight;      // Screen height
    uint32 m_dwBitDepth;    // Screen bitdepth
} ScreenDisplayResolution;

typedef struct ScreenDisplayRenderer_t
{
    bool	m_bHWTnL;

	char	m_DeviceName[200];		// The description of the renderer

	// An array of video resolutions
	CMoArray<ScreenDisplayResolution>	m_resolutionArray;
} ScreenDisplayRenderer;



class CScreenDisplay : public CBaseScreen
{
public:
	enum eFlag
	{
		eFlag_ScreenResolutionWarning	= 0x00000001,
		eFlagWarningMask				= (	eFlag_ScreenResolutionWarning )
	};

public:
	CScreenDisplay();
	virtual ~CScreenDisplay();

	// Build the screen
    bool   Build();

	// Handle input
    bool   OnLButtonUp(int x, int y);
    bool   OnRButtonUp(int x, int y);
    bool   OnLeft();
    bool   OnRight();
    bool   OnEnter();

	void	Escape();
    void    OnFocus(bool bFocus);

	void	ConfirmHardwareCursor(bool bReturn);

	// Build the array of resolutions
	static void		GetRendererData( ScreenDisplayRenderer& rendererData );

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Setup the resolution control based on the currently selected resolution
	void	SetupResolutionCtrl();

	// Sort the render resolution based on screen width and height
	static void		SortRenderModes( ScreenDisplayRenderer& rendererData );

	// Gets a RMode structure based on resolution index
	RMode	GetRendererModeStruct(int nResolutionIndex);

	// Returns the currently selected resolution
	ScreenDisplayResolution	GetCurrentSelectedResolution();

	// Set the resolution for the resolution control.  If it cannot be found the
	// next highest resolution is selected.
	void	SetCurrentCtrlResolution(ScreenDisplayResolution resolution);

	// updates the color of the resolution cycle
	void	UpdateResolutionColor();
	// checks to see if the user has blown their video memory limit
	void	CheckResolutionMemory();


private:
	uint32							m_dwFlags;
	bool							m_bEscape;
	bool							m_bHardwareCursor;
	bool							m_bVSync;
	bool							m_bRestartRenderBetweenMaps;
	bool							m_bUseTextScaling;
	int								m_nGamma;
	uint32							m_nWarningColor;

	CLTGUICycleCtrl					*m_pResolutionCtrl;		// The resolution control
	CLTGUITextCtrl					*m_pWarning;

	ScreenDisplayRenderer			m_rendererData;

	CLTGUISlider					*m_pGamma;
	CLTGUICycleCtrl					*m_pHardwareCursor;



};

#endif // _SCREEN_DISPLAY_H_