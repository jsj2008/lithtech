#ifndef __PROPPAGEOPTIONSMODELS_H__
#define __PROPPAGEOPTIONSMODELS_H__

class CPropPageOptionsModels : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsModels)

// Construction
public:
	CPropPageOptionsModels();
	~CPropPageOptionsModels();

	//saves the options back into the global options database
	void SaveOptions();

	//handles updating the enabled status of the controls
	void UpdateEnableStatus();

	//saves the data, and updates the enabled states
	void UpdateAll();

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsModels)
	enum { IDD = IDD_PROPPAGE_MODELS };
	CSpinButtonCtrl	m_spinMaxMemoryUsage;
	BOOL	m_bDrawBoxAtDist;
	DWORD	m_nDrawBoxDist;
	BOOL	m_bLimitMemUse;
	BOOL	m_bAlwaysShowModels;
	DWORD	m_nMaxMemUse;
	BOOL	m_bLowPriorityLoading;
	int		m_nPerspective;
	int		m_nOrthographic;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif 
