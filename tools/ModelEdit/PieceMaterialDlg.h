#if !defined(AFX_PIECEMATERIALDLG_H__ECCECBD7_F78F_42CD_9746_157CE5EAB27A__INCLUDED_)
#define AFX_PIECEMATERIALDLG_H__ECCECBD7_F78F_42CD_9746_157CE5EAB27A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PieceMaterialDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPieceMaterialDlg dialog

// Includes....

#include "model.h"
#include "ModelEditDlg.h"
#include <vector>

using namespace std;


class CPieceMaterialDlg : public CDialog
{
// Construction
public:
	CPieceMaterialDlg( const vector<PieceLODInfo>& selection, CWnd* pParent = NULL );   // standard constructor

	void				UpdateEnabledStatus();

	vector<PieceLODInfo> m_Selection;		// the currently selected piece LODs
	bool m_LODDistChanged;					// set to true on exit if any lod distances have been changed
	float m_StartDist;						// lod distance when dialog is launched

// Dialog Data
	//{{AFX_DATA(CPieceMaterialDlg)
	enum { IDD = IDD_PIECEMATERIAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPieceMaterialDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPieceMaterialDlg)
	afx_msg void OnSelNumTextures();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PIECEMATERIALDLG_H__ECCECBD7_F78F_42CD_9746_157CE5EAB27A__INCLUDED_)
