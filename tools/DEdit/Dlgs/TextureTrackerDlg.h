#ifndef __TEXTURETRACKERDLG_H__
#define __TEXTURETRACKERDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CTextureTrackerDlg dialog

class CTextureTrackerDlg : public CDialog
{
// Construction
public:
	CTextureTrackerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTextureTrackerDlg)

	//note that since the functionality is the same as the prefab tracker we can just use
	//its dialog
	enum { IDD = IDD_PREFAB_TRACKER };

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureTrackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	struct	STextureInfo
	{
		CString		m_sFilename;
		uint32		m_nTotalRefCount;
		
		struct STextureRef
		{
			CString	m_sFile;
			uint32	m_nRefCount;
		};

		CMoArray<STextureRef>	m_References;
	};

	CMoArray<STextureInfo>	m_Textures;

	//helper functions for the rich edit
	void		ClearText();
	void		AddString(const char* pszStr);
	void		FillEditText();


	CRichEditCtrl*	GetTextureEdit()	{ return (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT_PREFABTEXT); }
	bool			IsShowReferences()	{ return ((CButton*)GetDlgItem(IDC_CHECK_SHOWFILEREFERENCES))->GetCheck() > 0; }

	// Generated message map functions
	//{{AFX_MSG(CTextureTrackerDlg)
		afx_msg void OnButtonGenerate();
		afx_msg void OnButtonSave();
		afx_msg void OnCheckShowReferences();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif 
