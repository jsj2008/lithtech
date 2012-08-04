// LTGUIColumnTextCtrl.h: interface for the CLTGUIColumnTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUICOLUMNTEXTCTRL_H__0D6ABE62_5C86_11D2_BDA2_0060971BDC6D__INCLUDED_)
#define AFX_LTGUICOLUMNTEXTCTRL_H__0D6ABE62_5C86_11D2_BDA2_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "LTGUICtrl.h"

struct LTGUIColumn
{
	LTGUIColumn()
	{
		m_nWidth=0;
		m_hString=DNULL;
		m_nJustify=CF_JUSTIFY_LEFT;
	}

	int			m_nWidth;		// Width of column
	HSTRING		m_hString;		// Text string for the column
	short		m_nJustify;		// CF_JUSTIFY_LEFT, CF_JUSTIFY_CENTER, CF_JUSTIFY_RIGHT
};

class CLTGUIColumnTextCtrl : public CLTGUIFadeColorCtrl
{
public:
	CLTGUIColumnTextCtrl();
	virtual ~CLTGUIColumnTextCtrl();

	// Create the control
	// pClientDE	- Pointer to the client interface.
	// dwCommandID	- The command ID which is sent when "enter" is pressed.	
	// pFont		- The font to use for rendering the strings.
	// pMessageCtrl - control which receives messages when the "enter" key is pressed.
	virtual DBOOL	Create ( CClientDE *pClientDE, DDWORD dwCommandID, CLTGUIFont *pFont,
							 CLTGUICommandHandler *pCommandHandler );

	// Destroys the control
	void			Destroy ( );

	// Adds a column to the control
	// nWidth	  - Width of the column
	// hString	  - The initial text for the column
	DBOOL			AddColumn(HSTRING hString, int nWidth, short nJustify=CF_JUSTIFY_LEFT);
	DBOOL			AddColumn(int messageCode, int nWidth, short nJustify=CF_JUSTIFY_LEFT);

	// Gets a string at a specific column index.  This returns a copy (new handle).
	HSTRING			GetString(int nColumnIndex);

	// Sets a string for a column.  This copies the string from hString to an internal string.
	void			SetString(int nColumnIndex, HSTRING hString);
	void			SetString(int nColumnIndex, int messageCode);

	void			RemoveColumn(int nIndex);			// Removes a column
	void			RemoveAllColumns();					// Removes all of the columns

	// Return the number of columns
	int				GetNumColumns()						{ return m_columnArray.GetSize(); }	

	// Render the control
	void			Render ( HSURFACE hDestSurf );

	// Set the font
	void			SetFont ( CLTGUIFont *pFont );	

	// Width/Height calculations
	int				GetWidth ( )						{ return m_dwWidth; }
	int				GetHeight ( )						{ return m_dwHeight; }

	// Handle the Enter key being pressed
	void			OnEnter ( );	

protected:
	// The selection for this control has changed	
	void			OnSelChange();

protected:		
	DDWORD			m_dwCommandID;			// The command which is sent when "enter" is pressed	
	
	CLTGUIFont		*m_pFont;				// The font for this control
	
	DDWORD			m_dwWidth;				// The width of the control
	DDWORD			m_dwHeight;				// The height of the control
	
	CMoArray<LTGUIColumn *>	m_columnArray;	// The array of columns	

	// Receives a message when the "enter" key is pressed.
	CLTGUICommandHandler	*m_pCommandHandler;
};

#endif // !defined(AFX_LTGUICOLUMNTEXTCTRL_H__0D6ABE62_5C86_11D2_BDA2_0060971BDC6D__INCLUDED_)
