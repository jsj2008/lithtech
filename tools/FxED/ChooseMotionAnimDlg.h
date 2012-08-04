#if !defined(AFX_CHOOSEMOTIONANIMDLG_H__6001E8C1_A6F2_11D2_9B4D_0060971BDAD8__INCLUDED_)
#define AFX_CHOOSEMOTIONANIMDLG_H__6001E8C1_A6F2_11D2_9B4D_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseMotionAnimDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseMotionAnimDlg dialog

class CChooseMotionAnimDlg : public CDialog
{
// Construction
public:
	CChooseMotionAnimDlg(CWnd* pParent = NULL);   // standard constructor

	MK_FAVOURITE				*m_pFav;

// Dialog Data
	//{{AFX_DATA(CChooseMotionAnimDlg)
	enum { IDD = IDD_CHOOSEMVANIM };
	CListBox	m_favourites;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseMotionAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseMotionAnimDlg)
	afx_msg void OnDelclrfav();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEMOTIONANIMDLG_H__6001E8C1_A6F2_11D2_9B4D_0060971BDAD8__INCLUDED_)
