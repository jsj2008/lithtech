#if !defined(AFX_ADVANCEDSELECTPROPERTYDLG_H__10CBBDA0_5117_11D3_A61A_0060971BDC6D__INCLUDED_)
#define AFX_ADVANCEDSELECTPROPERTYDLG_H__10CBBDA0_5117_11D3_A61A_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvancedSelectPropertyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSelectPropertyDlg dialog

class CAdvancedSelectPropertyDlg : public CDialog
{
// Construction
public:
	CAdvancedSelectPropertyDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAdvancedSelectPropertyDlg)
	enum { IDD = IDD_ADVANCEDSELECT_PROPERTY };
	CString	m_sName;
	CString	m_sValue;
	int		m_nSelPropertyIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvancedSelectPropertyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	// This allocates a property object and fills it in with the
	// data from the dialog box.  The function returns NULL if
	// a valid property cannot be made.
	CBaseProp	*AllocPropertyFromData();

// Implementation
protected:
	// Returns the property type for a specific index in the combo box
	int			GetPropertyTypeFromIndex(int nIndex);

	// Returns the formatting text for the selected property type
	CString		GetSelectedFormattingText();

	// This returns TRUE if the value text is valid with the
	// selected property type.
	BOOL		IsValueStringValid();

	// This updates the enabled status of controls in the dialog
	// depending on the values entered into the controls.
	void		UpdateEnabledStatus();

	// Generated message map functions
	//{{AFX_MSG(CAdvancedSelectPropertyDlg)
	afx_msg void OnSelchangeComboPropertyTypes();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditPropertyValue();
	afx_msg void OnChangeEditPropertyName();
	afx_msg void OnButtonColorPick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCEDSELECTPROPERTYDLG_H__10CBBDA0_5117_11D3_A61A_0060971BDC6D__INCLUDED_)
