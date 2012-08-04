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

/*
// This control displays a button
class CLTGUIButton : public CLTGUICtrl
{
public:
	CLTGUIButton();
	virtual ~CLTGUIButton();

	// Create the control
    bool           Create ( uint32 nCommandID, uint32 nHelpID, uint32 nWidth, uint32 nHeight,
								HTEXTURE hNormal, HTEXTURE hSelected = NULL, HTEXTURE hDisabled = NULL, 
								CLTGUICommandHandler *pCommandHandler = NULL, uint32 nParam1 = 0, uint32 nParam2 = 0);


    virtual void    SetBasePos (LTIntPt pos );
	virtual void	SetScale(float fScale);
	virtual void	SetSize(uint32 nWidth, uint32 nHeight );

	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint32	GetBaseWidth ( )			{ return m_nBaseWidth; }
	virtual uint32	GetBaseHeight ( )			{ return m_nBaseHeight; }

	// Commonly used keyboard messages
    virtual bool   OnEnter ( );
    virtual bool   OnLButtonUp(int x, int y)			{ return OnEnter(); }

	// Set the font, note this always expects an unscaled height
	virtual bool	SetFont ( const CFontInfo& Font );

	//this fails if no font has been set
	virtual bool	SetText(const wchar_t *pText, bool bHighlightText);


	void	SetTexture(HTEXTURE hNormal, HTEXTURE hSelected = NULL, HTEXTURE hDisabled = NULL, bool bFreeOld = false);

	virtual void	SetCommandHandler(CLTGUICommandHandler *pCommandHandler)
									{ m_pCommandHandler = pCommandHandler; }

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();


protected:
	void			SetRenderState();
	void			InitPoly();
	void			ScalePoly();

protected:

	LT_POLYGT4	m_Poly;
	HTEXTURE	m_hNormal;			// normal texture
	HTEXTURE	m_hSelected;		// highlighted texture
	HTEXTURE	m_hDisabled;		// disabled texture

	uint32		m_nBaseWidth;          // The width of the control
	uint32		m_nBaseHeight;         // The height of the control

	CLTGUIString	m_Text;
	CFontInfo		m_Font;					// The font for this control
	uint32			m_nBaseFontSize;		// The font size before scaling
	bool			m_bHighlightText;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

*/

#endif // _LTGUIBUTTON_H_