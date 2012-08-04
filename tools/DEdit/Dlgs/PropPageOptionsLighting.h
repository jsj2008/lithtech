#ifndef __PROPPAGEOPTIONSLIGHTING_H__
#define __PROPPAGEOPTIONSLIGHTING_H__

class CPropPageOptionsLighting : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsLighting)

// Construction
public:

	enum	{	LIGHT_NONE,
				LIGHT_SHADED,
				LIGHT_VERTEX,
				LIGHT_LIGHTMAPPED
			};

	CPropPageOptionsLighting();
	~CPropPageOptionsLighting();

	//saves the options back into the global options database
	void SaveOptions();

	//handles updating the enabled status of the controls
	void UpdateEnableStatus();

	//saves the data, and updates the enabled states
	void UpdateAll();

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsLighting)
	enum { IDD = IDD_PROPPAGE_LIGHTING };

	CSpinButtonCtrl	m_spinTimeSlice;
	CSpinButtonCtrl	m_spinLMMaxSize;
	CSpinButtonCtrl	m_spinLMMinSize;
	CSpinButtonCtrl	m_spinTexelSize;
	CSpinButtonCtrl	m_spinLeakDist;

	BOOL m_bShadows;
	BOOL m_bLambertian;
	int	 m_nLightMapSize;
	int  m_nTexelSize;
	int  m_nTimeSlice;
	int  m_nLightMode;
	int	 m_nMinLightMapSize;
	int  m_nLightLeakDist;

	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptionsLighting)
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

	BOOL	m_bInit;

	DECLARE_MESSAGE_MAP()

};

#endif 
