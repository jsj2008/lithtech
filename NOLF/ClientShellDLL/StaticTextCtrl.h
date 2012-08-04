// StaticTextCtrl.h: interface for the CStaticTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_STATICTEXTCTRL_H_)
#define _STATICTEXTCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ltguimgr.h"

// This control displays a string of text which fades out upon being unselected.
// It can contain multiple strings (see CLTGUICtrl::AddString()) which are cycled
// when OnLeft() and OnRight() are called.  This is useful in menu code for selecting
// options.
class CLTGUIFont;
class CStaticTextCtrl : public CLTGUIFadeColorCtrl
{
public:
	CStaticTextCtrl();
	virtual ~CStaticTextCtrl();

	// Create the control
	// pClientDE	- Pointer to the client interface.
	// dwCommandID	- The command ID which is sent when "enter" is pressed.
    // hText        - The initial text that is displayed for this control. Pass in LTNULL if you do not want initial text.
	// pFont		- Font to use for this string.
	// pMessageCtrl - control which receives messages when the "enter" key is pressed.
	// dwWidth		- Width to format the text. 0 indicate not formatting
    LTBOOL           Create ( ILTClient *pClientDE, uint32 dwCommandID, HSTRING hText, CLTGUIFont *pFont,
                             CLTGUICommandHandler *pCommandHandler, uint32 dwWidth = 0,  uint32 dwHeight = 0,
                             uint32 dwParam1 = 0, uint32 dwParam2 = 0);

	// Destroys the control
	void			Destroy ( );

	//frees the surfaces used for the control
	void			Purge();

	void			SetString(HSTRING hString);
	void			SetString(int messageCode);
	void			SetString(char *pszString);

	HSTRING			GetString();
	void			RemoveString();

	// Render the control
	void			Render ( HSURFACE hDestSurf );

	// Set the font
    LTBOOL          SetFont ( CLTGUIFont *pFont);

	// Width/Height calculations
	int				GetWidth ( );
	int				GetHeight ( );

	// Commonly used keyboard messages
    virtual LTBOOL   OnEnter ( );
    virtual LTBOOL   OnLButtonUp (int x, int y) {return OnEnter();}
	// [blg] ctrl id
    uint32          GetID() { return(m_dwCommandID); }

protected:
	// The selection for this control has changed
	virtual void	OnSelChange();
	virtual void	CalculateSize();

	void			CreateSurface();

protected:

	CLTGUIFont		*m_pFont;				// The font for this control

    uint32          m_dwWidth;              // The width of the control
    uint32          m_dwHeight;             // The height of the control
    LTBOOL           m_bFixedWidth;          // TRUE is we should format the text to fit a set width
    LTBOOL           m_bFixedHeight;         // TRUE is we should clip the text to fit a set height

	HSTRING			m_hString;
	HSURFACE		m_hSurface;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

#endif // !defined(_STATICTEXTCTRL_H_)