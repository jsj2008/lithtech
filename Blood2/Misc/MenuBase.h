// MenuBase.h: interface for the CMenuBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUBASE_H__D7668B31_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUBASE_H__D7668B31_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUIMgr.h"
#include "stdlith.h"

class CLTGUIFadeItemCtrl;
class CLTGUITextItemCtrl;
class CMenuBase : public CLTGUICommandHandler
{
public:
	CMenuBase();
	virtual ~CMenuBase();

	// Initialization/Termination
	// NOTE:  You must also initialize the static members before using this class.  See the member
	//        variable access methods below.
	virtual	DBOOL	Init(CClientDE *pClientDE, CMenuBase *pParentMenu, DWORD dwMenuID, int nMenuHeight);
	virtual void	Term();

	// Builds the menu
	virtual DBOOL	Build()								{ m_bBuilt=DTRUE; return TRUE; }
	DBOOL			IsBuilt()							{ return m_bBuilt; }

	// Get the parent menu
	CMenuBase		*GetParentMenu()					{ return m_pParentMenu; }

	// Get the menu ID
	DDWORD			GetMenuID()							{ return m_dwMenuID; }

	// Renders the menu to a surface
	DBOOL			Render(HSURFACE hDestSurf);

	// Creates the title for the menu from the path to the image.  The non-Enlish version
	// overwrites the surface with a text string specified by the string ID.
	DBOOL			CreateTitle(char *lpszTitleSurf, int nStringID, DIntPt titlePos, DBOOL bUseBitmapSurf);
	DBOOL			CreateTitle(char *lpszTitleSurf, int nStringID, int xPos, int yPos, DBOOL bUseBitmapSurf);

	// This is called when the menu gets or loses focus
	virtual void	OnFocus(DBOOL bFocus)				{ }

	/////////////////////////////////////////
	// Adds a menu options using an HSTRING
	CLTGUIFadeItemCtrl		*AddFadeItemOption(char *lpszOptionSurfPrefix, int nSurfaces, HSTRING hOptionText, DWORD dwCommandID, int xPos=-1, int yPos=-1);
	CLTGUITextItemCtrl		*AddTextItemOption(HSTRING hText, DWORD dwCommandID, CLTGUIFont *pFontArray, int nNumFonts=1, DBOOL bDrawSolid=DTRUE, int *pnValue=DNULL);
	CLTGUIColumnTextCtrl	*AddColumnTextOption(DWORD dwCommandID, CLTGUIFont *pFont);
	CLTGUISliderCtrl		*AddSliderOption(HSTRING hText, CLTGUIFont *pFont, int nSliderOffset, HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue=DNULL);
	CLTGUIOnOffCtrl			*AddOnOffOption(HSTRING hText, CLTGUIFont *pFont, int nRightColumnOffset, DBOOL *pbValue=DNULL);
	CLTGUIEditCtrl			*AddEditOption(HSTRING hDescription, DWORD dwCommandID, CLTGUIFont *pFont, int nEditStringOffset, int nBufferSize, char *lpszValue=DNULL);

	// Just a wrapper for adding fading text options with large fonts
	CLTGUITextItemCtrl		*AddLargeTextItemOption(HSTRING hText, DWORD dwCommandID, DBOOL bUseBitmapFontArray);	

	///////////////////////////////////////////////////////////////////////////////////
	// Wrappers for adding menu items with a resource string ID instead of an HSTRING
	CLTGUIFadeItemCtrl		*AddFadeItemOption(char *lpszOptionSurfPrefix, int nSurfaces, int messageCode, DWORD dwCommandID, int xPos=-1, int yPos=-1);
	CLTGUITextItemCtrl		*AddTextItemOption(int messageCode, DWORD dwCommandID, CLTGUIFont *pFontArray, int nNumFonts=1, DBOOL bDrawSolid=DTRUE, int *pnValue=DNULL);	
	CLTGUISliderCtrl		*AddSliderOption(int messageCode, CLTGUIFont *pFont, int nSliderOffset, HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue=DNULL);
	CLTGUIOnOffCtrl			*AddOnOffOption(int messageCode, CLTGUIFont *pFont, int nRightColumnOffset, DBOOL *pbValue=DNULL);
	CLTGUIEditCtrl			*AddEditOption(int messageCode, DWORD dwCommandID, CLTGUIFont *pFont, int nEditStringOffset, int nBufferSize, char *lpszValue=DNULL);	
	CLTGUITextItemCtrl		*AddLargeTextItemOption(int messageCode, DWORD dwCommandID, DBOOL bUseBitmapFontArray);	

	///////////////////////////////////////////////////////////////////////////////////
	// Wrappers for adding menu items with a char* instead of a string ID or HSTRING
	inline CLTGUIFadeItemCtrl	*AddFadeItemOption(char *lpszOptionSurfPrefix, int nSurfaces, char *lpszOptionText, DWORD dwCommandID, int xPos=-1, int yPos=-1);
	inline CLTGUITextItemCtrl	*AddTextItemOption(char *lpszText, DWORD dwCommandID, CLTGUIFont *pFontArray, int nNumFonts=1, DBOOL bDrawSolid=DTRUE, int *pnValue=DNULL);	
	inline CLTGUISliderCtrl		*AddSliderOption(char *lpszText, CLTGUIFont *pFont, int nSliderOffset, HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue=DNULL);
	inline CLTGUIOnOffCtrl		*AddOnOffOption(char *lpszText, CLTGUIFont *pFont, int nRightColumnOffset, DBOOL *pbValue=DNULL);
	inline CLTGUIEditCtrl		*AddEditOption(char *lpszDescription, DWORD dwCommandID, CLTGUIFont *pFont, int nEditStringOffset, int nBufferSize, char *lpszValue=DNULL);
	inline CLTGUITextItemCtrl	*AddLargeTextItemOption(char *lspzText, DWORD dwCommandID, DBOOL bUseBitmapFontArray);	

	// Sets the parent menu
	void			SetParentMenu(CMenuBase *pParent)		{ m_pParentMenu=pParent; }

	// Calls UpdateData on each control in the menu
	void			UpdateData(DBOOL bSaveAndValidate=DTRUE);

	// Set the current menu item selection index.  Note that this will reset any animations.
	void			SetCurrentItem ( int nItemIndex );

	// Gets the currently selected item
	int				GetCurrentItemIndex()					{ return m_listOption.GetSelectedItem(); }
	CLTGUICtrl		*GetCurrentItem();

	// Sets the position of the title and menu option
	void			SetTitlePos(int x, int y)				{ m_titlePos.x=x; m_titlePos.y=y; }
	void			SetTitlePos(DIntPt pt)					{ SetTitlePos(pt.x, pt.y); }
	void			SetTitleCenter(DBOOL bCenter)			{ m_bTitleCenter=bCenter; }

	void			SetOptionPos(int x, int y)				{ m_listOption.SetPos(x, y); }
	void			SetOptionPos(DIntPt pt)					{ SetOptionPos(pt.x, pt.y); }
	void			SetOptionCenter(DBOOL bCenter)			{ m_listOption.SetCenter(bCenter); }

	// Sets the height of the menu
	void			SetMenuHeight(int nHeight)				{ m_listOption.SetHeight(nHeight); }

	// Turns on and off low resolution fonts for the menus
	void			SetLowResolutionFonts(DBOOL bLowRes, DBOOL bUseBitmapFontArray);

	// Sets the item spacing
	void			SetItemSpacing(int nSpacing)			{ m_listOption.SetItemSpacing(nSpacing); }

	// Sets whether or not the items should wrap when scrolling
	void			SetScrollWrap(DBOOL bWrap)				{ m_listOption.SetScrollWrap(bWrap); }

	// Call this if you wish to enable highlighting the item that is under the mouse
	// cursor and changing selection when the mouse is moved.
	void			EnableMouseMoveSelect(DBOOL bEnable);

	// Set the maximum number of items to scroll per second when using the mouse
	void			SetMaxScrollRate(int nItemsPerSecond)	{ m_nMaxScrollRate=nItemsPerSecond; }

	// Turns the up/down arrows on/off and sets the X position that they should be pointing
	void			UseArrows(DBOOL bUse, int xCenter);

	// Handles a key press.  Returns FALSE if the key was not processed through this method.
	// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
	virtual	DBOOL	HandleKeyDown(int key, int rep);

	// Handle a device tracking message.
	// Return DTRUE if the device tracking should stop.
	virtual DBOOL	HandleDeviceTrack(DeviceInput *pInput, DDWORD dwNumDevices);

	// Mouse messages
	virtual DBOOL	OnLButtonDown(int x, int y);
	virtual DBOOL	OnLButtonUp(int x, int y);
	virtual DBOOL	OnLButtonDblClick(int x, int y);
	virtual DBOOL	OnRButtonDown(int x, int y);
	virtual DBOOL	OnRButtonUp(int x, int y);
	virtual DBOOL	OnRButtonDblClick(int x, int y);
	virtual DBOOL	OnMouseMove(int x, int y);

protected:
	virtual DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

	// Handle input
	virtual void	OnUp();
	virtual void	OnDown();
	virtual void	OnLeft();
	virtual void	OnRight();
	virtual void	OnEnter();	

	// Play sounds
	virtual void	PlaySelectSound();
	virtual void	PlayEnterSound();

	// Removes all of the options from the menu
	void		RemoveAllOptions();

	// Sorts a file list.  This was taken from the old menu code.
	FileEntry*	SortFileList(FileEntry *pfe);

	// Renders the up/down arrows
	void		RenderArrows(HSURFACE hDestSurf);
	
	// Returns the bouding box for where the up/down arrows are
	DBOOL		GetArrowBoundingBox(DRect *prcUpArrow, DRect *prcDownArrow);

	// Handle scrolling up and down from mouse clicks
	void		HandleMouseScrolling();

	void		SetTransparentColor(HDECOLOR hTrans) { m_hTransparentColor = hTrans; }

protected:
	// Pure virtual function calls.  Basically these are used to get access
	// to members of the class that contains all of the menus.
	virtual void			FreeSharedSurface(HSURFACE hSurf)=0;
	virtual HSURFACE		GetSharedSurface(char *lpszSurf)=0;
	
	virtual CLTGUIFont		*GetTitleFont()=0;
	virtual CLTGUIFont		*GetSmallFont()=0;
	virtual CLTGUIFont		*GetLargeFont()=0;

	virtual CLTGUIFont		*GetLargeFadeFonts()=0;
	virtual int				GetNumLargeFadeFonts()=0;

	virtual HSURFACE		GetSurfaceUpArrow()=0;
	virtual HSURFACE		GetSurfaceDownArrow()=0;
	virtual DBOOL			IsLowResolution()=0;

protected:
	DBOOL			m_bInit;
	DBOOL			m_bBuilt;

	CClientDE		*m_pClientDE;		// Client interface

	DDWORD			m_dwMenuID;			// The ID of this menu
	CMenuBase		*m_pParentMenu;		// The parent menu

	HSURFACE		m_hTitleSurf;		// The title surface
	HSTRING			m_hTitleString;		// The title string

	DIntPt			m_titlePos;			// The title position
	DBOOL			m_bTitleCenter;		// Center the title on/off

	CLTGUIListCtrl	m_listOption;		// The list of menu items	

	DBOOL			m_bBoxFormat;		// Should we format the list options in a box? or have free locations

	DBOOL			m_bShowArrows;		// TRUE if the up/down arrows should be drawn
	int				m_nArrowCenter;		// The X coordinate of where the arrow center should be

	DBOOL			m_bScrollingUp;		// TRUE if we are scrolling up from an arrow click
	DBOOL			m_bScrollingDown;	// TRUE if we are scrolling down from an arrow click
	int				m_nMaxScrollRate;	// The max number of items per second to scroll
	float			m_fLastScrollTime;	// The last time that the menu was scrolled
	HDECOLOR		m_hTransparentColor; // The transparent color

	// Array of controls that this menu owns
	CMoArray<CLTGUICtrl *>	m_controlArray;	

	// Stores controls added by AddLargeTextItemOption
	CMoArray<CLTGUITextItemCtrl *>	m_largeFontItemArray;
};

inline CLTGUIFadeItemCtrl *CMenuBase::AddFadeItemOption(char *lpszOptionSurfPrefix, int nSurfaces, char *lpszOptionText, DWORD dwCommandID, int xPos, int yPos)
{
	HSTRING hString=m_pClientDE->CreateString(lpszOptionText);
	CLTGUIFadeItemCtrl *pCtrl=AddFadeItemOption(lpszOptionSurfPrefix, nSurfaces, hString, dwCommandID, xPos, yPos);
	m_pClientDE->FreeString(hString);

	return pCtrl;
}

inline CLTGUITextItemCtrl *CMenuBase::AddTextItemOption(char *lpszText, DWORD dwCommandID, CLTGUIFont *pFontArray, int nNumFonts, DBOOL bDrawSolid, int *pnValue)
{
	HSTRING hString=m_pClientDE->CreateString(lpszText);
	CLTGUITextItemCtrl *pCtrl=AddTextItemOption(hString, dwCommandID, pFontArray, nNumFonts, bDrawSolid, pnValue);
	m_pClientDE->FreeString(hString);

	return pCtrl;
}


inline CLTGUISliderCtrl *CMenuBase::AddSliderOption(char *lpszText, CLTGUIFont *pFont, int nSliderOffset, HSURFACE hBarSurf, HSURFACE hTabSurf, int *pnValue)
{
	HSTRING hString=m_pClientDE->CreateString(lpszText);
	CLTGUISliderCtrl *pCtrl=AddSliderOption(hString, pFont, nSliderOffset, hBarSurf, hTabSurf, pnValue);
	m_pClientDE->FreeString(hString);

	return pCtrl;
}

inline CLTGUIOnOffCtrl *CMenuBase::AddOnOffOption(char *lpszText, CLTGUIFont *pFont, int nRightColumnOffset, DBOOL *pbValue)
{
	HSTRING hString=m_pClientDE->CreateString(lpszText);
	CLTGUIOnOffCtrl *pCtrl=AddOnOffOption(hString, pFont, nRightColumnOffset, pbValue);
	m_pClientDE->FreeString(hString);

	return pCtrl;
}

inline CLTGUIEditCtrl *CMenuBase::AddEditOption(char *lpszDescription, DWORD dwCommandID, CLTGUIFont *pFont, int nEditStringOffset, int nBufferSize, char *lpszValue)
{
	HSTRING hString=m_pClientDE->CreateString(lpszDescription);
	CLTGUIEditCtrl *pCtrl=AddEditOption(hString, dwCommandID, pFont, nEditStringOffset, nBufferSize, lpszValue);
	m_pClientDE->FreeString(hString);

	return pCtrl;
}

inline CLTGUITextItemCtrl *CMenuBase::AddLargeTextItemOption(char *lpszText, DWORD dwCommandID, DBOOL bUseBitmapFontArray)
{
	HSTRING hString=m_pClientDE->CreateString(lpszText);
	CLTGUITextItemCtrl *pCtrl=AddLargeTextItemOption(lpszText, dwCommandID, bUseBitmapFontArray);
	m_pClientDE->FreeString(hString);

	return pCtrl;
}

#endif // !defined(AFX_MENUBASE_H__D7668B31_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
