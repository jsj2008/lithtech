//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// WorldClassDlg.h : header file
//


#ifndef __CLASSDLG_H__
#define __CLASSDLG_H__


// Includes....
#include "classtree.h"
#include "classlist.h"
#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// CClassDlg dialog

class CClassDlg : public CDialog
{
// Construction
public:
	CClassDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CClassDlg)
	enum { IDD = IDD_CLASSDLG };
	CClassTree	m_treeClasses;
	CClassList	m_listClasses;
	CListBox	m_listRecent;
	BOOL	m_bShowTree;
	BOOL	m_bBindIndividually;
	//}}AFX_DATA

	// These are to be called before DoModal
	void	SetTitle(CString sTitle)					{ m_sTitleString=sTitle; }
	void	SetProject(CEditProjectMgr *pProject)		{ m_pProject=pProject; }
	void	SetClass(CString sClass)					{ m_sInitialClass=sClass; }
	void	SetEnableBindIndividually(BOOL bEnable)		{ m_bEnableBindIndividually=bEnable; }

	// Access to member variables
	BOOL	GetBindIndividually()						{ return m_bBindIndividually; }

	// Returns the selected class
	CString	GetSelectedClass()							{ return m_sSelectedClass; }
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClassDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CClassDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnCheckShowHierarchy();
	afx_msg void OnKillfocusClasstree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeListClasses();
	afx_msg void OnDblclkListClasses();
	afx_msg void OnDblclkListRecentClasses();
	afx_msg void OnDblclkClasstree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeListRecentClasses();
	virtual void OnOK();
	afx_msg void OnButtonRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Update which controls (tree or list) to display
	void				UpdateControlVisibility();

	// Updates the currently selected class string
	void				UpdateSelectedClass();	

	// Selects a class
	void				SelectClass(CString sClass);

	// Load/Save settings to and from the registry
	void				LoadSettings();
	void				SaveSettings();

	// Builds the recent class array from an array of strings.  The method
	// compares the given strings with the classes defined in the project.
	void				BuildRecentClassArray(CStringArray &srcArray, CStringArray &destArray);

protected:	
	CString				m_sTitleString;				// The title for the dialog
	CEditProjectMgr		*m_pProject;				// The project class
	
	BOOL				m_bEnableBindIndividually;	// True if the "bind individually" checkbox is to be enabled
	CString				m_sInitialClass;			// The initial class that is selected
	CString				m_sSelectedClass;			// The currently selected class

	int					m_nMaxRecentClasses;		// The max number of recent classes that can be in the array
	CStringArray		m_recentClasses;			// Array of strings of the recently added classes
};


#endif  // __CLASSDLG_H__



