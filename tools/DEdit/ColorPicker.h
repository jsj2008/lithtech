//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ColorPicker.h: interface for the CColorPicker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLORPICKER_H__B621A034_8BB8_11D2_BDD9_0060971BDC6D__INCLUDED_)
#define AFX_COLORPICKER_H__B621A034_8BB8_11D2_BDD9_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CColorPicker : public CColorDialog  
{
public:	
	CColorPicker(COLORREF clrInit = 0, DWORD dwFlags = 0, CWnd* pParentWnd = NULL)
	{
		CColorDialog(clrInit, dwFlags, pParentWnd);
	}

	virtual ~CColorPicker();

	// Initializes the color picker by loading the custom colors
	// from the registry.
	BOOL	Init();

	// Sets the current color.  Call this before calling DoModal
	void	SetCurrentColor(COLORREF color);

	// DoModal override which saves the custom colors to the registry
	// before returning.
	int		DoModal();
	
protected:
	COLORREF	m_customColors[16];		// Custom colors
};

#endif // !defined(AFX_COLORPICKER_H__B621A034_8BB8_11D2_BDD9_0060971BDC6D__INCLUDED_)
