//-----------------------------------------------------------------------------
// File: RenderStyle Editor.cpp
//
// Desc: Main file for the RenderStyle Editor...
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <mmsystem.h>
#include "HeightMaptoNormalMap.h"
#include "UpdateDlg.h"
#include "d3d_shell.h"
#include "d3d_device.h"
#include "Utilities.h"
#include "commandline_parser.h"
#include "tdguard.h"

BEGIN_MESSAGE_MAP(CApp, CWinApp)
	//{{AFX_MSG_MAP(CApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// GLOBALS
CApp        g_App;
HWND		g_hWnd_RenderWindow	= NULL;
uint32		g_ScreenWidth		= 0;
uint32		g_ScreenHeight		= 0;
bool		g_VerboseMode		= false;
bool		g_bWin				= false;
float		g_fScale			= 0.1f;
float		g_fMinimumUp		= 0.0f;		// Do we need to expose this to the user?
string		g_InputFile, g_OutputFile;
CUpdateDlg	g_UpdateDlg;
char		g_StartingDirectory[MAX_PATH];
HEIGHTSRC	g_HeightSource		= eAlpha;	// Where are you getting your height values from?	

UINT StartDlgThread(LPVOID lpvParam) 
{
	DoConversion(g_InputFile.c_str(), g_OutputFile.c_str());
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


	CommandLineParser cmdInfo;							// Parse the commandline (use the custom parser)...
	ParseCommandLine(cmdInfo);
	if (cmdInfo.hasVerbose())							{ g_VerboseMode = true; }
	if (cmdInfo.hasWin())								{ g_bWin		= true; }
	if (cmdInfo.GetParamVal("scale"))					{ g_fScale		= (float)atof(cmdInfo.GetParamVal("scale")); }
	if (cmdInfo.GetParamVal("source"))					{ 
		if (stricmp(cmdInfo.GetParamVal("source"),"alpha")==0)      g_HeightSource = eAlpha;
		else if (stricmp(cmdInfo.GetParamVal("source"),"red")==0)   g_HeightSource = eRed;
		else if (stricmp(cmdInfo.GetParamVal("source"),"green")==0) g_HeightSource = eGreen;
		else if (stricmp(cmdInfo.GetParamVal("source"),"blue")==0)  g_HeightSource = eBlue; }
	if (cmdInfo.hasHelp())								{ 
		string szTmp = "HeightMaptoNormalMap command line...\n\n";
		szTmp += "Required Stuff: -input <InputFile> -output <OutputFile>\n";
		szTmp += "Optional Flags: -source <SRC: alpha/red/green/blue> -scale <fval> -verbose -win";
		OutputMsg(const_cast<char*>(szTmp.c_str())); }

	// If we've got input, start up the process thread...
	getcwd(g_StartingDirectory,MAX_PATH); 
	if (cmdInfo.GetInFile())							g_InputFile  = cmdInfo.GetInFile();
	if (cmdInfo.GetOutFile())							g_OutputFile = cmdInfo.GetOutFile();
	if (!g_InputFile.empty() && !g_OutputFile.empty() && !g_bWin) {
		::AfxBeginThread((AFX_THREADPROC)StartDlgThread,NULL,THREAD_PRIORITY_NORMAL); }
	else { 
		g_UpdateDlg.YouGetToStartIt(); }

	g_UpdateDlg.SetInputFilename(g_InputFile.c_str());
	g_UpdateDlg.SetOutputFilename(g_OutputFile.c_str());
	g_UpdateDlg.DoModal();

    return true;
}

uint8 GetHeight(uint32* pArray, int i, int j, int Width, int Height, int Pitch)
{
	uint8 RetHeight = 0;
	i = max((int)0, i);
	j = max((int)0, j);
	i = min(Width  - 1, i);
	j = min(Height - 1, j);

	switch (g_HeightSource) {
	case eAlpha : RetHeight = pArray[j * (Pitch/4) + i] >> 24; break;
	case eRed	: RetHeight = pArray[j * (Pitch/4) + i] >> 16; break;
	case eGreen : RetHeight = pArray[j * (Pitch/4) + i] >> 8;  break;
	case eBlue	: RetHeight = pArray[j * (Pitch/4) + i] >> 0;  break; }
	return RetHeight;
}

D3DXVECTOR3 GetNormal(const D3DXVECTOR3& thePoint0, const D3DXVECTOR3& thePoint1, const D3DXVECTOR3& thePoint2)
{
	D3DXVECTOR3 theNormal;
	D3DXVECTOR3 v1 = thePoint1 - thePoint0;
	D3DXVECTOR3 v2 = thePoint2 - thePoint0;

	D3DXVec3Cross(&theNormal,&v1,&v2);
	D3DXVec3Normalize(&theNormal,&theNormal);
	return theNormal;
}

void AlphaAndVectorToRGBA(const unsigned char& theHeight, const D3DXVECTOR3& inVector, uint32& outColor, unsigned int min_up)
{
	unsigned int red   = min(255u, (unsigned int)((inVector.x + 1.0f) * 127.5f));
	unsigned int green = min(255u, (unsigned int)((inVector.y + 1.0f) * 127.5f));
	unsigned int blue  = min(255u, (unsigned int)((inVector.z + 1.0f) * 127.5f));

	if (green < min_up) green = min_up;
	outColor = ((theHeight << 24) | (red << 16) | (green << 8) | (blue << 0));
}

void DoConversion(const char* szInputFile, const char* szOutputFile)
{
	Sleep(500);											// Slow yourself down for windows (want to be after the main window starts up)...

	uint32 iAdapterID			  = 0;					// D3D's adapter counter number for this adapter...
	D3DAdapterInfo* pAdapterInfo  = NULL;
	D3DDeviceInfo*  pDeviceInfo	  = NULL;
	D3DModeInfo*	pModeInfo	  = NULL;
	LPDIRECT3DTEXTURE9 pSrcSurf	  = NULL;
	D3DXVECTOR3*	pNormalArray  = NULL;
	uint32*			pUpdateArray  = NULL;
	string			myOutputFile  = szOutputFile;
	D3DSURFACE_DESC SurfDesc; D3DLOCKED_RECT LockedRect; uint32 j,i; int iplus, jplus; uint32 theNewColor;
    double oneTexelWidth,oneTexelHeight,theHeight0,theHeight1,theHeight2,theHeight3,theHeight4;
    double u = 0.0f; double v = 0.0f;

	if (!szInputFile)									{ OutputMsg("Error: No input file specified");  goto BAIL_AND_EXIT; }
	if (!szOutputFile)									{ OutputMsg("Error: No output file specified"); goto BAIL_AND_EXIT; }
	if (g_VerboseMode)									{ OutputMsg("InputFile: %s.\nOutputFile: %s\n\n",g_InputFile.c_str(),g_OutputFile.c_str()); }

	// Output in tga format - If there's a different extension on already, bail...
	if (strrchr(szOutputFile, '.') == NULL) {			// Append .tga to the filename
		myOutputFile += ".tga"; }
	else {												// If extension is not ".tga" then fail
		if (strstr(szOutputFile, ".tga") == NULL)		{ OutputMsg("Error: Outputs in TGA format only."); goto BAIL_AND_EXIT; } }

	// Create the D3DShell (it'll enumerate all the hardware devices)...
	if (!g_D3DShell.Create())							{ OutputMsg("Error: Couldn't create the D3DShell"); goto BAIL_AND_EXIT; }

	// First try to find your favorite device. If you can't, pick a default one.
	if (!pDeviceInfo) pDeviceInfo = g_D3DShell.PickDefaultDev(&pAdapterInfo,true);
	if (!pDeviceInfo)									{ OutputMsg("Can't find any d3d devices to use!"); goto BAIL_AND_EXIT; }
	pModeInfo					  = g_D3DShell.PickDefaultMode(pDeviceInfo,32);
	if (!pModeInfo)										{ OutputMsg("Can't find an appropriate display mode!"); goto BAIL_AND_EXIT; }

	// Create the Device...
	g_hWnd_RenderWindow			  = g_UpdateDlg.m_hWnd;
	if (!g_Device.CreateDevice(pAdapterInfo,pDeviceInfo,pModeInfo)) { OutputMsg("Couldn't create D3D Device!"); goto BAIL_AND_EXIT; }

	// Load the source image (and get some info)...
	if (D3DXCreateTextureFromFileEx(PD3DDEVICE,szInputFile,D3DX_DEFAULT,D3DX_DEFAULT,1,NULL,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,D3DX_FILTER_NONE,D3DX_DEFAULT,0,NULL,NULL,&pSrcSurf) != D3D_OK)		{ OutputMsg("Error: Couldn't load input file"); goto BAIL_AND_EXIT; }
	if (pSrcSurf->GetLevelDesc(0,&SurfDesc) != D3D_OK)	{ OutputMsg("Error: Get SrcSurf Info"); goto BAIL_AND_EXIT; }
	if (pSrcSurf->LockRect(0,&LockedRect,NULL,NULL) != D3D_OK)	{ OutputMsg("Error: Couldn't lock SrcSurf"); goto BAIL_AND_EXIT; }
    oneTexelWidth  = 100.0 / (double)(SurfDesc.Width  - 1);
    oneTexelHeight = 100.0 / (double)(SurfDesc.Height - 1);

	// Alloc an array for the normals...
    pNormalArray = new D3DXVECTOR3[SurfDesc.Height * SurfDesc.Width];

	// Figure out the normals...
	for (j = 0; j < SurfDesc.Height; ++j) {
		g_UpdateDlg.SetProgress((uint32)((float)j/(float)SurfDesc.Height * 50.0f));
		if (g_UpdateDlg.WasCancelPressed())				{ goto BAIL_AND_EXIT; }
		for (i = 0; i < SurfDesc.Width; ++i) {
			iplus = (i + 1) % SurfDesc.Width;			// Wrap back to zero for greatest i & j edges.
			jplus = (j + 1) % SurfDesc.Height;

            theHeight0 = GetHeight((uint32*)LockedRect.pBits, i,	 j,     SurfDesc.Width, SurfDesc.Height, LockedRect.Pitch);		// Select alplha component via >> 24
            theHeight1 = GetHeight((uint32*)LockedRect.pBits, iplus, j,     SurfDesc.Width, SurfDesc.Height, LockedRect.Pitch);
            theHeight2 = GetHeight((uint32*)LockedRect.pBits, i,	 jplus, SurfDesc.Width, SurfDesc.Height, LockedRect.Pitch);
            theHeight3 = GetHeight((uint32*)LockedRect.pBits, iplus, jplus, SurfDesc.Width, SurfDesc.Height, LockedRect.Pitch);

            theHeight0 *= g_fScale;
            theHeight1 *= g_fScale;
            theHeight2 *= g_fScale;
            theHeight3 *= g_fScale;

            theHeight4 = (theHeight0 + theHeight1 + theHeight2 + theHeight3) / 4.0;

            D3DXVECTOR3 thePoint0((float)u,								(float)v,								(float)theHeight0);
            D3DXVECTOR3 thePoint1((float)u + (float)oneTexelWidth,		(float)v,								(float)theHeight1);
            D3DXVECTOR3 thePoint2((float)u,								(float)v + (float)oneTexelHeight,		(float)theHeight2);
            D3DXVECTOR3 thePoint3((float)u + (float)oneTexelWidth,		(float)v + (float)oneTexelHeight,		(float)theHeight3);
            D3DXVECTOR3 thePoint4((float)u + (float)oneTexelWidth/2.0f,	(float)v + (float)oneTexelHeight/2.0f,	(float)theHeight4);

            pNormalArray[j * SurfDesc.Width + i]  = GetNormal(thePoint0, thePoint1, thePoint4);
            pNormalArray[j * SurfDesc.Width + i] += GetNormal(thePoint1, thePoint3, thePoint4);
            pNormalArray[j * SurfDesc.Width + i] += GetNormal(thePoint3, thePoint2, thePoint4);
            pNormalArray[j * SurfDesc.Width + i] += GetNormal(thePoint2, thePoint0, thePoint4);
            
            u += oneTexelWidth; } }

    // Convert the normals into colors...
    pUpdateArray = new uint32[SurfDesc.Height * SurfDesc.Width];
    for (j = 0; j < SurfDesc.Height; ++j) {
		g_UpdateDlg.SetProgress((uint32)(50.0f + ((float)j/(float)SurfDesc.Height * 40.0f)));
		if (g_UpdateDlg.WasCancelPressed())				{ goto BAIL_AND_EXIT; }
        for (i = 0; i < SurfDesc.Width; ++i) {
            D3DXVECTOR3 theNormal = pNormalArray[j * SurfDesc.Width + i];
		    D3DXVec3Normalize(&theNormal,&theNormal);
            
            theHeight0 = GetHeight((uint32*)LockedRect.pBits, i, j, SurfDesc.Width, SurfDesc.Height, LockedRect.Pitch);

			AlphaAndVectorToRGBA((uint8)theHeight0, theNormal, theNewColor, (uint32)g_fMinimumUp);
            
            pUpdateArray[j * SurfDesc.Width + i] = theNewColor; } }
	pSrcSurf->UnlockRect(0);

	// Save the output texture...
	if (!SaveImageFile_TGA(myOutputFile.c_str(),(unsigned char*)pUpdateArray,SurfDesc.Width,SurfDesc.Height)) { OutputMsg("Error: Could not save file: %s",myOutputFile.c_str()); goto BAIL_AND_EXIT; }

	g_UpdateDlg.SetProgress(100);						// We're done...
	if (g_UpdateDlg.WasCancelPressed())					{ goto BAIL_AND_EXIT; }
	if (g_VerboseMode)									{ OutputMsg("Done."); }

BAIL_AND_EXIT:
	g_Device.FreeAll(); g_D3DShell.FreeAll();
	if (pNormalArray)									{ delete[] pNormalArray; pNormalArray = NULL; }
	if (pUpdateArray)									{ delete[] pUpdateArray; pUpdateArray = NULL; }
	if (pSrcSurf)										{ pSrcSurf->Release(); pSrcSurf = NULL; }
	if (g_UpdateDlg.m_hWnd && !g_UpdateDlg.GetDontCloseWhenDone()) { g_UpdateDlg.SendMessage(WM_COMMAND,IDCANCEL); }
	else { g_UpdateDlg.ResetYourself(); }
}
