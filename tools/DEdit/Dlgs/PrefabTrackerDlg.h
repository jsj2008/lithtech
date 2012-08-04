#ifndef __PREFABTRACKERDLG_H__
#define __PREFABTRACKERDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CPrefabTrackerDlg dialog

class CPrefabTrackerDlg : public CDialog
{
// Construction
public:
	CPrefabTrackerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefabTrackerDlg)
	enum { IDD = IDD_PREFAB_TRACKER };

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefabTrackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	struct	SPrefabInfo
	{
		CString		m_sFilename;
		uint32		m_nTotalRefCount;
		
		struct SPrefabRef
		{
			CString	m_sFile;
			uint32	m_nRefCount;
		};

		CMoArray<SPrefabRef>	m_References;
	};

	CMoArray<SPrefabInfo>	m_Prefabs;

	//helper functions for the rich edit
	void		ClearText();
	void		AddString(const char* pszStr);
	void		FillEditText();


	CRichEditCtrl*	GetPrefabEdit()		{ return (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT_PREFABTEXT); }
	bool			IsShowReferences()	{ return ((CButton*)GetDlgItem(IDC_CHECK_SHOWFILEREFERENCES))->GetCheck() > 0; }

	// Generated message map functions
	//{{AFX_MSG(CPrefabTrackerDlg)
		afx_msg void OnButtonGenerate();
		afx_msg void OnButtonSave();
		afx_msg void OnCheckShowReferences();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif 
