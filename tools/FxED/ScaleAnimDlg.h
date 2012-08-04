#if !defined(AFX_SCALEANIMDLG_H__0C360705_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_SCALEANIMDLG_H__0C360705_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScaleAnimDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScaleAnimDlg dialog

class CScaleAnimDlg : public CDialog
{
// Construction
public:
	CScaleAnimDlg(CWnd* pParent = NULL);   // standard constructor

	SK_FAVOURITE				*m_pFavourite;

// Dialog Data
	//{{AFX_DATA(CScaleAnimDlg)
	enum { IDD = IDD_CHOOSESCLANIM };
	CListBox	m_favourites;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScaleAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScaleAnimDlg)
	afx_msg void OnDblclkFavourites();
	afx_msg void OnDelclrfav();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALEANIMDLG_H__0C360705_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_)
