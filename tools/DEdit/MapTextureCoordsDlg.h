#if !defined(AFX_MAPTEXTURECOORDSDLG_H__E21766E1_D02B_11D0_99E3_0060970987C3__INCLUDED_)
#define AFX_MAPTEXTURECOORDSDLG_H__E21766E1_D02B_11D0_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MapTextureCoordsDlg.h : header file
//

class CRegionView;

#include "editpoly.h"

/////////////////////////////////////////////////////////////////////////////
// CMapTextureCoordsDlg dialog

class CMapTextureCoordsDlg : public CDialog
{
// Construction
public:
	CMapTextureCoordsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapTextureCoordsDlg)
	enum { IDD = IDD_MAPTEXTURECOORDS };
	float	m_UOffset;
	float	m_VOffset;
	float	m_UScale;
	float	m_VScale;
	float	m_Rotation;
	//}}AFX_DATA

	BOOL m_bAutoApply;
	int  m_nWindowX;
	int  m_nWindowY;

	//the region in which this brush exists (for undo purposes)
	CRegionView *m_pView;

	//the dimensions of the texture
	int m_TextureWidth, m_TextureHeight;

	// Used to figure out how much to rotate by.
	float m_RefRotation; 

	// The initial U and V values to create deltas
	float m_UStart, m_VStart; 
	
	void DoFlip(DWORD iAxis);
	void ApplyChanges();

private:

	//determine if an undo will be needed or not
	BOOL m_bCreateUndo;

	//determine if this dialog has been initialized yet....this is to avoid
	//problems with windows sending update messages too early
	BOOL m_bInitialized;

	void SetupUndo();

	BOOL OnInitDialog();
	void OnOK();
	void OnCancel();

	bool	m_bUMirror, m_bVMirror;

	CMoArray<LTVector>	m_OriginalOPQs;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapTextureCoordsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//reads the values from the controls into the internal member variables
	//but doesn't use DDX so dialogs will not pop up to prompt for numbers, etc
	void UpdateDataNoDDX();

	// Generated message map functions
	//{{AFX_MSG(CMapTextureCoordsDlg)
	afx_msg void OnApply();
	afx_msg void OnAutoApply();
	afx_msg void OnFlipX();
	afx_msg void OnFlipY();
	afx_msg void GenericChangeHandler();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPTEXTURECOORDSDLG_H__E21766E1_D02B_11D0_99E3_0060970987C3__INCLUDED_)
