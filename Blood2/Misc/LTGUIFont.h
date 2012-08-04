// LTGUIFont.h: interface for the CLTGUIFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUIFONT_H__30D313E1_736B_11D2_BDBE_0060971BDC6D__INCLUDED_)
#define AFX_LTGUIFONT_H__30D313E1_736B_11D2_BDBE_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Duel purpose font class.  It can either be inited to use an english bitmap font (coolfont)
// or use an engine font for other languages.
class CLTGUIFont  
{
public:
	CLTGUIFont();
	virtual ~CLTGUIFont();

	// Initialize a bitmap font
	DBOOL	Init(CClientDE *pClientDE, char *lpszBitmapFile, char *lpszFontWidths);

	// Initialize an engine font
	DBOOL	Init(CClientDE *pClientDE, char *pFontName, int nWidth, int nHeight, 
				 DBOOL bItalic, DBOOL bUnderline, DBOOL bBold);

	// Termination
	void	Term();

	// Drawing functions.  Note that engine fonts are always solid.  Calling Draw or DrawFormat
	// on an engine font will just draw the text white.
	void	Draw(HSTRING hString, HSURFACE hDest, int x, int y, int justify);
	void	DrawFormat(HSTRING hString, HSURFACE hDest, int x, int y, int nWidth);	
	void	DrawSolid(HSTRING hString, HSURFACE hDest, int x, int y, int justify, HDECOLOR color);
	void	DrawSolidFormat(HSTRING hString, HSURFACE hDest, int x, int y, int nWidth, HDECOLOR color);

	// Returns the height and width of a text string
	DIntPt	GetTextExtents(HSTRING hString);
	DIntPt	GetTextExtentsFormat(HSTRING hString, int nWidth);

	// Returns the height of the font
	int		GetHeight();

	// Sets the wrapping method between using spaces and non-space based wrapping (Japanese)
	// Note that non-space based wrapping only works with engine fonts.
	static	void	SetWrapMethod(DBOOL bUseSpaces)		{ s_bWrapUseSpaces=bUseSpaces; }

protected:
	CClientDE	*m_pClientDE;	// Pointer to the client interface
	DBOOL		m_bBitmap;		// True if this is a bitmap font
	
	CoolFont	*m_pCoolFont;	// Pointer to the bitmap font
	HDEFONT		m_hFont;		// Handle to an "engine" font
	int			m_nFontHeight;	// The height passed in when creating an engine font.

	static DBOOL	s_bWrapUseSpaces;	// TRUE if the wrapping method is to use a space as a word separator
};

#endif // !defined(AFX_LTGUIFONT_H__30D313E1_736B_11D2_BDBE_0060971BDC6D__INCLUDED_)
