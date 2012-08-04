#ifndef __STRINGPROMPT_H__
#define __STRINGPROMPT_H__

// StringPrompt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStringPrompt dialog

class CStringPrompt : public CDialog
{
// Construction
public:
	CStringPrompt(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStringPrompt)
	enum { IDD = IDD_STRING_PROMPT };
	CString	m_sPrompt;
	CString	m_sString;
	//}}AFX_DATA

	//the title for the dialog
	CString		m_sTitle;


// Overrides
	//{{AFX_VIRTUAL(CStringPrompt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringPrompt)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif
