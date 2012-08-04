// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUILargeText.h
//
// PURPOSE : Simple text control which may be used as a menu item.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUILARGETEXT_H_)
#define _LTGUILARGETEXT_H_


// This control is designed to display large pieces of text which may need to word-wrap
// or scroll... it is not intended to be a menu item
class CLTGUILargeText : public CLTGUITextCtrl
{
public:
	CLTGUILargeText();
	virtual ~CLTGUILargeText();

	// Create the control
    // pText			- The initial text that is displayed for this control. Pass in 
	//						LTNULL if you do not want initial text. A copy of this text
	//						is made so the string may be discarded after making this call.
	// pFont			- The font to use for this string.
	// nFontSize		- The font size to use for this string.
	// ptTextSize		- The width and height of the control
    LTBOOL           Create (	const char *pText, CUIFont *pFont, uint8 nFontSize, LTIntPt ptTextSize);


	// Destroys the control
	virtual void	Destroy ( ); 

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);


	virtual void	SetIndent(uint16 nIndent);

	virtual LTBOOL	SetSize(LTIntPt ptTextSize);

	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }

	// Sets the width of the list's frame, set to 0 to not show the frame
	void		SetFrameWidth(uint8 nFrameWidth);
	LTBOOL		UseArrows(LTFLOAT fTextureScale, HTEXTURE hUpNormal,  HTEXTURE hUpSelected, 
						HTEXTURE hDownNormal,  HTEXTURE hDownSelected);


	// Commonly used keyboard messages
    virtual LTBOOL   OnUp ( );
    virtual LTBOOL   OnDown ( );

	// Mouse messages
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);
    virtual LTBOOL   OnMouseMove(int x, int y);

	
	enum Consts
	{
		kMaxNumLines = 64
	};

protected:
	void	SetRenderState();

	virtual void	CalculateSize();
	virtual void	CalculateLines();

protected:

	uint16	m_nFirstLine;
	uint16	m_nLastLine;
	uint16	m_nNumLines;
	uint16  m_nLineOffsets[kMaxNumLines];
	uint16	m_nFixedHeight;

	uint16		m_nLBDownSel;				// The control index that is selected from the current left button message
	uint16		m_nRBDownSel;				// The control index that is selected from the current right button message

	uint8		m_nFrameWidth;
	LT_POLYF4	m_Frame[4];

	CLTGUIButton*	m_pUp;
	CLTGUIButton*	m_pDown;



};

#endif // _LTGUILARGETEXT_H_