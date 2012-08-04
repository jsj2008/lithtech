//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_ALPHAFROMCOLORDLG_H__65AF7BD2_47BA_11D2_A81F_006097726515__INCLUDED_)
#define AFX_ALPHAFROMCOLORDLG_H__65AF7BD2_47BA_11D2_A81F_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AlphaFromColorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AlphaFromColorDlg dialog

class AlphaFromColorDlg : public CDialog
{
// Construction
public:
	AlphaFromColorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AlphaFromColorDlg)
	enum { IDD = IDD_ALPHAFROMCOLOR };
	CSpinButtonCtrl	m_ScaleSpin;
	CSpinButtonCtrl	m_OffsetSpin;
	CString	m_Offset;
	CString	m_Scale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AlphaFromColorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AlphaFromColorDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALPHAFROMCOLORDLG_H__65AF7BD2_47BA_11D2_A81F_006097726515__INCLUDED_)
