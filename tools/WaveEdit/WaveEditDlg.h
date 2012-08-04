// WaveEditDlg.h : header file
//

#if !defined(AFX_WAVEEDITDLG_H__0440E646_29C3_11D3_B781_444553540000__INCLUDED_)
#define AFX_WAVEEDITDLG_H__0440E646_29C3_11D3_B781_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wave.h"
#include "ltbasedefs.h"

/////////////////////////////////////////////////////////////////////////////
// CWaveEditDlg dialog

class CWaveEditDlg : public CFileDialog
{
// Construction
public:
	CWaveEditDlg(CWnd* pParent = NULL);	// standard constructor
	~CWaveEditDlg();

// Dialog Data
	//{{AFX_DATA(CWaveEditDlg)
	enum { IDD = IDD_WAVEEDIT_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

protected:

	void		OnSelection( );
	void		UpdateUI( );
	BOOL		SetWaveInfo( const char *pszPath, CWaveHeader &waveHeader );
	void		EnableApply( BOOL bTrue );
	BOOL		ParseFiles( CString &sDir, ConParse &fileNames );
	void		DoDirectory( CString &sDir, ConParse &parse, CWaveHeader &waveHeader, BOOL bRecurse );

	char		m_szFile[2048];

	int			m_nPlayMode;
	CString		m_sPitch;
	BOOL		m_bSelectionValid;
	CString		m_sInitDir;

	// Generated message map functions
	//{{AFX_MSG(CWaveEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnApply();
	afx_msg void OnChange();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDITDLG_H__0440E646_29C3_11D3_B781_444553540000__INCLUDED_)
