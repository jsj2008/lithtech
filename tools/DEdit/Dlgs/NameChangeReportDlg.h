#if !defined(AFX_NAMECHANGEREPORTDLG_H__777AF9A3_07E4_11D3_BE22_0060971BDC6D__INCLUDED_)
#define AFX_NAMECHANGEREPORTDLG_H__777AF9A3_07E4_11D3_BE22_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NameChangeReportDlg.h : header file
//

#include "columnctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CNameChangeReportDlg dialog

class CNameChangeReportDlg : public CDialog
{
// Construction
public:
	CNameChangeReportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNameChangeReportDlg)
	enum { IDD = IDD_NAME_CHANGE_REPORT };
	CColumnCtrl	m_listProperties;
	CColumnCtrl	m_listNames;
	//}}AFX_DATA

	// Sets the name arrays.  Call this before calling DoModal
	void	SetObjectNameArrays(CStringArray *pOriginal, CStringArray *pUpdated);

	// Sets the property name arrays.  Call this before calling DoModal
	void	SetPropertyNameArrays(CStringArray *pNameArray, CStringArray *pOriginal, CStringArray *pUpdated);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNameChangeReportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNameChangeReportDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// This fills in the object name column control
	void				BuildObjectNameCtrl(CColumnCtrl &columnCtrl, CStringArray *pOriginal, CStringArray *pUpdated);

	// This fills in the property value column control
	void				BuildPropertyValueCtrl(CColumnCtrl &columnCtrl, CStringArray *pNameArray, CStringArray *pOriginal, CStringArray *pUpdated);

protected:
	// These store the pointers to the object name changes
	CStringArray		*m_pOriginalNameArray;
	CStringArray		*m_pUpdatedNameArray;

	// These store the pointers to the property name changes
	CStringArray		*m_pPropertyNameArray;				// The property name that has had its value change
	CStringArray		*m_pPropertyOriginalValueArray;		// The original value for the property
	CStringArray		*m_pPropertyUpdatedValueArray;		// The new value for the property
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NAMECHANGEREPORTDLG_H__777AF9A3_07E4_11D3_BE22_0060971BDC6D__INCLUDED_)
