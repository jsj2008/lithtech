#if !defined(AFX_WEIGHTEDITDLG_H__4FB2F7D1_1DD4_11D3_999C_00A0C9696F4D__INCLUDED_)
#define AFX_WEIGHTEDITDLG_H__4FB2F7D1_1DD4_11D3_999C_00A0C9696F4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WeightEditDlg.h : header file
//

#include "model.h"


class CModelEditDlg;


/////////////////////////////////////////////////////////////////////////////
// WeightEditDlg dialog


class WeightEditDlg : public CDialog
{
// Construction
public:
	WeightEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(WeightEditDlg)
	enum { IDD = IDD_WEIGHTEDIT };
	CEdit	m_Weight;
	CListBox	m_NodeList;
	CListBox	m_WeightSets;
	//}}AFX_DATA

	void		FillWeightSetList(BOOL bPreserveSel);
	void		FillNodeList(BOOL bPreserveSel);

	WeightSet*	GetCurSelections(int *pNodes, DWORD nodeListSizeBytes, int &nNodes);
	void		HandleSelChange();

	
	CModelEditDlg	*m_pDlg;
	Model			*m_pModel;
	BOOL			m_bUpdateWeights;
	
	// This tells if any changes were made.
	BOOL			m_bChangesMade;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WeightEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WeightEditDlg)
	afx_msg void OnAddSet();
	afx_msg void OnRemoveSet();
	afx_msg void OnUpdateWeight();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeNodes();
	afx_msg void OnSelchangeWeightsets();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEIGHTEDITDLG_H__4FB2F7D1_1DD4_11D3_999C_00A0C9696F4D__INCLUDED_)
