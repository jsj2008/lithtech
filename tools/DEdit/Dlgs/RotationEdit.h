//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_ROTATIONEDIT_H__5999A6E2_4297_11D1_B4A7_00A024805738__INCLUDED_)
#define AFX_ROTATIONEDIT_H__5999A6E2_4297_11D1_B4A7_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RotationEdit.h : header file
//


#include "resource.h"


typedef void (*TRotationEditUpdateCallback)(const LTVector& vEuler, void* pUserData);

/////////////////////////////////////////////////////////////////////////////
// CRotationEdit dialog

class CRotationEdit : public CDialog
{
// Construction
public:
	CRotationEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRotationEdit)
	enum { IDD = IDD_ROTATIONEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotationEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	LTVector	m_EulerAngles;
	CScrollBar	*m_pYawBar, *m_pPitchBar, *m_pRollBar;
	BOOL		m_bAutoApply;

	void UpdateScrollTexts();
	void DrawGraphical();
	void DrawAngleToWnd(CWnd *pWnd, float radians);
	void SetUserCallback(TRotationEditUpdateCallback CallbackFn, void* pUserData);

// Implementation
protected:

	TRotationEditUpdateCallback		m_UserCallback;
	void*							m_pUserData;

	void DoCallback();

	//icons for the buttons
	HICON		m_hCopyIcon;
	HICON		m_hPasteIcon;

	//this boolean is used to indicate that we do not want to listen to the changes
	//being made to the edit boxes (probably because we are making the changes)
	bool	m_bIgnoreEditChanges;

	//given the ID of an edit control, it will get its floating point value
	float	GetEditValue(DWORD nID);

	// Generated message map functions
	//{{AFX_MSG(CRotationEdit)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg void OnEditAngleChanged();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnAutoApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROTATIONEDIT_H__5999A6E2_4297_11D1_B4A7_00A024805738__INCLUDED_)
