#if !defined(AFX_EDITOBBDLG_H__26205818_B864_49E9_A8EF_5DE12F00831D__INCLUDED_)
#define AFX_EDITOBBDLG_H__26205818_B864_49E9_A8EF_5DE12F00831D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditOBBDlg.h : header file
//

class CEditOBBDlg;

class COBBTracker : public SolidRectWnd
{
public:
	COBBTracker();
	~COBBTracker();

	CEditOBBDlg* m_ParentDlg;		// the dialog that will have its callback called
	UINT m_CtlID;					// the ID of this control

private:
	bool m_HasCapture;				// true if this tracker currently has the capture
	CPoint m_InitPoint;				// initial point of the mouse down

protected:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnCaptureChanged( CWnd* pWnd );

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CEditOBBDlg dialog

class CEditOBBDlg : public CDialog
{
private :
	SOBB m_OBB ;				// current OBB
	SOBB m_InitOBB;				// initial OBB
	EulerAngles m_CurEA;		// current euler angles
	bool m_InitEnable;			// initial OBB enabled setting
	bool m_Initialized;			// true if the controls in the dialog are initialized

	//bool m_DisableChecked ; // was the disable do-dad checked?
	ModelNode *m_pModelNode;

	COBBTracker trackers[9];

// Construction
public:
	//CEditOBBDlg(const SOBB &obb ,CWnd* pParent = NULL);   // standard constructor
	CEditOBBDlg(ModelNode*pNode ,CWnd* pParent = NULL);   // standard constructor

	void TrackerCallback( UINT ctlID, float offset );		// called by the mouse trackers with value offset

// Dialog Data
	//{{AFX_DATA(CEditOBBDlg)
	enum { IDD = IDD_EDIT_OBB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditOBBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditOBBDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OBBChange( void );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITOBBDLG_H__26205818_B864_49E9_A8EF_5DE12F00831D__INCLUDED_)
