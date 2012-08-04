//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_VECTOREDIT_H__BA288E82_0342_11D1_99E3_0060970987C3__INCLUDED_)
#define AFX_VECTOREDIT_H__BA288E82_0342_11D1_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// VectorEdit.h : header file
//

typedef void (*TVectorEditUpdateCallback)(const LTVector& vVec, void* pUserData);

/////////////////////////////////////////////////////////////////////////////
// CVectorEdit dialog

class CVectorEdit : public CDialog
{
// Construction
public:
	CVectorEdit(CWnd* pParent = NULL);   // standard constructor

	void SetCallback(TVectorEditUpdateCallback pCallback, void* pUserData);

// Dialog Data
	//{{AFX_DATA(CVectorEdit)
	enum { IDD = IDD_VECTOR_EDIT };
	CString	m_sVecX;
	CString	m_sVecY;
	CString	m_sVecZ;
	CString m_sIncrement;
	BOOL	m_bAutoApply;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVectorEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVectorEdit)
	afx_msg void OnChangeVectorEdit();
	afx_msg void OnChangeIncrement();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnAutoApply();
	virtual BOOL OnInitDialog();
	afx_msg void OnSpinX(NMHDR * pNotifyStruct, LRESULT * result);
	afx_msg void OnSpinY(NMHDR * pNotifyStruct, LRESULT * result);
	afx_msg void OnSpinZ(NMHDR * pNotifyStruct, LRESULT * result);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Call this before DoModal is called
	void	SetVector(CVector v)	{ m_vVector=v; }

	// Call this after DoModal
	CVector	GetVector()				{ return m_vVector; }

protected:

	// This is called to trim the trailing zeros from a floating point number
	void	TrimZeros(CString &sNumber);

	//handles the spin controls
	void	HandleSpin(DWORD nField, NMHDR * pNotifyStruct);

protected:

	//user callback data
	TVectorEditUpdateCallback	m_UserCallback;
	void*						m_pUserCallbackData;

	float		m_fIncrement;

	//icons for the buttons
	HICON		m_hCopyIcon;
	HICON		m_hPasteIcon;

	// The current vector
	CVector		m_vVector;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VECTOREDIT_H__BA288E82_0342_11D1_99E3_0060970987C3__INCLUDED_)
