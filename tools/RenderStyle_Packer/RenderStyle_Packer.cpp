//-----------------------------------------------------------------------------
// File: RenderStyle_Packer.cpp
//
// Desc: mfc file for the RenderStyle Editor...
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <mmsystem.h>
#include "ltamgr.h"
#include "RenderStyle_Packer.h"
#include "PackerDlg.h"
#include "Utilities.h"
#include "commandline_parser.h"
#include "ltb.h"
#include "tdguard.h"


BEGIN_MESSAGE_MAP(CApp, CWinApp)
	//{{AFX_MSG_MAP(CApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// GLOBALS
CApp        g_App;
CPackerDlg	g_PackerDlg;


extern bool		g_VerboseMode;
extern bool		g_bWin;
extern bool		g_bNoAutoStart;
extern string	g_InputFile[MAX_RENDERSTYLE_INPUTS], g_OutputFile;

extern char		g_StartingDirectory[MAX_PATH];
extern string	g_WorkingDir;

extern E_LTB_FILE_TYPES g_PackType ;

void SetPlatform			(const char *str );
void PackIt					(string szInputFile[], const char* szOutputFile);
void SetProgress( uint32 );


// ------------------------------------------------------------------------
// windows progress bar
// ------------------------------------------------------------------------
void SetProgress( uint32 percent )
{
	g_PackerDlg.SetProgress(percent);
}

// ------------------------------------------------------------------------
// ShutDownApp
// ------------------------------------------------------------------------
void ShutDownApp()
{
	if (g_PackerDlg.m_hWnd && !g_PackerDlg.GetDontCloseWhenDone()) { 
		g_PackerDlg.SendMessage(WM_COMMAND,IDCANCEL); }
	else { g_PackerDlg.ResetYourself(); }

}

UINT StartDlgThread(LPVOID lpvParam) 
{
	Sleep(500);	// Slow yourself down for windows (want to be after the main window starts up)...
	PackIt(g_InputFile,g_OutputFile.c_str());
	return NULL;
}

//	This is the main entry point for the application...
BOOL CApp::InitInstance()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	CommandLineParser cmdInfo;								// Parse the commandline (use the custom parser)...
	ParseCommandLine(cmdInfo);
	if (cmdInfo.hasVerbose())								{ g_VerboseMode  = true; }
	if (cmdInfo.hasWin())									{ g_bWin		 = true; }
	if (cmdInfo.hasNoAutoStart())							{ g_bNoAutoStart = true; }
	if (cmdInfo.hasHelp())									{ 
		string szTmp = "RenderStyle_Packer command line...\n\n";
		szTmp += "Required Stuff: -input <InputFile> -output <OutputFile>\n";
		szTmp += "Optional Flags: -verbose -win";
		OutputMsg(const_cast<char*>(szTmp.c_str())); }


	if( cmdInfo.GetParamVal( "platform" ) != NULL )
	{
		SetPlatform(cmdInfo.GetParamVal("platform" ));
	}

	// If we've got input, start up the process thread...
	getcwd(g_StartingDirectory,MAX_PATH);
	
	if (cmdInfo.GetParamVal("workdir"))
	{ 
		g_WorkingDir	= cmdInfo.GetParamVal("workdir"); 
		chdir(g_WorkingDir.c_str()); 
	} 
	else 
	{ 
		g_WorkingDir = g_StartingDirectory; 
	}
	
	for (uint32 i = 0; i < MAX_RENDERSTYLE_INPUTS; ++i)		
	{
		if (cmdInfo.GetInFile(i))							
			g_InputFile[i]	= cmdInfo.GetInFile(i); 
	}

	if (cmdInfo.GetOutFile())								
		g_OutputFile	= cmdInfo.GetOutFile();

	if (!g_InputFile[0].empty() && !g_OutputFile.empty() && !g_bNoAutoStart)	
	{
		if (g_bWin) {
			::AfxBeginThread((AFX_THREADPROC)StartDlgThread,NULL,THREAD_PRIORITY_NORMAL); 

			for (uint32 i = 0; i < 8; ++i) { 
				if (!g_InputFile[i].empty()) g_PackerDlg.SetInputFilename(i, g_InputFile[i].c_str()); }
			g_PackerDlg.SetOutputFilename(g_OutputFile.c_str());
			g_PackerDlg.DoModal(); }
		else {
			// Do the Conversion...
			Sleep(500);	// Slow yourself down for windows (want to be after the main window starts up)...
			PackIt(g_InputFile,g_OutputFile.c_str()); 
		} 
	}
	else { 
		g_PackerDlg.YouGetToStartIt(); 

		for (uint32 i = 0; i < 8; ++i) { 
			if (!g_InputFile[i].empty()) 
				g_PackerDlg.SetInputFilename(i, g_InputFile[i].c_str()); 
		}
		g_PackerDlg.SetOutputFilename(g_OutputFile.c_str());
		g_PackerDlg.DoModal(); 
	}

    return true;
}


