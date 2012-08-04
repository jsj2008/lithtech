// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUITextItemCtrl.h
//
// PURPOSE : Simple text control which may be used as a menu item.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUITEXTITEMCTRL_H_)
#define _LTGUITEXTITEMCTRL_H_


// This control displays a string of text which fades out upon being unselected.
// It can contain multiple strings (see CLTGUICtrl::AddString()) which are cycled
// when OnLeft() and OnRight() are called.  This is useful in menu code for selecting
// options.
class CLTGUITextCtrl : public CLTGUICtrl
{
public:
	CLTGUITextCtrl();
	virtual ~CLTGUITextCtrl();

	// Create the control
	// nCommandID		- The command ID which is sent when "enter" is pressed.
	// nHelpID			- The help ID which is used to display help text.
    // pText			- The initial text that is displayed for this control. Pass in 
	//						LTNULL if you do not want initial text. A copy of this text
	//						is made so the string may be discarded after making this call.
	// pFont			- The font to use for this string.
	// nFontSize		- The font size to use for this string.
	// pCommandHandler	- handler which receives messages when the "enter" key is pressed.
    LTBOOL           Create (	const char *pText, uint32 nCommandID, uint32 nHelpID, 
								CUIFont *pFont, uint8 nFontSize, CLTGUICommandHandler *pCommandHandler, 
								uint32 nParam1 = 0, uint32 nParam2 = 0);

	// Destroys the control
	virtual void	Destroy ( ); 

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);

	virtual void	SetFixedWidth(uint16 nWidth, LTBOOL bClip = LTFALSE);
	virtual uint16	GetFixedWidth() {return m_nFixedWidth;}

	virtual void	SetIndent(uint16 nIndent);

	virtual void	SetCommandHandler(CLTGUICommandHandler *pCommandHandler)
										{ m_pCommandHandler = pCommandHandler; }

	// Add more strings to the control.  These are cycled when OnLeft() and OnRight() are called
	virtual void	SetString(const char *pText);
	CUIFormattedPolyString*	GetString()  {return m_pString;}

	// Render the control
	virtual void	Render ();

	// Set the font, pass in a NULL pointer to change only the font size
	//   pass in a font size of 0 to retain the current size.
    virtual LTBOOL	SetFont ( CUIFont *pFont, uint8 nFontSize);

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }

	// Commonly used keyboard messages
    virtual LTBOOL   OnEnter ( );
    virtual LTBOOL   OnLButtonUp(int x, int y) {return OnEnter();}

protected:
	virtual void	CalculateSize();

protected:

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The height of the control

    uint16      m_nIndent;

    uint16      m_nFixedWidth;
	LTBOOL		m_bClip;


	CUIFormattedPolyString*		m_pString;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

#endif // _LTGUITEXTITEMCTRL_H_