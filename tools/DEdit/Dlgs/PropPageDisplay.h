//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_PROPPAGEDISPLAY_H__5E3D9E54_D8AA_11D2_BE02_0060971BDC6D__INCLUDED_)
#define AFX_PROPPAGEDISPLAY_H__5E3D9E54_D8AA_11D2_BE02_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageDisplay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropPageDisplay dialog

class CPropPageDisplay : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageDisplay)

// Construction
public:
	CPropPageDisplay();
	~CPropPageDisplay();

// Dialog Data
	//{{AFX_DATA(CPropPageDisplay)
	enum { IDD = IDD_PROPPAGE_DISPLAY };
	CSpinButtonCtrl	m_spinVertexSize;
	CSpinButtonCtrl	m_spinHandleSize;
	CSpinButtonCtrl m_spinClassIconSize;
	CListBox	m_listColorChoices;
	BOOL	m_bShowSurfaceColor;
	BOOL	m_bOrientObjectBoxes;
	BOOL	m_bShowSelectedDecals;
	int		m_nHandleSize;
	int		m_nVertexSize;
	int		m_nVertexDrawRule;
	int		m_nPerspectiveFarZ;
	int		m_nClassIconSize;
	BOOL	m_bTintSelected;
	BOOL	m_bTintFrozen;
	BOOL	m_bShadePolygons;
	BOOL	m_bShowClassIcons;
	CString	m_sClassIconDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageDisplay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageDisplay)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonChangeColor();
	afx_msg void OnPaint();
	afx_msg void OnSelchangeListColorChoices();
	afx_msg void OnDblclkListColorChoices();
	afx_msg void OnStaticColorBox();
	afx_msg void OnDestroy();
	afx_msg void OnButtonReset();
	afx_msg void UpdateDataHandler();
	afx_msg void UpdateDataHandlerAndDraw();
	afx_msg void OnBrowseClassIcons();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void	UpdateDisplayOptions();		// Updates the display options class with the dialog data

protected:
	BOOL	m_bInit;	// Indicates if the dialog has been initialized
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEDISPLAY_H__5E3D9E54_D8AA_11D2_BE02_0060971BDC6D__INCLUDED_)
