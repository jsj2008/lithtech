#if !defined(AFX_WEIGHTSETSELECTDLG_H__4656181C_77EB_4A8D_A0D2_AC565B28F2DA__INCLUDED_)
#define AFX_WEIGHTSETSELECTDLG_H__4656181C_77EB_4A8D_A0D2_AC565B28F2DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WeightSetSelectDlg.h : header file
//


#include "model.h"


/////////////////////////////////////////////////////////////////////////////
// WeightSetSelectDlg dialog

class WeightSetSelectDlg : public CDialog
{
// Construction
public:
	WeightSetSelectDlg(Model *pModel, DWORD iStartingSet, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(WeightSetSelectDlg)
	enum { IDD = IDD_WEIGHTSETSELECT };
	CListBox	m_WeightSets;
	//}}AFX_DATA


	// The set they selected or INVALID_MODEL_WEIGHTSET if none.
	DWORD		m_SelectedSet;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WeightSetSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	Model		*m_pModel;

	// Generated message map functions
	//{{AFX_MSG(WeightSetSelectDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEIGHTSETSELECTDLG_H__4656181C_77EB_4A8D_A0D2_AC565B28F2DA__INCLUDED_)
