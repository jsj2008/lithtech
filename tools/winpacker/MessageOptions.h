#ifndef __MESSAGEOPTIONS_H__
#define __MESSAGEOPTIONS_H__

/////////////////////////////////////////////////////////////////////////////
// CMessageOptions dialog

#ifndef __COLORWELL_H__
#	include "ColorWell.h"
#endif

#ifndef __IPACKEROUTPUT_H__
#	include "IPackerOutput.h"
#endif

#ifndef __AUTOTOOLTIPCTRL_H__
#	include "AutoToolTipCtrl.h"
#endif

//the number of severties that exist
#define		NUM_SEVERITIES		(MSG_DEBUG + 1)

class CMessageOptions : public CDialog
{
// Construction
public:
	CMessageOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMessageOptions)
	enum { IDD = IDD_MESSAGE_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	//the colors, one per severity
	COLORREF	m_Colors[NUM_SEVERITIES];

	//the prefixes, one per severity
	CString		m_sPrefixes[NUM_SEVERITIES];

// Overrides
	//{{AFX_VIRTUAL(CMessageOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CAutoToolTipCtrl	m_ToolTip;

	CColorWell			m_ColorWell;

	// Generated message map functions
	//{{AFX_MSG(CMessageOptions)
	afx_msg void OnSeverityChanged();
	afx_msg void OnButtonColor();
	afx_msg void OnChangePrefix();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}


#endif 
