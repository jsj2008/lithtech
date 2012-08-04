// WinLTCDlg.h : header file
//

#if !defined(AFX_WINLTCDLG_H__55B27DEF_C2DC_4F68_A9AD_DCC046E03E5E__INCLUDED_)
#define AFX_WINLTCDLG_H__55B27DEF_C2DC_4F68_A9AD_DCC046E03E5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWinLTCDlg dialog

class CWinLTCDlg : public CDialog
{
// Construction
public:
	CWinLTCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CWinLTCDlg)
	enum { IDD = IDD_WINLTC_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinLTCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//gets the path name given a specified file
	static CString		GetPathName(const CString& sFile);

	//gets the extension of the given filename
	static CString		GetExtension(const CString& sFile);

	//modifies the string in the file window to reflect the given string
	void	UpdateFileName(const CString& sFile);

	//updates the text in the information view
	void	UpdateFileInfo(const CString& sInfo);

	//retreives the size of the specified file
	DWORD	GetFileSize(const CString& sFileName);

	//allows the enabling and disabling of the control buttons
	void	EnableControlButtons(BOOL bEnable);

	//updates the names of the buttons to accomodate for the extension, changing
	//compress to expand accordingly
	void CWinLTCDlg::UpdateButtonNames(const CString& sFilename);

	//switches between two files. It will load in the first file, and save it to the
	//other file using the opposite compression flag.
	BOOL ToggleCompression(	const CString& sInFile, const CString sOutFile, 
							BOOL bIsInFileCompressed);

	//given a file name, it will return the name with the extension properly switched
	//for the opposite type of compression
	CString ToggleExtension(const CString& sFileName);

	//determines if the designated file exists
	BOOL DoesFileExist(const CString& sFileName);

	//sets up the dialog to reflect a new file name
	void SetNewFileName(const CString& sFileName);

	//updates the information string to display compression statistics
	void ShowCompressionStatistics(DWORD nOldSize, DWORD nNewSize);

	HICON m_hIcon;

	//the file that is currently selected
	CString		m_sFileName;

	//the current file size
	DWORD		m_nFileSize;

	// Generated message map functions
	//{{AFX_MSG(CWinLTCDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBrowse();
	afx_msg void OnCompress();
	afx_msg void OnView();
	afx_msg void OnCompressTo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINLTCDLG_H__55B27DEF_C2DC_4F68_A9AD_DCC046E03E5E__INCLUDED_)
