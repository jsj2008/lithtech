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


typedef struct ScreenDisplayResolution_t
{
    uint32 m_dwWidth;       // Screen width
    uint32 m_dwHeight;      // Screen height
    uint32 m_dwBitDepth;    // Screen bitdepth
} ScreenDisplayResolution;

typedef struct ScreenDisplayRenderer_t
{
    LTBOOL   m_bHardware;

	char	m_renderDll[200];		// The DLL name for the renderer
	char	m_internalName[200];	// This is what the DLLs use to identify a card
	char	m_description[200];		// The description of the renderer

	// An array of video resolutions
	CMoArray<ScreenDisplayResolution>	m_resolutionArray;
} ScreenDisplayRenderer;



class CScreenDisplay : public CBaseScreen
{
public:
	CScreenDisplay();
	virtual ~CScreenDisplay();

	// Build the screen
    LTBOOL   Build();

	// Handle input
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnEnter();

	void	Escape();
    void    OnFocus(LTBOOL bFocus);

	void	ConfirmHardwareCursor(LTBOOL bReturn);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Build the array of resolutions
	void	GetRendererData();

	// Setup the resolution control based on the currently selected resolution
	void	SetupResolutionCtrl();

	// Sort the render resolution based on screen width and height
	void	SortRenderModes();

	// Gets a RMode structure based on resolution index
	RMode	GetRendererModeStruct(int nResolutionIndex);

	// Returns the currently selected resolution
	ScreenDisplayResolution	GetCurrentSelectedResolution();

	// Set the resolution for the resolution control.  If it cannot be found the
	// next highest resolution is selected.
	void	SetCurrentCtrlResolution(ScreenDisplayResolution resolution);


private:
    LTBOOL                          m_bBitDepth32;
    LTBOOL                          m_bTexture32;
	LTBOOL							m_bEscape;
	LTBOOL							m_bHardwareCursor;
	int								m_nGamma;

	CLTGUICycleCtrl					*m_pResolutionCtrl;		// The resolution control

	ScreenDisplayRenderer			m_rendererData;

	CLTGUISlider					*m_pGamma;
	CLTGUICycleCtrl					*m_pHardwareCursor;



};

#endif // _SCREEN_DISPLAY_H_