//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_SCALESELECTDLG_H__CFB84172_40DC_11D1_B4A5_00A024805738__INCLUDED_)
#define AFX_SCALESELECTDLG_H__CFB84172_40DC_11D1_B4A5_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ScaleSelectDlg.h : header file
//

class CProjectMgr;
class CProjectClass;

/////////////////////////////////////////////////////////////////////////////
// ScaleSelectDlg dialog

class CScaleSelectDlg : public CDialog
{
// Construction
public:

	//the different scale modes
	enum	{		SCALE_PERCENT,
					SCALE_WORLD_UNITS
			};

	CScaleSelectDlg(CWnd* pParent = NULL);   // standard constructor

	void SetValues(CVector *dims) { m_SelectionSize = dims; } // set the size of the selection

// Dialog Data
	//{{AFX_DATA(ScaleSelectDlg)
	enum { IDD = IDD_SCALESELECT };
	int			m_nScaleXAmount;
	int			m_nScaleYAmount;
	int			m_nScaleZAmount;
	int			m_nScaleMode;
	BOOL		m_bKeepUniform;
	BOOL		m_bKeepTextures;

	//}}AFX_DATA

	void	AddItemsToBox( CMoArray<CProjectClass*> &classes );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScaleSelectDlg)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ScaleSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnXAmount();
	afx_msg void OnYAmount();
	afx_msg void OnZAmount();
	afx_msg void OnChangeScaleMode();
	afx_msg void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// Updates the enabled/disabled status of the controls
	void	UpdateEnabledStatus();

private:

	CVector *m_SelectionSize; // a vector of three ints, showing selection size

	int  m_LastUpdated;   // which of the three axes was updated last (0=x, 1=y, etc)
	int  m_HasFocus;	  // which of the three axes has input focus  (0=x, 1=y, etc)
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALESELECTDLG_H__CFB84172_40DC_11D1_B4A5_00A024805738__INCLUDED_)
