#if !defined(AFX_SELECTCONFIGDLG_H__F3DCDFB1_C80A_41BE_AD06_F1577BE6E79B__INCLUDED_)
#define AFX_SELECTCONFIGDLG_H__F3DCDFB1_C80A_41BE_AD06_F1577BE6E79B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectConfigDlg.h : header file
//

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectConfigDlg dialog

class CSelectConfigDlg : public CDialog
{
// Construction
public:
	CSelectConfigDlg(CWnd* pParent = NULL);   // standard constructor

	// Returns the selected config.
	CString GetSelectedConfig( ) { return m_sConfig; }

// Dialog Data
	//{{AFX_DATA(CSelectConfigDlg)
	enum { IDD = IDD_SELECTCONFIG };
	CButton	m_OKCtrl;
	CComboBox	m_ConfigCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Adds all the config files to the combobox.
	BOOL AddConfigFilesToControl( );

	// Generated message map functions
	//{{AFX_MSG(CSelectConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString m_sConfig;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTCONFIGDLG_H__F3DCDFB1_C80A_41BE_AD06_F1577BE6E79B__INCLUDED_)
