#ifndef __PROPPAGEOPTIONSCONTROLS_H__
#define __PROPPAGEOPTIONSCONTROLS_H__

class CPropPageOptionsControls : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsControls)

// Construction
public:
	CPropPageOptionsControls();
	~CPropPageOptionsControls();

	//saves the options back into the global options database
	void SaveOptions();

	//loads options from the global options database
	void LoadOptions();

	//handles updating the enabled status of the controls
	void UpdateEnableStatus();

	//saves the data, and updates the enabled states
	void UpdateAll();

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsModels)
	enum { IDD = IDD_PROPPAGE_CONTROLS };
	BOOL	m_bInvertMouseY;
	BOOL	m_bZoomToCursor;
	BOOL	m_bOrbitAroundSel;
	BOOL	m_bAutoCaptureFocus;
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
	afx_msg void OnButtonHotKeys();
	afx_msg void OnApplyStyle();
	afx_msg void OnStyleChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//gets the text from the style dropdown
	CString		GetStyleName();

	//updates the edit box to reflect the description of the selected style
	void		UpdateStyleText();

};

#endif 
