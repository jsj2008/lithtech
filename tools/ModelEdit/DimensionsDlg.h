#if !defined(AFX_DIMENSIONSDLG_H__FDFC39C1_8E6D_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_DIMENSIONSDLG_H__FDFC39C1_8E6D_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DimensionsDlg.h : header file
//


#include "model.h"


/////////////////////////////////////////////////////////////////////////////
// CDimensionsDlg dialog

class CDimensionsDlg : public CDialog
{
// Construction
public:
	CDimensionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDimensionsDlg)
	enum { IDD = IDD_DIMENSIONS };
	CEdit	m_ZDim;
	CEdit	m_YDim;
	CEdit	m_XDim;
	//}}AFX_DATA


	// Set this before running the dialog.
	Model *m_pModel;
	int m_Anims[100];
	int m_nAnims;

	// put the current dimensions into the edit controls
	void UpdateControls();

	//given an animation and a keyframe, this will find the dims that encompass the
	//model
	static bool FindAnimDims(Model* pModel, uint32 nAnim, uint32 nKeyFrame, LTVector& vDims);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDimensionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDimensionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDone();
	afx_msg void OnUseCurAnim();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// modify the models dimensions based on values in the dialog
	void SetDimensions();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIMENSIONSDLG_H__FDFC39C1_8E6D_11D1_99E4_0060970987C3__INCLUDED_)
