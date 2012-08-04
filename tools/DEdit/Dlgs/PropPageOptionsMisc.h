#ifndef __PROPPAGEOPTIONSMISC_H__
#define __PROPPAGEOPTIONSMISC_H__

class CPropPageOptionsMisc : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsMisc)

// Construction
public:
	CPropPageOptionsMisc();
	~CPropPageOptionsMisc();

	//saves the options back into the global options database
	void SaveOptions();

	//handles updating the enabled status of the controls
	void UpdateEnableStatus();

	//saves the data, and updates the enabled states
	void UpdateAll();

	DWORD	 m_dwOriginalUndos; // How large was the undo buffer when the dialog was opened?	

	bool	DidUndoValueChange() const	{ return m_dwUndos != m_dwOriginalUndos; }
	//void	SetNumUndos(DWORD num)		{ m_dwUndos = m_dwOriginalUndos = num; }

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsModels)
	enum { IDD = IDD_PROPPAGE_MISC };
	BOOL	m_bParentFolder;
	BOOL	m_bShowIcons;
	BOOL	m_bAutoExtractIcons;
	BOOL	m_bAutoExtractHelp;
	BOOL	m_bShowThumbnails;
	BOOL	m_bShowUndoWarnings;
	BOOL	m_bShowFullPath;
	BOOL	m_bAutoLoadProj;
	BOOL	m_bDefaultCompressed;
	BOOL	m_bLoadLYTFile;
	BOOL	m_bUndoFreezeHide;
	DWORD	m_dwUndos;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptionsRun)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageOptionsRun)
	virtual BOOL OnInitDialog();
	afx_msg void DefaultHandler();
	afx_msg void OnSetShowIcons();
	afx_msg void OnSetShowThumbnails();
	afx_msg void OnSetShowUndoWarnings();
	afx_msg void OnOK();
	afx_msg void OnNumUndo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif 
