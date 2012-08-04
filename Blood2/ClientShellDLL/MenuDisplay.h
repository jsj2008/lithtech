// MenuDisplay.h: interface for the CMenuDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUDISPLAY_H__BB7AF961_655B_11D2_BDAC_0060971BDC6D__INCLUDED_)
#define AFX_MENUDISPLAY_H__BB7AF961_655B_11D2_BDAC_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

typedef struct MenuDisplayResolution_t
{
	DDWORD m_dwWidth;		// Screen width
	DDWORD m_dwHeight;		// Screen height
	DDWORD m_dwBitDepth;	// Screen bitdepth
} MenuDisplayResolution;

typedef struct MenuDisplayRenderer_t
{
	DBOOL	m_bHardware;

	char	m_renderDll[200];		// The DLL name for the renderer
	char	m_internalName[200];	// This is what the DLLs use to identify a card
	char	m_description[200];		// The description of the renderer
	
	// An array of video resolutions
	CMoArray<MenuDisplayResolution>	m_resolutionArray;
} MenuDisplayRenderer;

class CMenuDisplay : public CMenuBase  
{
public:
	CMenuDisplay();
	virtual ~CMenuDisplay();

	// Build the menu
	void	Build();	

	// This is called when the menu gets or loses focus
	void	OnFocus(DBOOL bFocus);

protected:
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

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
	DBOOL	SetRenderer(int nRendererIndex, int nResolutionIndex);

	// Gets a RMode structure based on a renderer index and a resolution index
	RMode	GetRendererModeStruct(int nRendererIndex, int nResolutionIndex);

	// Returns TRUE if two renderers are the same
	DBOOL	IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2);

	// Returns the currently selected resolution
	MenuDisplayResolution	GetCurrentSelectedResolution();

	// Set the resolution for the resolution control.  If it cannot be found the
	// next highest resolution is selected.
	void	SetCurrentCtrlResolution(MenuDisplayResolution resolution);

	// Saves the global detail level
	void	SaveDetailSetting();

	// Override the left and right controls
	void	OnLeft();
	void	OnRight();

protected:
	CLTGUITextItemCtrl				*m_pRendererCtrl;		// The renderer control
	CLTGUITextItemCtrl				*m_pResolutionCtrl;		// The resolution control
	int								m_nDetailLevel;			// The detail level

	CMoArray<MenuDisplayRenderer>	m_rendererArray;		// The array of renderers
};

#endif // !defined(AFX_MENUDISPLAY_H__BB7AF961_655B_11D2_BDAC_0060971BDC6D__INCLUDED_)
