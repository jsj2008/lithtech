// MessageBoxCtrl.h: interface for the CMessageBoxCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MESSAGEBOXCTRL_H_
#define _MESSAGEBOXCTRL_H_

#include "ltguimgr.h"

class CMessageBoxCtrl : public CLTGUICommandHandler
{
public:
	CMessageBoxCtrl();
	virtual ~CMessageBoxCtrl();

	// Create the message box
	// pClientDE	   - Pointer to the client interface
	// hBack		   - Surface to use for the background
	// lpszBackground  - Pathname to the pcx file for the background
	// pFont		   - Font which is used to display text in the message box.
	// hText		   - Text to be displayed in the message box
	// pCommandHandler - Command handler which recieves a message when a key (call AddKey first) is pressed
    LTBOOL       Create(ILTClient *pClientDE, HSURFACE hBack, CLTGUIFont *pFont, HSTRING hText=LTNULL, CLTGUICommandHandler *pCommandHandler=LTNULL);
    LTBOOL       Create(ILTClient *pClientDE, char *lpszBackground, CLTGUIFont *pFont, HSTRING hText=LTNULL, CLTGUICommandHandler *pCommandHandler=LTNULL);
	void		Destroy();

	// Render the message box.
	void		Render(HSURFACE hDestSurf);

	// Sets the message box position
	// Pass in LTGUI_MB_CENTER for the positions and a destination surface if you wish to
	// center the message box.
    void        SetPos(int xPos, int yPos, HSURFACE hDestSurf=LTNULL);

	void		SetSelection(int nSelect);

	// Set the text for the message box
	void		SetText(HSTRING hString);

	// Sets the text color
    void        SetTextColor(HLTCOLOR color)                        { m_textColor=color; }

	// Sets the transparent color
    void        SetTransparentColor(HLTCOLOR hColor)                {m_hTransColor = hColor;}


	// Sets the "text buttons" color and highlight color
    void        SetTextButtonColor(HLTCOLOR color, HLTCOLOR highlightColor);

	// Sets the number of pixels between each text button
	void		SetTextButtonHorzSpace(int nPixels)					{ m_nTextButtonHorzSpace=nPixels; }

	// Sets the number of pixels between the bottom of the message box and the bottom of the text button
	void		SetTextButtonVertSpace(int nPixels)					{ m_nTextButtonVertSpace=nPixels; }

	// Set the command handler to receive messages from
	void		SetCommandHandler(CLTGUICommandHandler *pHandler)	{ m_pCommandHandler=pHandler; }

	// Add a key and tie it to a message.  The message is sent when the key is passed into HandleKeyDown
    void        AddKey(int nVKeyCode, uint32 dwMessageID);

	// Removes all keys assignments
	void		RemoveKeys()										{ m_keyBindings.SetSize(0); }

	// Adds a "text button" to the bottom row of the message box.  The message is sent when the user clicks
	// on the button with the mouse.
    void        AddMessageButton(HSTRING hString, uint32 dwMessageID);

	// Removes all text buttons
	void		RemoveMessageButtons();

	// Height/Width
	int			GetWidth();
	int			GetHeight();

	// Process a key message
    LTBOOL       HandleKeyDown(int key, int rep);

	// Mouse messages
    LTBOOL       OnLButtonDown(int x, int y);
    LTBOOL       OnLButtonUp(int x, int y);
    LTBOOL       OnMouseMove(int x, int y);

protected:
	// Returns the bounding rectangle for a text button at a specific index relative to the
	// message box.
    LTRect       GetTextButtonRect(int nIndex);
	int			GetButtonFromPoint(int x, int y);

protected:
    ILTClient                   *m_pClientDE;

	HSURFACE					m_hBackgroundSurf;	// Background surface
	CLTGUIFont					*m_pFont;			// Font for rendering the text

    LTIntPt                      m_screenPos;        // The position of the message box

	HSTRING						m_hText;			// Text for the message box
    HLTCOLOR                    m_textColor;        // The text color
    HLTCOLOR                    m_hTransColor;      // The transparent color

    HLTCOLOR                    m_textButtonColor;          // The color of the text buttons
    HLTCOLOR                    m_textButtonHighlightColor; // The color of the text buttons when they are higlighted
	int							m_nTextButtonHorzSpace;		// The number of pixels between each text button
	int							m_nTextButtonVertSpace;		// The number of pixels between the bottom of the message box and the bottom of the text button
	CLTGUICommandHandler		*m_pCommandHandler;	// Command handler used for when a key is pressed
	int							m_nMouseDownItemSel;		// The button index that is selected from the current mouse down message

	CMoArray<LTGUIKeyMessage>	m_keyBindings;		// Array of key bindings
	CMoArray<LTGUITextButton>	m_textButtonArray;	// Array of "text buttons" for the message box

	int							m_nSelection;

	LTFLOAT						m_fScale;
};

#endif
