#ifndef __OBJECTTRACKERDLG_H__
#define __OBJECTTRACKERDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CObjectTrackerDlg dialog

class CObjectTrackerDlg : public CDialog
{
// Construction
public:
	CObjectTrackerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CObjectTrackerDlg)
	enum { IDD = IDD_PREFAB_TRACKER };

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectTrackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	struct	SObjectInfo
	{
		CString		m_sClass;
		uint32		m_nTotalRefCount;
		
		struct SObjectRef
		{
			CString	m_sFile;
			uint32	m_nRefCount;
		};

		CMoArray<SObjectRef>	m_References;
	};

	struct SPrefabInfo
	{
		CString		m_sPrefab;
		uint32		m_nNumRefs;
	};

	CMoArray<SObjectInfo>	m_Objects;

	//helper functions for the rich edit
	void		ClearText();
	void		AddString(const char* pszStr);
	void		FillEditText();
	void		AddWorldObjects(const char* pszWorld, uint32 nScale);


	CRichEditCtrl*	GetObjectEdit()		{ return (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT_PREFABTEXT); }
	bool			IsShowReferences()	{ return ((CButton*)GetDlgItem(IDC_CHECK_SHOWFILEREFERENCES))->GetCheck() > 0; }

	// Generated message map functions
	//{{AFX_MSG(CObjectTrackerDlg)
		afx_msg void OnButtonGenerate();
		afx_msg void OnButtonSave();
		afx_msg void OnCheckShowReferences();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif 
