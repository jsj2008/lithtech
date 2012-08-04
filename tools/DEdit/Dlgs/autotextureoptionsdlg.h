#if !defined(AFX_AUTOTEXTUREOPTIONSDLG_H__BAEE8DC2_B0CC_11D4_B95A_00609709830E__INCLUDED_)
#define AFX_AUTOTEXTUREOPTIONSDLG_H__BAEE8DC2_B0CC_11D4_B95A_00609709830E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// autotextureoptionsdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAutoTextureOptionsDlg dialog

class CAutoTextureOptionsDlg : public CDialog
{
// Construction
public:
	CAutoTextureOptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAutoTextureOptionsDlg)
	enum { IDD = IDD_AUTOTEXTURE_OPTIONS };
	BOOL	m_bOffset;
	BOOL	m_bScale;
	int		m_nStyle;
	BOOL	m_bRestrictDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoTextureOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAutoTextureOptionsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Load the options from the registry
	void LoadOptionsFromReg();
	// Save the options to the registry
	void SaveOptionsToReg();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOTEXTUREOPTIONSDLG_H__BAEE8DC2_B0CC_11D4_B95A_00609709830E__INCLUDED_)
