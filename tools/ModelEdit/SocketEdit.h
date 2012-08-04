#if !defined(AFX_SOCKETEDIT_H__DE978DE7_9C4F_11D3_B959_00609709830E__INCLUDED_)
#define AFX_SOCKETEDIT_H__DE978DE7_9C4F_11D3_B959_00609709830E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SocketEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSocketEdit dialog

class CSocketEdit : public CDialog
{
// Construction
public:
	CSocketEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSocketEdit)
	enum { IDD = IDD_SOCKETEDIT };
	CButton	m_cApply;
	CString	m_Attachment;
	CString	m_Name;
	CString m_NodeName;
	float	m_PosX;
	float	m_PosY;
	float	m_PosZ;
	float	m_RotX;
	float	m_RotY;
	float	m_RotZ;
	float	m_SclX;
	float	m_SclY;
	float	m_SclZ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSocketEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSocketEdit)
	afx_msg void OnAttachmentLoad();
	afx_msg void OnApply();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOCKETEDIT_H__DE978DE7_9C4F_11D3_B959_00609709830E__INCLUDED_)
