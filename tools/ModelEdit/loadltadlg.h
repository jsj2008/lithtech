#if !defined(AFX_LOADLTADLG_H__F5AB928A_D1C4_49C8_BF1C_14E924EA9DE8__INCLUDED_)
#define AFX_LOADLTADLG_H__F5AB928A_D1C4_49C8_BF1C_14E924EA9DE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// loadltadlg.h : header file
//

#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CLoadLTADlg dialog

class CLoadLTADlg : public CDialog
{
// Construction
public:
	CLoadLTADlg(bool bAutoConfirm, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadLTADlg)
	enum { IDD = IDD_LOADLTA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	//Thread safe way to add text onto the end of the load log
	static void	AppendLoadLog(const char* pszString);

	//Thread safe way to set if the loading thread is done
	static void	SetLoadThreadDone(BOOL bDone);

	//Thread safe way to clear the load log
	static void ClearLoadLog();

	//the name of the file that is currently being loaded, so that it
	//can be displayed on the dialog
	CString		m_sFilename;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadLTADlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoadLTADlg)
	afx_msg void OnSaveLoadLog();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//Thread safe way to update the load log
	void	UpdateLoadLog();

	//Thread safe way to check the done flag, and update the dialog box
	//appropriately
	BOOL	IsDone();

	//the status of the loader from the previous check. 
	BOOL	m_bPrevLoaderDone;

	enum	{TIMER_ID	= 1000};

private:

	// if true, the dialog exits automatically once the load is done
	bool m_bAutoConfirm;

	//DO NOT ACCESS DIRECTLY. THIS IS A COMMUNICATION POINT
	//FOR MULTIPLE THREADS
	//this string holds all the data that a separate thread loading
	//the LTA file writes to, so it can be appended onto the existing
	//load log
	static CString		sm_sLoadLogPipe;

	//DO NOT ACCESS DIRECTLY. THIS IS A COMMUNICATION POINT
	//FOR MULTIPLE THREADS
	//This tells the dialog box if the other thread has finished. If it is
	//false, the thread is still running and the dialog cannot allow the
	//user to select any of the buttons
	static BOOL			sm_bLoadThreadDone;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADLTADLG_H__F5AB928A_D1C4_49C8_BF1C_14E924EA9DE8__INCLUDED_)
