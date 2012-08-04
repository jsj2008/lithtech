#if !defined(AFX_CHOOSECLRANIMDLG_H__E1AB8B01_8876_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_CHOOSECLRANIMDLG_H__E1AB8B01_8876_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseClrAnimDlg.h : header file
//

// Includes....

#include "ChooseClrAnimList.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseClrAnimDlg dialog

class CChooseClrAnimDlg : public CDialog
{
// Construction
public:
	CChooseClrAnimDlg(CLinkList<CK_FAVOURITE *> *pList, CWnd* pParent = NULL);   // standard constructor

	CLinkList<CK_FAVOURITE *>				*m_pList;
	CK_FAVOURITE							*m_pFavourite;

// Dialog Data
	//{{AFX_DATA(CChooseClrAnimDlg)
	enum { IDD = IDD_CHOOSECLRANIM };
	CChooseClrAnimList	m_favourites;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseClrAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseClrAnimDlg)
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDblclkFavourites();
	virtual void OnOK();
	afx_msg void OnDelClrFav();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSECLRANIMDLG_H__E1AB8B01_8876_11D2_9B4A_0060971BDAD8__INCLUDED_)
