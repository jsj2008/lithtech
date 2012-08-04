// FolderDisplay.h: interface for the CFolderDisplay class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_DISPLAY_H_
#define _FOLDER_DISPLAY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"


typedef struct FolderDisplayResolution_t
{
    uint32 m_dwWidth;       // Screen width
    uint32 m_dwHeight;      // Screen height
    uint32 m_dwBitDepth;    // Screen bitdepth
} FolderDisplayResolution;

typedef struct FolderDisplayRenderer_t
{
    LTBOOL   m_bHardware;

	char	m_renderDll[200];		// The DLL name for the renderer
	char	m_internalName[200];	// This is what the DLLs use to identify a card
	char	m_description[200];		// The description of the renderer

	// An array of video resolutions
	CMoArray<FolderDisplayResolution>	m_resolutionArray;
} FolderDisplayRenderer;


class CFolderDisplay : public CBaseFolder
{
public:
	CFolderDisplay();
	virtual ~CFolderDisplay();

	// Build the folder
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
	// Build the array of renderers
	void	BuildRendererArray();

	// Returns an index into m_rendererArray for this renderer.
	// -1 is returned if it cannot be found
	int		GetRendererIndex(RMode *pMode);

	// Setup the resolution control based on the currently selected resolution
	void	SetupResolutionCtrl();

	// Sort the render resolution based on screen width and height
	void	SortRenderModes(int nRendererIndex);

	// Sets the renderer based on renderer index and resolution index
    LTBOOL   SetRenderer(int nRendererIndex, int nResolutionIndex);

	// Gets a RMode structure based on a renderer index and a resolution index
	RMode	GetRendererModeStruct(int nRendererIndex, int nResolutionIndex);

	// Returns TRUE if two renderers are the same
    LTBOOL   IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2);

	// Returns the currently selected resolution
	FolderDisplayResolution	GetCurrentSelectedResolution();

	// Set the resolution for the resolution control.  If it cannot be found the
	// next highest resolution is selected.
	void	SetCurrentCtrlResolution(FolderDisplayResolution resolution);


private:
    LTBOOL                          m_bBitDepth32;
    LTBOOL                          m_bTexture32;
	LTBOOL							m_bEscape;
	LTBOOL							m_bHardwareCursor;

	CLTGUITextItemCtrl				*m_pRendererLabel;		// The renderer label control
	CLTGUITextItemCtrl				*m_pResolutionLabel;	// The resolution label control
	CCycleCtrl						*m_pRendererCtrl;		// The renderer control
	CCycleCtrl						*m_pResolutionCtrl;		// The resolution control
	CToggleCtrl						*m_pHardwareCursor;

	CMoArray<FolderDisplayRenderer>	m_rendererArray;		// The array of renderers


};

#endif // _FOLDER_DISPLAY_H_