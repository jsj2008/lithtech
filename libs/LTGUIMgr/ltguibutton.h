// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIButton.h
//
// PURPOSE : button control with three states (normal, selected, and disabled)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUIBUTTON_H_)
#define _LTGUIBUTTON_H_


// This control displays a string of text which fades out upon being unselected.
// It can contain multiple strings (see CLTGUICtrl::AddString()) which are cycled
// when OnLeft() and OnRight() are called.  This is useful in menu code for selecting
// options.
class CLTGUIButton : public CLTGUICtrl
{
public:
	CLTGUIButton();
	virtual ~CLTGUIButton();

	// Create the control
	// nCommandID		- The command ID which is sent when "enter" is pressed.
	// nHelpID			- The help ID which is used to display help text.
	// hNormal			- normal texture
	// hSelected		- highlighted texture
	// hDisabled		- disabled texture
	// pCommandHandler	- handler which receives messages when the "enter" key is pressed.
    LTBOOL           Create ( uint32 nCommandID, uint32 nHelpID,
								HTEXTURE hNormal, HTEXTURE hSelected = LTNULL, HTEXTURE hDisabled = LTNULL,
								CLTGUICommandHandler *pCommandHandler = LTNULL, uint32 nParam1 = 0, uint32 nParam2 = 0);

	// Destruction
	virtual void	Destroy();

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);
	virtual void	SetTextureScale(float fScale);

	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }

	// Commonly used keyboard messages
    virtual LTBOOL   OnEnter ( );
    virtual LTBOOL   OnLButtonUp(int x, int y)			{ return OnEnter(); }

	// Set the font, pass in a NULL pointer to change only the font size
	//   pass in a font size of 0 to retain the current size.
    LTBOOL  SetFont ( CUIFont *pFont, uint8 nFontSize);

	//this fails, if no font has been defined
	LTBOOL	SetText(const char* pText, LTBOOL bHighlightText = LTFALSE);


	void	SetTexture(HTEXTURE hNormal, HTEXTURE hSelected = LTNULL, HTEXTURE hDisabled = LTNULL, LTBOOL bFreeOld = LTFALSE);

	virtual void	SetCommandHandler(CLTGUICommandHandler *pCommandHandler)
									{ m_pCommandHandler = pCommandHandler; }



protected:
	void			SetRenderState();
	void			InitPoly();
	void			ScalePoly();

protected:

	LT_POLYGT4	m_Poly;
	HTEXTURE	m_hNormal;			// normal texture
	HTEXTURE	m_hSelected;		// highlighted texture
	HTEXTURE	m_hDisabled;		// disabled texture
	LTFLOAT		m_fTextureScale;

    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The height of the control

	CUIFormattedPolyString* m_pText;
	LTBOOL		m_bHighlightText;
	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

#endif // _LTGUIBUTTON_H_
