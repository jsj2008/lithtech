//-----------------------------------------------------------------------------
// File: RenderStylesEditor.cpp
//
// Desc: Main file for the RenderStyle Editor...
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <mmsystem.h>
#include "process.h"
#include "RenderStylesEditor.h"
#include "RenderPassDlg.h"
#include "d3d_shell.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"
#include "d3d_effectmgr.h"
//#include "dtx_files.h"
#include "Utilities.h"
#include "tdguard.h"
#include ".\renderstyleseditor.h"


// MFX MACROS
IMPLEMENT_DYNCREATE(CAppDoc,      CDocument)
IMPLEMENT_DYNCREATE(CAppFrameWnd, CFrameWnd)
IMPLEMENT_DYNCREATE(CAppForm,     CFormView)

BEGIN_MESSAGE_MAP(CApp, CWinApp)
//{{AFX_MSG_MAP(CApp)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CAppForm, CFormView)
//{{AFX_MSG_MAP(CAppForm)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(IDM_PROJECT_OPEN, OnProjectOpen)
	ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_SAVE, OnFileSave)
	ON_COMMAND(IDM_FILE_SAVEAS, OnFileSaveAs)
	ON_COMMAND(IDM_FILE_SETDEFAULTS, OnSetDefaults)
	ON_COMMAND(IDM_FILE_COMPILE_PCD3D, OnCompilePCD3D)
	ON_COMMAND(IDM_HELP_SAMPLES, OnHelpSamples)
	ON_COMMAND(IDM_HELP_GCCG, OnHelpGameContentCreationGuide)
	ON_COMMAND(IDM_HELP_WEB, OnHelpWeb)
	ON_COMMAND(IDM_HELP_ABOUT, OnAppAbout)
	ON_BN_CLICKED(IDC_ENABLE_REFRAST, OnRefRast_Enable)
	ON_BN_CLICKED(IDC_DBGPARAM_RENDONIDLE, OnRendOnIdle_Enable)
	ON_BN_CLICKED(IDC_DBGPARAM_ROTATEMODEL, OnRotateModel_Enable)
	ON_CBN_SELCHANGE(IDC_DBGPARAM_LIGHTCONFIG, OnDlgParam_LightConfig)
	ON_EN_CHANGE(IDC_DBGPARAM_LIGHTCOUNT, OnDlgParam_LightCount)
	ON_BN_CLICKED(IDC_DBGPARAM_LIGHTCOLOR_CONFIG, OnDlgParam_LightColorConfig)
	ON_EN_CHANGE(IDC_DBGPARAM_BACKGNDIMG_FILENAME, OnBackGndImg_Filename)
	ON_BN_CLICKED(IDC_DBGPARAM_BACKGNDIMG_BROWSE, OnBackGndImg_Browse)
	ON_BN_CLICKED(IDC_DBGPARAM_BACKGNDCOLOR_CONFIG, OnDlgParam_BackGndColor)
	ON_BN_CLICKED(IDC_RESETCAMERA, OnDlgParam_ResetCamera)
	ON_CBN_SELCHANGE(IDC_RENDERSTYLE_FILENAME, OnRenderStyle_Filename)
	ON_CBN_SELCHANGE(IDC_DBGPARAM_MODEL, OnDbgParam_Model)
	ON_BN_CLICKED(IDC_ENABLE_EFFECT, OnBnClickedEnableEffect)
	ON_BN_CLICKED(IDC_ENABLE_RENDERPASS1, OnConfig_RenderPass_Enable1)
	ON_BN_CLICKED(IDC_ENABLE_RENDERPASS2, OnConfig_RenderPass_Enable2)
	ON_BN_CLICKED(IDC_ENABLE_RENDERPASS3, OnConfig_RenderPass_Enable3)
	ON_BN_CLICKED(IDC_ENABLE_RENDERPASS4, OnConfig_RenderPass_Enable4)
	ON_BN_CLICKED(IDC_CONFIG_RENDERPASS1, OnConfig_RenderPass1)
	ON_BN_CLICKED(IDC_CONFIG_RENDERPASS2, OnConfig_RenderPass2)
	ON_BN_CLICKED(IDC_CONFIG_RENDERPASS3, OnConfig_RenderPass3)
	ON_BN_CLICKED(IDC_CONFIG_RENDERPASS4, OnConfig_RenderPass4)
	ON_BN_CLICKED(IDC_TEXTURELIST_BROWSE1, OnTextureList_Browse1)
	ON_BN_CLICKED(IDC_TEXTURELIST_BROWSE2, OnTextureList_Browse2)
	ON_BN_CLICKED(IDC_TEXTURELIST_BROWSE3, OnTextureList_Browse3)
	ON_BN_CLICKED(IDC_TEXTURELIST_BROWSE4, OnTextureList_Browse4)
	ON_BN_CLICKED(IDC_EFFECTFILE_BROWSE1, OnEffectFile_Browse1)
	ON_EN_CHANGE(IDC_TEXTURELIST_FILENAME1, OnTextureList_Filename1)
	ON_EN_CHANGE(IDC_TEXTURELIST_FILENAME2, OnTextureList_Filename2)
	ON_EN_CHANGE(IDC_TEXTURELIST_FILENAME3, OnTextureList_Filename3)
	ON_EN_CHANGE(IDC_TEXTURELIST_FILENAME4, OnTextureList_Filename4)
	ON_EN_CHANGE(IDC_MATERIAL_AMBIENTALPHA_EDIT, OnLightMaterial_AmbientAlpha)
	ON_EN_CHANGE(IDC_MATERIAL_DIFFUSEALPHA_EDIT, OnLightMaterial_DiffuseAlpha)
	ON_EN_CHANGE(IDC_MATERIAL_EMISSIVEALPHA_EDIT, OnLightMaterial_EmissiveAlpha)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARALPHA_EDIT, OnLightMaterial_SpecularAlpha)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARALPHA_EDIT, OnLightMaterial_SpecularAlpha)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARPOWER_EDIT, OnLightMaterial_SpecularPower)
	ON_BN_CLICKED(IDC_MATERIALCOLOR_AMBIENT_CONFIG, OnLightMaterial_AmbientConfig)
	ON_BN_CLICKED(IDC_MATERIALCOLOR_DIFFUSE_CONFIG, OnLightMaterial_DiffuseConfig)
	ON_BN_CLICKED(IDC_MATERIALCOLOR_EMISSIVE_CONFIG, OnLightMaterial_EmissiveConfig)
	ON_BN_CLICKED(IDC_MATERIALCOLOR_SPECULAR_CONFIG, OnLightMaterial_SpecularConfig)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify)


//}}AFX_MSG_MAP
ON_EN_CHANGE(IDC_EFFECT_ID, OnEnChangeEffectId)
ON_CBN_EDITCHANGE(IDC_EFFECT_TECHNIQUE, OnCbnEditchangeEffectTechnique)
ON_CBN_EDITUPDATE(IDC_EFFECT_TECHNIQUE, OnCbnEditupdateEffectTechnique)
ON_CBN_CLOSEUP(IDC_EFFECT_TECHNIQUE, OnCbnCloseupEffectTechnique)
ON_LBN_DBLCLK(IDC_OUTPUT, OnLbnDblclkOutput)
END_MESSAGE_MAP()

// GLOBALS
CApp        g_App;
CAppForm*   g_AppFormView		= NULL;
uint32		g_ScreenWidth		= 0;
uint32		g_ScreenHeight		= 0;
uint32		g_CurFrameCode		= 0;
HWND		g_hWnd_RenderWindow	= NULL;
char		g_StartingDirectory[MAX_PATH];

//	Change the window style (so it cannot maximize or be sized) before the main frame window
// is created.
BOOL CAppFrameWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
	return CFrameWnd::PreCreateWindow(cs);
}

//	This is the main entry point for the application. The MFC window stuff is initialized here.
// See also the main initialization routine for the CAppForm class, which is called indirectly
// from here.
BOOL CApp::InitInstance()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	// Asscociate the MFC app with the frame window and doc/view classes

#ifdef _RS_VIEWER
	AddDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME_VIEWER, RUNTIME_CLASS(CAppDoc), RUNTIME_CLASS(CAppFrameWnd), RUNTIME_CLASS(CAppForm)));
#else
	AddDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME, RUNTIME_CLASS(CAppDoc), RUNTIME_CLASS(CAppFrameWnd), RUNTIME_CLASS(CAppForm)));
#endif

	// Dispatch commands specified on the command line (req'd by MFC). This
	// also initializes the the CAppDoc, CAppFrameWnd, and CAppForm classes.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo)) return false;
	if (!g_AppFormView->IsReady()) return false;

	// Get rid of nasty scrollbars
	g_AppFormView->ResizeParentToFit(FALSE);

	// Set the size and margin for the main dialog window
	int newdialogX, newdialogY, marginX, marginY, screenX, screenY, dialogX, dialogY;
	RECT screen, dialog;

	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &screen, NULL);
	m_pMainWnd->GetWindowRect(&dialog);

	screenX = screen.right - screen.left;
	screenY = screen.bottom - screen.top;

	dialogX = dialog.right - dialog.left;
	dialogY = dialog.bottom - dialog.top;

	screenX > dialogX ? newdialogX = dialogX : newdialogX = screenX;
	screenY > dialogY ? newdialogY = dialogY : newdialogY = screenY;

	marginX = (screenX - dialogX) / 2;
	marginY = (screenY - dialogY) / 2;

	if (marginX < 0)  marginX = 0;	if (marginY < 0)  marginY = 0;

	// The formview window has been initialized, so show and update it.
	m_pMainWnd->MoveWindow(marginX, marginY, newdialogX, newdialogY, true);

#ifdef _RS_VIEWER
	m_pMainWnd->SetWindowText("Render Styles Viewer");
#else
	m_pMainWnd->SetWindowText("Render Styles Editor");
#endif

	m_pMainWnd->UpdateWindow();

	// Set where the renderpass dialogs will appear
	for (int i=0; i<4; i++)
	{
		g_AppFormView->m_RenderPassData[i].PositionX	= marginX + 340 + 12*i;
		g_AppFormView->m_RenderPassData[i].PositionY	= marginY + 60  + 12*i;
	}

	// Show viewport help
	g_AppFormView->DrawHelp(g_AppFormView->GetDlgItem(IDC_RENDERVIEW));

	return true;
}

//-----------------------------------------------------------------------------
// Name: OnIdle()
// Desc: Uses idle time to render the 3D scene.
//-----------------------------------------------------------------------------
BOOL CApp::OnIdle(LONG)
{
	// Do not render if the app is minimized
	if (m_pMainWnd->IsIconic()) return false;

	// Update and render a frame
	if (g_AppFormView->IsReady())  {
		g_AppFormView->OnIdle(); }

   	return true;									// Keep requesting more idle time...
}

// Constructor for the dialog resource form

#ifdef _RS_VIEWER
	CAppForm::CAppForm() : CFormView(IDD_FORMVIEW_VIEWER)
#else
	CAppForm::CAppForm() : CFormView(IDD_FORMVIEW)
#endif
{
	g_AppFormView						= this;
	m_bActive							= false;
	m_bReady							= false;
	m_bLoading							= false;
	m_bNoRecompile						= false;
	m_bResetingRaster					= false;
	m_bRecompiling						= false;
	m_bRendering						= false;
	m_bLoadingTextures					= false;
	m_bWindowed							= true;
	m_pRenderStyle						= NULL;
	m_pEB_LightMat_Ambient_Alpha		= NULL;
	m_pEB_LightMat_Diffuse_Alpha		= NULL;
	m_pEB_LightMat_Emissive_Alpha		= NULL;
	m_pEB_LightMat_Specular_Alpha		= NULL;
	m_pEB_LightMat_SpecularPower		= NULL;
	m_pRenderObject						= NULL;
	m_pAntEyeRenderObject				= NULL;
	m_pRenderStyle						= NULL;
	m_pAntEyeRenderStyle				= NULL;
	m_BackGndTexture					= NULL;
	m_fModelRotation					= 0.0f;
	m_fCameraRotation					= 0.0f;
	m_iLastRotationUpdateTime			= 0;
	m_bLeftMouseLook					= false;
	m_bRightMouseLook					= false;
	m_MouseLook_Rotation				= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_MouseLook_Translation				= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_ProjectPath[0]					= '\0';
	m_RenderStyle_FileName[0]			= '\0';
//	m_PlatformD3D						= false;
//	m_PlatformPS2						= false;
//	m_PlatformXBox						= false;
	m_RenderPassDlg[0]					= NULL;
	m_RenderPassDlg[1]					= NULL;
	m_RenderPassDlg[2]					= NULL;
	m_RenderPassDlg[3]					= NULL;

	m_pWnd_LightMat_Ambient  = NULL;
	m_pWnd_LightMat_Diffuse  = NULL;
	m_pWnd_LightMat_Emissive = NULL;
	m_pWnd_LightMat_Specular = NULL;

	m_pLB_OutputWindow         = NULL;
}

// Destructor for the dialog resource form. Shuts down the app
CAppForm::~CAppForm()
{
	m_pLB_OutputWindow					= NULL;

	DeleteAndClearArray(m_RenderStyle_Files);	// clear out list of RenderStyle files
	Destroy3DEnv();
}

// Button presses for configuring the texture passes (starts up a thread with StartDlgThread_ConfigRenderPass)...
UINT StartDlgThread_ConfigRenderPass(LPVOID lpvParam)
{
	CRenderPassData* pRenderData = (CRenderPassData*)lpvParam;
	CRenderPassDlg dlg;

	// If the user configures a renderpass, they probably want to use it
//	g_AppFormView->m_pBn_Enable_RenderPass[pRenderData->id]->SetCheck(true);	// Dave: You can't just do this - you need to rebuild the renderstyle data. Causes a bug when you do this and then save.
//	g_AppFormView->m_MainDlgData.RenderPass_Enable[0] = true;

	pRenderData->bDialogOpen = true;
	dlg.SetRenderPassData(pRenderData);

	CRenderPassData OriginalRenderPassData = *pRenderData;	// Make a copy so we can reset it if they cancel...

	g_AppFormView->m_bNoRecompile = true;

	if (!dlg.Create(IDD_RENDERPASS))
	{
		g_AppFormView->m_bNoRecompile = false;
		return 0;
	}

//	dlg.LimitDialogByPlatforms(g_AppFormView->m_PlatformD3D,
//							   g_AppFormView->m_PlatformPS2,
//							   g_AppFormView->m_PlatformXBox); // change for platform

	dlg.ShowWindow(SW_SHOW);
	g_AppFormView->m_RenderPassDlg[pRenderData->id] = &dlg;
	g_AppFormView->m_bNoRecompile = false;
	g_AppFormView->RenderStyleDataChanged();

	if (dlg.RunModalLoop(MLF_SHOWONIDLE) == IDCANCEL)
	{
		*pRenderData = OriginalRenderPassData;
	}

	// Save the position of the dialog
	RECT  size;
	dlg.GetWindowRect(&size);
	pRenderData->PositionX = size.left;
	pRenderData->PositionY = size.top;

	g_AppFormView->m_RenderPassDlg[pRenderData->id] = NULL;
	dlg.DestroyWindow();
	pRenderData->bDialogOpen = false;
	g_AppFormView->RenderStyleDataChanged();

	return 0;
}

void CAppForm::OnConfig_RenderPass1() {
	if (m_RenderPassData[0].bDialogOpen) return;	// Already started up this dialog - skip out...
	::AfxBeginThread((AFX_THREADPROC)StartDlgThread_ConfigRenderPass,&m_RenderPassData[0],THREAD_PRIORITY_NORMAL); }
void CAppForm::OnConfig_RenderPass2() {
	if (m_RenderPassData[1].bDialogOpen) return;	// Already started up this dialog - skip out...
	::AfxBeginThread((AFX_THREADPROC)StartDlgThread_ConfigRenderPass,&m_RenderPassData[1],THREAD_PRIORITY_NORMAL); }
void CAppForm::OnConfig_RenderPass3() {
	if (m_RenderPassData[2].bDialogOpen) return;	// Already started up this dialog - skip out...
	::AfxBeginThread((AFX_THREADPROC)StartDlgThread_ConfigRenderPass,&m_RenderPassData[2],THREAD_PRIORITY_NORMAL); }
void CAppForm::OnConfig_RenderPass4() {
	if (m_RenderPassData[3].bDialogOpen) return;	// Already started up this dialog - skip out...
	::AfxBeginThread((AFX_THREADPROC)StartDlgThread_ConfigRenderPass,&m_RenderPassData[3],THREAD_PRIORITY_NORMAL); }

bool CAppForm::StripPath(const char *target, const char *path, char *output)
{
	char capsPath[512], capsTarget[512];

	ASSERT(target); ASSERT(path); ASSERT(output);

	if((path[0] == '\0') || (target[0] == '\0'))
	{
		strcpy(output, target);
		return false;
	}

	strcpy(capsPath, path);  	strcpy(capsTarget, target);
	_strupr(capsPath);			_strupr(capsTarget);

	// check for path in target string
	LPTSTR substring = strstr(capsTarget, capsPath);

	// verify that the path is at the start of the target
	if ((substring != NULL) && (strlen(target) == strlen(substring)))
	{
		strcpy(output, &target[strlen(path)]);
		return true;
	}
	else
	{
		strcpy(output, target);
		return false;
	}
}

// Browse for a file to open...
void CAppForm::OnTextureList_Browse1()
{
	CFileDialog BrowseBox(true,"dtx",m_DebugParams.TextureList_Filename[0].c_str(),OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Image Files|*.*||");
	if (BrowseBox.DoModal() == IDOK)
	{
		m_pEB_TextureList_Filename[0]->SetWindowText(BrowseBox.GetPathName());
		m_DebugParams.TextureList_Filename[0] = BrowseBox.GetPathName();
	}
	DebugTextureSettingChanged();
	RenderStyleDataChanged();
}

void CAppForm::OnTextureList_Browse2()
{
	CFileDialog BrowseBox(true,"dtx",m_DebugParams.TextureList_Filename[1].c_str(),OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Image Files|*.*||");
	if (BrowseBox.DoModal() == IDOK)
	{
		m_pEB_TextureList_Filename[1]->SetWindowText(BrowseBox.GetPathName());
		m_DebugParams.TextureList_Filename[1] = BrowseBox.GetPathName();
	}
	DebugTextureSettingChanged();
	RenderStyleDataChanged();
}

void CAppForm::OnTextureList_Browse3()
{
	CFileDialog BrowseBox(true,"dtx",NULL,OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Image Files|*.*||");
	if (BrowseBox.DoModal() == IDOK)
	{
		m_pEB_TextureList_Filename[2]->SetWindowText(BrowseBox.GetPathName());
		m_DebugParams.TextureList_Filename[2] = BrowseBox.GetPathName();
	}
	DebugTextureSettingChanged();
	RenderStyleDataChanged();
}

void CAppForm::OnTextureList_Browse4()
{
	CFileDialog BrowseBox(true,"dtx",m_DebugParams.TextureList_Filename[3].c_str(),OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Image Files|*.*||");
	if (BrowseBox.DoModal() == IDOK)
	{
		m_pEB_TextureList_Filename[3]->SetWindowText(BrowseBox.GetPathName());
		m_DebugParams.TextureList_Filename[3] = BrowseBox.GetPathName();
	}
	DebugTextureSettingChanged();
	RenderStyleDataChanged();
}

void CAppForm::OnEffectFile_Browse1()
{

	std::string buffer;
	CFileDialog BrowseBox(true,"fx",buffer.c_str(),OFN_HIDEREADONLY|OFN_CREATEPROMPT,"DirectX Effect Files|*.*||");
	if (BrowseBox.DoModal() == IDOK)
	{
		 m_pEB_EffectFilename->SetWindowText(BrowseBox.GetPathName());
		
		//TODO: load the effect file...
		m_DebugParams.EffectFilename = BrowseBox.GetPathName();
	}
	DebugTextureSettingChanged();
	RenderStyleDataChanged();
}


// Grab the dialog items...
void CAppForm::InitializeDialogControls()
{

#ifndef _RS_VIEWER
	// Output window...
	m_pLB_OutputWindow				= (CListBox*)GetDlgItem(IDC_OUTPUT);						assert(m_pLB_OutputWindow);

	// Material Properties Controls...
	m_pWnd_LightMat_Ambient			= (CWnd*)GetDlgItem(IDC_MATERIALCOLOR_AMBIENT);				assert(m_pWnd_LightMat_Ambient);
	m_pWnd_LightMat_Diffuse			= (CWnd*)GetDlgItem(IDC_MATERIALCOLOR_DIFFUSE);				assert(m_pWnd_LightMat_Diffuse);
	m_pWnd_LightMat_Emissive		= (CWnd*)GetDlgItem(IDC_MATERIALCOLOR_EMISSIVE);			assert(m_pWnd_LightMat_Emissive);
	m_pWnd_LightMat_Specular		= (CWnd*)GetDlgItem(IDC_MATERIALCOLOR_SPECULAR);			assert(m_pWnd_LightMat_Specular);
	m_pBn_LightMat_Ambient_Config	= (CButton*)GetDlgItem(IDC_MATERIALCOLOR_AMBIENT_CONFIG);	assert(m_pBn_LightMat_Ambient_Config);
	m_pBn_LightMat_Diffuse_Config	= (CButton*)GetDlgItem(IDC_MATERIALCOLOR_DIFFUSE_CONFIG);	assert(m_pBn_LightMat_Diffuse_Config);
	m_pBn_LightMat_Emissive_Config	= (CButton*)GetDlgItem(IDC_MATERIALCOLOR_EMISSIVE_CONFIG);	assert(m_pBn_LightMat_Emissive_Config);
	m_pBn_LightMat_Specular_Config	= (CButton*)GetDlgItem(IDC_MATERIALCOLOR_SPECULAR_CONFIG);	assert(m_pBn_LightMat_Specular_Config);
	m_pEB_LightMat_Ambient_Alpha	= (CEdit*)GetDlgItem(IDC_MATERIAL_AMBIENTALPHA_EDIT);		assert(m_pEB_LightMat_Ambient_Alpha);
	m_pEB_LightMat_Diffuse_Alpha	= (CEdit*)GetDlgItem(IDC_MATERIAL_DIFFUSEALPHA_EDIT);		assert(m_pEB_LightMat_Diffuse_Alpha);
	m_pEB_LightMat_Emissive_Alpha	= (CEdit*)GetDlgItem(IDC_MATERIAL_EMISSIVEALPHA_EDIT);		assert(m_pEB_LightMat_Emissive_Alpha);
	m_pEB_LightMat_Specular_Alpha	= (CEdit*)GetDlgItem(IDC_MATERIAL_SPECULARALPHA_EDIT);		assert(m_pEB_LightMat_Specular_Alpha);
	m_pEB_LightMat_SpecularPower	= (CEdit*)GetDlgItem(IDC_MATERIAL_SPECULARPOWER_EDIT);		assert(m_pEB_LightMat_SpecularPower);

	// Texture Pass Controls...
	m_pBn_EnableEffect				= (CButton*)GetDlgItem(IDC_ENABLE_EFFECT);					assert(m_pBn_EnableEffect);
	m_pEB_EffectID					= (CEdit*)GetDlgItem(IDC_EFFECT_ID);						assert(m_pEB_EffectID);
	m_pBn_Enable_RenderPass[0]		= (CButton*)GetDlgItem(IDC_ENABLE_RENDERPASS1);				assert(m_pBn_Enable_RenderPass[0]);
	m_pBn_Enable_RenderPass[1]		= (CButton*)GetDlgItem(IDC_ENABLE_RENDERPASS2);				assert(m_pBn_Enable_RenderPass[1]);
	m_pBn_Enable_RenderPass[2]		= (CButton*)GetDlgItem(IDC_ENABLE_RENDERPASS3);				assert(m_pBn_Enable_RenderPass[2]);
	m_pBn_Enable_RenderPass[3]		= (CButton*)GetDlgItem(IDC_ENABLE_RENDERPASS4);				assert(m_pBn_Enable_RenderPass[3]);
	m_pBn_Config_RenderPass[0]		= (CButton*)GetDlgItem(IDC_CONFIG_RENDERPASS1);				assert(m_pBn_Config_RenderPass[0]);
	m_pBn_Config_RenderPass[1]		= (CButton*)GetDlgItem(IDC_CONFIG_RENDERPASS2);				assert(m_pBn_Config_RenderPass[1]);
	m_pBn_Config_RenderPass[2]		= (CButton*)GetDlgItem(IDC_CONFIG_RENDERPASS3);				assert(m_pBn_Config_RenderPass[2]);
	m_pBn_Config_RenderPass[3]		= (CButton*)GetDlgItem(IDC_CONFIG_RENDERPASS4);				assert(m_pBn_Config_RenderPass[3]);
#endif

	CSpinButtonCtrl* pSpin			= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN2);					if (pSpin) pSpin->SetRange32(0,255);
	pSpin							= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN3);					if (pSpin) pSpin->SetRange32(0,255);
	pSpin							= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4);					if (pSpin) pSpin->SetRange32(0,255);
	pSpin							= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN5);					if (pSpin) pSpin->SetRange32(0,255);
	pSpin							= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN6);					if (pSpin) pSpin->SetRange32(0,255);
	pSpin							= (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1);					if (pSpin) pSpin->SetRange32(0,3);

	// RenderStyle Editor: Debug Params...
	m_pBn_UseRefRast				= (CButton*)GetDlgItem(IDC_ENABLE_REFRAST);					assert(m_pBn_UseRefRast);
	m_pBn_RenderOnIdle				= (CButton*)GetDlgItem(IDC_DBGPARAM_RENDONIDLE);			assert(m_pBn_RenderOnIdle);
	m_pBn_RotateModel				= (CButton*)GetDlgItem(IDC_DBGPARAM_ROTATEMODEL);			assert(m_pBn_RotateModel);
	m_pCB_LightConfig				= (CComboBox*)GetDlgItem(IDC_DBGPARAM_LIGHTCONFIG);			assert(m_pCB_LightConfig);
	m_pEB_LightCount				= (CEdit*)GetDlgItem(IDC_DBGPARAM_LIGHTCOUNT);				assert(m_pEB_LightCount);
	m_pWnd_LightColor				= (CWnd*)GetDlgItem(IDC_DBGPARAM_LIGHTCOLOR);				assert(m_pWnd_LightColor);
	m_pBn_LightColor_Config			= (CButton*)GetDlgItem(IDC_DBGPARAM_LIGHTCOLOR_CONFIG);		assert(m_pBn_LightColor_Config);
	m_pSl_CameraRotateSpeed			= (CSliderCtrl*)GetDlgItem(IDC_DBGPARAM_ROTATESPEED);		assert(m_pSl_CameraRotateSpeed);
	m_pEB_BackGndImg_Filename		= (CEdit*)GetDlgItem(IDC_DBGPARAM_BACKGNDIMG_FILENAME);		assert(m_pEB_BackGndImg_Filename);
	m_pBn_BackGndImg_Browse			= (CButton*)GetDlgItem(IDC_DBGPARAM_BACKGNDIMG_BROWSE);		assert(m_pBn_BackGndImg_Browse);
	m_pWnd_BackGndColor				= (CWnd*)GetDlgItem(IDC_DBGPARAM_BACKGNDCOLOR);				assert(m_pWnd_BackGndColor);
	m_pBn_BackGndColor_Config		= (CButton*)GetDlgItem(IDC_DBGPARAM_BACKGNDCOLOR_CONFIG);	assert(m_pBn_BackGndColor_Config);
	m_pSl_CameraRotateSpeed->SetRange(-100,100,true); m_pSl_CameraRotateSpeed->SetPos(0);

	m_pCB_DBGParam_Model			= (CComboBox*)GetDlgItem(IDC_DBGPARAM_MODEL);				assert(m_pCB_DBGParam_Model);
	m_pCB_RenderStyle_Files		= (CComboBox*)GetDlgItem(IDC_RENDERSTYLE_FILENAME);				assert(m_pCB_RenderStyle_Files);
	m_pBn_TextureList_Browse[0]		= (CButton*)GetDlgItem(IDC_TEXTURELIST_BROWSE1);			assert(m_pBn_TextureList_Browse[0]);
	m_pBn_TextureList_Browse[1]		= (CButton*)GetDlgItem(IDC_TEXTURELIST_BROWSE2);			assert(m_pBn_TextureList_Browse[1]);
	m_pBn_TextureList_Browse[2]		= (CButton*)GetDlgItem(IDC_TEXTURELIST_BROWSE3);			assert(m_pBn_TextureList_Browse[2]);
	m_pBn_TextureList_Browse[3]		= (CButton*)GetDlgItem(IDC_TEXTURELIST_BROWSE4);			assert(m_pBn_TextureList_Browse[3]);
	m_pEB_TextureList_Filename[0]	= (CEdit*)GetDlgItem(IDC_TEXTURELIST_FILENAME1);			assert(m_pEB_TextureList_Filename[0]);
	m_pEB_TextureList_Filename[1]	= (CEdit*)GetDlgItem(IDC_TEXTURELIST_FILENAME2);			assert(m_pEB_TextureList_Filename[1]);
	m_pEB_TextureList_Filename[2]	= (CEdit*)GetDlgItem(IDC_TEXTURELIST_FILENAME3);			assert(m_pEB_TextureList_Filename[2]);
	m_pEB_TextureList_Filename[3]	= (CEdit*)GetDlgItem(IDC_TEXTURELIST_FILENAME4);			assert(m_pEB_TextureList_Filename[3]);
	m_pEB_EffectFilename			=  (CEdit*)GetDlgItem(IDC_EFFECTFILE_FILENAME1);			assert(m_pEB_EffectFilename);
}

void CAppForm::PopulateDialogControls_And_CreateIDtoRSEnum_Maps()
{
	// Debug Params...
	m_pCB_DBGParam_Model->AddString("Sphere (Low Poly)");
	m_pCB_DBGParam_Model->AddString("Sphere (High Poly)");
	m_pCB_DBGParam_Model->AddString("Box");
	m_pCB_DBGParam_Model->AddString("Cylinder");
	m_pCB_DBGParam_Model->AddString("Wall");
	m_pCB_DBGParam_Model->AddString("The Teapot");
	//m_pCB_DBGParam_Model->AddString("Seal"); //Later...

	/*
	m_pCB_DBGParam_Model->AddString("Severed Ant Head");
	m_pCB_DBGParam_Model->AddString("3 Bone Skinned Thing");
	m_pCB_DBGParam_Model->AddString("LowPoly Sphere w/ Basis Vectors");
	m_pCB_DBGParam_Model->AddString("HighPoly Sphere w/ Basis Vectors");
	m_pCB_DBGParam_Model->AddString("Box w/ Basis Vectors");
	m_pCB_DBGParam_Model->AddString("Cylinder w/ Basis Vectors");
	m_pCB_DBGParam_Model->AddString("Skinned Thing w/ Basis Vectors");
	*/

	m_pCB_LightConfig->AddString("Light Config 1");
	m_pCB_LightConfig->AddString("Light Config 2");
	m_pCB_LightConfig->AddString("Light Config 3"); 
	m_pCB_LightConfig->SetCurSel(0);

	// Set RenderStyle File text
	SetDlgItemText(IDC_RENDERSTYLE_FILENAME, "[Open a Project from the File Menu]");
}

void CAppForm::SetDialogControls_From_RenderStyleData(bool bInitialUpdate)
{
#ifndef _RS_VIEWER
	// RenderPass Data...

	m_pBn_EnableEffect->SetCheck(m_MainDlgData.Effect_Enable);
	char szBuf[16];
	sprintf(szBuf, "%d", m_MainDlgData.Effect_ID);
	m_pEB_EffectID->SetWindowText(szBuf);
	
	m_pBn_Enable_RenderPass[0]->SetCheck(m_MainDlgData.RenderPass_Enable[0]);
	m_pBn_Enable_RenderPass[1]->SetCheck(m_MainDlgData.RenderPass_Enable[1]);
	m_pBn_Enable_RenderPass[2]->SetCheck(m_MainDlgData.RenderPass_Enable[2]);
	m_pBn_Enable_RenderPass[3]->SetCheck(m_MainDlgData.RenderPass_Enable[3]);
#endif

#ifndef _RS_VIEWER
	// Material Lighting Data...
	char szBuff[32];
	m_pEB_LightMat_Ambient_Alpha->SetWindowText(itoa(FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.a),szBuff,10));
	m_pEB_LightMat_Diffuse_Alpha->SetWindowText(itoa(FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.a),szBuff,10));
	m_pEB_LightMat_Emissive_Alpha->SetWindowText(itoa(FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.a),szBuff,10));
	m_pEB_LightMat_Specular_Alpha->SetWindowText(itoa(FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.a),szBuff,10));
	m_pEB_LightMat_SpecularPower->SetWindowText(itoa((int)m_MainDlgData.LightingMaterial_SpecularPower,szBuff,10));
#endif

	// Debug Settings...
	char szTmp[32];
	m_pBn_UseRefRast->SetCheck(m_DebugParams.bUseRefRast); if (!bInitialUpdate) OnRefRast_Enable();																		// Need to trigger a change on all those that really need to know it...
	m_pBn_RenderOnIdle->SetCheck(m_DebugParams.bRenderOnIdle);
	m_pBn_RotateModel->SetCheck(m_DebugParams.bRotateModel);
	m_pCB_LightConfig->SetCurSel(m_DebugParams.LightConfig);
	sprintf(szTmp,"%d",m_DebugParams.LightCount); m_pEB_LightCount->SetWindowText(szTmp);
	m_pSl_CameraRotateSpeed->SetPos((int)(m_DebugParams.fCameraRotationSpeed * 100.0f / MAX_CAMERAROTATE_SPEED));
	if (stricmp(m_DebugParams.ModelName.c_str(),"LowPoly_Sphere")==0) m_pCB_DBGParam_Model->SetCurSel(0);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"HighPoly_Sphere")==0) m_pCB_DBGParam_Model->SetCurSel(1);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Box")==0) m_pCB_DBGParam_Model->SetCurSel(2);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Cylinder")==0) m_pCB_DBGParam_Model->SetCurSel(3);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Wall")==0) m_pCB_DBGParam_Model->SetCurSel(4);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"The_Teapot")==0) m_pCB_DBGParam_Model->SetCurSel(5);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Seal")==0) m_pCB_DBGParam_Model->SetCurSel(6);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Severed_Ant_Head")==0) m_pCB_DBGParam_Model->SetCurSel(6);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Three_Bone_Skinned_Thing")==0) m_pCB_DBGParam_Model->SetCurSel(7);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"LowPoly_Sphere_WithBasisVectors")==0) m_pCB_DBGParam_Model->SetCurSel(8);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"HighPoly_Sphere_WithBasisVectors")==0) m_pCB_DBGParam_Model->SetCurSel(9);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Box_WithBasisVectors")==0) m_pCB_DBGParam_Model->SetCurSel(10);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Cylinder_WithBasisVectors")==0) m_pCB_DBGParam_Model->SetCurSel(11);
	else if (stricmp(m_DebugParams.ModelName.c_str(),"Skinned_Thing_WithBasisVectors")==0) m_pCB_DBGParam_Model->SetCurSel(12);
	if (!bInitialUpdate) OnDbgParam_Model();
	m_pEB_TextureList_Filename[0]->SetWindowText(m_DebugParams.TextureList_Filename[0].c_str()); if (!bInitialUpdate) OnTextureList_Filename1();
	m_pEB_TextureList_Filename[1]->SetWindowText(m_DebugParams.TextureList_Filename[1].c_str()); if (!bInitialUpdate) OnTextureList_Filename2();
	m_pEB_TextureList_Filename[2]->SetWindowText(m_DebugParams.TextureList_Filename[2].c_str()); if (!bInitialUpdate) OnTextureList_Filename3();
	m_pEB_TextureList_Filename[3]->SetWindowText(m_DebugParams.TextureList_Filename[3].c_str()); if (!bInitialUpdate) OnTextureList_Filename4();
	m_pEB_EffectFilename->SetWindowText(m_DebugParams.EffectFilename.c_str());
}

// Set the render style data (CMainDlgData, CRenderPassData, etc) to the defaults...
void CAppForm::SetDefaultRenderStyleData()
{
	// Material Lighting Data...
	m_MainDlgData.LightingMaterial_Ambient.r			= 1.0f;
	m_MainDlgData.LightingMaterial_Ambient.g			= 1.0f;
	m_MainDlgData.LightingMaterial_Ambient.b			= 1.0f;
	m_MainDlgData.LightingMaterial_Ambient.a			= 1.0f;
	m_MainDlgData.LightingMaterial_Diffuse.r			= 0.8f;
	m_MainDlgData.LightingMaterial_Diffuse.g			= 0.8f;
	m_MainDlgData.LightingMaterial_Diffuse.b			= 0.8f;
	m_MainDlgData.LightingMaterial_Diffuse.a			= 1.0f;
	m_MainDlgData.LightingMaterial_Emissive.r			= 0.0f;
	m_MainDlgData.LightingMaterial_Emissive.g			= 0.0f;
	m_MainDlgData.LightingMaterial_Emissive.b			= 0.0f;
	m_MainDlgData.LightingMaterial_Emissive.a			= 1.0f;
	m_MainDlgData.LightingMaterial_Specular.r			= 0.0f;
	m_MainDlgData.LightingMaterial_Specular.g			= 0.0f;
	m_MainDlgData.LightingMaterial_Specular.b			= 0.0f;
	m_MainDlgData.LightingMaterial_Specular.a			= 1.0f;
	m_MainDlgData.LightingMaterial_SpecularPower		= 20.0f;
	if (m_pWnd_LightMat_Ambient)  { CRect rcWnd; m_pWnd_LightMat_Ambient->GetWindowRect(&rcWnd);  InvalidateRect(&rcWnd, FALSE); }
	if (m_pWnd_LightMat_Diffuse)  { CRect rcWnd; m_pWnd_LightMat_Diffuse->GetWindowRect(&rcWnd);  InvalidateRect(&rcWnd, FALSE); }
	if (m_pWnd_LightMat_Emissive) { CRect rcWnd; m_pWnd_LightMat_Emissive->GetWindowRect(&rcWnd); InvalidateRect(&rcWnd, FALSE); }
	if (m_pWnd_LightMat_Specular) { CRect rcWnd; m_pWnd_LightMat_Specular->GetWindowRect(&rcWnd); InvalidateRect(&rcWnd, FALSE); }

	// RenderPass Data...
	m_MainDlgData.Effect_Enable							= false;
	m_MainDlgData.Effect_ID								= 0;

	m_MainDlgData.RenderPass_Enable[0]					= true;
	m_MainDlgData.RenderPass_Enable[1]					= false;
	m_MainDlgData.RenderPass_Enable[2]					= false;
	m_MainDlgData.RenderPass_Enable[3]					= false;

	// RenderPassDlg Data...
	for (uint16 i=0;i<4;++i)
	{
		// ID
		m_RenderPassData[i].id = i;

		// Render State Data...
		m_RenderPassData[i].AlphaRef					= 128;
		m_RenderPassData[i].TextureFactor				= 0x80808080;
		m_RenderPassData[i].AlphaTestMode				= RENDERSTYLE_NOALPHATEST;
		m_RenderPassData[i].ZBufferTestMode				= RENDERSTYLE_ALPHATEST_LESSEQUAL;
		m_RenderPassData[i].FillMode					= RENDERSTYLE_FILL;
		m_RenderPassData[i].CullMode					= RENDERSTYLE_CULL_CCW;
		if (i == 0) m_RenderPassData[i].DynamicLight	= true;
		else m_RenderPassData[i].DynamicLight			= false;
		if (i == 0) m_RenderPassData[i].Blend			= RENDERSTYLE_NOBLEND;
		else m_RenderPassData[i].Blend					= RENDERSTYLE_BLEND_MOD_SRCALPHA;
		if (i == 0) m_RenderPassData[i].ZBuffer			= RENDERSTYLE_ZRW;
		else m_RenderPassData[i].ZBuffer				= RENDERSTYLE_ZRO;

		// Texture State Data...
		m_RenderPassData[i].TextureParam[0]				= RENDERSTYLE_USE_TEXTURE1;
		m_RenderPassData[i].TextureParam[1]				= RENDERSTYLE_NOTEXTURE;
		m_RenderPassData[i].TextureParam[2]				= RENDERSTYLE_NOTEXTURE;
		m_RenderPassData[i].TextureParam[3]				= RENDERSTYLE_NOTEXTURE;
		m_RenderPassData[i].ColorOp[0]					= RENDERSTYLE_COLOROP_MODULATE;
		m_RenderPassData[i].ColorOp[1]					= RENDERSTYLE_COLOROP_DISABLE;
		m_RenderPassData[i].ColorOp[2]					= RENDERSTYLE_COLOROP_DISABLE;
		m_RenderPassData[i].ColorOp[3]					= RENDERSTYLE_COLOROP_DISABLE;
		m_RenderPassData[i].ColorArg1[0]				= RENDERSTYLE_COLORARG_TEXTURE;
		m_RenderPassData[i].ColorArg1[1]				= RENDERSTYLE_COLORARG_TEXTURE;
		m_RenderPassData[i].ColorArg1[2]				= RENDERSTYLE_COLORARG_TEXTURE;
		m_RenderPassData[i].ColorArg1[3]				= RENDERSTYLE_COLORARG_TEXTURE;
		m_RenderPassData[i].ColorArg2[0]				= RENDERSTYLE_COLORARG_DIFFUSE;
		m_RenderPassData[i].ColorArg2[1]				= RENDERSTYLE_COLORARG_CURRENT;
		m_RenderPassData[i].ColorArg2[2]				= RENDERSTYLE_COLORARG_CURRENT;
		m_RenderPassData[i].ColorArg2[3]				= RENDERSTYLE_COLORARG_CURRENT;
		m_RenderPassData[i].AlphaOp[0]					= RENDERSTYLE_ALPHAOP_MODULATE;
		m_RenderPassData[i].AlphaOp[1]					= RENDERSTYLE_ALPHAOP_DISABLE;
		m_RenderPassData[i].AlphaOp[2]					= RENDERSTYLE_ALPHAOP_DISABLE;
		m_RenderPassData[i].AlphaOp[3]					= RENDERSTYLE_ALPHAOP_DISABLE;
		m_RenderPassData[i].AlphaArg1[0]				= RENDERSTYLE_ALPHAARG_TEXTURE;
		m_RenderPassData[i].AlphaArg1[1]				= RENDERSTYLE_ALPHAARG_TEXTURE;
		m_RenderPassData[i].AlphaArg1[2]				= RENDERSTYLE_ALPHAARG_TEXTURE;
		m_RenderPassData[i].AlphaArg1[3]				= RENDERSTYLE_ALPHAARG_TEXTURE;
		m_RenderPassData[i].AlphaArg2[0]				= RENDERSTYLE_ALPHAARG_DIFFUSE;
		m_RenderPassData[i].AlphaArg2[1]				= RENDERSTYLE_ALPHAARG_CURRENT;
		m_RenderPassData[i].AlphaArg2[2]				= RENDERSTYLE_ALPHAARG_CURRENT;
		m_RenderPassData[i].AlphaArg2[3]				= RENDERSTYLE_ALPHAARG_CURRENT;
		m_RenderPassData[i].UVSource[0]					= RENDERSTYLE_UVFROM_MODELDATA_UVSET1;
		m_RenderPassData[i].UVSource[1]					= RENDERSTYLE_UVFROM_MODELDATA_UVSET2;
		m_RenderPassData[i].UVSource[2]					= RENDERSTYLE_UVFROM_MODELDATA_UVSET3;
		m_RenderPassData[i].UVSource[3]					= RENDERSTYLE_UVFROM_MODELDATA_UVSET4;
		m_RenderPassData[i].UAddress[0]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].UAddress[1]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].UAddress[2]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].UAddress[3]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].VAddress[0]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].VAddress[1]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].VAddress[2]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].VAddress[3]					= RENDERSTYLE_UVADDR_WRAP;
		m_RenderPassData[i].TexFilter[0]				= RENDERSTYLE_TEXFILTER_TRILINEAR;
		m_RenderPassData[i].TexFilter[1]				= RENDERSTYLE_TEXFILTER_TRILINEAR;
		m_RenderPassData[i].TexFilter[2]				= RENDERSTYLE_TEXFILTER_TRILINEAR;
		m_RenderPassData[i].TexFilter[3]				= RENDERSTYLE_TEXFILTER_TRILINEAR;
		m_RenderPassData[i].UVTransform_Enable[0]		= false;
		m_RenderPassData[i].UVTransform_Enable[1]		= false;
		m_RenderPassData[i].UVTransform_Enable[2]		= false;
		m_RenderPassData[i].UVTransform_Enable[3]		= false;
		m_RenderPassData[i].TexCoordCount[0]			= 2;
		m_RenderPassData[i].TexCoordCount[1]			= 2;
		m_RenderPassData[i].TexCoordCount[2]			= 2;
		m_RenderPassData[i].TexCoordCount[3]			= 2;
		m_RenderPassData[i].ProjectTexCoord[0]			= false;
		m_RenderPassData[i].ProjectTexCoord[1]			= false;
		m_RenderPassData[i].ProjectTexCoord[2]			= false;
		m_RenderPassData[i].ProjectTexCoord[3]			= false;

		D3DXMatrixIdentity(&m_RenderPassData[i].UVTransform_Matrix[0]);
		D3DXMatrixIdentity(&m_RenderPassData[i].UVTransform_Matrix[1]);
		D3DXMatrixIdentity(&m_RenderPassData[i].UVTransform_Matrix[2]);
		D3DXMatrixIdentity(&m_RenderPassData[i].UVTransform_Matrix[3]);

		// Direct3D Data...
		m_RenderPassData[i].VertexShader_Enable								= false;
		m_RenderPassData[i].VertexShader_ID									= 0;
		m_RenderPassData[i].PixelShader_Enable								= false;
		m_RenderPassData[i].PixelShader_ID									= 0;
/*
		m_RenderPassData[i].Direct3DData.VertexShaderFilename.erase(m_RenderPassData[i].Direct3DData.VertexShaderFilename.begin(),m_RenderPassData[i].Direct3DData.VertexShaderFilename.end());
		m_RenderPassData[i].Direct3DData.bExpandForSkinning					= false;
		m_RenderPassData[i].Direct3DData.ConstVector_ConstReg1				= -1;
		m_RenderPassData[i].Direct3DData.ConstVector_Param1					= FourFloatVector(0.0f,0.5f,1.0f,2.0f);
		m_RenderPassData[i].Direct3DData.ConstVector_ConstReg2				= -1;
		m_RenderPassData[i].Direct3DData.ConstVector_Param2					= FourFloatVector(0.0f,0.0f,0.0f,0.0f);
		m_RenderPassData[i].Direct3DData.ConstVector_ConstReg3				= -1;
		m_RenderPassData[i].Direct3DData.ConstVector_Param3					= FourFloatVector(0.0f,0.0f,0.0f,0.0f);
		m_RenderPassData[i].Direct3DData.WorldViewTransform_ConstReg		= -1;
		m_RenderPassData[i].Direct3DData.WorldViewTransform_Count			= 1;
		m_RenderPassData[i].Direct3DData.ProjTransform_ConstReg				= -1;
		m_RenderPassData[i].Direct3DData.WorldViewProjTransform_ConstReg	= 0;
		m_RenderPassData[i].Direct3DData.ViewProjTransform_ConstReg			= -1;
		m_RenderPassData[i].Direct3DData.CamPos_MSpc_ConstReg				= -1;
		m_RenderPassData[i].Direct3DData.Light_Count						= 1;
		m_RenderPassData[i].Direct3DData.LightPosition_MSpc_ConstReg		= 4;
		m_RenderPassData[i].Direct3DData.LightPosition_CSpc_ConstReg		= -1;
		m_RenderPassData[i].Direct3DData.LightColor_ConstReg				= -1;
		m_RenderPassData[i].Direct3DData.LightAtt_ConstReg					= -1;
		m_RenderPassData[i].Direct3DData.Material_AmbDifEm_ConstReg			= -1;
		m_RenderPassData[i].Direct3DData.Material_Specular_ConstReg			= -1;
		m_RenderPassData[i].Direct3DData.AmbientLight_ConstReg				= -1;
		m_RenderPassData[i].Direct3DData.PrevWorldViewTrans_ConstReg		= -1;
		m_RenderPassData[i].Direct3DData.PrevWorldViewTrans_Count			= 1;
		m_RenderPassData[i].Direct3DData.Last_ConstReg						= 5;
 */
		m_RenderPassData[i].bUseBumpEnvMap									= false;
		m_RenderPassData[i].BumpEnvMapStage									= 1;
		m_RenderPassData[i].fBumpEnvMap_Scale								= 1.0f;
		m_RenderPassData[i].fBumpEnvMap_Offset								= 0.0f;
/*
		m_RenderPassData[i].Direct3DData.bDeclaration_Stream_Position[0]	= true;
		for (uint32 z=1;z<4;++z) m_RenderPassData[i].Direct3DData.bDeclaration_Stream_Position[z] = false;
		m_RenderPassData[i].Direct3DData.bDeclaration_Stream_Normal[0]		= true;
		for (z=1;z<4;++z) m_RenderPassData[i].Direct3DData.bDeclaration_Stream_Normal[z]		= false;
		m_RenderPassData[i].Direct3DData.bDeclaration_Stream_UVSets[0]		= true;
		for (z=1;z<4;++z) m_RenderPassData[i].Direct3DData.bDeclaration_Stream_UVSets[z]		= false;
		m_RenderPassData[i].Direct3DData.Declaration_Stream_UVCount[0]		= 1;
		for (z=1;z<4;++z) m_RenderPassData[i].Direct3DData.Declaration_Stream_UVCount[z]		= 0;
		m_RenderPassData[i].Direct3DData.bDeclaration_Stream_BasisVectors[0]= false;
		for (z=1;z<4;++z) m_RenderPassData[i].Direct3DData.bDeclaration_Stream_BasisVectors[z]= false;
 */
	}

	// Debug Params...
	m_DebugParams.bUseRefRast							= false; //Don't default to the ref rasterizer
	m_DebugParams.bRenderOnIdle							= false;
	m_DebugParams.bRotateModel							= false;
	m_DebugParams.LightConfig							= 0;
	m_DebugParams.LightCount							= 2;
	m_DebugParams.LightColor							= FourFloatColor(0.5f,0.5f,0.5f,1.0f); if (m_pWnd_LightColor) { CRect rcWnd; m_pWnd_LightColor->GetWindowRect(&rcWnd); InvalidateRect(&rcWnd, FALSE); }
	m_DebugParams.fCameraRotationSpeed					= 0.0f;
	m_DebugParams.BackGndImg_Filename.erase(m_DebugParams.BackGndImg_Filename.begin(),m_DebugParams.BackGndImg_Filename.end()); m_pEB_BackGndImg_Filename->SetWindowText("");
	m_DebugParams.BackGndImg_Color						= FourFloatColor(0.0f,0.05f,0.3f,1.0f); if (m_pWnd_LightColor) { CRect rcWnd; m_pWnd_BackGndColor->GetWindowRect(&rcWnd); InvalidateRect(&rcWnd, FALSE); }
	m_DebugParams.ModelName								= "LowPoly_Sphere";
	m_DebugParams.TextureList_Filename[0].erase(m_DebugParams.TextureList_Filename[0].begin(),m_DebugParams.TextureList_Filename[0].end());
	m_DebugParams.TextureList_Filename[1].erase(m_DebugParams.TextureList_Filename[1].begin(),m_DebugParams.TextureList_Filename[1].end());
	m_DebugParams.TextureList_Filename[2].erase(m_DebugParams.TextureList_Filename[2].begin(),m_DebugParams.TextureList_Filename[2].end());
	m_DebugParams.TextureList_Filename[3].erase(m_DebugParams.TextureList_Filename[3].begin(),m_DebugParams.TextureList_Filename[3].end());
	m_MouseLook_Rotation								= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_MouseLook_Translation								= D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_fModelRotation									= 0.0f;
	m_fCameraRotation									= 0.0f;
}

//	When the AppForm object is created, this function is called to initialize it. Here we getting
// access ptrs to some of the controls, and setting the initial state of some of them as well.
void CAppForm::OnInitialUpdate()
{
	// Update the UI
	CFormView::OnInitialUpdate();
	EnableToolTips(TRUE);
	InitializeDialogControls();

	// Save static reference to the render window
	g_hWnd_RenderWindow = GetDlgItem(IDC_RENDERVIEW)->GetSafeHwnd();

	// Init our m_RenderPassData[] members...
	m_RenderPassData[0].DialogText = "Render Pass Dialog: 1st Pass...";	// The dialog sets this as it's title when it's created...
	m_RenderPassData[1].DialogText = "Render Pass Dialog: 2nd Pass...";
	m_RenderPassData[2].DialogText = "Render Pass Dialog: 3rd Pass...";
	m_RenderPassData[3].DialogText = "Render Pass Dialog: 4th Pass...";

	// Populate our dialog controls (and make our maps)...
	PopulateDialogControls_And_CreateIDtoRSEnum_Maps();

	// Set our data to the defaults & then update our dialog to those values...
	SetDefaultRenderStyleData();
	SetDialogControls_From_RenderStyleData(true);

	// Get our starting directory...
	getcwd(g_StartingDirectory,MAX_PATH);
	OutputMsg("CWD: %s.",g_StartingDirectory);

	// Create the 3D Env...
	if (Create3DEnv()) m_bReady = true;
}

// Load a texture file...
LPDXTexture CAppForm::LoadTexture(const char* szFilename)
{
	//LPDIRECT3DTEXTURE9 pTexture = NULL;
	char* szDot = strstr(szFilename,".");

	if ( szDot && (szDot[0]=='.') && (szDot[1]=='d') && (szDot[2]=='t') && (szDot[3]=='x') ) 
	{
		// Let the DTX file loader handle it...
		return LoadDTXFile(szFilename); 
	}

	// We only load DTX files now.
	
	else 
	{

		D3DXIMAGE_INFO info;
		HRESULT hr = D3DXGetImageInfoFromFile( szFilename, &info);
		if(hr == D3D_OK)
		{
			switch(info.ResourceType)
			{
			case D3DRTYPE_TEXTURE:
				{
					LPDIRECT3DTEXTURE9 pTexture;
					if(D3DXCreateTextureFromFile(PD3DDEVICE, szFilename, &pTexture) == D3D_OK)
					{
						DXTexture *pDXTexture = new DXTexture;
						if(pDXTexture)
						{
							pDXTexture->m_eTexType = DXTEXTYPE_2D;
							pDXTexture->m_pTexture = pTexture;
							return pDXTexture;
						}
					}
				}
				break;
			case D3DRTYPE_VOLUMETEXTURE:
				{
					// Support this one day maybe...
				}
				break;
			case D3DRTYPE_CUBETEXTURE:
				{
					LPDIRECT3DCUBETEXTURE9 pTexture;
					if(D3DXCreateCubeTextureFromFile(PD3DDEVICE, szFilename, &pTexture) == D3D_OK)
					{
						DXTexture *pDXTexture = new DXTexture;
						if(pDXTexture)
						{
							pDXTexture->m_eTexType = DXTEXTYPE_CUBE;
							pDXTexture->m_pCubeTexture = pTexture;
							return pDXTexture;
						}
					}
				}
				break;
			default:
				break;
			};

			/*
			// Not a DTX file - let D3DX handle it...
			if (D3DXCreateTextureFromFile(PD3DDEVICE,szFilename,&pTexture) == D3D_OK) 
			{
			return pTexture; 
			} 
			*/
		}
		

	}
	

	return NULL;
}

void CAppForm::OnCompilePCD3D()
{
	SetRenderStyleFromRenderStyleData();

	// Export the file...
	LithTry {
		// Save off a TEMP LTA File...
		string szTmpFileName = g_StartingDirectory;

		//make sure it has a trailing slash
		if (!szTmpFileName.empty() && (szTmpFileName[szTmpFileName.size() - 1] != '\\'))
			szTmpFileName += '\\';

		szTmpFileName += "TMP_ModelEditLTAFile.lta";
		if (!SaveRenderStyle(szTmpFileName.c_str())) {
			OutputMsg("Error saving tmp renderstyle: %s",szTmpFileName);
			Msg("Error saving renderstyle: %s",szTmpFileName); return; }

		// Call the Model_Packer to convert it to LTB...
		const char* args[10]; string szOutput1,szOutput2;
		string szName      = "RenderStyle_Packer.exe";		args[0] = szName.c_str();
		string szPlatform1 = "-platform";					args[1] = szPlatform1.c_str();
		string szPlatform2 = "d3d";							args[2] = szPlatform2.c_str();
		string szInput1    = "-input1";						args[3] = szInput1.c_str();
		string szInput2    = "\""; szInput2 += szTmpFileName.c_str(); szInput2 += "\""; args[4] = szInput2.c_str();
		string szWin	   = "-win";						args[5] = szWin.c_str();
		string szNoAuto	   = "-noautostart";				args[6] = szNoAuto.c_str();
		args[7] = NULL;
		if (m_RenderStyle_FileName[0]) {
			char szTmp[MAX_PATH]; strcpy(szTmp,m_RenderStyle_FileName);
			char* szDot = strstr(szTmp,".");
			if (szDot && szDot[0]=='.' && szDot[1]=='l' && szDot[2]=='t' && szDot[3]=='a') {
				szDot[3] = 'b';
				szOutput1 = "-output";						args[7] = szOutput1.c_str();
				szOutput2 = "\""; szOutput2 += szTmp; szOutput2 += "\""; args[8] = szOutput2.c_str(); args[9] = NULL; } }
		string szRun       = g_StartingDirectory; szRun += "\\RenderStyle_Packer.exe";

		if (_spawnv(_P_WAIT,szRun.c_str(),args) < 0) {
			MessageBox ("Error: Could not export file.\nCheck that RenderStyle_Packer.exe is located in \nthe RenderStylesEditor launch dir.", "File Error", MB_OK | MB_ICONEXCLAMATION);
			return; }

		DeleteFile(szTmpFileName.c_str()); }		// Delete that temp LTA file...
	LithCatch(CLithException &exception) {
		exception = exception;
		MessageBox ("Error: Could not export file", "File Error", MB_OK | MB_ICONEXCLAMATION);
		return; }
}
/*
void CAppForm::OnCompilePS2()
{
	SetRenderStyleFromRenderStyleData();

	// Export the file...
	LithTry {
		// Save off a TEMP LTA File...
		string szTmpFileName = g_StartingDirectory;

		//make sure it has a trailing slash
		if (!szTmpFileName.empty() && (szTmpFileName[szTmpFileName.size() - 1] != '\\'))
			szTmpFileName += '\\';

		szTmpFileName += "TMP_ModelEditLTAFile.lta";
		if (!SaveRenderStyle(szTmpFileName.c_str())) {
			OutputMsg("Error saving tmp renderstyle: %s",szTmpFileName);
			Msg("Error saving renderstyle: %s",szTmpFileName); return; }

		// Call the Model_Packer to convert it to LTB...
		const char* args[10]; string szOutput1,szOutput2;
		string szName      = "RenderStyle_Packer.exe";		args[0] = szName.c_str();
		string szPlatform1 = "-platform";					args[1] = szPlatform1.c_str();
		string szPlatform2 = "ps2";							args[2] = szPlatform2.c_str();
		string szInput1    = "-input1";						args[3] = szInput1.c_str();
		string szInput2    = "\""; szInput2 += szTmpFileName.c_str(); szInput2 += "\""; args[4] = szInput2.c_str();
		string szWin	   = "-win";						args[5] = szWin.c_str();
		string szNoAuto	   = "-noautostart";				args[6] = szNoAuto.c_str();
		args[7] = NULL;
		if (m_RenderStyle_FileName[0]) {
			char szTmp[MAX_PATH]; strcpy(szTmp,m_RenderStyle_FileName);
			char* szDot = strstr(szTmp,".");
			if (szDot && szDot[0]=='.' && szDot[1]=='l' && szDot[2]=='t' && szDot[3]=='a') {
				szDot[3] = 'b';
				szOutput1 = "-output";						args[7] = szOutput1.c_str();
				szOutput2 = "\""; szOutput2 += szTmp; szOutput2 += "\""; args[8] = szOutput2.c_str(); args[9] = NULL; } }
		string szRun       = g_StartingDirectory; szRun += "\\RenderStyle_Packer.exe";

		if (_spawnv(_P_WAIT,szRun.c_str(),args) < 0) {
			MessageBox ("Error: Could not export file.\nCheck that RenderStyle_Packer.exe is located in \nthe RenderStylesEditor launch dir.", "File Error", MB_OK | MB_ICONEXCLAMATION);
			return; }

		DeleteFile(szTmpFileName.c_str()); }		// Delete that temp LTA file...
	LithCatch(CLithException &exception) {
		exception = exception;
		MessageBox ("Error: Could not export file", "File Error", MB_OK | MB_ICONEXCLAMATION);
		return; }
}
*/


void CAppForm::OnRefRast_Enable()
{
	m_bResetingRaster			= true;
	m_DebugParams.bUseRefRast = m_pBn_UseRefRast->GetCheck() ? true : false;

	for (uint32 i = 0; i < m_TextureArray.size(); ++i) 
	//{				// Free all the textures we have loaded...
		{
			if (m_TextureArray[i]) 
			{ 
				m_TextureArray[i]->Release(); 
				m_TextureArray[i] = NULL; 
			} 
		}
	m_TextureArray.clear();

	if (m_BackGndTexture) { 											// Free the background image...
   		m_BackGndTexture->Release(); m_BackGndTexture = NULL; }

	if ((g_Device.GetDeviceInfo()->DeviceType == D3DDEVTYPE_REF && !m_DebugParams.bUseRefRast) ||
		(g_Device.GetDeviceInfo()->DeviceType != D3DDEVTYPE_REF &&  m_DebugParams.bUseRefRast)) {
		Destroy3DEnv();													// Destroy the 3DEnv & Re-Create...
		if (Create3DEnv()) m_bReady = true; }

	DebugTextureSettingChanged();										// Force the textures to be re-loaded...
	RenderStyleDataChanged();
	BackGroundImageChanged();
	m_bResetingRaster			= false;
}

void CAppForm::OnBnClickedEnableEffect()
{
	int nCheck = m_pBn_EnableEffect->GetCheck();

	bool bCheck = true;
	if(nCheck)
	{
		bCheck = false;
	}

	RSD3DOptions rsD3DOptions;
	rsD3DOptions.bUseEffectShader = !!nCheck;

	g_EffectMgr.SetEnabled(rsD3DOptions.bUseEffectShader);

	char szBuf[32];
	rsD3DOptions.EffectShaderID = m_pEB_EffectID->GetWindowText(szBuf, 32);
	rsD3DOptions.EffectShaderID = atoi(szBuf);
	m_pRenderStyle->SetDirect3D_Options(rsD3DOptions);

	// Make sure to set the dialog form options too!
	m_MainDlgData.Effect_Enable = rsD3DOptions.bUseEffectShader;
	m_MainDlgData.Effect_ID = rsD3DOptions.EffectShaderID;

	CheckEnableEffect();
	RenderStyleDataChanged(true);
}

bool CAppForm::CreateTheRenderObject()
{
	// Destroy the old render object...
	if (m_pRenderObject)		{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; }
	if (m_pAntEyeRenderObject)	{ g_Device.DestroyRenderObject(m_pAntEyeRenderObject); m_pAntEyeRenderObject = NULL; }

	uint32 iCurrentSelection = m_pCB_DBGParam_Model->GetCurSel();
	uint32 RenderObjectID; bool IsSkel = false;
	switch (iCurrentSelection) {
	case 0 : m_DebugParams.ModelName = "LowPoly_Sphere";
   		RenderObjectID = IDR_LOWPOLYSPHERE; break;
	case 1 : m_DebugParams.ModelName = "HighPoly_Sphere";
   		RenderObjectID = IDR_HIGHPOLYSPHERE; break;
	case 2 : m_DebugParams.ModelName = "Box";
   		RenderObjectID = IDR_BOX; break;
	case 3 : m_DebugParams.ModelName = "Cylinder";
   		RenderObjectID = IDR_CYLINDER; break;
	case 4 : m_DebugParams.ModelName = "Wall";
   		RenderObjectID = IDR_WALL; break;
	case 5 : m_DebugParams.ModelName = "The_Teapot";
   		RenderObjectID = IDR_TEAPOT; break;
	case 6 : m_DebugParams.ModelName = "Seal"; IsSkel = true;
   		RenderObjectID = IDR_SKINNEDTHING;/*IDR_ANTHEADNOEYES;*/ break;
	case 7 : m_DebugParams.ModelName = "Three_Bone_Skinned_Thing"; IsSkel = true;
   		RenderObjectID = IDR_SKINNEDTHING; break;
	case 8 : m_DebugParams.ModelName = "LowPoly_Sphere_WithBasisVectors";
   		RenderObjectID = IDR_LOWPOLYSPHERE_WITH_BASISVECTORS; break;
	case 9 : m_DebugParams.ModelName = "HighPoly_Sphere_WithBasisVectors";
   		RenderObjectID = IDR_HIGHPOLYSPHERE_WITH_BASISVECTORS; break;
	case 10: m_DebugParams.ModelName = "Box_WithBasisVectors";
   		RenderObjectID = IDR_BOX_WITH_BASISVECTORS; break;
	case 11: m_DebugParams.ModelName = "Cylinder_WithBasisVectors";
   		RenderObjectID = IDR_CYLINDER_WITH_BASISVECTORS; break;
	case 12: m_DebugParams.ModelName = "Skinned_Thing_WithBasisVectors"; IsSkel = true;
   		RenderObjectID = IDR_SKINNEDTHING_WITH_BASISVECTORS; break;
	default : assert(0 && "Unknown Model Type"); return false; }

	if (!IsSkel) {														// Rigid...
		if (!(m_pRenderObject = (CD3DRigidMesh*)g_Device.CreateRenderObject(CRenderObject::eRigidMesh))) { Msg("Can't create render object!"); return false; }
		HINSTANCE hInst = ::AfxGetInstanceHandle();
		HRSRC hRc	 = FindResource(hInst,MAKEINTRESOURCE(RenderObjectID),"RENDEROBJECT"); if (!hRc) { g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; }
		HGLOBAL hGlb = LoadResource(hInst,hRc);	if (!hGlb)				{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; }
		void* pData  = LockResource(hGlb); if (!pData)					{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; }
		if (!((CD3DRigidMesh*)m_pRenderObject)->Load((uint8*)pData))	{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; } }
	else {																// Skel...
		if (!(m_pRenderObject = (CD3DSkelMesh*)g_Device.CreateRenderObject(CRenderObject::eSkelMesh)))   { Msg("Can't create render object!"); return false; }
		HRSRC hRc	 = FindResource(NULL,MAKEINTRESOURCE(RenderObjectID),"RENDEROBJECT"); if (!hRc) { g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; }
		HGLOBAL hGlb = LoadResource(NULL,hRc); if (!hGlb)				{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; }
		void* pData  = LockResource(hGlb); if (!pData)					{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; }
		if (!((CD3DSkelMesh*)m_pRenderObject)->Load((uint8*)pData))		{ g_Device.DestroyRenderObject(m_pRenderObject); m_pRenderObject = NULL; Msg("Can't load render object!"); return false; } }

	if (iCurrentSelection == 6) {										// Special case the ant head (to draw the eyes as well)...
		RenderObjectID = IDR_ANTEYES;
		if (!(m_pAntEyeRenderObject = (CD3DRigidMesh*)g_Device.CreateRenderObject(CRenderObject::eRigidMesh))) { Msg("Can't create render object!"); }
		HRSRC hRc	 = FindResource(NULL,MAKEINTRESOURCE(RenderObjectID),"RENDEROBJECT"); if (!hRc) { g_Device.DestroyRenderObject(m_pAntEyeRenderObject); m_pAntEyeRenderObject = NULL; }
		HGLOBAL hGlb = LoadResource(NULL,hRc); if (!hGlb)				{ g_Device.DestroyRenderObject(m_pAntEyeRenderObject); m_pAntEyeRenderObject = NULL; }
		void* pData  = LockResource(hGlb); if (!pData)					{ g_Device.DestroyRenderObject(m_pAntEyeRenderObject); m_pAntEyeRenderObject = NULL; }
		if (!m_pAntEyeRenderObject->Load((uint8*)pData))				{ g_Device.DestroyRenderObject(m_pAntEyeRenderObject); m_pAntEyeRenderObject = NULL; } }

	return true;
}

void CAppForm::OnDbgParam_Model()
{
   	CreateTheRenderObject();
   	RenderScene();														// Force an update...
}

void CAppForm::OnRenderStyle_Filename()
{
	char buffer[255], buffer2[255], filename[255];

	if(m_pCB_RenderStyle_Files->GetCurSel() == CB_ERR)  return; //invalid selection

	// Find filename from combobox in list of RenderStyles in project
	m_pCB_RenderStyle_Files->GetLBText(m_pCB_RenderStyle_Files->GetCurSel(), filename);

	int32 nLBItem = -1;
	for (uint32 i=0; i < m_RenderStyle_Files; i++)
	{
		if (strcmp(m_RenderStyle_Files[i]->file, filename) == 0)  nLBItem = i;
	}
	if (nLBItem == -1)  return;

	strcpy(m_RenderStyle_FileName, m_RenderStyle_Files[nLBItem]->path);
	m_bLoading				= true;
	SetDefaultRenderStyleData();					// Reset us...
	if (LoadRenderStyle(m_RenderStyle_FileName))
	{
			m_bNoRecompile		= true;
			SetRenderStyleDataFromRenderStyle();
			SetDialogControls_From_RenderStyleData(true);
			m_bNoRecompile		= false;
			OnRefRast_Enable();							// Make sure we've got the right rast mode...
			OnDbgParam_Model();							// Make sure the model is up to date...
			OutputMsg("Renderstyle %s loaded successfully.",m_RenderStyle_FileName);
			SetRenderStyleFromRenderStyleData();

			// Change title bar
			_splitpath(m_RenderStyle_FileName, buffer, buffer, buffer, buffer2);
			sprintf(filename, "%s%s", buffer, buffer2);
#ifndef _RS_VIEWER
			sprintf(buffer, "Render Styles Editor - %s", filename);
#else
			sprintf(buffer, "Render Styles Viewer - %s", filename);
#endif
			g_App.m_pMainWnd->SetWindowText(buffer);
	}
	else
	{
			OutputMsg("Error loading renderstyle: %s",m_RenderStyle_FileName);
	}
	m_bLoading = false;

	RenderScene();														// Force an update...
}

void CAppForm::OutputMsg(LPSTR fmt, ...)
{
	if (!m_pLB_OutputWindow) return;
	int32 iIndex = m_pLB_OutputWindow->GetCount();
	char buff[1024]; 
	va_list va; 
	int32 iByte = sprintf(buff,"%d: ",iIndex);

	va_start(va, fmt); 
	wvsprintf(&buff[iByte], fmt, va); 
	va_end(va);

   	iIndex = m_pLB_OutputWindow->AddString(buff);
   	m_pLB_OutputWindow->SetTopIndex(iIndex);
}

bool CAppForm::GetColor(int* pRed, int* pGreen, int* pBlue)
{
	CColorDialog dlg(RGB(*pRed, *pGreen, *pBlue), CC_FULLOPEN);

	if (dlg.DoModal() == IDOK) {
		COLORREF cref = dlg.GetColor();
		*pRed = cref & 0x000000FF;
		*pGreen = (cref & 0x0000FF00) >> 8;
		*pBlue  = (cref & 0x00FF0000) >> 16;
		return true; }

	return false;
}

void CAppForm::DrawColor(CWnd *pWnd, int r, int g, int b)
{
	if (!pWnd) return;
	CRect rcWnd; pWnd->GetWindowRect(&rcWnd);

	ScreenToClient(&rcWnd);
	CDC* pDC = GetDC();
	if (pDC) {
		pDC->FillSolidRect(rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), RGB(r, g, b)); }
}

void CAppForm::DrawHelp(CWnd *pWnd)
{
	CRect rcWnd; pWnd->GetWindowRect(&rcWnd);

	ScreenToClient(&rcWnd);
	CDC* pDC = GetDC();

	if (pDC)
	{
		CBitmap bitmap;

#ifdef _RS_VIEWER
		bitmap.LoadBitmap(IDR_HELPSCREEN_VIEWER);
#else
		bitmap.LoadBitmap(IDR_HELPSCREEN);
#endif

		CDC dcMem;
		dcMem.CreateCompatibleDC(pDC);

		if (dcMem.SelectObject(bitmap)!=ERROR)
		{
			dcMem.SetMapMode(pDC->GetMapMode());
			pDC->BitBlt(rcWnd.left+4, rcWnd.top+4, rcWnd.Width(), rcWnd.Height(),&dcMem,0,0,SRCCOPY);
		}
	}
}

void CAppForm::OnProjectOpen()
{
	//FilenameNode  fileListRoot, *current;
	char buffer[256], resourcePath[512], drive[16];
	CFileDialog BrowseBox(true,"dep",NULL,OFN_CREATEPROMPT,"Project File (*.dep)|*.dep||");
	if (BrowseBox.DoModal() == IDOK)
	{
		m_pCB_RenderStyle_Files->ResetContent();	// clear combobox
		DeleteAndClearArray(m_RenderStyle_Files);	// clear out list of RenderStyle files
		SetDlgItemText(IDC_RENDERSTYLE_FILENAME, "[Select a renderstyle file]");

		_splitpath(BrowseBox.GetPathName(), drive, resourcePath, buffer, buffer);
		sprintf(m_ProjectPath, "%s%s", drive, resourcePath);

		// Update potentially project-relative filenames

		OnTextureList_Filename1();	OnTextureList_Filename2();
		OnTextureList_Filename3();	OnTextureList_Filename4();
		OnBackGndImg_Filename();

		// Search through the project folders, and add all RenderStyles
		// to the list combobox

		EnumerateAllLTAs(m_ProjectPath); // adds all LTAs found to m_RenderStyle_Files

		for (uint32 i=0; i<m_RenderStyle_Files.GetSize(); i++)
		{
			m_pCB_RenderStyle_Files->AddString(m_RenderStyle_Files[i]->file);
		}
	}
}

// Tests the directory in the path for the filename given in pName

bool CAppForm::DoesDirHaveFile(char *path, char *pName)
{
	HANDLE hFile;
	char fullName[500];
	WIN32_FIND_DATA findData;

	sprintf(fullName, "%s\\%s", path, pName);
	hFile = FindFirstFile(fullName, &findData);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		FindClose(hFile);
		return true;
	}
}

// Adds all LTAs in Renderstyle directories to m_RenderStyle_Files
void CAppForm::EnumerateAllLTAs(char *path)
{
	bool				isRSdir = false;	// is this directory a RenderStyles directory?
	WIN32_FIND_DATA		findData;			// pointer to the file data
	HANDLE				fileHandle;			// handle to file search
	char				buffer[512];		// temporary string
	char				temp[255];

	if (path == NULL)  return;

	// Check the directory to see if it contains RS LTAs

	if (DoesDirHaveFile(path, "DirTypeRenderStyles"))  isRSdir = true;


	// Now loop through all the files in the directory, exploring subdirectories,
	// and adding files if it's a RenderStyle folder

	sprintf(buffer, "%s*", path);
	fileHandle = FindFirstFile(buffer, &findData);
	if (fileHandle == INVALID_HANDLE_VALUE)  return;

	do
	{
		// If it's a directory, we explore it recursively
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (findData.cFileName[0] != '.')  // skip system directories
			{
				sprintf(buffer, "%s%s\\", path, findData.cFileName);
				EnumerateAllLTAs(buffer);
			}
		}
		else
		{
		// If it's a file, add it if it's an LTA and in a RSdir
			if (isRSdir)
			{
				// check if file has an lta extension
				sprintf(buffer, "%s%s", path, findData.cFileName);
				_splitpath(buffer, temp, temp, temp, temp);

				if (_stricmp(".lta", temp) == 0)
				{
					sprintf(buffer, "%s%s", path, findData.cFileName);
					m_RenderStyle_Files.Append(new Filename(buffer, findData.cFileName));
				}
			}
		}

	} while (FindNextFile(fileHandle, &findData));
}

void CAppForm::OnFileOpen()
{
	char filename[255], buffer[255], buffer2[128], extension[8]; // temporary files for path splitting

	CFileDialog BrowseBox(true,"lta",m_RenderStyle_FileName,OFN_HIDEREADONLY|OFN_CREATEPROMPT,"LTA File (*.lta)|*.lta||");
	if (BrowseBox.DoModal() == IDOK)
	{
		strcpy(m_RenderStyle_FileName, BrowseBox.GetPathName());
		m_bLoading				= true;
		if (m_ProjectPath[0] != '\0')  SetDlgItemText(IDC_RENDERSTYLE_FILENAME, "[No RenderStyle loaded]");
		SetDefaultRenderStyleData();					// Reset us...
		if (LoadRenderStyle(m_RenderStyle_FileName))
		{
			m_bNoRecompile		= true;
			SetRenderStyleDataFromRenderStyle();
			SetDialogControls_From_RenderStyleData(true);
			m_bNoRecompile		= false;
			OnRefRast_Enable();							// Make sure we've got the right rast mode...
			OnDbgParam_Model();							// Make sure the model is up to date...
			OutputMsg("Renderstyle %s loaded successfully.",m_RenderStyle_FileName);
			SetRenderStyleFromRenderStyleData(); 		// Just to force a re-compile...

			// Select the RenderStyle in the RS Files Combo
			_splitpath(m_RenderStyle_FileName, buffer2, buffer2, buffer, extension);
			sprintf(filename, "%s%s", buffer, extension);

			// Change title bar
#ifndef _RS_VIEWER
			sprintf(buffer, "Render Styles Editor - %s", filename);
#else
			sprintf(buffer, "Render Styles Viewer - %s", filename);
#endif
			g_App.m_pMainWnd->SetWindowText(buffer);

			if (m_ProjectPath[0] != '\0')
			if(m_pCB_RenderStyle_Files->SelectString(-1,filename) == CB_ERR)
			{
				//sprintf(buffer, "[RenderStyle \"%s\" not in project]", filename);
				SetDlgItemText(IDC_RENDERSTYLE_FILENAME, "[RenderStyle not in project]");
			}
		}
		else
		{
			OutputMsg("Error loading renderstyle: %s",m_RenderStyle_FileName);
		}
	}
	m_bLoading = false;
}

void CAppForm::OnFileSave()
{
	if (m_RenderStyle_FileName[0] == '\0') { OnFileSaveAs(); return; }

	SetRenderStyleFromRenderStyleData();
	if (SaveRenderStyle(m_RenderStyle_FileName)) {
		OutputMsg("Successfully Saved Renderstyle: %s",m_RenderStyle_FileName); }
	else {
		OutputMsg("Error saving renderstyle: %s",m_RenderStyle_FileName);
		Msg("Error saving renderstyle: %s",m_RenderStyle_FileName); }
}

void CAppForm::OnFileSaveAs()
{
	char filename[255], buffer[255], extension[255]; // temporary files for path splitting

	CFileDialog BrowseBox(false,"lta",m_RenderStyle_FileName,OFN_HIDEREADONLY|OFN_CREATEPROMPT,"LTA File (*.lta)|*.lta||");
	if (BrowseBox.DoModal() == IDOK) {
		strcpy(m_RenderStyle_FileName, BrowseBox.GetPathName());

		// Select the RenderStyle in the RS Files Combo
		_splitpath(m_RenderStyle_FileName, buffer, buffer, filename, extension);
		sprintf(filename, "%s%s", filename, extension);

		// Change title bar
#ifndef _RS_VIEWER
		sprintf(buffer, "Render Styles Editor - %s", filename);
#else
		sprintf(buffer, "Render Styles Viewer - %s", filename);
#endif
		g_App.m_pMainWnd->SetWindowText(buffer);


		SetRenderStyleFromRenderStyleData();
		if (SaveRenderStyle(m_RenderStyle_FileName)) {
			OutputMsg("Successfully Saved Renderstyle: %s",m_RenderStyle_FileName); }
		else {
			OutputMsg("Error saving renderstyle: %s",m_RenderStyle_FileName);
			Msg("Error saving renderstyle: %s",m_RenderStyle_FileName); } }
}

void CAppForm::OnPaint()
{
	// Draw the 4 colour bars....
	CPaintDC dc(this);		// device context for painting
	int r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.r);
	int g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.g);
	int b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.b);
	CWnd* pWnd = (CWnd *)GetDlgItem(IDC_MATERIALCOLOR_AMBIENT);
	DrawColor(pWnd, r, g, b);

	r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.r);
	g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.g);
	b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.b);
	pWnd = (CWnd *)GetDlgItem(IDC_MATERIALCOLOR_DIFFUSE);
	DrawColor(pWnd, r, g, b);

	r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.r);
	g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.g);
	b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.b);
	pWnd = (CWnd *)GetDlgItem(IDC_MATERIALCOLOR_EMISSIVE);
	DrawColor(pWnd, r, g, b);

	r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.r);
	g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.g);
	b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.b);
	pWnd = (CWnd *)GetDlgItem(IDC_MATERIALCOLOR_SPECULAR);
	DrawColor(pWnd, r, g, b);

	r = FLOAT01_TO_INT0255(m_DebugParams.LightColor.r);
	g = FLOAT01_TO_INT0255(m_DebugParams.LightColor.g);
	b = FLOAT01_TO_INT0255(m_DebugParams.LightColor.b);
	pWnd = (CWnd *)GetDlgItem(IDC_DBGPARAM_LIGHTCOLOR);
	DrawColor(pWnd, r, g, b);

	r = FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.r);
	g = FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.g);
	b = FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.b);
	pWnd = (CWnd *)GetDlgItem(IDC_DBGPARAM_BACKGNDCOLOR);
	DrawColor(pWnd, r, g, b);

	// Draw the Screen...
	RenderScene();

	// Do not call CDialog::OnPaint() for painting messages
}

void CAppForm::OnLButtonDown(UINT nFlags,CPoint point)
{
	if (!g_hWnd_RenderWindow) return; RECT rcMainWndRect,rcRenWndRect;
	GetWindowRect(&rcMainWndRect); ::GetWindowRect(g_hWnd_RenderWindow,&rcRenWndRect);
	rcRenWndRect.left -= rcMainWndRect.left + 5; rcRenWndRect.right -= rcMainWndRect.left + 5; rcRenWndRect.top -= rcMainWndRect.top + 12; rcRenWndRect.bottom -= rcMainWndRect.top + 12;

	// If it's inside the render window...
	if (point.x > rcRenWndRect.left && point.x < rcRenWndRect.right && point.y > rcRenWndRect.top  && point.y < rcRenWndRect.bottom) {
		m_bLeftMouseLook = true; }
}

void CAppForm::OnRButtonDown(UINT nFlags,CPoint point)
{
	if (!g_hWnd_RenderWindow) return; RECT rcMainWndRect,rcRenWndRect;
	GetWindowRect(&rcMainWndRect); ::GetWindowRect(g_hWnd_RenderWindow,&rcRenWndRect);
	rcRenWndRect.left -= rcMainWndRect.left + 5; rcRenWndRect.right -= rcMainWndRect.left + 5; rcRenWndRect.top -= rcMainWndRect.top + 12; rcRenWndRect.bottom -= rcMainWndRect.top + 12;

	// If it's inside the render window...
	if (point.x > rcRenWndRect.left && point.x < rcRenWndRect.right && point.y > rcRenWndRect.top  && point.y < rcRenWndRect.bottom) {
		m_bRightMouseLook = true; }
}

void CAppForm::OnLButtonUp(UINT nFlags,CPoint point)
{
	m_bLeftMouseLook = false;
}

void CAppForm::OnRButtonUp(UINT nFlags,CPoint point)
{
	m_bRightMouseLook = false;
}

void CAppForm::OnMouseMove(UINT nFlags,CPoint point)
{
	if (m_bLeftMouseLook && m_bRightMouseLook && (nFlags & MK_LBUTTON) && (nFlags & MK_RBUTTON)) {
		m_MouseLook_Translation.x -= ((float)(m_LastKnownMousePos.x - point.x)) * 0.05f;
		m_MouseLook_Translation.y += ((float)(m_LastKnownMousePos.y - point.y)) * 0.05f; }
	else if (m_bLeftMouseLook && (nFlags & MK_LBUTTON)) {
		m_MouseLook_Rotation.x -= ((float)(m_LastKnownMousePos.y - point.y)) * 0.01f;
		m_MouseLook_Rotation.y -= ((float)(m_LastKnownMousePos.x - point.x)) * 0.01f;
		if (m_MouseLook_Rotation.x >= 2*D3DX_PI) m_MouseLook_Rotation.x -= 2*D3DX_PI; if (m_MouseLook_Rotation.x <= -2*D3DX_PI) m_MouseLook_Rotation.x += 2*D3DX_PI;
		if (m_MouseLook_Rotation.y >= 2*D3DX_PI) m_MouseLook_Rotation.y -= 2*D3DX_PI; if (m_MouseLook_Rotation.y <= -2*D3DX_PI) m_MouseLook_Rotation.y += 2*D3DX_PI; }
	else if (m_bRightMouseLook && (nFlags & MK_RBUTTON)) {
		m_MouseLook_Translation.z -= ((float)(m_LastKnownMousePos.y - point.y)) * 0.1f; }
	m_LastKnownMousePos = point;				// Save off the last known mouse position (for deltas)...
}

// Create the 3D device, stuff we'll need...
bool CAppForm::Create3DEnv()
{
	OutputMsg("Creating 3D Environment...");

	D3DAdapterInfo* pAdapterInfo  = NULL;
	D3DDeviceInfo*  pDeviceInfo	  = NULL;
	D3DModeInfo*	pModeInfo	  = NULL;

	// Get the 3D display window size...
	RECT rcClientRenderRect;
	if (::GetClientRect(g_hWnd_RenderWindow,&rcClientRenderRect)) {
		g_ScreenWidth  = rcClientRenderRect.right  - rcClientRenderRect.left;
		g_ScreenHeight = rcClientRenderRect.bottom - rcClientRenderRect.top; }

   	// Create the D3DShell (it'll enumerate all the hardware devices)...
	if (!g_D3DShell.Create()) return false;		// Create the Shell...

	// First try to find your favorite device. If you can't, pick a default one.
	uint32 iAdapterID			  = 0;			// D3D's adapter counter number for this adapter...
	if (!pDeviceInfo) pDeviceInfo = g_D3DShell.PickDefaultDev(&pAdapterInfo,m_DebugParams.bUseRefRast);
	if (!pDeviceInfo) {							// Couldn't find any devices that make the cut - fail out
		Msg("Can't find any d3d devices to use!"); return false; }

	pModeInfo					  = g_D3DShell.PickDefaultMode(pDeviceInfo,32);
	if (!pModeInfo) {							// Couldn't find any devices that make the cut - fail out
		Msg("Can't find an appropriate display mode!"); return false; }

	// Create the Device...
	if (!g_Device.CreateDevice(pAdapterInfo,pDeviceInfo,pModeInfo)) {
		Msg("Couldn't create D3D Device!"); return false; }
	g_RenderStateMgr.Reset();					// Reset the RenderStateMgr's internal state...

	// Set the Viewport...
	g_Device.SetupViewport(g_ScreenWidth,g_ScreenHeight,0.0f,1.0f);

	g_EffectMgr.Init();

	CreateTheRenderObject();					// Create our default render object...

	PD3DDEVICE->SetSoftwareVertexProcessing( true );

	// Create me a render style to mess with...
	if (!(m_pRenderStyle = g_Device.CreateRenderStyle())) 
	{
		OutputMsg("Can't create render style!"); return false; 
	}

	if (!SetRenderStyleFromRenderStyleData()) 
	{
		OutputMsg("Could not create render style from dialog's render data."); 
	}
	if (!(m_pAntEyeRenderStyle = g_Device.CreateRenderStyle())) 
	{
		OutputMsg("Can't create render style!"); return false; 
	}

	m_pAntEyeRenderStyle->SetDefaults(); LightingMaterial lightMat;

	if (m_pAntEyeRenderStyle->GetLightingMaterial(&lightMat)) 
	{
		lightMat.Ambient.a = 0; lightMat.Ambient.r = 0; lightMat.Ambient.g = 0; lightMat.Ambient.b = 0;
		lightMat.Diffuse.a = 0; lightMat.Diffuse.r = 0; lightMat.Diffuse.g = 0; lightMat.Diffuse.b = 0;
		m_pAntEyeRenderStyle->SetLightingMaterial(lightMat); 
	}

	// Setup the scene lights...
	SceneLightsChanged();

	// Force the debug textures to load up (or really just create an empty array)...
	DebugTextureSettingChanged();

	// Setup the background image...
	BackGroundImageChanged();

	// Go ahead and render scene...
	RenderScene();

	return true;
}

void CAppForm::RenderStyleDataChanged(bool bForceRecompile)
{
	if (!SetRenderStyleFromRenderStyleData(bForceRecompile)) {					// Setup our render style...
		OutputMsg("Could not create render style from dialog's render data."); }

	RenderScene();
}

void CAppForm::DebugTextureSettingChanged()
{
	char buffer[512], filePath[512], projectPath[512], drive[8];
	m_bLoadingTextures = true;
	m_RenderCS.Enter();

	// Free all the textures we have loaded...
	for (uint32 i = 0; i < m_TextureArray.size(); ++i) 
	{
		if (m_TextureArray[i]) 
		{ 
			m_TextureArray[i]->Release(); 
			m_TextureArray[i] = NULL; 
		} 
	}
	m_TextureArray.clear();

	// Record the file-relative and project-relative paths
	_splitpath(m_RenderStyle_FileName, drive, buffer, filePath, filePath);
	sprintf(filePath, "%s%s", drive, buffer);

	_splitpath(m_ProjectPath, drive, buffer, projectPath, projectPath);
	sprintf(projectPath, "%s%s", drive, buffer);

	// Load up the new list...
	CString WindowText; 
	//LPDIRECT3DTEXTURE9 pTexture = NULL;
	LPDXTexture pTexture = NULL;
	for (i = 0; i < 4; ++i)
	{
		if (!m_DebugParams.TextureList_Filename[i].empty())
		{
			// If the texture path is explicit, strip out the file or project paths, if possible
			_splitpath(m_DebugParams.TextureList_Filename[i].c_str(), drive, buffer, buffer, buffer);

			if (drive[0] != '\0')
			{
				if (StripPath(m_DebugParams.TextureList_Filename[i].c_str(), filePath, buffer))
				{
					m_pEB_TextureList_Filename[i]->SetWindowText(buffer);
					m_DebugParams.TextureList_Filename[i] = buffer;
				}
				else if (StripPath(m_DebugParams.TextureList_Filename[i].c_str(), projectPath, buffer))
				{
					m_pEB_TextureList_Filename[i]->SetWindowText(buffer);
					m_DebugParams.TextureList_Filename[i] = buffer;
				}
			}

			// Now the current path should be relative if possible.  Next up is loading the
			// texture into RSE

			// First, try loading it as-is

			if (pTexture = LoadTexture(m_DebugParams.TextureList_Filename[i].c_str()))
			{
				m_TextureArray.push_back(pTexture);
			}
			else // Next, try loading as file-relative
			{
				sprintf(buffer, "%s%s", filePath, m_DebugParams.TextureList_Filename[i].c_str());

				if (pTexture = LoadTexture(buffer))
				{
					m_TextureArray.push_back(pTexture);
				}
				else // Next, try loading as project-relative
				{
					sprintf(buffer, "%s%s", projectPath, m_DebugParams.TextureList_Filename[i].c_str());

					if (pTexture = LoadTexture(buffer))
					{
						m_TextureArray.push_back(pTexture);
					}
					else // Finally, try doing a directory search for the texture
					{
						m_pEB_TextureList_Filename[i]->GetWindowText(WindowText);
						string szTmp = g_StartingDirectory; szTmp += "\\"; szTmp += LPCSTR(WindowText);

						if (pTexture = LoadTexture(szTmp.c_str()))
						{
							m_TextureArray.push_back(pTexture);
						}
						else
						{
							m_TextureArray.push_back(NULL);
						}
					}
				}
			}
		}
		else
		{
			m_TextureArray.push_back(NULL);
		}
	}

	//Load an effect file if one exists.
	if (!m_DebugParams.EffectFilename.empty())
	{
		// If the texture path is explicit, strip out the file or project paths, if possible
		_splitpath(m_DebugParams.EffectFilename.c_str(), drive, buffer, buffer, buffer);

		bool bEffectLoaded = g_EffectMgr.Load(m_DebugParams.EffectFilename.c_str(), m_DebugParams.bUseRefRast);
		if(!bEffectLoaded)
		{
			OutputMsg("%s", g_EffectMgr.GetLastError()); 			
		}
		else
		{
			//Fill IDC_EFFECT_TECHNIQUE
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_EFFECT_TECHNIQUE);

			// Delete the old entries...
			pComboBox->ResetContent();

			// Enumerate the techniques...
			if(g_EffectMgr.GetEffect())
			{
				ID3DXEffect *pEffect = g_EffectMgr.GetEffect();

				for(int i = 0; i < 10; ++i)
				{
					D3DXHANDLE hHandle = pEffect->GetTechnique(i);
					if(hHandle == NULL)
					{
						break;
					}

					D3DXTECHNIQUE_DESC desc;
					if( FAILED(pEffect->GetTechniqueDesc(hHandle, &desc)) )
					{
						break;
					}

					HRESULT hrValidTechnique = pEffect->ValidateTechnique(hHandle);
					if(FAILED(hrValidTechnique))
					{
						OutputMsg("Technique: (%s) Failed to validate for this device.", desc.Name);
					}
					else
					{
						pComboBox->AddString(desc.Name);
					}					

				}

				pComboBox->SetCurSel(0);

				// Set the current technique
				CString sString;
				pComboBox->GetLBText(0, sString);
				g_EffectMgr.SetTechnique(sString);
				sString.ReleaseBuffer();

			}		
		}

		OnBnClickedEnableEffect();
	}

	m_RenderCS.Leave();
	m_bLoadingTextures = false;
}

void CAppForm::SceneLightsChanged()
{
	if (!PD3DDEVICE) return;

	// Ambient Light...
	g_RenderStateMgr.SetAmbientLight(50.0f,50.0f,50.0f);

	// Add a couple lights...
	D3DLIGHT9 Light; memset(&Light,0,sizeof(Light));
	Light.Type				= D3DLIGHT_POINT;
	Light.Ambient.r			= 0.0f;     Light.Ambient.g    = 0.0f;   Light.Ambient.b    = 0.0f; Light.Ambient.a   = 1.0f;
	Light.Diffuse.r			= m_DebugParams.LightColor.r;			 Light.Diffuse.g	= m_DebugParams.LightColor.g; Light.Diffuse.b = m_DebugParams.LightColor.b; Light.Diffuse.a = 1.0f;
	Light.Specular.r		= m_DebugParams.LightColor.r;			 Light.Specular.g	= m_DebugParams.LightColor.g; Light.Specular.b		= m_DebugParams.LightColor.b; Light.Specular.a = 1.0f;
	Light.Range				= 5000.0f;
	Light.Attenuation0		= 1.0f;     Light.Attenuation1 = 0.0f;   Light.Attenuation2 = 20.0f / powf(300,2);
	switch (m_DebugParams.LightConfig) {
	case 0 :											// Light Config 0...
		Light.Direction.x	= 0.707f;	Light.Direction.y  = 0.0f;   Light.Direction.z  = -0.707f;	// Note about direction (these are point lights, but some vertex shaders want to know direction for a directional light fake)...
		Light.Position.x	= 30.0f;    Light.Position.y   = 20.0f;  Light.Position.z   = -30.0f;
		g_RenderStateMgr.SetLight(0,&Light);
		Light.Position.x	= -30.0f;   Light.Position.y   = 20.0f;  Light.Position.z   = -30.0f;
		Light.Direction.x	= -0.707f;	Light.Direction.y  = 0.0f;   Light.Direction.z  = -0.707f;	// Note about direction (these are point lights, but some vertex shaders want to know direction for a directional light fake)...
		g_RenderStateMgr.SetLight(1,&Light);
		Light.Position.x	= 0.0f;     Light.Position.y   = -30.0f; Light.Position.z   = 30.0f;
		g_RenderStateMgr.SetLight(2,&Light); break;
	case 1 :											// Light Config 1...
		Light.Direction.x	= -0.707f;	Light.Direction.y  = 0.0f;   Light.Direction.z  = -0.707f;	// Note about direction (these are point lights, but some vertex shaders want to know direction for a directional light fake)...
		Light.Position.x	= 0.0f;     Light.Position.y   = -20.0f; Light.Position.z   = -30.0f;
		g_RenderStateMgr.SetLight(0,&Light);
		Light.Direction.x	= 0.707f;	Light.Direction.y  = 0.0f;   Light.Direction.z  = -0.707f;	// Note about direction (these are point lights, but some vertex shaders want to know direction for a directional light fake)...
		Light.Position.x	= 30.0f;    Light.Position.y   = 20.0f;  Light.Position.z   = 30.0f;
		g_RenderStateMgr.SetLight(1,&Light);
		Light.Position.x	= -30.0f;   Light.Position.y   = 20.0f;  Light.Position.z   = 30.0f;
		g_RenderStateMgr.SetLight(2,&Light); break;
	case 2 :											// Light Config 1...
		Light.Direction.x	= -0.577f;	Light.Direction.y  = 0.577f; Light.Direction.z  = -0.577f;	// Note about direction (these are point lights, but some vertex shaders want to know direction for a directional light fake)...
		Light.Position.x	= -10.0f;   Light.Position.y   = 10.0f;  Light.Position.z   = -15.0f;
		g_RenderStateMgr.SetLight(0,&Light);
		Light.Direction.x	= 0.577f;	Light.Direction.y  = 0.577f; Light.Direction.z  = -0.577f;	// Note about direction (these are point lights, but some vertex shaders want to know direction for a directional light fake)...
		Light.Position.x	= 40.0f;    Light.Position.y   = -10.0f; Light.Position.z   = -20.0f;
		Light.Diffuse.r		= 1.0f;		 Light.Diffuse.g   = 0.8f;  Light.Diffuse.b	= 0.6f;
		Light.Specular.r	= Light.Diffuse.r;			 Light.Specular.g  = Light.Diffuse.g; Light.Specular.b			= Light.Diffuse.b;
		g_RenderStateMgr.SetLight(1,&Light);
		Light.Position.x	= 50.0f;    Light.Position.y   = 30.0f;  Light.Position.z   = 40.0f;
		Light.Diffuse.r		= 2.0f;		 Light.Diffuse.g   = 2.0f;  Light.Diffuse.b	= 2.0f;
		Light.Specular.r	= Light.Diffuse.r;			 Light.Specular.g  = Light.Diffuse.g; Light.Specular.b			= Light.Diffuse.b;
		g_RenderStateMgr.SetLight(2,&Light); break; }

	for (uint32 i = 0; i < 8; ++i) 
	{	
		// Enable all the requested lights (make sure all the others are disabled)...
		if (i < m_DebugParams.LightCount) 
		{
			g_RenderStateMgr.LightEnable(i, true);
		}
		else
		{
			g_RenderStateMgr.LightEnable(i, false); 
		}
	}

	RenderScene();	// Force a redraw...
}

void CAppForm::BackGroundImageChanged()
{
	char buffer[512], filePath[512], projectPath[512], drive[8];

	if (m_BackGndTexture) 
	{ 							
		// Free the background image...
		m_BackGndTexture->Release(); 
		m_BackGndTexture = NULL; 
	}


	// Record the file-relative and project-relative paths
	_splitpath(m_RenderStyle_FileName, drive, buffer, filePath, filePath);
	sprintf(filePath, "%s%s", drive, buffer);

	_splitpath(m_ProjectPath, drive, buffer, projectPath, projectPath);
	sprintf(projectPath, "%s%s", drive, buffer);


	// Load up the new bkg image
	CString WindowText; 
	//LPDIRECT3DTEXTURE9 pTexture = NULL;
	LPDXTexture pTexture = NULL;

	if (!m_DebugParams.BackGndImg_Filename.empty())
	{
		// If the texture path is explicit, strip out the file or project paths, if possible
		_splitpath(m_DebugParams.BackGndImg_Filename.c_str(), drive, buffer, buffer, buffer);

		if (drive[0] != '\0')
		{
			if (StripPath(m_DebugParams.BackGndImg_Filename.c_str(), filePath, buffer))
			{
				m_pEB_BackGndImg_Filename->SetWindowText(buffer);
				m_DebugParams.BackGndImg_Filename = buffer;
			}
			else if (StripPath(m_DebugParams.BackGndImg_Filename.c_str(), projectPath, buffer))
			{
				m_pEB_BackGndImg_Filename->SetWindowText(buffer);
				m_DebugParams.BackGndImg_Filename = buffer;
			}
		}

		// Now the current path should be relative if possible.  Next up is loading the
		// texture into RSE

		// First, try loading it as-is

		if (pTexture = LoadTexture(m_DebugParams.BackGndImg_Filename.c_str()))
		{
			if(pTexture && pTexture->IsValid())
			{
				m_BackGndTexture = pTexture->m_pTexture;
			}
		}
		else // Next, try loading as file-relative
		{
			sprintf(buffer, "%s%s", filePath, m_DebugParams.BackGndImg_Filename.c_str());

			if (pTexture = LoadTexture(buffer))
			{
				//m_BackGndTexture = pTexture;
				if(pTexture && pTexture->IsValid())
				{
					m_BackGndTexture = pTexture->m_pTexture;
				}
			}
			else // Next, try loading as project-relative
			{
				sprintf(buffer, "%s%s", projectPath, m_DebugParams.BackGndImg_Filename.c_str());

				if (pTexture = LoadTexture(buffer))
				{
					//m_BackGndTexture = pTexture;
					if(pTexture && pTexture->IsValid())
					{
						m_BackGndTexture = pTexture->m_pTexture;
					}
				}
				else // Finally, try doing a directory search for the texture
				{
					m_pEB_BackGndImg_Filename->GetWindowText(WindowText);
					string szTmp = g_StartingDirectory; szTmp += "\\"; szTmp += LPCSTR(WindowText);

					if (pTexture = LoadTexture(szTmp.c_str()))
					{
						//m_BackGndTexture = pTexture;
						if(pTexture && pTexture->IsValid())
						{
							m_BackGndTexture = pTexture->m_pTexture;
						}
					}
					else
					{
						m_BackGndTexture = NULL;
					}
				}
			}
		}
	}
	else
	{
		m_BackGndTexture = NULL;
	}

	if(pTexture)
	{
		pTexture->m_bKeepAlive = true;
	}

	RenderScene();
}

char* ScanParam(char* szString, char* szOutParam)
{
	uint32 iCh = 0; uint32 iOutCh = 0;					// Strip white space...
	while (szString[iCh]==' ') { ++iCh; }

	szOutParam[0] = NULL;								// Copy over as much as you can...
	while (szString[iCh]!=',' && szString[iCh]!=NULL && szString[iCh]!='>' && iOutCh < 31) {
		szOutParam[iOutCh] = szString[iCh]; ++iCh; ++iOutCh; }
	szOutParam[iOutCh] = NULL; if (iOutCh > 0) --iOutCh;

	if (szString[iCh] == ',') ++iCh;					// Skip over that comma...

	while ((iOutCh > 0) && (szOutParam[iOutCh]==' ')) {	// Strip off end white space...
		szOutParam[iOutCh] = NULL; --iOutCh; }

	return (szString + iCh);
}

// Pre-Process the shaderfile code (expand skin macros and stuff like that)...
bool PreProcessShaderFile(const char* szFileName, string& OutputCode, uint32 iType)
{
	bool bRigid = ((iType==0) ? true : false);
	FILE* f = fopen(szFileName,"r"); if (!f) return false;
	char ch[2]; ch[1] = NULL; char* pCh = NULL;			// Read in the whole file...
	while ((ch[0] = fgetc(f))!=EOF) { OutputCode += ch; }
	fclose(f);

	// LT_MACRO_IFRIGID Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_IFRIGID<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_IFRIGID<")) {		// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_IFRIGID<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();
		if (bRigid) {
			OutputCode.erase(iEndCh,1);
			OutputCode.erase(iStartCh,strlen("LT_MACRO_IFRIGID<")); }
		else {
			OutputCode.erase(iStartCh,iEndCh-iStartCh+1); } }

	// LT_MACRO_IFSKIN Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_IFSKIN<")) {		// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_IFSKIN<")) {		// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_IFSKIN<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();
		if (!bRigid) {
			OutputCode.erase(iEndCh,1);
			OutputCode.erase(iStartCh,strlen("LT_MACRO_IFSKIN<")); }
		else {
			OutputCode.erase(iStartCh,iEndCh-iStartCh+1); } }

	// LT_MACRO_RIGIDTRANS4 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_RIGIDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_RIGIDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_RIGIDTRANS4<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_RIGIDTRANS4<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (bRigid) {
			sprintf(szBuff,"dp4 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.w, %s, c%d\n",OutPos,InPos,iMatConst+3);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_RIGIDTRANS3 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_RIGIDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_RIGIDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_RIGIDTRANS3<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_RIGIDTRANS3<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (bRigid) {
			sprintf(szBuff,"dp3 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_SKINTRANS4 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINTRANS4<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINTRANS4<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) {
			sprintf(szBuff,"dp4 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.w, %s, c%d\n",OutPos,InPos,iMatConst+3);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_SKINTRANS3 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINTRANS3<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINTRANS3<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) {
			sprintf(szBuff,"dp3 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_LIGHT_ATT Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_LIGHT_ATT<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_LIGHT_ATT<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_LIGHT_ATT<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char OutVal[16],Light[16],NormalR[16],NormalS[16],Tmp1[16],Tmp2[16],AttVec[16],szBuff[128];
		pCh += strlen("LT_MACRO_LIGHT_ATT<");
		pCh = ScanParam(pCh,OutVal);		if (strlen(OutVal)==0)		return false;
		pCh = ScanParam(pCh,Light);			if (strlen(Light)==0)		return false;
		pCh = ScanParam(pCh,NormalR);		if (strlen(NormalR)==0)		return false;
		pCh = ScanParam(pCh,NormalS);		if (strlen(NormalS)==0)		return false;
		pCh = ScanParam(pCh,Tmp1);			if (strlen(Tmp1)==0)		return false;
		pCh = ScanParam(pCh,Tmp2);			if (strlen(Tmp2)==0)		return false;
		pCh = ScanParam(pCh,AttVec);		if (strlen(AttVec)==0)		return false;
		uint32 iAttVec	= atoi(&AttVec[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		sprintf(szBuff,"dp3 %s.w, %s, %s\n",Tmp1,Light,Light);						OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"rsq %s.w, %s.w\n",Tmp2,Tmp1);								OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"mul %s, %s, %s.w\n",Light,Light,Tmp2);						OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"dst %s, %s.w, %s.w\n",Tmp1,Tmp1,Tmp2);						OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"dp3 %s.w, %s.xyz, %s.xyz\n",Tmp1,Tmp1,AttVec);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"rcp %s.w, %s.w\n",Tmp2,Tmp1);								OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		if (bRigid) { sprintf(szBuff,"dp3 %s.x, %s, %s\n",Tmp1,NormalR,Light); }
		else { sprintf(szBuff,"dp3 %s.x, %s, %s\n",Tmp1,NormalS,Light); }			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"mul %s, %s.w, %s.x\n",OutVal,Tmp2,Tmp1);					OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); }

	// LT_MACRO_SKINBLENDTRANS4 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINBLENDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINBLENDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINBLENDTRANS4<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],Weights[16],Indicies[16],TmpReg[16],TmpReg2[16],StartConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINBLENDTRANS4<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,Weights);		if (strlen(Weights)==0)		return false;
		pCh = ScanParam(pCh,Indicies);		if (strlen(Indicies)==0)	return false;
		pCh = ScanParam(pCh,TmpReg);		if (strlen(TmpReg)==0)		return false;
		pCh = ScanParam(pCh,TmpReg2);		if (strlen(TmpReg2)==0)		return false;
		pCh = ScanParam(pCh,StartConst);	if (strlen(StartConst)==0)	return false;
		uint32 iStartConst	= atoi(&StartConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) { 									// Expand the sucka...
			switch (iType) {
			case 1 :									// Two Bones...
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 2 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 3 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.zzzz, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.w, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break; } } }

	// LT_MACRO_SKINBLENDTRANS3 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINBLENDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINBLENDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINBLENDTRANS3<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],Weights[16],Indicies[16],TmpReg[16],TmpReg2[16],StartConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINBLENDTRANS3<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,Weights);		if (strlen(Weights)==0)		return false;
		pCh = ScanParam(pCh,Indicies);		if (strlen(Indicies)==0)	return false;
		pCh = ScanParam(pCh,TmpReg);		if (strlen(TmpReg)==0)		return false;
		pCh = ScanParam(pCh,TmpReg2);		if (strlen(TmpReg2)==0)		return false;
		pCh = ScanParam(pCh,StartConst);	if (strlen(StartConst)==0)	return false;
		uint32 iStartConst	= atoi(&StartConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) { 									// Expand the sucka...
			switch (iType) {
			case 1 :									// Two Bones...
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 2 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 3 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.zzzz, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.w, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break; } } }

	return true;
}

bool CAppForm::SetRenderStyleFromRenderStyleData(bool bForceRecompile)
{
	if (!m_pRenderStyle) return false;

//	m_pRenderStyle->SetDefaults();						// Reset to defaults...

	// Light Material...
	LightingMaterial Material;
	Material.Ambient		= m_MainDlgData.LightingMaterial_Ambient;
	Material.Diffuse		= m_MainDlgData.LightingMaterial_Diffuse;
	Material.Emissive		= m_MainDlgData.LightingMaterial_Emissive;
	Material.Specular		= m_MainDlgData.LightingMaterial_Specular;
	Material.SpecularPower	= m_MainDlgData.LightingMaterial_SpecularPower;
	m_pRenderStyle->SetLightingMaterial(Material);

	// Add all the render passes...
	if (bForceRecompile) 
	{
		while (m_pRenderStyle->GetRenderPassCount()) 
		{
			m_pRenderStyle->RemoveRenderPass(0); 
		}
	}

	for (uint32 i = 0;i < 4; ++i)
	{
		if (m_MainDlgData.RenderPass_Enable[i])
		{
			RenderPassOp RenderPass;
			RenderPass.AlphaTestMode								= m_RenderPassData[i].AlphaTestMode;
			RenderPass.ZBufferTestMode								= m_RenderPassData[i].ZBufferTestMode;
			RenderPass.FillMode										= m_RenderPassData[i].FillMode;
			RenderPass.DynamicLight									= m_RenderPassData[i].DynamicLight;
			RenderPass.BlendMode									= m_RenderPassData[i].Blend;
			RenderPass.ZBufferMode									= m_RenderPassData[i].ZBuffer;
			RenderPass.CullMode										= m_RenderPassData[i].CullMode;
			RenderPass.TextureFactor								= m_RenderPassData[i].TextureFactor;
			RenderPass.AlphaRef										= m_RenderPassData[i].AlphaRef;
			RenderPass.bUseBumpEnvMap								= m_RenderPassData[i].bUseBumpEnvMap;
			RenderPass.BumpEnvMapStage								= m_RenderPassData[i].BumpEnvMapStage;
			RenderPass.fBumpEnvMap_Scale							= m_RenderPassData[i].fBumpEnvMap_Scale;
			RenderPass.fBumpEnvMap_Offset							= m_RenderPassData[i].fBumpEnvMap_Offset;
			for (uint32 j = 0;j < 4; ++j)
			{
				RenderPass.TextureStages[j].TextureParam			= m_RenderPassData[i].TextureParam[j];
				RenderPass.TextureStages[j].ColorOp					= m_RenderPassData[i].ColorOp[j];
				RenderPass.TextureStages[j].ColorArg1				= m_RenderPassData[i].ColorArg1[j];
				RenderPass.TextureStages[j].ColorArg2				= m_RenderPassData[i].ColorArg2[j];
				RenderPass.TextureStages[j].AlphaOp					= m_RenderPassData[i].AlphaOp[j];
				RenderPass.TextureStages[j].AlphaArg1				= m_RenderPassData[i].AlphaArg1[j];
				RenderPass.TextureStages[j].AlphaArg2				= m_RenderPassData[i].AlphaArg2[j];
				RenderPass.TextureStages[j].UVSource				= m_RenderPassData[i].UVSource[j];
				RenderPass.TextureStages[j].UAddress				= m_RenderPassData[i].UAddress[j];
				RenderPass.TextureStages[j].VAddress				= m_RenderPassData[i].VAddress[j];
				RenderPass.TextureStages[j].TexFilter				= m_RenderPassData[i].TexFilter[j];
				RenderPass.TextureStages[j].UVTransform_Enable		= m_RenderPassData[i].UVTransform_Enable[j];
				RenderPass.TextureStages[j].ProjectTexCoord			= m_RenderPassData[i].ProjectTexCoord[j];
				RenderPass.TextureStages[j].TexCoordCount			= m_RenderPassData[i].TexCoordCount[j];

				for (uint32 z = 0; z < 16; ++z)
					RenderPass.TextureStages[j].UVTransform_Matrix[z] = m_RenderPassData[i].UVTransform_Matrix[j][z];
			}

			if (m_pRenderStyle->GetRenderPassCount() > i)
			{
				if (!m_pRenderStyle->SetRenderPass(i,RenderPass))
					return false;
			}
			else
			{
				if (!m_pRenderStyle->AddRenderPass(RenderPass))
					return false;
			}

			// Now set the Direct3D render pass data...
			if (m_RenderPassData[i].VertexShader_Enable || m_RenderPassData[i].PixelShader_Enable)
			{
				RSD3DRenderPass D3DOptions;
				D3DOptions.bUseVertexShader							= m_RenderPassData[i].VertexShader_Enable;
				D3DOptions.VertexShaderID							= m_RenderPassData[i].VertexShader_ID;
				D3DOptions.bUsePixelShader							= m_RenderPassData[i].PixelShader_Enable;
				D3DOptions.PixelShaderID							= m_RenderPassData[i].PixelShader_ID;

				if (!m_pRenderStyle->SetRenderPass_D3DOptions(i,&D3DOptions))
				{
					return false;
				}
			}
		}
	}

	// Now set the Direct3D Option data...
	RSD3DOptions D3DOptions;
	D3DOptions.bUseEffectShader							= m_MainDlgData.Effect_Enable;
	D3DOptions.EffectShaderID							= m_MainDlgData.Effect_ID;

	if (!m_pRenderStyle->SetDirect3D_Options(D3DOptions))
	{
		return false;
	}

	bool bNeedsReCompile = bForceRecompile; RSD3DRenderPass* pOptions = NULL;
	if (!bNeedsReCompile && (m_pRenderStyle->m_RenderPasses.size() > 0)) 
	{
		list<CD3DRenderPass>::iterator it = m_pRenderStyle->m_RenderPasses.begin();

		while (it != m_pRenderStyle->m_RenderPasses.end()) 
		{
			CD3DRenderPass* pD3DRenderPass = &(*it); // If we want a shader and the handle is currently null, we need to do a recompile...
			if (pD3DRenderPass->pD3DRenderPass && pD3DRenderPass->pD3DRenderPass->bUseVertexShader && !pD3DRenderPass->lpD3DVertexShader[0]) 
			{
				bNeedsReCompile = true; 
			} 
			++it; 
		} 
	}

	return true;
}

bool CAppForm::SetRenderStyleDataFromRenderStyle()
{
	if (!m_pRenderStyle) return false;

	// Light Material...
	LightingMaterial Material;
	if (m_pRenderStyle->GetLightingMaterial(&Material)) {
		m_MainDlgData.LightingMaterial_Ambient				= Material.Ambient;
		m_MainDlgData.LightingMaterial_Diffuse				= Material.Diffuse;
		m_MainDlgData.LightingMaterial_Emissive				= Material.Emissive;
		m_MainDlgData.LightingMaterial_Specular				= Material.Specular;
		m_MainDlgData.LightingMaterial_SpecularPower		= Material.SpecularPower; }

	// Add all the render passes...
	RenderPassOp RenderPass; RSD3DRenderPass D3DOptions;
	for (uint32 i = 0;i < 4; ++i)
	{
		if (m_pRenderStyle->GetRenderPass(i,&RenderPass))
		{
			m_MainDlgData.RenderPass_Enable[i]				= true;
			m_RenderPassData[i].DynamicLight				= RenderPass.DynamicLight;
			m_RenderPassData[i].Blend						= RenderPass.BlendMode;
			m_RenderPassData[i].AlphaTestMode				= RenderPass.AlphaTestMode;
			m_RenderPassData[i].ZBufferTestMode				= RenderPass.ZBufferTestMode;
			m_RenderPassData[i].FillMode					= RenderPass.FillMode;
			m_RenderPassData[i].ZBuffer						= RenderPass.ZBufferMode;
			m_RenderPassData[i].CullMode					= RenderPass.CullMode;
			m_RenderPassData[i].TextureFactor				= RenderPass.TextureFactor;
			m_RenderPassData[i].AlphaRef					= RenderPass.AlphaRef;
			m_RenderPassData[i].bUseBumpEnvMap				= RenderPass.bUseBumpEnvMap;
			m_RenderPassData[i].BumpEnvMapStage				= RenderPass.BumpEnvMapStage;
			m_RenderPassData[i].fBumpEnvMap_Scale			= RenderPass.fBumpEnvMap_Scale;
			m_RenderPassData[i].fBumpEnvMap_Offset			= RenderPass.fBumpEnvMap_Offset;
			for (uint32 j = 0;j < 4; ++j)
			{
				m_RenderPassData[i].TextureParam[j]			= RenderPass.TextureStages[j].TextureParam;
				m_RenderPassData[i].ColorOp[j]				= RenderPass.TextureStages[j].ColorOp;
				m_RenderPassData[i].ColorArg1[j]			= RenderPass.TextureStages[j].ColorArg1;
				m_RenderPassData[i].ColorArg2[j]			= RenderPass.TextureStages[j].ColorArg2;
				m_RenderPassData[i].AlphaOp[j]				= RenderPass.TextureStages[j].AlphaOp;
				m_RenderPassData[i].AlphaArg1[j]			= RenderPass.TextureStages[j].AlphaArg1;
				m_RenderPassData[i].AlphaArg2[j]			= RenderPass.TextureStages[j].AlphaArg2;
				m_RenderPassData[i].UVSource[j]				= RenderPass.TextureStages[j].UVSource;
				m_RenderPassData[i].UAddress[j]				= RenderPass.TextureStages[j].UAddress;
				m_RenderPassData[i].VAddress[j]				= RenderPass.TextureStages[j].VAddress;
				m_RenderPassData[i].TexFilter[j]			= RenderPass.TextureStages[j].TexFilter;
				m_RenderPassData[i].UVTransform_Enable[j]	= RenderPass.TextureStages[j].UVTransform_Enable;
				m_RenderPassData[i].UVTransform_Matrix[j]	= RenderPass.TextureStages[j].UVTransform_Matrix;
				m_RenderPassData[i].TexCoordCount[j]		= RenderPass.TextureStages[j].TexCoordCount;
				m_RenderPassData[i].ProjectTexCoord[j]		= RenderPass.TextureStages[j].ProjectTexCoord;
			}

			if (m_pRenderStyle->GetRenderPass_D3DOptions(i,&D3DOptions))
			{
				m_RenderPassData[i].VertexShader_Enable								= D3DOptions.bUseVertexShader;
				m_RenderPassData[i].VertexShader_ID									= D3DOptions.VertexShaderID;
				m_RenderPassData[i].PixelShader_Enable								= D3DOptions.bUsePixelShader;
				m_RenderPassData[i].PixelShader_ID									= D3DOptions.PixelShaderID;
/*
				m_RenderPassData[i].Direct3DData.VertexShaderFilename				= D3DOptions.VertexShaderFilename;
				m_RenderPassData[i].Direct3DData.bExpandForSkinning					= D3DOptions.bExpandForSkinning;
				m_RenderPassData[i].Direct3DData.ConstVector_ConstReg1				= D3DOptions.ConstVector_ConstReg1;
				m_RenderPassData[i].Direct3DData.ConstVector_ConstReg2				= D3DOptions.ConstVector_ConstReg2;
				m_RenderPassData[i].Direct3DData.ConstVector_ConstReg3				= D3DOptions.ConstVector_ConstReg3;
				m_RenderPassData[i].Direct3DData.ConstVector_Param1					= D3DOptions.ConstVector_Param1;
				m_RenderPassData[i].Direct3DData.ConstVector_Param2					= D3DOptions.ConstVector_Param2;
				m_RenderPassData[i].Direct3DData.ConstVector_Param3					= D3DOptions.ConstVector_Param3;
				m_RenderPassData[i].Direct3DData.WorldViewTransform_ConstReg		= D3DOptions.WorldViewTransform_ConstReg;
				m_RenderPassData[i].Direct3DData.WorldViewTransform_Count			= D3DOptions.WorldViewTransform_Count;
				m_RenderPassData[i].Direct3DData.ProjTransform_ConstReg				= D3DOptions.ProjTransform_ConstReg;
				m_RenderPassData[i].Direct3DData.WorldViewProjTransform_ConstReg	= D3DOptions.WorldViewProjTransform_ConstReg;
				m_RenderPassData[i].Direct3DData.ViewProjTransform_ConstReg			= D3DOptions.ViewProjTransform_ConstReg;
				m_RenderPassData[i].Direct3DData.CamPos_MSpc_ConstReg				= D3DOptions.CamPos_MSpc_ConstReg;
				m_RenderPassData[i].Direct3DData.Light_Count						= D3DOptions.Light_Count;
				m_RenderPassData[i].Direct3DData.LightPosition_MSpc_ConstReg		= D3DOptions.LightPosition_MSpc_ConstReg;
				m_RenderPassData[i].Direct3DData.LightPosition_CSpc_ConstReg		= D3DOptions.LightPosition_CSpc_ConstReg;
				m_RenderPassData[i].Direct3DData.LightColor_ConstReg				= D3DOptions.LightColor_ConstReg;
				m_RenderPassData[i].Direct3DData.LightAtt_ConstReg					= D3DOptions.LightAtt_ConstReg;
				m_RenderPassData[i].Direct3DData.Material_AmbDifEm_ConstReg			= D3DOptions.Material_AmbDifEm_ConstReg;
				m_RenderPassData[i].Direct3DData.Material_Specular_ConstReg			= D3DOptions.Material_Specular_ConstReg;
				m_RenderPassData[i].Direct3DData.AmbientLight_ConstReg				= D3DOptions.AmbientLight_ConstReg;
				m_RenderPassData[i].Direct3DData.PrevWorldViewTrans_ConstReg		= D3DOptions.PrevWorldViewTrans_ConstReg;
				m_RenderPassData[i].Direct3DData.PrevWorldViewTrans_Count			= D3DOptions.PrevWorldViewTrans_Count;
				m_RenderPassData[i].Direct3DData.Last_ConstReg						= D3DOptions.Last_ConstReg;
				for (uint32 z=0;z<4;++z)
 				{
					m_RenderPassData[i].Direct3DData.bDeclaration_Stream_Position[z]		= D3DOptions.bDeclaration_Stream_Position[z];
					m_RenderPassData[i].Direct3DData.bDeclaration_Stream_Normal[z]			= D3DOptions.bDeclaration_Stream_Normal[z];
					m_RenderPassData[i].Direct3DData.bDeclaration_Stream_UVSets[z]			= D3DOptions.bDeclaration_Stream_UVSets[z];
					m_RenderPassData[i].Direct3DData.Declaration_Stream_UVCount[z]			= D3DOptions.Declaration_Stream_UVCount[z];
					m_RenderPassData[i].Direct3DData.bDeclaration_Stream_BasisVectors[z]	= D3DOptions.bDeclaration_Stream_BasisVectors[z];
  				}
*/
			}
  		}
	}

	RSD3DOptions rsD3DOptions;
	if (m_pRenderStyle->GetDirect3D_Options(&rsD3DOptions))
	{
		m_MainDlgData.Effect_Enable = rsD3DOptions.bUseEffectShader;
		m_MainDlgData.Effect_ID = rsD3DOptions.EffectShaderID;
		CheckEnableEffect();
	}

	// Debug Params...
	m_pEB_BackGndImg_Filename->SetWindowText(m_DebugParams.BackGndImg_Filename.empty() ? "" : m_DebugParams.BackGndImg_Filename.c_str());

	return true;
}

// Release the 3D device, free all that good stuff...
void CAppForm::Destroy3DEnv()
{
	OutputMsg("Destroying 3D Environment...");

	// Free all the textures we have loaded...
	for (uint32 i = 0; i < m_TextureArray.size(); ++i) 
	{				
		if (m_TextureArray[i])
		{ 
			m_TextureArray[i]->Release(); 
			m_TextureArray[i] = NULL; 
		} 
	}
	m_TextureArray.clear();

	if (m_BackGndTexture) 
	{ 	
		// Free the background image...
		m_BackGndTexture->Release(); 
		m_BackGndTexture = NULL; 
	}

	if (m_pRenderStyle)			{ g_Device.DestroyRenderStyle(m_pRenderStyle);			m_pRenderStyle			= NULL; }
	if (m_pAntEyeRenderStyle)	{ g_Device.DestroyRenderStyle(m_pAntEyeRenderStyle);	m_pAntEyeRenderStyle	= NULL; }
	if (m_pRenderObject)		{ g_Device.DestroyRenderObject(m_pRenderObject);		m_pRenderObject			= NULL; }
	if (m_pAntEyeRenderObject)	{ g_Device.DestroyRenderObject(m_pAntEyeRenderObject);	m_pAntEyeRenderObject	= NULL; }

	g_Device.FreeAll();
	g_D3DShell.FreeAll();
}

// Called by the App's OnIdle function - If we need to do stuff on idle, do it here...
void CAppForm::OnIdle()
{
	if (m_DebugParams.bRotateModel && !m_bLeftMouseLook && !m_bRightMouseLook) {						// Did they request that the model rotate?
		m_fModelRotation += (timeGetTime() - m_iLastRotationUpdateTime) * 0.03f;
		if (m_fModelRotation > 360.0f) m_fModelRotation -= 360.0f; }

	if ((fabs(m_DebugParams.fCameraRotationSpeed) > 0.02f) && !m_bLeftMouseLook && !m_bRightMouseLook) {// Should we rotate the lights...
		m_fCameraRotation += (timeGetTime() - m_iLastRotationUpdateTime) * m_DebugParams.fCameraRotationSpeed;
		if (m_fCameraRotation > 360.0f)  m_fCameraRotation -= 360.0f;
		if (m_fCameraRotation < -360.0f) m_fCameraRotation += 360.0f; }

	m_iLastRotationUpdateTime = timeGetTime();
	if (m_DebugParams.bRenderOnIdle || m_bLeftMouseLook || m_bRightMouseLook) {							// Did they request that we render on Idle?
		RenderScene(); }
}

void CAppForm::DrawBackGroundImage()
{
	if (m_BackGndTexture) 
	{
		uint32 iBGndCol = (FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.r) << 16) | (FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.g) << 8) | (FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.b));
		BASIC_VERTEX_TRANSFORMED_UV v[4]; 
		DXTexture tex;
		vector<LPDXTexture> vTex; 
		tex.m_eTexType = DXTEXTYPE_2D;
		tex.m_pTexture = m_BackGndTexture;
		tex.m_bKeepAlive = true;
		vTex.push_back(&tex); 
		RenderPassOp RenderPass;
		v[0].x = 0.0f;					v[0].y = 0.0f;					v[0].z = 0.0f; v[0].rhw = 1.0f; v[0].u = 0.0f; v[0].v = 0.0f; v[0].color = iBGndCol;
		v[1].x = (float)g_ScreenWidth;	v[1].y = 0.0f;					v[1].z = 0.0f; v[1].rhw = 1.0f; v[1].u = 1.0f; v[1].v = 0.0f; v[1].color = iBGndCol;
		v[2].x = (float)g_ScreenWidth;	v[2].y = (float)g_ScreenHeight;	v[2].z = 0.0f; v[2].rhw = 1.0f; v[2].u = 1.0f; v[2].v = 1.0f; v[2].color = iBGndCol;
		v[3].x = 0.0f;					v[3].y = (float)g_ScreenHeight;	v[3].z = 0.0f; v[3].rhw = 1.0f; v[3].u = 0.0f; v[3].v = 1.0f; v[3].color = iBGndCol;

		if (m_pAntEyeRenderStyle->GetRenderPass(0,&RenderPass)) 
		{
			RenderPass.ZBufferMode = RENDERSTYLE_NOZ;
			RenderPass.TextureStages[0].ColorOp = RENDERSTYLE_COLOROP_SELECTARG1;
			m_pAntEyeRenderStyle->SetRenderPass(0,RenderPass); 
		}

		g_RenderStateMgr.SetRenderStyleStates(m_pAntEyeRenderStyle,0,vTex);

		g_RenderStateMgr.SetFVF(BASIC_VERTEX_TRANSFORMED_UV_FLAGS);

		PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,2,v,sizeof(BASIC_VERTEX_TRANSFORMED_UV));

		if (m_pAntEyeRenderStyle->GetRenderPass(0,&RenderPass)) 
		{
			RenderPass.ZBufferMode = RENDERSTYLE_ZRW;
			RenderPass.TextureStages[0].ColorOp = RENDERSTYLE_COLOROP_SELECTARG2;
			m_pAntEyeRenderStyle->SetRenderPass(0,RenderPass); 
		} 
	}
}

// Render the 3D scene into the render window...
void CAppForm::RenderScene()
{
	if (!PD3DDEVICE || !m_bReady || m_bLoading || m_bResetingRaster || m_bRecompiling || m_bRendering || m_bLoadingTextures) return;	// Check to see if we initialized...
	m_bRendering = true;

	g_Device.SetDefaultRenderStates();											// Setup our default render states...

	D3DXMATRIX ViewMatrix,ProjMatrix,Tmp1,Tmp2;									// Setup your transforms...
	D3DXMatrixRotationX(&Tmp1,m_MouseLook_Rotation.x + D3DXToRadian(m_fCameraRotation));
	D3DXVECTOR4 vTransYAxis; D3DXVec3Transform(&vTransYAxis,&D3DXVECTOR3(0,1,0),D3DXMatrixInverse(&Tmp2,NULL,&Tmp1));
	Tmp1 *= *D3DXMatrixRotationAxis(&Tmp2,(D3DXVECTOR3*)&vTransYAxis,m_MouseLook_Rotation.y);
	D3DXVECTOR4 vTransZAxis; D3DXVec3Transform(&vTransZAxis,&D3DXVECTOR3(0,0,1),D3DXMatrixInverse(&Tmp2,NULL,&Tmp1));
	Tmp1 *= *D3DXMatrixRotationAxis(&Tmp2,(D3DXVECTOR3*)&vTransZAxis,m_MouseLook_Rotation.z);
	D3DXVECTOR3 vCameraVector(0,0,20); vCameraVector += m_MouseLook_Translation; D3DXVECTOR4 vTransCameraVector; // Figure out the camera position...

	g_EffectMgr.SetPosition(vCameraVector.x, vCameraVector.y, vCameraVector.z);

	D3DXVec3Transform(&vTransCameraVector,&vCameraVector,&Tmp1);
	D3DXMatrixTranslation(&ViewMatrix,vTransCameraVector.x,vTransCameraVector.y,vTransCameraVector.z);
	ViewMatrix *= *D3DXMatrixInverse(&Tmp1, NULL, &Tmp1);
	g_RenderStateMgr.SetTransform(D3DTS_VIEW,&ViewMatrix);
	D3DXMatrixPerspectiveFovLH(&ProjMatrix, D3DX_PI/4, (float)g_ScreenWidth/(float)g_ScreenHeight, 1.0f, 1000.0f);
	g_RenderStateMgr.SetTransform(D3DTS_PROJECTION, &ProjMatrix);

	uint32 iBGndCol = (FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.r) << 16) | (FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.g) << 8) | (FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.b));
	PD3DDEVICE->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,iBGndCol,1.0f,0);	// Clear the screen...
	g_Device.Start3D();

	DrawBackGroundImage();														// Draw a background image (if requested)...

	m_RenderCS.Enter();

	D3DXMATRIX WorldMatrix[3];													// Draw your render object...
	D3DXMatrixRotationY(&WorldMatrix[0],D3DXToRadian(m_fModelRotation));

	if(m_pRenderObject) 
	{
		switch (m_pRenderObject->GetType()) 
		{
		case CRenderObject::eRigidMesh : 
			{											// Rigid...
				if (m_pRenderObject)
				{
					((CD3DRigidMesh*)m_pRenderObject)->Render(WorldMatrix[0],m_pRenderStyle,m_TextureArray);
				}

				if (m_pAntEyeRenderObject) 
				{
					m_pAntEyeRenderObject->Render(WorldMatrix[0],m_pAntEyeRenderStyle,m_TextureArray); 
				}
			} break;
		case CRenderObject::eSkelMesh  : 
			{ 
				D3DXMATRIX Tmp;							// Skel..
				D3DXMatrixMultiply(&WorldMatrix[1],&WorldMatrix[0],D3DXMatrixRotationZ(&Tmp,sinf(D3DXToRadian(m_fModelRotation))));
				D3DXMatrixMultiply(&WorldMatrix[2],&WorldMatrix[1],D3DXMatrixRotationY(&Tmp,sinf(D3DXToRadian(m_fModelRotation)/2.0f)));
				
				if (m_pRenderObject)
				{
					((CD3DSkelMesh*)m_pRenderObject)->Render(WorldMatrix,m_pRenderStyle,m_TextureArray); 
				}
			} break;
		} 
	}

	g_Device.End3D();

	PD3DDEVICE->Present(NULL,NULL,NULL,NULL);									// Flip the screen...
	m_RenderCS.Leave();
	m_bRendering = false;
	++g_CurFrameCode;
}

void CAppForm::SetFilenamesBackToDialog()
{
	CString WindowText;

	for (int i=0; i<4; i++)
	{
		m_pEB_TextureList_Filename[i]->GetWindowText(WindowText);
		m_DebugParams.TextureList_Filename[i] = WindowText;
	}
}

void CAppForm::CheckEnableEffect()
{
	for(int i = 0; i < 4; ++i)
	{
		m_pBn_Enable_RenderPass[i]->EnableWindow(!m_MainDlgData.Effect_Enable);
		m_pBn_Config_RenderPass[i]->EnableWindow(!m_MainDlgData.Effect_Enable);
	}
}

void CAppForm::OnBackGndImg_Filename()
{
	if (!m_bReady) return;

	char drive[8], buffer[255];
	CString WindowText;

	m_pEB_BackGndImg_Filename->GetWindowText(WindowText);
	_splitpath(WindowText, drive, buffer, buffer, buffer);

	if (strcmp(drive, "") == 0) // if we have a relative path, use project path.
	{
		sprintf(buffer, "%s%s", m_ProjectPath, WindowText);
		m_DebugParams.BackGndImg_Filename = buffer;
	}
	else
	{
		m_DebugParams.BackGndImg_Filename  = WindowText;
	}

	m_DebugParams.BackGndImg_Filename = LPCSTR(WindowText);

	BackGroundImageChanged();
}


void CAppForm::OnTextureList_Filename1()
{
	CString WindowText;

	m_pEB_TextureList_Filename[0]->GetWindowText(WindowText);
	m_DebugParams.TextureList_Filename[0] = WindowText;

	if (!m_bLoadingTextures)
	{
		DebugTextureSettingChanged();
		RenderStyleDataChanged();
	}
}

void CAppForm::OnTextureList_Filename2()
{
	CString WindowText;

	m_pEB_TextureList_Filename[1]->GetWindowText(WindowText);
	m_DebugParams.TextureList_Filename[1] = WindowText;

	if (!m_bLoadingTextures)
	{
		DebugTextureSettingChanged();
		RenderStyleDataChanged();
	}
}

void CAppForm::OnTextureList_Filename3()
{
	CString WindowText;

	m_pEB_TextureList_Filename[2]->GetWindowText(WindowText);
	m_DebugParams.TextureList_Filename[2] = WindowText;

	if (!m_bLoadingTextures)
	{
		DebugTextureSettingChanged();
		RenderStyleDataChanged();
	}
}

void CAppForm::OnTextureList_Filename4()
{
	//char drive[8], buffer[255];
	CString WindowText;

	m_pEB_TextureList_Filename[3]->GetWindowText(WindowText);
	m_DebugParams.TextureList_Filename[3] = WindowText;

	/*_splitpath(WindowText, drive, buffer, buffer, buffer);

	if ((m_ProjectPath[0] != '\0') && (strcmp(drive, "") == 0) && !(strcmp(WindowText, "") == 0)) // if we have a relative path, use project path.
	{
		sprintf(buffer, "%s%s", m_ProjectPath, WindowText);
		m_DebugParams.TextureList_Filename[0] = buffer;
	}
	else
	{
		m_DebugParams.TextureList_Filename[0] = WindowText;
	}*/

	if (!m_bLoadingTextures)
	{
		DebugTextureSettingChanged();
		RenderStyleDataChanged();
	}
}


BOOL CAppForm::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	CString tipText;
    UINT nID =pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
			// Here which switch on the control ID and use the appropriate string from the string table.

			switch(nID)
			{
			case IDC_RENDERSTYLE_FILENAME:
				tipText.LoadString(IDS_RENDERSTYLE_FILENAME); break;
			case IDC_ENABLE_REFRAST:
				tipText.LoadString(IDS_ENABLE_REFRAST); break;
			case IDC_DBGPARAM_RENDONIDLE:
				tipText.LoadString(IDS_DBGPARAM_RENDONIDLE); break;
			case IDC_DBGPARAM_ROTATEMODEL:
				tipText.LoadString(IDS_DBGPARAM_ROTATEMODEL); break;
			case IDC_DBGPARAM_MODEL:
				tipText.LoadString(IDS_DBGPARAM_MODEL); break;
			case IDC_DBGPARAM_BACKGNDIMG_FILENAME:
				tipText.LoadString(IDS_DBGPARAM_BACKGNDIMG_FILENAME); break;
			case IDC_DBGPARAM_BACKGNDCOLOR_CONFIG:
				tipText.LoadString(IDS_DBGPARAM_BACKGNDCOLOR); break;
			case IDC_DBGPARAM_LIGHTCONFIG:
				tipText.LoadString(IDS_DBGPARAM_LIGHTCONFIG); break;
			case IDC_DBGPARAM_LIGHTCOUNT:
				tipText.LoadString(IDS_DBGPARAM_LIGHTCOUNT); break;
			case IDC_DBGPARAM_LIGHTCOLOR_CONFIG:
				tipText.LoadString(IDS_DBGPARAM_LIGHTCOLOR); break;

#ifdef _RS_VIEWER
			case IDC_TEXTURELIST_FILENAME1:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1_VIEWER); break;
			case IDC_TEXTURELIST_FILENAME2:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1_VIEWER); break;
			case IDC_TEXTURELIST_FILENAME3:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1_VIEWER); break;
			case IDC_TEXTURELIST_FILENAME4:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1_VIEWER); break;
#else
			case IDC_TEXTURELIST_FILENAME1:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1); break;
			case IDC_TEXTURELIST_FILENAME2:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1); break;
			case IDC_TEXTURELIST_FILENAME3:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1); break;
			case IDC_TEXTURELIST_FILENAME4:
				tipText.LoadString(IDS_TEXTURELIST_FILENAME1); break;
#endif

			case IDC_DBGPARAM_ROTATESPEED:
				tipText.LoadString(IDS_DBGPARAM_ROTATESPEED); break;
			case IDC_RESETCAMERA:
				tipText.LoadString(IDS_RESETCAMERA); break;
			case IDC_ENABLE_RENDERPASS1:
				tipText.LoadString(IDS_ENABLE_RENDERPASS1); break;
			case IDC_ENABLE_RENDERPASS2:
				tipText.LoadString(IDS_ENABLE_RENDERPASS2); break;
			case IDC_ENABLE_RENDERPASS3:
				tipText.LoadString(IDS_ENABLE_RENDERPASS3); break;
			case IDC_ENABLE_RENDERPASS4:
				tipText.LoadString(IDS_ENABLE_RENDERPASS4); break;
			case IDC_CONFIG_RENDERPASS1:
				tipText.LoadString(IDS_CONFIG_RENDERPASS1); break;
			case IDC_CONFIG_RENDERPASS2:
				tipText.LoadString(IDS_CONFIG_RENDERPASS1); break;
			case IDC_CONFIG_RENDERPASS3:
				tipText.LoadString(IDS_CONFIG_RENDERPASS1); break;
			case IDC_CONFIG_RENDERPASS4:
				tipText.LoadString(IDS_CONFIG_RENDERPASS1); break;
			case IDC_MATERIALCOLOR_AMBIENT_CONFIG:
				tipText.LoadString(IDS_MATERIALCOLOR_AMBIENT); break;
			case IDC_MATERIAL_AMBIENTALPHA_EDIT:
				tipText.LoadString(IDS_MATERIAL_AMBIENTALPHA_EDIT); break;
			case IDC_MATERIALCOLOR_DIFFUSE_CONFIG:
				tipText.LoadString(IDS_MATERIALCOLOR_DIFFUSE); break;
			case IDC_MATERIAL_DIFFUSEALPHA_EDIT:
				tipText.LoadString(IDS_MATERIAL_DIFFUSEALPHA_EDIT); break;
			case IDC_MATERIALCOLOR_EMISSIVE_CONFIG:
				tipText.LoadString(IDS_MATERIALCOLOR_EMISSIVE); break;
			case IDC_MATERIAL_EMISSIVEALPHA_EDIT:
				tipText.LoadString(IDS_MATERIAL_EMISSIVEALPHA_EDIT); break;
			case IDC_MATERIALCOLOR_SPECULAR_CONFIG:
				tipText.LoadString(IDS_MATERIALCOLOR_SPECULAR); break;
			case IDC_MATERIAL_SPECULARALPHA_EDIT:
				tipText.LoadString(IDS_MATERIAL_SPECULARALPHA_EDIT); break;
			case IDC_MATERIAL_SPECULARPOWER_EDIT:
				tipText.LoadString(IDS_MATERIAL_SPECULARPOWER_EDIT); break;
			case IDC_RS_CLIPMODE:
				tipText.LoadString(IDS_RS_CLIPMODE); break;
			case IDC_RENDERVIEW:
				tipText.LoadString(IDS_RENDERVIEW); break;
			}
			strcpy(m_ToolTipText, tipText.GetBuffer(0));
			pTTT->lpszText = m_ToolTipText;
            pTTT->hinst = NULL;
            return(TRUE);
        }
    }
    return(FALSE);
}


/*
void CAppForm::OnPlatformD3D() 
{
	UpdateData();

	m_PlatformD3D = !m_PlatformD3D;

	LimitDialogByPlatforms();
}

void CAppForm::OnUpdatePlatformD3D(CCmdUI *pCmdUI) 
{
	UpdateData();

	pCmdUI->SetCheck(m_PlatformD3D);
}


void CAppForm::OnPlatformPS2() 
{
	UpdateData();

	m_PlatformPS2 = !m_PlatformPS2;

	LimitDialogByPlatforms();
}

void CAppForm::OnUpdatePlatformPS2(CCmdUI *pCmdUI) 
{
	UpdateData();

	pCmdUI->SetCheck(m_PlatformPS2);
}


void CAppForm::OnPlatformXBox() 
{
	UpdateData();

	m_PlatformXBox = !m_PlatformXBox;

	LimitDialogByPlatforms();
}

void CAppForm::OnUpdatePlatformXBox(CCmdUI *pCmdUI) 
{
	UpdateData();

	pCmdUI->SetCheck(m_PlatformXBox);
}

void CAppForm::LimitDialogByPlatforms()
{
	// Render Pass sub-dialogs

	for (int i=0; i<4; i++)
	{
		// If dialog is open, we need to update the dialog controls
		if (m_RenderPassData[i].bDialogOpen)
		{
			m_RenderPassDlg[i]->LimitDialogByPlatforms(m_PlatformD3D, m_PlatformPS2, m_PlatformXBox);
		}
	}
}
*/

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)

#ifdef _RS_VIEWER
	enum { IDD = IDD_ABOUTBOX_VIEWER };
#else
	enum { IDD = IDD_ABOUTBOX };
#endif
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog();
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CAppForm::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// Launch Game Content Creation Guide
void CAppForm::OnHelpGameContentCreationGuide()
{
	char szPath[MAX_PATH];
	int shellVal, stringLen;

	
	sprintf(szPath, "%s..\\..\\docs\\GameContentCreationGuide.pdf", g_StartingDirectory);
	shellVal = (int)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath,NULL,NULL,SW_SHOWNORMAL);
	

	// Check installed path in registry if we don't find the file in the normal location
	if ((shellVal <= 32) && ((shellVal == ERROR_FILE_NOT_FOUND) || (shellVal == ERROR_PATH_NOT_FOUND)))
	{
		char szPath2[MAX_PATH];
		HKEY hKey;
		unsigned long dataSize = sizeof(char) * MAX_PATH, keyType = REG_SZ;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\LithTech Inc.\\Jupiter\\", NULL, KEY_ALL_ACCESS, &hKey) == NO_ERROR)
		{
			if (RegQueryValueEx(hKey, "InstallDir", NULL, &keyType, (unsigned char *)szPath, &dataSize) == NO_ERROR)
			{
				if (szPath[0] == '\"') // Remove quotes from path
				{
					stringLen = strlen(szPath)-2;				ASSERT(stringLen > 0);
					strncpy(szPath2, &szPath[1], stringLen);
					szPath2[stringLen] = '\0';
					strcpy(szPath, szPath2);
				}
				sprintf(szPath2, "%s\\docs\\GameContentCreationGuide.pdf", szPath);
				shellVal = (int)ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open",szPath2,NULL,NULL,SW_SHOWNORMAL);
			}
			else
			{
				shellVal = 33; // in order to skip switch below
				AppMessageBox(AFX_IDP_REGISTRY_ERROR, MB_OK);
			}

			RegCloseKey(hKey);
		}
		else
		{
			shellVal = 33; // in order to skip switch below
			AppMessageBox(AFX_IDP_REGISTRY_ERROR, MB_OK);
		}
	}

	if (shellVal <= 32)
	{
		switch (shellVal)
		{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				AppMessageBox(AFX_IDP_NO_CREATION_GUIDE, MB_OK);
				break;

			case SE_ERR_ASSOCINCOMPLETE:
			case SE_ERR_NOASSOC:
				AppMessageBox(AFX_IDP_NO_PDF_READER, MB_OK);
				break;

			default:
				CString str;
				str.FormatMessage(AFX_IDP_GENERIC_CREATION_GUIDE_FALURE, shellVal);
				AppMessageBox(str, MB_OK);
		}
	}

	return;
}

void CAppForm::OnHelpWeb()
{
	ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),"open","http://www.touchdownentertainment.com",NULL,NULL,SW_SHOW);

	return;
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	/*// Set the warning and build text
	CEdit *pEdit;
	CString str;

	pEdit = (CEdit *)GetDlgItem(IDC_ABOUTWARN);
	if( pEdit )
	{
		str.LoadString( IDS_ABOUTWARN );
		pEdit->SetWindowText( str );
	}

	pEdit = (CEdit *)GetDlgItem(IDC_BUILDSTRING);
	if (pEdit)
	{
		LTVersionInfo cVersion;
		char *pStr = str.GetBufferSetLength(256);
		if (GetLTExeVersion(GetApp()->m_hInstance, cVersion) == LT_OK)
			cVersion.GetString(pStr, 256);
		else
			str.LoadString( IDS_BUILDSTRING );
		pEdit->SetWindowText( str );
	}*/

	return TRUE;
}

int AppMessageBox( UINT idString, UINT nType )
{
	CString		str;
	
	str.LoadString( idString );
	return AfxGetMainWnd()->MessageBox( str, AfxGetAppName(), nType );
}

int AppMessageBox( const char *pStr, UINT nType )
{
	return AfxGetMainWnd()->MessageBox( pStr, AfxGetAppName(), nType );
}

// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
	return malloc(size);
}

void DefStdlithFree(void *ptr)
{
	free(ptr);
}



void CAppForm::OnCbnEditchangeEffectTechnique()
{
	// TODO: Add your control notification handler code here

}

void CAppForm::OnCbnEditupdateEffectTechnique()
{
	// TODO: Add your control notification handler code here

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_EFFECT_TECHNIQUE);

	int nIndex = pComboBox->GetCurSel();

	CString sString;
	pComboBox->GetLBText(nIndex, sString);

	g_EffectMgr.SetTechnique(sString);

	sString.ReleaseBuffer();
}

void CAppForm::OnCbnCloseupEffectTechnique()
{
	// TODO: Add your control notification handler code here
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_EFFECT_TECHNIQUE);

	int nIndex = pComboBox->GetCurSel();

	CString sString;
	pComboBox->GetLBText(nIndex, sString);

	g_EffectMgr.SetTechnique(sString);

	sString.ReleaseBuffer();
}

void CAppForm::OnLbnDblclkOutput()
{
	// If the user double clicks an entry in the console output list,
	// let's pop up a message box so that they can read the whole message.
	int nIndex = m_pLB_OutputWindow->GetCurSel();

	CString text;
	m_pLB_OutputWindow->GetText(nIndex, text);

	MessageBox(text, "Console Text", MB_OK);

	text.ReleaseBuffer();
}
