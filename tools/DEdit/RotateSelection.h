//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_ROTATESELECTION_H__1F63EAA1_F9D0_11D0_99E3_0060970987C3__INCLUDED_)
#define AFX_ROTATESELECTION_H__1F63EAA1_F9D0_11D0_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RotateSelection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRotateSelection dialog

class CRotateSelection : public CDialog
{
// Construction
public:
	CRotateSelection(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRotateSelection)
	enum { IDD = IDD_ROTATE_SELECTION };
	float	m_rRotation;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotateSelection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRotateSelection)
	afx_msg void OnButtonRotate45();
	afx_msg void OnButtonRotate90();
	afx_msg void OnButtonRotate180();
	afx_msg void OnButtonRotateNeg45();
	afx_msg void OnButtonRotateNeg90();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROTATESELECTION_H__1F63EAA1_F9D0_11D0_99E3_0060970987C3__INCLUDED_)
