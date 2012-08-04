// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIListCtrl.h
//
// PURPOSE : Control to handle text entry
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUIEDITCTRL_H_)
#define _LTGUIEDITCTRL_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ltguictrl.h"

class CLTGUIEditCtrl : public CLTGUICtrl
{
public:
	CLTGUIEditCtrl();
	virtual ~CLTGUIEditCtrl();

	// Create the control
	// pClientDE	   - Pointer to the client interface.
	// nCommandID	   - The command ID which is sent when "enter" is pressed.
	// pFont		   - The font to use for rendering the strings.
	// nMaxLength	   - The size of the buffer to store the string in.
	// pMessageCtrl    - control which receives messages when the "enter" key is pressed.
	// pStringValue	   - pointer to the string to be modified in UpdateData.
    virtual LTBOOL   Create ( ILTClient *pLTClient, uint32 nCommandID, CUIFont *pFont, uint8 nFontSize, 
								uint16 nMaxLength, CLTGUICommandHandler *pCommandHandler = LTNULL, 
								char *pStringValue=LTNULL, uint32 nParam1 = 0, uint32 nParam2 = 0 );

	// Destroys the control
	void			Destroy ( );

	// Update data
    void            UpdateData(LTBOOL bSaveAndValidate=LTTRUE);

	// Render the control
	void			Render ();

	// Set the font
	LTBOOL			SetFont ( CUIFont *pFont, uint8 nFontSize );

	// Set/Get the text for the edit string
	void			SetText(const char *pString);
	const char*		GetText();

	virtual void	SetFixedWidth(uint16 nWidth, LTBOOL bUseFrame = LTFALSE);
	virtual uint16	GetFixedWidth() {return m_nFixedWidth;}

	uint16	GetMaxLength() {return m_nMaxLength;}
	void	SetMaxLength(uint16 nMaxLength);

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);


	// Width/Height calculations
	uint16			GetWidth ( )	{return m_nWidth;}
	uint16			GetHeight ( )	{return m_nHeight;}

	// Handles a key press
    LTBOOL           HandleKeyDown(int key, int rep);
    LTBOOL           HandleChar(unsigned char c);

	// Handle the Enter key being pressed
    virtual LTBOOL   OnEnter ( );
    virtual LTBOOL   OnLButtonUp(int x, int y);

	// Set the caret usage (default is no caret)
    void            EnableCaret(LTBOOL bUseCaret, float fToggleTime = 0.333f, uint32 argbColor = 0xFFFFFFFF);

	static const uint16	kMaxLength;

	// Set the character input mode
	enum eInputMode
	{
		kInputAll,
		kInputAlphaNumeric,
		kInputAlphaOnly,
		kInputNumberOnly,
		kInputFileFriendly,
		kInputSprintfFriendly
	};
	void			SetInputMode(eInputMode mode)	{ m_eInputMode = mode;}


protected:
	// Add a character to the end
	void			AddCharacter(char c);

	// Remove a character from the end
	void			RemoveCharacter();

	// Should the caret be rendered
    LTBOOL           IsCaretOn();

	virtual void	CalculateSize();

protected:
    ILTClient       *m_pLTClient;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling

	CUIFormattedPolyString*		m_pDescription;
	CUIFormattedPolyString*		m_pText;

	uint16			m_nMaxLength;			// The maximum length for the string
	uint16			m_nCaretPos;
	char			*m_pStringValue;		// The pointer that is updated in the UpdateData function

    LTBOOL          m_bCaretEnabled;
    uint32			m_argbCaretColor;
	float			m_fCaretTime;
	LT_POLYF4		m_Caret;
	LT_POLYF4		m_Frame[4];

    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The height of the control

	uint16		m_nFixedWidth;
	LTBOOL		m_bUseFrame;

	eInputMode		m_eInputMode;

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

#endif // !defined(_LTGUIEDITCTRL_H_)