// LTGUITextItemCtrl.h: interface for the CLTGUITextItemCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUITEXTITEMCTRL_H__DDDEFA74_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_LTGUITEXTITEMCTRL_H__DDDEFA74_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUICtrl.h"
#include "stdlith.h"

// This control displays a string of text which fades out upon being unselected.
// It can contain multiple strings (see CLTGUICtrl::AddString()) which are cycled
// when OnLeft() and OnRight() are called.  This is useful in menu code for selecting
// options.
class CLTGUIFont;
class CLTGUITextItemCtrl : public CLTGUIFadeColorCtrl  
{
public:
	CLTGUITextItemCtrl();
	virtual ~CLTGUITextItemCtrl();

	// Create the control
	// pClientDE	- Pointer to the client interface.
	// dwCommandID	- The command ID which is sent when "enter" is pressed.
	// hText		- The initial text that is displayed for this control. Pass in DNULL if you do not want initial text.
	// pFontArray	- An array of fonts to use for this string.  You can use multiple fonts and fade between them.
	// nNumFonts	- The number of fonts in the array.
	// bDrawSolid	- If TRUE the fonts draw solid using a color using the first font in the array.
	//				  If FALSE the text is draw using the array of fonts passed in fading from selected to
	//				  non selected.  The last font in the array is used as the disabled text font.
	// pMessageCtrl - control which receives messages when the "enter" key is pressed.
	// pnValue		- Pointer to the value which receives the selection index when UpdateData is called.
	DBOOL			Create ( CClientDE *pClientDE, DDWORD dwCommandID, HSTRING hText, CLTGUIFont *pFontArray,
							 int nNumFonts, DBOOL bDrawSolid, CLTGUICommandHandler *pCommandHandler, int *pnValue=DNULL );

	// Destroys the control
	void			Destroy ( );

	// Update data
	void			UpdateData(DBOOL bSaveAndValidate);

	// Add more strings to the control.  These are cycled when OnLeft() and OnRight() are called
	void			AddString(HSTRING hString);		
	void			AddString(int messageCode);

	HSTRING			GetString(int nIndex);			// Return a string at a specific index	
	void			RemoveString(int nIndex);		// Remove a string at a specific index	
	void			RemoveAll();					// Remove all strings

	// Return the number of strings	
	int				GetNumStrings()						{ return m_stringArray.GetSize(); }

	// Sets/Get the currently selected index
	int				GetSelIndex()						{ return m_nSelIndex; }
	void			SetSelIndex(int nIndex);	

	// Render the control
	void			Render ( HSURFACE hDestSurf );	

	// Set the font
	DBOOL			SetFont ( CLTGUIFont *pFontArray, int nNumFonts=1, DBOOL bDrawSolid=DTRUE );	

	// Width/Height calculations
	int				GetWidth ( )						{ return m_dwWidth; }
	int				GetHeight ( )						{ return m_dwHeight; }

	// Commonly used keyboard messages	
	void			OnEnter ( );	
	void			OnLeft ( );
	void			OnRight ( );

protected:
	// The selection for this control has changed	
	void			OnSelChange();

protected:		
	DDWORD			m_dwCommandID;			// The command which is sent when "enter" is pressed	
		
	CLTGUIFont		*m_pFontArray;			// The fonts for this control
	int				m_nNumFonts;			// The number of fonts
	DBOOL			m_bDrawSolid;			// TRUE if we should draw the text solid

	DDWORD			m_dwWidth;				// The width of the control
	DDWORD			m_dwHeight;				// The height of the control

	int				*m_pnValue;				// Receives the current selection when UpdateData is called

	int					m_nSelIndex;		// The currently selected index
	CMoArray<HSTRING>	m_stringArray;		// The array of strings	

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

#endif // !defined(AFX_LTGUITEXTITEMCTRL_H__DDDEFA74_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
