
#ifndef RENDERSTYLE_EDITOR_H
#define RENDERSTYLE_EDITOR_H

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include "stdlith.h"
#include "resource.h"
#include "d3d_renderstyle.h"
#include "d3dmeshrendobj_rigid.h"
#include "d3dmeshrendobj_skel.h"
#include "Utilities.h"
#include "dtx_files.h"
#include <string>
#include <vector>


using namespace std;

// DEFINES
#define FLOAT01_TO_INT0255(X)	((uint32)(X*255.0f))
#define INT0255_TO_FLOAT01(X)	((float)X/255.0f)
#define MAX_CAMERAROTATE_SPEED	0.15f

// The DebugParams (kept by the main window)...
class CDebugParams {
public:
	bool					bUseRefRast;
	bool					bRenderOnIdle;
	bool					bRotateModel;
	uint32					LightConfig;
	uint32					LightCount;
	FourFloatColor			LightColor;
	float					fCameraRotationSpeed;
	string					BackGndImg_Filename;
	FourFloatColor			BackGndImg_Color;
	string					ModelName;
	string					TextureList_Filename[4];
	string					EffectFilename;
};

class CDirect3DData {
public:
	CDirect3DData()			{ bDialogOpen = false; }
	bool					bDialogOpen;

	string					VertexShaderFilename;
	bool					bExpandForSkinning;
	int32					ConstVector_ConstReg1;	// Should be -1 if not used (same goes for all const regs)...
	FourFloatVector			ConstVector_Param1;
	int32					ConstVector_ConstReg2;
	FourFloatVector			ConstVector_Param2;
	int32					ConstVector_ConstReg3;
	FourFloatVector			ConstVector_Param3;
	int32					WorldViewTransform_ConstReg;
	uint32					WorldViewTransform_Count;
	int32					ProjTransform_ConstReg;
	int32					WorldViewProjTransform_ConstReg;
	int32					ViewProjTransform_ConstReg;
	int32					CamPos_MSpc_ConstReg;
	uint32					Light_Count;
	int32					LightPosition_MSpc_ConstReg;
	int32					LightPosition_CSpc_ConstReg;
	int32					LightColor_ConstReg;
	int32					LightAtt_ConstReg;
	int32					Material_AmbDifEm_ConstReg;
	int32					Material_Specular_ConstReg;
	int32					AmbientLight_ConstReg;
	int32					PrevWorldViewTrans_ConstReg;
	uint32					PrevWorldViewTrans_Count;
	int32					Last_ConstReg;

	bool					bDeclaration_Stream_Position[4];	// Declaration flags...
	bool					bDeclaration_Stream_Normal[4];
	bool					bDeclaration_Stream_UVSets[4];
	int32					Declaration_Stream_UVCount[4];
	bool					bDeclaration_Stream_BasisVectors[4]; };

// All the data for a render pass (kept by the main dialog - pass to this dialog on creation)...
class CRenderPassData {
public:
	CRenderPassData()		{ bDialogOpen = false; }
	bool					bDialogOpen;
	string					DialogText;
	int						PositionX, PositionY;	// Screen position of the dialog holding this info

	// Texture Stage Params...
	ERenStyle_TextureParam	TextureParam[4];		// An index into the texture list...
	ERenStyle_ColorOp		ColorOp[4];
	ERenStyle_ColorArg		ColorArg1[4];
	ERenStyle_ColorArg		ColorArg2[4];
	ERenStyle_AlphaOp		AlphaOp[4];
	ERenStyle_AlphaArg		AlphaArg1[4];
	ERenStyle_AlphaArg		AlphaArg2[4];
	ERenStyle_UV_Source		UVSource[4];
	ERenStyle_UV_Address	UAddress[4];
	ERenStyle_UV_Address	VAddress[4];
	ERenStyle_TexFilter		TexFilter[4];
	bool					UVTransform_Enable[4];
	D3DXMATRIX				UVTransform_Matrix[4];
	bool					ProjectTexCoord[4];
	uint32					TexCoordCount[4];

	// Render States...
	ERenStyle_BlendMode		Blend;
	ERenStyle_ZBufferMode	ZBuffer;
	ERenStyle_CullMode		CullMode;
	ERenStyle_TestMode		AlphaTestMode;
	ERenStyle_TestMode		ZBufferTestMode;
	ERenStyle_FillMode		FillMode;
	bool					DynamicLight;
	uint32					AlphaRef;
	uint32					TextureFactor;

	// Direct3D Options...
	bool					VertexShader_Enable;
	int						VertexShader_ID;
	bool					PixelShader_Enable;
	int						PixelShader_ID;
	CDirect3DData			Direct3DData;

	bool					bUseBumpEnvMap;			// BumpEnvMap Params...
	uint32					BumpEnvMapStage;
	float					fBumpEnvMap_Scale;
	float					fBumpEnvMap_Offset;
	uint32					id;						// Stores which of the four dialogs this is
};

// All the render style data in main dialog...
class CMainDlgData {
public:
	// Render Pass Data...
	bool					Effect_Enable;
	int						Effect_ID;

	bool					RenderPass_Enable[4];

	// Lighting Material Data...
	FourFloatColor			LightingMaterial_Ambient;
	FourFloatColor			LightingMaterial_Diffuse;
	FourFloatColor			LightingMaterial_Emissive;
	FourFloatColor			LightingMaterial_Specular;
	float					LightingMaterial_SpecularPower;

	// Direct3D Options...
};

//-----------------------------------------------------------------------------
// Name: class CAppDoc
// Desc: Overridden CDocument class needed for the CFormView
//-----------------------------------------------------------------------------
class CAppDoc : public CDocument
{
protected:
    DECLARE_DYNCREATE(CAppDoc)
};

//-----------------------------------------------------------------------------
// Name: class CMyToolTipCtrl
// Desc: Overridden CToolTipCtrl to add some additional functionality
//-----------------------------------------------------------------------------
class CMyToolTipCtrl : public CToolTipCtrl
{
public:
	bool AddWindowTool (CWnd*, LPCTSTR);
	bool AddRectTool (CWnd*, LPCTSTR, LPCRECT, UINT);
};

//-----------------------------------------------------------------------------
// Name: class CAppFrameWnd
// Desc: CFrameWnd-based class needed to override the CFormView's window style
//-----------------------------------------------------------------------------
class CAppFrameWnd : public CFrameWnd
{
protected:
    DECLARE_DYNCREATE(CAppFrameWnd)
public:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
};

//-----------------------------------------------------------------------------
// Name: class CApp
// Desc: Main MFCapplication class derived from CWinApp.
//-----------------------------------------------------------------------------
class CApp : public CWinApp
{
public:
    //{{AFX_VIRTUAL(CD3DApp)
    virtual BOOL InitInstance();
    virtual BOOL OnIdle( LONG );
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CApp)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------
// Name: class CRenderPassDlg
// Desc: See RenderPassDlg.h
//-----------------------------------------------------------------------------

class CRenderPassDlg;

//-----------------------------------------------------------------------------
// Name: class CAppForm
// Desc: CFormView-based class which allows the UI to be created with a form
//       (dialog) resource. This class manages all the controls on the form.
//-----------------------------------------------------------------------------
class CAppForm : public CFormView
{
public:
    bool			IsReady()						{ return m_bReady; }

	bool			Create3DEnv();					// Create the 3D device, stuff we'll need...
	void			Destroy3DEnv();					// Release the 3D device, free all that good stuff...

	void			RenderStyleDataChanged(bool bForceRecompile = false);	// To keep the render style up to date...
	void			DebugTextureSettingChanged();	// To keep the texture list up to date...
	void			SceneLightsChanged();			// To keep the scene lights up to date...
	void			BackGroundImageChanged();		// To keep the background image up to date...

	void			OnIdle();
	void			RenderScene();					// Render the scene into the render window...

	bool			m_bNoRecompile;					// Don't do re-compiles while this is true (so you can much with a bunch of stuff and not have to recompile a bunch of times)...
protected:
    DECLARE_DYNCREATE(CAppForm)

					CAppForm();
    virtual			~CAppForm();

	void			InitializeDialogControls();		// Grab all the dialog controls....

private:
	bool			m_bWindowed;
	bool			m_bActive;
	bool			m_bReady;
	bool			m_bLoading;
	bool			m_bResetingRaster;				
	bool			m_bRecompiling;
	bool			m_bRendering;
	bool			m_bLoadingTextures;
public:
	char			m_RenderStyle_FileName[255];	// No longer class type string, since the string was behaving poorly

	// Dialog controls...
	CListBox*		m_pLB_OutputWindow;
	CButton*		m_pBn_EnableEffect;
	CEdit*			m_pEB_EffectID;
	CButton*		m_pBn_Enable_RenderPass[4];		// Render Pass Params...
private:
	CButton*		m_pBn_Config_RenderPass[4];
	CButton*		m_pBn_UseRefRast;				// Debug Params...
	CButton*		m_pBn_RenderOnIdle;
	CButton*		m_pBn_RotateModel;
	CComboBox*		m_pCB_LightConfig;
	CEdit*			m_pEB_LightCount;
	CWnd*			m_pWnd_LightColor;
	CButton*		m_pBn_LightColor_Config;
	CSliderCtrl*	m_pSl_CameraRotateSpeed;
	CEdit*			m_pEB_BackGndImg_Filename;
	CButton*		m_pBn_BackGndImg_Browse;
	CWnd*			m_pWnd_BackGndColor;
	CButton*		m_pBn_BackGndColor_Config;
	CComboBox*		m_pCB_DBGParam_Model;
	CButton*		m_pBn_TextureList_Browse[4];
	CEdit*			m_pEB_TextureList_Filename[4];
	CEdit*			m_pEB_EffectFilename;
	CWnd*			m_pWnd_LightMat_Ambient;		// Material Properities...
	CWnd*			m_pWnd_LightMat_Diffuse;
	CWnd*			m_pWnd_LightMat_Emissive;
	CWnd*			m_pWnd_LightMat_Specular;
	CButton*		m_pBn_LightMat_Ambient_Config;
	CButton*		m_pBn_LightMat_Diffuse_Config;
	CButton*		m_pBn_LightMat_Emissive_Config;
	CButton*		m_pBn_LightMat_Specular_Config;
	CEdit*			m_pEB_LightMat_Ambient_Alpha;
	CEdit*			m_pEB_LightMat_Diffuse_Alpha;
	CEdit*			m_pEB_LightMat_Emissive_Alpha;
	CEdit*			m_pEB_LightMat_Specular_Alpha;
	CEdit*			m_pEB_LightMat_SpecularPower;
	CComboBox*		m_pCB_RenderStyle_Files;
public:
	char			m_ProjectPath[512];				// path of the project file

	// RenderStyle Data (Keeps all our data - should pretty much always matches the dialog control data. We create our render style from this data)...
	CDebugParams	m_DebugParams;					// The Debug Param data...
	CMainDlgData	m_MainDlgData;					// All the RenderStyle data in the main dialog...
	CRenderPassData	m_RenderPassData[4];			// We have a set of RenderPass dialogs (one to go along with each of the supported number of passes)...
	CRenderPassDlg *m_RenderPassDlg[4];				// Pointers to the four dialogs

private:

	// Mouse look (in render window data)...
	bool			m_bLeftMouseLook;				// Left button is down (they're doing mouse look right now)...
	bool			m_bRightMouseLook;				// Right button is down (they're doing mouse look right now)...
	D3DXVECTOR3		m_MouseLook_Rotation;			// Rotation caused by mouselook...
	D3DXVECTOR3		m_MouseLook_Translation;		// Translation caused by mouselook...
	CPoint			m_LastKnownMousePos;			// Last known mouse position...

	// Scene Data...
	CRenderObject*	m_pRenderObject;				// The render object we're displaying in the 3d window...
	CD3DRigidMesh*	m_pAntEyeRenderObject;			// There's a little backup render object used just for the ant head (the eyes)...Just for fun...
	CD3DRenderStyle* m_pRenderStyle;				// The render style we're using...
	CD3DRenderStyle* m_pAntEyeRenderStyle;			// The render style we're using...
	vector<LPDXTexture> m_TextureArray;				// The debug setting texture array (stores the real textures we use)...
	LPDIRECT3DTEXTURE9	m_BackGndTexture;	// Background Image texture...
	float			m_fModelRotation;				// Current rotation of the world model...
	float			m_fCameraRotation;				// Current rotation of the lights...
	uint32			m_iLastRotationUpdateTime;		// Used to figure out how much to update model rotation (last time you did it)...

	// Misc Helper Data...
	CriticalSection m_RenderCS;						// Re-Compiles can come in when we're rendering - don't want that...

	// Load/Save Functions...
	void			SetDefaultRenderStyleData();	// Set the render style data (CMainDlgData, CRenderPassData, etc) to the defaults...
	bool			LoadRenderStyle(const char* szFileName);				// Load the render style into the RenderStyle...
	bool			SaveRenderStyle(const char* szFileName);				// Save the render style from the RenderStyle...

	class Filename
	{
	public:
		char path[512];
		char file[255];

		Filename()	{ file[0] = path[0] = '\0'; } 

		Filename(char* inpath, char* infile)
		{
			sprintf(path, "%s", inpath);
			sprintf(file, "%s", infile);
		}
	};

	CMoArray<Filename*>	m_RenderStyle_Files;			// list of renderstyle files in the current project

	//CMyToolTipCtrl m_ctlTT;  // Tool Tip manager
	BOOL CAppForm::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	char m_ToolTipText[512]; // buffer to store current text

	public:
//	bool m_PlatformD3D;		// status of the checkmark for D3D
//	bool m_PlatformPS2;		// status of the checkmark for PS2
//	bool m_PlatformXBox;	// status of the checkmark for XBox
//	void LimitDialogByPlatforms();	// eliminate all menu items not supported by the current platforms

private:
	void			PopulateDialogControls_And_CreateIDtoRSEnum_Maps();
	void			SetDialogControls_From_RenderStyleData(bool bInitialUpdate = false);

	// Render Style functions...
	bool			SetRenderStyleFromRenderStyleData(bool bForceRecompile = false);
	bool			SetRenderStyleDataFromRenderStyle();					// After we do a load (into the render style we use this function)...

	// Helper functions...
	void			OutputMsg(LPSTR fmt, ...);
	bool			CreateTheRenderObject();								// Call this to create the render object...
	bool			GetColor(int* pRed, int* pGreen, int* pBlue);			// Create a dialog and get a color...
	void			DrawColor(CWnd *pWnd, int r, int g, int b);				// Fill the window with the color...
public:	
	void			DrawHelp(CWnd *pWnd);									// Draw help BMP to given CWnd
	bool			StripPath(const char *target, const char *path, char *output);		// Remove the given path from the given target string
private:	
	void			DrawBackGroundImage();									// Actually draw the background image...
	bool			DoesDirHaveFile(char *path, char *pName);				// Tests the directory in the path for the filename given in pName
	void			EnumerateAllLTAs(char *path);							// Adds all RenderStyle LTAs found in directory and subdirectories to m_RenderStyle_Files
	LPDXTexture		LoadTexture(const char* szFilename);					// Load a texture file...	

	void SetFilenamesBackToDialog();										// This resets the internal paths back to what's specified in the edit box (used for project-relative paths)
	void			CheckEnableEffect();
		
protected:
	//{{AFX_DATA(CAppForm)
	enum { IDD = IDD_FORMVIEW };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAppForm)
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAppForm)
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT lParam, UINT hParam, CScrollBar* pCtrlWnd)		{ if (pCtrlWnd == NULL)  { CFormView::OnHScroll(lParam, hParam, pCtrlWnd); } else { OnDlgParam_CameraRotSpeed(); }} // If the message is to the window, pass it on.  Otherwise, send to to the camera speed slider.
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags,CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags,CPoint point);
	afx_msg void OnMouseMove(UINT nFlags,CPoint point);
	afx_msg void OnProjectOpen();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnSetDefaults()											{ m_bNoRecompile = true; SetDefaultRenderStyleData(); SetDialogControls_From_RenderStyleData(true); m_bNoRecompile = false; OnRefRast_Enable(); OnDbgParam_Model(); SetRenderStyleFromRenderStyleData(); }
	afx_msg void OnCompilePCD3D();
	afx_msg void OnCompilePS2();
	afx_msg void OnRefRast_Enable();
	afx_msg void OnRendOnIdle_Enable()										{ m_DebugParams.bRenderOnIdle = m_pBn_RenderOnIdle->GetCheck() ? true : false; }
	afx_msg void OnRotateModel_Enable()										{ m_DebugParams.bRotateModel = m_pBn_RotateModel->GetCheck() ? true : false; }
	afx_msg void OnDlgParam_LightConfig()									{ if (!m_bReady) return; m_DebugParams.LightConfig = m_pCB_LightConfig->GetCurSel(); SceneLightsChanged(); }
	afx_msg void OnDlgParam_LightCount()									{ if (!m_bReady) return; CString WindowText; m_pEB_LightCount->GetWindowText(WindowText); m_DebugParams.LightCount = atoi(LPCSTR(WindowText)); SceneLightsChanged(); }
	afx_msg void OnDlgParam_LightColorConfig()								{ if (!m_bReady) return; int r = FLOAT01_TO_INT0255(m_DebugParams.LightColor.r), g = FLOAT01_TO_INT0255(m_DebugParams.LightColor.g), b = FLOAT01_TO_INT0255(m_DebugParams.LightColor.b); if (GetColor(&r, &g, &b)) { m_DebugParams.LightColor.r = INT0255_TO_FLOAT01(r); m_DebugParams.LightColor.g = INT0255_TO_FLOAT01(g); m_DebugParams.LightColor.b = INT0255_TO_FLOAT01(b); } if (m_pWnd_LightColor) { CRect rcWnd; m_pWnd_LightColor->GetWindowRect(&rcWnd); InvalidateRect(&rcWnd, FALSE); SceneLightsChanged(); } }
	afx_msg void OnDlgParam_CameraRotSpeed()								{ if (!m_bReady) return; m_DebugParams.fCameraRotationSpeed = ((float)(m_pSl_CameraRotateSpeed->GetPos()) / 100.0f * MAX_CAMERAROTATE_SPEED); }
	afx_msg void OnBackGndImg_Filename();
	afx_msg void OnBackGndImg_Browse()										{ if (!m_bReady) return; CFileDialog BrowseBox(true,"dtx",m_DebugParams.TextureList_Filename[0].c_str(),OFN_HIDEREADONLY|OFN_CREATEPROMPT,"Image Files|*.*||");if (BrowseBox.DoModal() == IDOK) { m_pEB_BackGndImg_Filename->SetWindowText(BrowseBox.GetPathName()); m_DebugParams.BackGndImg_Filename = BrowseBox.GetPathName();} BackGroundImageChanged(); }
	afx_msg void OnDlgParam_BackGndColor()									{ if (!m_bReady) return; int r = FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.r), g = FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.g), b = FLOAT01_TO_INT0255(m_DebugParams.BackGndImg_Color.b); if (GetColor(&r, &g, &b)) { m_DebugParams.BackGndImg_Color.r = INT0255_TO_FLOAT01(r); m_DebugParams.BackGndImg_Color.g = INT0255_TO_FLOAT01(g); m_DebugParams.BackGndImg_Color.b = INT0255_TO_FLOAT01(b); } if (m_pWnd_BackGndColor) { CRect rcWnd; m_pWnd_BackGndColor->GetWindowRect(&rcWnd); InvalidateRect(&rcWnd, FALSE); } }
	afx_msg void OnDlgParam_ResetCamera()									{ m_MouseLook_Rotation = D3DXVECTOR3(0.0f,0.0f,0.0f); m_MouseLook_Translation = D3DXVECTOR3(0.0f,0.0f,0.0f); m_fModelRotation = 0.0f; m_fCameraRotation = 0.0f; RenderScene();}
	afx_msg void OnDbgParam_Model();
	afx_msg void OnRenderStyle_Filename();
	afx_msg void OnBnClickedEnableEffect();
	afx_msg void OnEnChangeEffectId()										{ if (!m_pEB_EffectID) return; CString buf; m_pEB_EffectID->GetWindowText(buf); m_MainDlgData.Effect_ID = (int)atoi(LPCTSTR(buf)); RenderStyleDataChanged(); }
	afx_msg void OnConfig_RenderPass_Enable1()								{ m_MainDlgData.RenderPass_Enable[0] = m_pBn_Enable_RenderPass[0]->GetCheck() ? true : false; RenderStyleDataChanged(true); }
	afx_msg void OnConfig_RenderPass_Enable2()								{ m_MainDlgData.RenderPass_Enable[1] = m_pBn_Enable_RenderPass[1]->GetCheck() ? true : false; RenderStyleDataChanged(true); }
	afx_msg void OnConfig_RenderPass_Enable3()								{ m_MainDlgData.RenderPass_Enable[2] = m_pBn_Enable_RenderPass[2]->GetCheck() ? true : false; RenderStyleDataChanged(true); }
	afx_msg void OnConfig_RenderPass_Enable4()								{ m_MainDlgData.RenderPass_Enable[3] = m_pBn_Enable_RenderPass[3]->GetCheck() ? true : false; RenderStyleDataChanged(true); }
	afx_msg void OnConfig_RenderPass1();
	afx_msg void OnConfig_RenderPass2();
	afx_msg void OnConfig_RenderPass3();
	afx_msg void OnConfig_RenderPass4();
	afx_msg void OnTextureList_Browse1();
	afx_msg void OnTextureList_Browse2();
	afx_msg void OnTextureList_Browse3();
	afx_msg void OnTextureList_Browse4();
	afx_msg void OnEffectFile_Browse1();
	afx_msg void OnTextureList_Filename1();
	afx_msg void OnTextureList_Filename2();
	afx_msg void OnTextureList_Filename3();
	afx_msg void OnTextureList_Filename4();
	afx_msg void OnLightMaterial_AmbientAlpha()								{ if (!m_pEB_LightMat_Ambient_Alpha) return; CString buf; m_pEB_LightMat_Ambient_Alpha->GetWindowText(buf); m_MainDlgData.LightingMaterial_Ambient.a = INT0255_TO_FLOAT01(atoi(LPCTSTR(buf))); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_DiffuseAlpha()								{ if (!m_pEB_LightMat_Diffuse_Alpha) return; CString buf; m_pEB_LightMat_Diffuse_Alpha->GetWindowText(buf); m_MainDlgData.LightingMaterial_Diffuse.a = INT0255_TO_FLOAT01(atoi(LPCTSTR(buf))); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_EmissiveAlpha()							{ if (!m_pEB_LightMat_Emissive_Alpha) return; CString buf; m_pEB_LightMat_Emissive_Alpha->GetWindowText(buf); m_MainDlgData.LightingMaterial_Emissive.a = INT0255_TO_FLOAT01(atoi(LPCTSTR(buf))); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_SpecularAlpha()							{ if (!m_pEB_LightMat_Specular_Alpha) return; CString buf; m_pEB_LightMat_Specular_Alpha->GetWindowText(buf); m_MainDlgData.LightingMaterial_Specular.a = INT0255_TO_FLOAT01(atoi(LPCTSTR(buf))); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_SpecularPower()							{ if (!m_pEB_LightMat_SpecularPower) return; CString buf; m_pEB_LightMat_SpecularPower->GetWindowText(buf); m_MainDlgData.LightingMaterial_SpecularPower = (float)atof(LPCTSTR(buf)); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_AmbientConfig()							{ int r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.r), g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.g), b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Ambient.b); if (GetColor(&r, &g, &b)) { m_MainDlgData.LightingMaterial_Ambient.r = INT0255_TO_FLOAT01(r); m_MainDlgData.LightingMaterial_Ambient.g = INT0255_TO_FLOAT01(g); m_MainDlgData.LightingMaterial_Ambient.b = INT0255_TO_FLOAT01(b); } CRect rcWnd; m_pWnd_LightMat_Ambient->GetWindowRect(&rcWnd); ScreenToClient(&rcWnd); InvalidateRect(&rcWnd, FALSE); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_DiffuseConfig()							{ int r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.r), g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.g), b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Diffuse.b); if (GetColor(&r, &g, &b)) { m_MainDlgData.LightingMaterial_Diffuse.r = INT0255_TO_FLOAT01(r); m_MainDlgData.LightingMaterial_Diffuse.g = INT0255_TO_FLOAT01(g); m_MainDlgData.LightingMaterial_Diffuse.b = INT0255_TO_FLOAT01(b); } CRect rcWnd; m_pWnd_LightMat_Diffuse->GetWindowRect(&rcWnd); ScreenToClient(&rcWnd); InvalidateRect(&rcWnd, FALSE); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_EmissiveConfig()							{ int r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.r), g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.g), b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Emissive.b); if (GetColor(&r, &g, &b)) { m_MainDlgData.LightingMaterial_Emissive.r = INT0255_TO_FLOAT01(r); m_MainDlgData.LightingMaterial_Emissive.g = INT0255_TO_FLOAT01(g); m_MainDlgData.LightingMaterial_Emissive.b = INT0255_TO_FLOAT01(b); } CRect rcWnd; m_pWnd_LightMat_Emissive->GetWindowRect(&rcWnd); ScreenToClient(&rcWnd); InvalidateRect(&rcWnd, FALSE); RenderStyleDataChanged(); }
	afx_msg void OnLightMaterial_SpecularConfig()							{ int r = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.r), g = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.g), b = FLOAT01_TO_INT0255(m_MainDlgData.LightingMaterial_Specular.b); if (GetColor(&r, &g, &b)) { m_MainDlgData.LightingMaterial_Specular.r = INT0255_TO_FLOAT01(r); m_MainDlgData.LightingMaterial_Specular.g = INT0255_TO_FLOAT01(g); m_MainDlgData.LightingMaterial_Specular.b = INT0255_TO_FLOAT01(b); } CRect rcWnd; m_pWnd_LightMat_Specular->GetWindowRect(&rcWnd); ScreenToClient(&rcWnd); InvalidateRect(&rcWnd, FALSE); RenderStyleDataChanged(); }
	afx_msg void OnPlatformD3D();									
	afx_msg void OnPlatformPS2();
	afx_msg void OnPlatformXBox();
	afx_msg void OnUpdatePlatformD3D(CCmdUI *pCmdUI);									
	afx_msg void OnUpdatePlatformPS2(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePlatformXBox(CCmdUI *pCmdUI);
	afx_msg void OnAppAbout();
	afx_msg void OnHelpGameContentCreationGuide();
	afx_msg void OnHelpWeb();
	afx_msg void OnHelpSamples()											{ MessageBox ("To open the sample Render Styles, in the\nFile menu select \"Open Project...\" and open\n<LTDS>\\engine\\sdk\\rez\\RenderStyles\\Samples.dep\n\nThen, in the \"Render Styles in Project\" area,\nselect a Render Style from the list.", "Viewing Samples", MB_OK | MB_ICONINFORMATION);}
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:	
	afx_msg void OnCbnEditchangeEffectTechnique();
	afx_msg void OnCbnEditupdateEffectTechnique();
	afx_msg void OnCbnCloseupEffectTechnique();
	afx_msg void OnLbnDblclkOutput();
};

// GLOBALS...
extern CAppForm* g_AppFormView;
extern char		 g_StartingDirectory[MAX_PATH];

int AppMessageBox( UINT idString, UINT nType );
int AppMessageBox( const char *pStr, UINT nType );

// EXTERNALS...
bool PreProcessShaderFile(const char* szFileName, string& OutputCode, uint32 iType);

#endif
