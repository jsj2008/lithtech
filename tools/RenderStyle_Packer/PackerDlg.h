
#ifndef __UPDATEDLG_H__
#define __UPDATEDLG_H__

#include "resource.h"
#include <string>

#ifndef __LTB_H__
#include "ltb.h"
#endif


#ifndef __RENDERSTYLEPACKER_H__
#include "renderstylepacker.h"
#endif

using namespace std;

// DEFINES...
//#define MAX_RENDERSTYLE_INPUTS		32

// EXTERNS....
extern string		g_InputFile[MAX_RENDERSTYLE_INPUTS], g_OutputFile;
extern char			g_StartingDirectory[MAX_PATH];
extern E_LTB_FILE_TYPES g_PackType;


// PROTOTYPES...
void PackIt(string szInputFile[], const char* szOutputFile);

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
class CPackerDlg : public CDialog
{
public:
	enum ETargetPlatform { D3D_PC, PS2 };

    CPackerDlg() : CDialog(IDD_UPDATEDLG), m_PlatformPicked(D3D_PC)	{ for (uint32 i = 0; i < 8; ++i) { m_pInputFilename[i] = NULL; } m_pOutputFilename = NULL; m_pDontCloseWhenDone = NULL; m_pProgress = NULL; m_bCanceled = false; m_bIGetToStartIt = false; m_bStarted = false; }

	// Set by the parent dialog (tell us how to set our init conditions and where changes go)...
	void SetProgress(uint32 iProgressPercent)						{ if (m_pProgress) m_pProgress->SetPos(iProgressPercent); }
	void SetInputFilename(uint32 i, const char* szFilename)			{ if (m_pInputFilename[i]) m_pInputFilename[i]->SetWindowText(szFilename); }
	void SetOutputFilename(const char* szFilename)					{ if (m_pOutputFilename) m_pOutputFilename->SetWindowText(szFilename); }
	bool GetDontCloseWhenDone()										{ if (!m_pDontCloseWhenDone) return false; return (m_pDontCloseWhenDone->GetCheck() ? true : false); }
	void YouGetToStartIt()											{ m_bIGetToStartIt = true; }
	void ResetYourself()											{ SetProgress(0); for (uint32 i = 0; i < 8; ++i) { m_pInputFilenameBrowse[i]->EnableWindow(true); } m_pOutputFilenameBrowse->EnableWindow(true); m_pStart->EnableWindow(true); }

	bool WasCancelPressed()											{ return m_bCanceled; }

	int GetPlatformPicked()                                  { return m_PlatformPicked ; }
private:
	// Dialog controls...
	CStatic*			m_pInputFilename[8];
	CStatic*			m_pOutputFilename;
	CButton*			m_pInputFilenameBrowse[8];
	CButton*			m_pOutputFilenameBrowse;
	CButton*			m_pStart;
	CProgressCtrl*		m_pProgress;
	CButton*			m_pCancelReq;
	CButton*			m_pDontCloseWhenDone;
	CComboBox*			m_pPlatformPicker ;

	bool				m_bCanceled;
	bool				m_bIGetToStartIt;
	bool				m_bStarted;
	void				InitializeDialogControls();					// Grab the Dialog's controls...
	ETargetPlatform		m_PlatformPicked ;


	void _OnPlatformPicker() ;
	void _OnStart();

public:
	// CDialog Overrides...
	virtual BOOL		OnInitDialog();

	//{{AFX_MSG(CAppForm)
	afx_msg void		OnCancelReq()								{ if (m_bIGetToStartIt && !m_bStarted) SendMessage(WM_COMMAND,IDCANCEL); m_bCanceled = true; }
	afx_msg void		OnInputFileBrowse1()						{ string StartFile; if (!g_InputFile[0].empty()) StartFile = g_InputFile[0].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[0] = BrowseBox.GetPathName(); m_pInputFilename[0]->SetWindowText(g_InputFile[0].c_str()); } }
	afx_msg void		OnInputFileBrowse2()						{ string StartFile; if (!g_InputFile[1].empty()) StartFile = g_InputFile[1].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[1] = BrowseBox.GetPathName(); m_pInputFilename[1]->SetWindowText(g_InputFile[1].c_str()); } }
	afx_msg void		OnInputFileBrowse3()						{ string StartFile; if (!g_InputFile[2].empty()) StartFile = g_InputFile[2].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[2] = BrowseBox.GetPathName(); m_pInputFilename[2]->SetWindowText(g_InputFile[2].c_str()); } }
	afx_msg void		OnInputFileBrowse4()						{ string StartFile; if (!g_InputFile[3].empty()) StartFile = g_InputFile[3].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[3] = BrowseBox.GetPathName(); m_pInputFilename[3]->SetWindowText(g_InputFile[3].c_str()); } }
	afx_msg void		OnInputFileBrowse5()						{ string StartFile; if (!g_InputFile[4].empty()) StartFile = g_InputFile[4].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[4] = BrowseBox.GetPathName(); m_pInputFilename[4]->SetWindowText(g_InputFile[4].c_str()); } }
	afx_msg void		OnInputFileBrowse6()						{ string StartFile; if (!g_InputFile[5].empty()) StartFile = g_InputFile[5].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[5] = BrowseBox.GetPathName(); m_pInputFilename[5]->SetWindowText(g_InputFile[5].c_str()); } }
	afx_msg void		OnInputFileBrowse7()						{ string StartFile; if (!g_InputFile[6].empty()) StartFile = g_InputFile[6].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[6] = BrowseBox.GetPathName(); m_pInputFilename[6]->SetWindowText(g_InputFile[6].c_str()); } }
	afx_msg void		OnInputFileBrowse8()						{ string StartFile; if (!g_InputFile[7].empty()) StartFile = g_InputFile[7].c_str(); else { StartFile = g_StartingDirectory; StartFile += "\\InputFile"; } CFileDialog BrowseBox(true,"lta",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTA File (*.lta)|*.lta||"); if (BrowseBox.DoModal() == IDOK) { g_InputFile[7] = BrowseBox.GetPathName(); m_pInputFilename[7]->SetWindowText(g_InputFile[7].c_str()); } }
	afx_msg void		OnOutputFileBrowse()						{ string StartFile; if (!g_OutputFile.empty()) StartFile = g_OutputFile.c_str(); else { if (!g_InputFile[0].empty()) { char szTmp[MAX_PATH]; strcpy(szTmp,g_InputFile[0].c_str()); char* szDot = strstr(szTmp,"."); if (szDot && szDot[0]=='.' && szDot[1]=='l' && szDot[2]=='t' && szDot[3]=='a') { szDot[3] = 'b'; StartFile = szTmp; } else { StartFile = g_StartingDirectory; StartFile += "\\OutputFile"; } } else { StartFile = g_StartingDirectory; StartFile += "\\OutputFile"; } } CFileDialog BrowseBox(true,"ltb",StartFile.c_str(),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"LTB File (*.ltb)|*.ltb||"); if (BrowseBox.DoModal() == IDOK) { g_OutputFile = BrowseBox.GetPathName(); m_pOutputFilename->SetWindowText(g_OutputFile.c_str()); } }
	afx_msg void		OnStart()									{ _OnStart() ;}
	afx_msg void		OnPlatformPicker()                          { _OnPlatformPicker() ; }
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

// ------------------------------------------------------------------------
// I N L I N E S 
// ------------------------------------------------------------------------


inline
void CPackerDlg::_OnPlatformPicker()
{
	int cur_sel = m_pPlatformPicker->GetCurSel();
	switch ( cur_sel )
	{
	case 0: m_PlatformPicked = D3D_PC ;  break ;
	case 1: m_PlatformPicked = PS2    ;  break ;
	default : 
		m_PlatformPicked = D3D_PC ;
	}
}

inline 
void CPackerDlg::_OnStart()
{
	for (uint32 i = 0; i < 8; ++i) 
	{ m_pInputFilenameBrowse[i]->EnableWindow(false); } 
	m_pOutputFilenameBrowse->EnableWindow(false); 
	m_pStart->EnableWindow(false); 

	if( m_PlatformPicked == D3D_PC )
		g_PackType = LTB_D3D_RENDERSTYLE_FILE;
	else 
		g_PackType = LTB_PS2_RENDERSTYLE_FILE;	

	// Call pack it.
	PackIt(g_InputFile, g_OutputFile.c_str()); 
}
#endif //eof


