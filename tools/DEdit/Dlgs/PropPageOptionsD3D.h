//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#if !defined(AFX_PROPPAGEOPTIONSD3D_H__32D023E3_E3B9_11D2_BE0D_0060971BDC6D__INCLUDED_)
#define AFX_PROPPAGEOPTIONSD3D_H__32D023E3_E3B9_11D2_BE0D_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageOptionsD3D.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsD3D dialog

class CPropPageOptionsD3D : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsD3D)

// Construction
public:
	CPropPageOptionsD3D();
	~CPropPageOptionsD3D();

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsD3D)
	enum { IDD = IDD_PROPPAGE_DIRECT3D };
	CComboBox	m_comboD3Dmode;
	CComboBox	m_comboD3Ddevices;
	BOOL		m_bBlurryTextures;
	BOOL		m_bSaturateLightmaps;
	BOOL		m_bZBufferLines;
	BOOL		m_bDetailTexEnable;
	BOOL		m_bDetailTexAdditive;
	CString		m_sDetailTexScale;
	CString		m_sDetailTexAngle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptionsD3D)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageOptionsD3D)
	virtual BOOL OnInitDialog();
	afx_msg void GenericUpdateAll();
	afx_msg void OnBlurryTextures();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	//updates everything
	void		UpdateAll(bool bRedrawViews = false);

	// Updates the display options with the current dialog data
	void		UpdateDisplayOptions();

	// Update the enabled status of the controls
	void		UpdateEnabledStatus();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEOPTIONSD3D_H__32D023E3_E3B9_11D2_BE0D_0060971BDC6D__INCLUDED_)
