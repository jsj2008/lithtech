#if !defined(AFX_POPUPNOTEDLG_H__2593B752_FD91_11D2_BE1C_0060971BDC6D__INCLUDED_)
#define AFX_POPUPNOTEDLG_H__2593B752_FD91_11D2_BE1C_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PopupNoteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupNoteDlg dialog

class CPopupNoteDlg : public CDialog
{
// Construction
public:
	CPopupNoteDlg(CWnd* pParent = NULL);   // standard constructor

	// Creates the popup note dialog with the specified string as the text.
	BOOL		CreateDlg(CString sNoteText, int nDialogWidth=300, CWnd *pParentWindow=NULL);
	
// Dialog Data
	//{{AFX_DATA(CPopupNoteDlg)
	enum { IDD = IDD_POPUP_NOTE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupNoteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPopupNoteDlg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Updates the window position.  A DC is needed to calculate the size of the text
	void		UpdateWindowPos(CDC *pDC);

protected:	
	CFont		m_font;				// The font that is used to display the text
	CString		m_sNoteText;		// The text that is to be displayed in the note
	int			m_nDialogWidth;		// The width of the dialog in pixels.
	CPoint		m_initialCursorPos;	// The initial cursor position which is used to position the window
	int			m_nBorderSize;		// The size of the border (in pixels) between the edge of the window and the text
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPNOTEDLG_H__2593B752_FD91_11D2_BE1C_0060971BDC6D__INCLUDED_)
