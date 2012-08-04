#if !defined(AFX_LODEDIT_H__09F74AF1_550E_11D3_99B1_00A0C9696F4D__INCLUDED_)
#define AFX_LODEDIT_H__09F74AF1_550E_11D3_99B1_00A0C9696F4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LODEdit.h : header file
//


#include "newgenlod.h"


/////////////////////////////////////////////////////////////////////////////
// LODEdit dialog

class LODEdit : public CDialog
{
// Construction
public:
	LODEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(LODEdit)
	enum { IDD = IDD_LODEDIT };
	float	m_fDistance;
	float	m_fReduction;
	//}}AFX_DATA


	// Convert our data to and from LODRequestInfos.
	void	ToLOD(Model *pModel, LODRequestInfo *pInfo);
	void	FromLOD(Model *pModel, LODRequestInfo *pInfo);
	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LODEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LODEdit)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LODEDIT_H__09F74AF1_550E_11D3_99B1_00A0C9696F4D__INCLUDED_)
