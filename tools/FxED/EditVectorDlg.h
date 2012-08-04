#if !defined(AFX_EDITVECTORDLG_H__7DE1CCE1_D56C_11D2_9B4F_0060971BDAD8__INCLUDED_)
#define AFX_EDITVECTORDLG_H__7DE1CCE1_D56C_11D2_9B4F_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditVectorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditVectorDlg dialog

class CEditVectorDlg : public CDialog
{
// Construction
public:
	CEditVectorDlg(float *pfVec, CWnd* pParent = NULL);   // standard constructor

	float					m_fPitch;
	float					m_fYaw;
	CFXVector					m_vec;
	CFXVector					m_tran;
	
// Dialog Data
	//{{AFX_DATA(CEditVectorDlg)
	enum { IDD = IDD_EDITVECTOR };
	CStatic	m_pitch;
	CStatic	m_yaw;
	float	m_magnitude;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditVectorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditVectorDlg)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchangePresets();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITVECTORDLG_H__7DE1CCE1_D56C_11D2_9B4F_0060971BDAD8__INCLUDED_)
