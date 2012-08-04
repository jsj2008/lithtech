// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIColumnCtrl.h
//
// PURPOSE : Control to display columns of text
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUICOLUMNCTRL_H_)
#define _LTGUICOLUMNCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


typedef std::vector<CLTGUITextCtrl*> TextControlArray;

class CLTGUIColumnCtrl : public CLTGUICtrl
{
public:
	CLTGUIColumnCtrl();
	virtual ~CLTGUIColumnCtrl();

	// Create the control
	// pClientDE	- Pointer to the client interface.
	// dwCommandID	- The command ID which is sent when "enter" is pressed.
	// pFont		- The default font to use for creating the strings.
	// pMessageCtrl - control which receives messages when the "enter" key is pressed.
    virtual LTBOOL   Create ( uint32 dwCommandID, uint32 dwHelpID, CUIFont *pFont, uint8 nFontSize,
                             CLTGUICommandHandler *pCommandHandler, uint32 dwParam1 = 0, uint32 dwParam2 = 0 );

	// Destroys the control
	virtual void			Destroy ( );

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);
    virtual void    SetColors(uint32 argbSelected, uint32 argbNormal, uint32 argbDisabled);


	// Enable/Disable the control
    virtual void    Enable ( LTBOOL bEnabled );


	// Adds a column to the control
	// nWidth	  - Width of the column
	// hString	  - The initial text for the column
	//returns column index
	static const uint8	kMaxNumColumns;
    uint8          AddColumn(const char *pString, uint16 nWidth, LTBOOL bClip = LTFALSE);

	// Gets a string at a specific column index.  This returns a copy (new handle).
	CLTGUITextCtrl*			GetColumn(uint8 nColumnIndex) const;
	CUIFormattedPolyString*	GetPolyString(uint8 nColumnIndex) const;
	const char*				GetString(uint8 nColumnIndex) const;

	// Sets a string for a column.  This copies the string from hString to an internal string.
	void			SetString(uint8 nColumnIndex, const char *pString);

	void			RemoveColumn(uint8 nIndex);			// Removes a column
	void			RemoveAllColumns();					// Removes all of the columns

	// Return the number of columns
	uint8			GetNumColumns()						{ return m_columnArray.size(); }

	// Render the control
	void			Render ();

	// Set the font
    void            SetFont (CUIFont *pFont, uint8 nFontSize, LTBOOL bSetForAll = LTFALSE);

	// Width/Height calculations
	uint16			GetWidth ( )	{ return m_nWidth; }
	uint16			GetHeight ( )	{ return m_nHeight; }

	// Handle the Enter key being pressed
    virtual LTBOOL   OnEnter ( );
    virtual LTBOOL   OnLButtonUp(int x, int y) {return OnEnter();}

protected:
	virtual void	OnSelChange();
	void			CalculateSize();

protected:

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

    uint16		m_nWidth;              // The width of the control
    uint16		m_nBaseWidth;          // The unscaled width of the control
    uint16		m_nHeight;             // The height of the control

	TextControlArray	m_columnArray;	// The array of columns

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};


#endif // !defined(_LTGUICOLUMNCTRL_H_)