//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_NAVIGATORDLG_H__2D646A92_C779_11D2_BDF3_0060971BDC6D__INCLUDED_)
#define AFX_NAVIGATORDLG_H__2D646A92_C779_11D2_BDF3_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// navigatordlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNavigatorDlg dialog

class CNavigatorDlg : public CDialog
{
// Construction
public:
	CNavigatorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNavigatorDlg)
	enum { IDD = IDD_NAVIGATORDLG };
	CListBox	m_listNavigatorItems;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNavigatorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNavigatorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonGoto();
	afx_msg void OnButtonMoveUp();
	afx_msg void OnButtonMoveDown();
	afx_msg void OnButtonRename();
	afx_msg void OnButtonDelete();
	afx_msg void OnDblclkListNavigatorItems();
	afx_msg void OnSelchangeListNavigatorItems();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Set the array pointer for the navigation array
	void	SetNavigatorPosArray(CNavigatorPosArray *pArray)	{ m_pNavigatorPosArray=pArray; }

	// Set the pointer for the region view
	void	SetRegionView(CRegionView *pView)					{ m_pRegionView=pView; }

protected:
	// Build the navigator list box
	void	BuildNavigatorList();

	// Update the enabled status of the buttons
	void	UpdateEnabledStatus();

protected:
	// Pointer to the navigation array
	CNavigatorPosArray	*m_pNavigatorPosArray;

	// Pointer to the region view
	CRegionView			*m_pRegionView;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NAVIGATORDLG_H__2D646A92_C779_11D2_BDF3_0060971BDC6D__INCLUDED_)
