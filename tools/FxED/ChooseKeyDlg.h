#if !defined(AFX_CHOOSEKEYDLG_H__2CDEAF83_9394_11D2_9B4C_0060971BDAD8__INCLUDED_)
#define AFX_CHOOSEKEYDLG_H__2CDEAF83_9394_11D2_9B4C_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseKeyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseKeyDlg dialog

class CChooseKeyDlg : public CDialog
{
// Construction
public:

	CChooseKeyDlg(CWnd* pParent = NULL);   // standard constructor


	FK_FAVOURITE						*m_pFav;

// Dialog Data
	//{{AFX_DATA(CChooseKeyDlg)
	enum { IDD = IDD_CHOOSESFAVKEY };
	CListBox	m_favourites;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseKeyDlg)
	afx_msg void OnDelFavKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEKEYDLG_H__2CDEAF83_9394_11D2_9B4C_0060971BDAD8__INCLUDED_)
