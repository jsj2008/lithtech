//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_ADVANCEDSELECTDLG_H__CFB84172_40DC_11D1_B4A5_00A024805738__INCLUDED_)
#define AFX_ADVANCEDSELECTDLG_H__CFB84172_40DC_11D1_B4A5_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AdvancedSelectDlg.h : header file
//

class CProjectMgr;
class CProjectClass;

/////////////////////////////////////////////////////////////////////////////
// AdvancedSelectDlg dialog

class AdvancedSelectDlg : public CDialog
{
// Construction
public:
	AdvancedSelectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AdvancedSelectDlg)
	enum { IDD = IDD_ADVANCEDSELECT };
	CComboBox	m_comboClass;
	BOOL	m_bNodesOfClass;
	CString	m_ObjectName;
	CString	m_ClassName;
	BOOL	m_bObjectsWithName;
	int		m_nSelect;
	BOOL	m_bMatchWholeWord;
	BOOL	m_bShowResults;
	CComboBox	m_comboPropType;
	CString m_sPropName;
	int		m_nPropType;
	CString m_sPropValue;
	BOOL	m_bNodesWithProperty;
	BOOL	m_bMatchValue;
	//}}AFX_DATA

	void	AddItemsToBox( CMoArray<CProjectClass*> &classes );

	void	OnChangePropType();
	CString	GetFormattingText();
	int		GetPropertyTypeFromIndex(int nIndex);
	bool	IsValueStringValid();
	CBaseProp*	AllocPropertyFromData();

	// Loads/Saves the settings from the registry
	void	LoadRegistrySettings();
	void	SaveRegistrySettings();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AdvancedSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AdvancedSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNodesOfClass();
	afx_msg void OnObjectsWithName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Updates the enabled/disabled status of the controls
	void	UpdateEnabledStatus();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCEDSELECTDLG_H__CFB84172_40DC_11D1_B4A5_00A024805738__INCLUDED_)
