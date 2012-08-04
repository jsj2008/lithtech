
#ifndef __UPDATEDLG_H__
#define __UPDATEDLG_H__

#include "resource.h"
#include <string>

using namespace std;

// ENUMS
enum HEIGHTSRC		{ eAlpha, eRed, eGreen, eBlue };

// EXTERNS....
extern string		g_InputFile, g_OutputFile;
extern float		g_fScale;
extern char			g_StartingDirectory[MAX_PATH];
extern HEIGHTSRC	g_HeightSource;

// PROTOTYPES...
void DoConversion(const char* szInputFile, const char* szOutputFile);

class CUpdateDlg : public CDialog
{
public:
    CUpdateDlg() : CDialog(IDD_UPDATEDLG)							{ m_pInputFilename = NULL; m_pOutputFilename = NULL; m_pDontCloseWhenDone = NULL; m_pScaleEdit = NULL; m_pProgress = NULL; m_bCanceled = false; m_bIGetToStartIt = false; m_bStarted = false; }

	// Set by the parent dialog (tell us how to set our init conditions and where changes go)...
	void SetProgress(uint32 iProgressPercent)						{ if (m_pProgress) m_pProgress->SetPos(iProgressPercent); }
	void SetInputFilename(const char* szFilename)					{ if (m_pInputFilename) m_pInputFilename->SetWindowText(szFilename); }
	void SetOutputFilename(const char* szFilename)					{ if (m_pOutputFilename) m_pOutputFilename->SetWindowText(szFilename); }
	bool GetDontCloseWhenDone()										{ if (!m_pDontCloseWhenDone) return false; return (m_pDontCloseWhenDone->GetCheck() ? true : false); }
	void YouGetToStartIt()											{ m_bIGetToStartIt = true; }
	void ResetYourself()											{ SetProgress(0); m_pInputFilenameBrowse->EnableWindow(true); m_pOutputFilenameBrowse->EnableWindow(true); m_pStart->EnableWindow(true); }
	bool WasCancelPressed()											{ return m_bCanceled; }

private:
	// Dialog controls...
	CStatic*			m_pInputFilename;
	CStatic*			m_pOutputFilename;
	CEdit*				m_pScaleEdit;
	CComboBox*			m_pSourceChannel;
	CButton*			m_pInputFilenameBrowse;
	CButton*			m_pOutputFilenameBrowse;
	CButton*			m_pStart;
	CButton*			m_pDontCloseWhenDone;
	CProgressCtrl*		m_pProgress;
	CButton*			m_pCancelReq;

	bool				m_bCanceled;
	bool				m_bIGetToStartIt;
	bool				m_bStarted;
	void				InitializeDialogControls();					// Grab the Dialog's controls...

public:
	// CDialog Overrides...
	virtual BOOL		OnInitDialog();

	//{{AFX_MSG(CAppForm)
	afx_msg void		OnCancelReq()								{ if (m_bIGetToStartIt && !m_bStarted) SendMessage(WM_COMMAND,IDCANCEL); m_bCanceled = true; }
	afx_msg void		OnInputFileBrowse()							{ string StartFile; if (!g_InputFile.empty()) StartFile = g_InputFile.c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,NULL,StartFile.c_str()); if (BrowseBox.DoModal() == IDOK) { g_InputFile = BrowseBox.GetPathName(); m_pInputFilename->SetWindowText(g_InputFile.c_str()); } }
	afx_msg void		OnOutputFileBrowse()						{ string StartFile; if (!g_OutputFile.empty()) StartFile = g_OutputFile.c_str(); else { if (!g_InputFile.empty()) { char szTmp[MAX_PATH]; strcpy(szTmp,g_InputFile.c_str()); char* szDot = strstr(szTmp,"."); if (szDot && szDot[0]=='.') { szDot[0] = 'b'; szDot[1] = '.'; szDot[2] = 't'; szDot[3] = 'g'; szDot[4] = 'a'; szDot[5] = NULL; StartFile = szTmp; } else { StartFile = g_StartingDirectory; StartFile += "\\OutputFile"; } } else { StartFile = g_StartingDirectory; StartFile += "\\OutputFile"; } } CFileDialog BrowseBox(true,NULL,StartFile.c_str()); if (BrowseBox.DoModal() == IDOK) { g_OutputFile = BrowseBox.GetPathName(); m_pOutputFilename->SetWindowText(g_OutputFile.c_str()); } }
	afx_msg void		OnScale()									{ CString buf; m_pScaleEdit->GetWindowText(buf); g_fScale = (float)atof(LPCSTR(buf)); }
	afx_msg void		OnStart()									{ m_pInputFilenameBrowse->EnableWindow(false); m_pOutputFilenameBrowse->EnableWindow(false); m_pStart->EnableWindow(false); DoConversion(g_InputFile.c_str(), g_OutputFile.c_str()); }
	afx_msg void		OnSourceChannel()							{ g_HeightSource = (HEIGHTSRC)m_pSourceChannel->GetCurSel(); }
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif


