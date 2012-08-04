#ifndef __PROPPAGEOPTIONSPREFABS_H__
#define __PROPPAGEOPTIONSPREFABS_H__

class CPropPageOptionsPrefabs : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsPrefabs)

// Construction
public:
	CPropPageOptionsPrefabs();
	~CPropPageOptionsPrefabs();

	//saves the options back into the global options database
	void SaveOptions();

	//handles updating the enabled status of the controls
	void UpdateEnableStatus();

	//saves the data, and updates the enabled states
	void UpdateAll();

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsPrefabs)
	enum { IDD = IDD_PROPPAGE_PREFAB };
	int		m_nDrawContents;
	BOOL	m_bShowOutline;
	BOOL	m_bShowOrientation;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptionsPrefab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageOptionsPrefab)
	virtual BOOL OnInitDialog();
	afx_msg void DefaultHandler();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif 
