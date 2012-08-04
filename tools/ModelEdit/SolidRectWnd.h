#if !defined(AFX_SOLIDRECTWND_H__97897E7F_394C_4FBE_B1DB_71DECE0AE183__INCLUDED_)
#define AFX_SOLIDRECTWND_H__97897E7F_394C_4FBE_B1DB_71DECE0AE183__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SolidRectWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SolidRectWnd window

class SolidRectWnd : public CWnd
{
// Construction
public:
	SolidRectWnd();

// Attributes
public:


protected:

	COLORREF		m_Color;
	HWND			m_hParentWnd;


// Operations
public:

	// Set the rectangle's color.  It's fast to call this with the same color
	// multiple times - it checks if the color is different before redrawing.
	void			SetColor(COLORREF color);

	// You must call this if you want it to forward mouse messages to its parent window.
	void			SetForward(BOOL bForward);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SolidRectWnd)
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~SolidRectWnd();

	// Translate from our client space to the parent window's client space and 
	// send the message to the parent window.
	void	SendMouseMessageToParent(UINT msgID, WPARAM nFlags, CPoint point);

	// Generated message map functions
protected:
	//{{AFX_MSG(SolidRectWnd)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOLIDRECTWND_H__97897E7F_394C_4FBE_B1DB_71DECE0AE183__INCLUDED_)
