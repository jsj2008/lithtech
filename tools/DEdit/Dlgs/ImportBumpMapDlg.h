#ifndef __IMPORTBUMPMAPDLG_H__
#define __IMPORTBUMPMAPDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CImportBumpMapDlg dialog

class CImportBumpMapDlg : public CDialog
{
// Construction
public:
	CImportBumpMapDlg(CWnd* pParent = NULL);   // standard constructor

	//channel types
	enum {	RED, GREEN, BLUE, ALPHA, NONE };

	//if this is not empty, this string will replace the text next to the luminance field
	CString	m_sLuminanceText;

// Dialog Data
	//{{AFX_DATA(CImportBumpMapDlg)
	enum { IDD = IDD_IMPORTBUMPMAP };

	float	m_fHeight;

	int		m_nHeightChannel;
	int		m_nLuminanceChannel;

	CString	m_sImageFile;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportBumpMapDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportBumpMapDlg)
		afx_msg void OnButtonBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif 
