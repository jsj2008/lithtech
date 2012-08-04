// LTGUIMessageBox.h: interface for the CLTGUIMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUIMESSAGEBOX_H__AF849261_6786_11D2_BDAD_0060971BDC6D__INCLUDED_)
#define AFX_LTGUIMESSAGEBOX_H__AF849261_6786_11D2_BDAD_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUICommandHandler.h"
#include "stdlith.h"

#define LTGUI_MB_CENTER		-1

typedef struct LTGUIKeyMessage_t
{
	int		m_nVKeyCode;
	DDWORD	m_dwMessageID;
} LTGUIKeyMessage;

class CLTGUIMessageBox : public CLTGUICommandHandler  
{
public:
	CLTGUIMessageBox();
	virtual ~CLTGUIMessageBox();

	// Create the message box
	// pClientDE	   - Pointer to the client interface
	// lpszBackground  - Pathname to the pcx file for the background
	// pFont		   - Font which is used to display text in the message box.
	// hText		   - Text to be displayed in the message box
	// pCommandHandler - Command handler which recieves a message when a key (call AddKey first) is pressed
	DBOOL		Create(CClientDE *pClientDE, char *lpszBackground, CLTGUIFont *pFont, HSTRING hText=DNULL, CLTGUICommandHandler *pCommandHandler=DNULL);
	void		Destroy();

	// Render the message box.  Default coordinates center the box along that axis
	void		Render(HSURFACE hDestSurf, int xPos=LTGUI_MB_CENTER, int yPos=LTGUI_MB_CENTER);

	// Set the text for the message box
	void		SetText(HSTRING hString);

	// Sets the text color
	void		SetTextColor(HDECOLOR color)						{ m_textColor=color; }

	// Set the command handler to receive messages from
	void		SetCommandHandler(CLTGUICommandHandler *pHandler)	{ m_pCommandHandler=pHandler; }

	// Add a key and tie it to a message.  The message is sent when the key is passed into HandleKeyDown
	void		AddKey(int nVKeyCode, DDWORD dwMessageID);

	// Removes all keys assignments
	void		RemoveKeys()										{ m_keyBindings.SetSize(0); }

	// Height/Width
	int			GetWidth();
	int			GetHeight();

	// Process a key message
	DBOOL		HandleKeyDown(int key, int rep);

protected:
	CClientDE					*m_pClientDE;
	
	HSURFACE					m_hBackgroundSurf;	// Background surface
	CLTGUIFont					*m_pFont;			// Font for rendering the text
	
	HSTRING						m_hText;			// Text for the message box
	HDECOLOR					m_textColor;		// The text color

	CLTGUICommandHandler		*m_pCommandHandler;	// Command handler used for when a key is pressed
	CMoArray<LTGUIKeyMessage>	m_keyBindings;		// Array of key bindings
};

#endif // !defined(AFX_LTGUIMESSAGEBOX_H__AF849261_6786_11D2_BDAD_0060971BDC6D__INCLUDED_)
