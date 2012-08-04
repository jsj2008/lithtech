
#include "precompile.h"
#include "3d_ops.h"
#include "common_init.h"
#include "tagnodes.h"
#include "renderinfostruct.h"

#include "d3d_shell.h"
#include "d3d_device.h"
#include "d3d_texture.h"
#include "d3d_draw.h"
#include "drawobjects.h"
#include "render.h"
#include "rendershadowlist.h"
#include "objectgroupmgr.h"
#include "screenglowmgr.h"
#include "rendererframestats.h"

// Shadow texture map related includes...
#include "..\shadows\d3dshadowtexture.h"


RMode* g_pModeList;
RMode* rdll_GetSupportedModes();
void rdll_FreeModeList(RMode *pModes);
void rdll_RenderDLLSetup(RenderStruct *pStruct);


// ---------------------------------------------------------------- //
// RenderStruct functions in other modules..
// ---------------------------------------------------------------- //
bool d3d_StartOptimized2D();
void d3d_EndOptimized2D();
bool d3d_SetOptimized2DBlend(LTSurfaceBlend blend);
bool d3d_GetOptimized2DBlend(LTSurfaceBlend &blend);
bool d3d_SetOptimized2DColor(HLTCOLOR color);
bool d3d_GetOptimized2DColor(HLTCOLOR &color);
bool d3d_IsInOptimized2D();
int  d3d_RenderScene(SceneDesc *pScene);
void d3d_SwapBuffers(uint flags);
bool d3d_GetScreenFormat(PFormat *pFormat);
bool d3d_OptimizeSurface(HLTBUFFER hBuffer, uint32 transparentColor);
void d3d_UnoptimizeSurface(HLTBUFFER hBuffer);
bool d3d_PostInitializeDevice(RenderStructInit *pInit,bool bFullInit);
HRENDERCONTEXT d3d_CreateContext();
void d3d_DeleteContext(HRENDERCONTEXT hContext);
void d3d_RenderCommand(int argc, char **argv);
bool d3d_AddGlowRenderStyleMapping(const char*, const char*);
bool d3d_SetGlowDefaultRenderStyle(const char*);
bool d3d_SetNoGlowRenderStyle(const char*);


HLTBUFFER d3d_CreateSurface(int width, int height);
void d3d_DeleteSurface(HLTBUFFER hSurf);
void d3d_GetSurfaceInfo(HLTBUFFER hSurf, uint32 *pWidth, uint32 *pHeight);
void* d3d_LockSurface(HLTBUFFER hSurf, uint32& Pitch);
void d3d_UnlockSurface(HLTBUFFER hSurf);
bool d3d_LockScreen(int left, int top, int right, int bottom, void **pData, long *pPitch);
void d3d_UnlockScreen();
void d3d_BlitToScreen(BlitRequest *pRequest);
void d3d_BlitFromScreen(BlitRequest *pRequest);
bool d3d_WarpToScreen(BlitRequest *pRequest);
void d3d_MakeScreenShot(const char *pFilename);
void d3d_MakeCubicEnvMap(const char* pszPrefix, uint32 nSize, const SceneDesc& InSceneDesc);
LPDIRECT3DDEVICE9 d3d_GetD3DDevice();
bool d3d_SetLightGroupColor(uint32 nID, const LTVector &vColor);
LTRESULT d3d_SetOccluderEnabled(uint32 nID, bool bEnabled);
LTRESULT d3d_GetOccluderEnabled(uint32 nID, bool *pEnabled);
uint32 d3d_GetTextureEffectVarID(const char* pszName, uint32 nStage);
bool   d3d_SetTextureEffectVar(uint32 nVarID, uint32 nVar, float fVar);

static bool d3d_IsNullRenderOn()
{
	return !!g_pStruct->GetParameterValueFloat(g_pStruct->GetParameter("nullrender"));
}

bool g_bInStandby = false;						// The renderer can be put into standby (by calling d3d_Term(false)) - then reinited (d3d_Init(...)) later on... (Mostly for ALT-Tab)
void d3d_Term(bool bFullTerm)						// We don't do a FullTerm on Alt-Tab (will be coming back up, basically - don't really release the device)...
{
	d3d_TermObjectModules();

	g_Device.ReleaseDevObjects(bFullTerm);		// Let the RenderObject release their D3D Data...

	g_TextureManager.Term(bFullTerm);			// Term the TextureManager...

	delete D3DShadowTextureFactory::Get( );		// Shadow texture map related termination stuff...
	CRenderShadowList::GetSingleton().InvalidateShadowTextures(); // make sure that future shadows will be regenerated

	if (bFullTerm) 
	{
		//we need to make sure that the render styles associated with the glow texture are released before
		//they are deleted by D3DShell
		CScreenGlowMgr::GetSingleton().GetRenderStyleMap().FreeList();

		g_Device.FreeAll(); 					// Free the Device...
		g_D3DShell.FreeAll();					// Free the Shell...

		g_bInStandby = false;

	}
	else
	{
		g_bInStandby = g_Device.Standby();
	}


	ShowCursor(true);							// Show the cursor
}

int d3d_Init(RenderStructInit *pInit)
{
	char* pStr; RECT wndRect,screenRect;
	pInit->m_RendererVersion = LTRENDER_VERSION;

#ifdef _DEBUG
	pStr = g_pStruct->GetParameterValueString(g_pStruct->GetParameter("RenderDebugBreak"));
	if (pStr && atoi(pStr) == 1) { DebugBreak(); }
#endif

	d3d_CreateConsoleVariables();					// Read the initial console variables in.
	d3d_ReadConsoleVariables();

	// Run windowed?
	pStr = g_pStruct->GetParameterValueString(g_pStruct->GetParameter("windowed"));
	g_bRunWindowed = (pStr && atoi(pStr) == 1);

	// Init globals.
	g_hWnd				= (HWND)pInit->m_hWnd;
	g_ScreenWidth		= pInit->m_Mode.m_Width;
	g_ScreenHeight		= pInit->m_Mode.m_Height;

	D3DAdapterInfo* pAdapterInfo	 = NULL;
	D3DDeviceInfo*  pDeviceInfo		 = NULL;
	D3DModeInfo*	pModeInfo		 = NULL;

	// If we're in standby mode, try to bring the device back the easy way
	if (g_bInStandby) 
	{
		pAdapterInfo				  = g_Device.GetAdapterInfo();
		pDeviceInfo					  = g_Device.GetDeviceInfo();
		pModeInfo					  = g_D3DShell.PickDefaultMode(pDeviceInfo, pInit->m_Mode.m_Width, pInit->m_Mode.m_Height, 
																				pInit->m_Mode.m_BitDepth, pInit->m_Mode.m_bHWTnL);

		if (!pModeInfo)								
		{
			// Couldn't find any devices that make the cut - try without standby
			// Shut down the rest of the way if we failed...
			d3d_Term(true);
			pDeviceInfo = NULL;
			g_bInStandby = false;
		}
		else 
		{
			// Try to restore the display mode
			if (!g_Device.SetMode(pModeInfo))
			{
				// Shut down the renderer if we failed...
				g_Device.FreeDevice();
				pDeviceInfo = NULL;
				g_bInStandby = false;
			}
			else
				g_Device.SetDefaultRenderStates();		// Set the default render states...
		}
	}		

	if (!g_bInStandby) 
	{
		// Create the D3DShell (it'll enumerate all the hardware devices)...
		if (!g_D3DShell.Create()) 
			return RENDER_ERROR;

		// First try to find your favorite device. If you can't, pick a default one.
		uint32 iAdapterID			  = 0;			// D3D's adapter counter number for this adapter...
		
		if (*pInit->m_Mode.m_InternalName) 
			g_D3DShell.FindDevice(pInit->m_Mode.m_InternalName,&pAdapterInfo);
		
		if (!pDeviceInfo) 
			pDeviceInfo = g_D3DShell.PickDefaultDev(&pAdapterInfo);

		if (!pDeviceInfo) 
		{	
			// Couldn't find any devices that make the cut - fail out
			d3d_Term(true); 
			AddDebugMessage(0, "Can't find any d3d devices to use!"); 
			return RENDER_ERROR; 
		}

		pModeInfo  = g_D3DShell.PickDefaultMode(pDeviceInfo, pInit->m_Mode.m_Width, pInit->m_Mode.m_Height, 
																					pInit->m_Mode.m_BitDepth, pInit->m_Mode.m_bHWTnL);

		if (!pModeInfo) 
		{	
			// Couldn't find any devices that make the cut - fail out
			d3d_Term(true); 
			AddDebugMessage(0, "Can't find an appropriate display mode!"); 
			return RENDER_ERROR; 
		}

		// Create the Device...
		if (!g_Device.CreateDevice(pAdapterInfo,pDeviceInfo,pModeInfo)) 
		{
			d3d_Term(true); 
			AddDebugMessage(0, "Couldn't create D3D Device!"); 
			return RENDER_ERROR; 
		}
	}

	// We're definitely not in standby mode any more
	g_bInStandby = false;

	ShowCursor(false);

	g_Device.RestoreDevObjects();					// Let the render objects restore their D3D data (if there is any already created)...

	// Copy the info for the device we used into the RMode.
	LTStrCpy(pInit->m_Mode.m_InternalName, pDeviceInfo->strDesc, sizeof(pInit->m_Mode.m_InternalName));
	LTStrCpy(pInit->m_Mode.m_Description,  pAdapterInfo->AdapterID.Description, sizeof(pInit->m_Mode.m_Description));

	// Finish initializing...
	AddDebugMessage(0, "Using Direct3D Device %s", pDeviceInfo->strDesc);

	// Either maximize the window or set its size.
	if (g_bRunWindowed || d3d_IsNullRenderOn()) 
	{
		GetWindowRect(GetDesktopWindow(), &screenRect);
		
		// Setup the client rectangle.
		wndRect.left		= ((screenRect.right - screenRect.left) - pInit->m_Mode.m_Width) / 2;
		wndRect.top			= ((screenRect.bottom - screenRect.top) - pInit->m_Mode.m_Height) / 2;
		wndRect.right		= wndRect.left + pInit->m_Mode.m_Width;
		wndRect.bottom		= wndRect.top + pInit->m_Mode.m_Height;

		// Figure out the full window coordinates given the client coordinates.
		AdjustWindowRect(&wndRect, GetWindowLong(g_hWnd,GWL_STYLE), false);

		if (wndRect.left < 0) 
		{
			wndRect.right	-= wndRect.left;
			wndRect.left	 = 0; 
		}
		if (wndRect.top < 0) 
		{
			wndRect.bottom	-= wndRect.top;
			wndRect.top		 = 0; 
		}

		SetWindowPos(g_hWnd, 0, wndRect.left, wndRect.top, wndRect.right-wndRect.left,
			wndRect.bottom-wndRect.top, SWP_NOREPOSITION); 
	}
	else 
	{
		SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE); 
	}

	if (!d3d_PostInitializeDevice(pInit,!g_bInStandby)) 
		return RENDER_ERROR;

	if (!d3d_GetVisibleSet()->Init()) 
	{
		d3d_Term(true);
		AddDebugMessage(0, "VisibleSet::Init failed (invalid object list size?).");
 
		return RENDER_ERROR; 
	} 

	LT_MEM_TRACK_ALLOC(new D3DShadowTextureFactory(),LT_MEM_TYPE_RENDERER); // Shadow texture map related initialization stuff added here

	return RENDER_OK;
}

bool d3d_LoadWorldData(ILTStream *pStream)
{
	return g_Device.LoadWorldData(pStream);
}

void d3d_GetRenderInfo(RenderInfoStruct* pStruct)
{
	if (pStruct) 
	{
		pStruct->m_dwWorldPolysDrawn     = FrameStat(eFS_WorldTriangles);
		pStruct->m_dwWorldPolysProcessed = FrameStat(eFS_WorldTriangles);
		pStruct->m_dwModelPolysDrawn     = FrameStat(eFS_WorldTriangles); 
	}
}

// Note: if you want to use these in a launcher of some sort, you need to also add the 
//	CD3D_Shell class. See the Launcher sample.
RMode* rdll_GetSupportedModes() 
{
	// Make sure we've created our shell...
	if (!PDIRECT3D) 
		g_D3DShell.Create();

	if (!PDIRECT3D) 
		return NULL;

	g_pModeList = NULL;
	g_D3DShell.GetSupportedModes(g_pModeList);
	return g_pModeList;
}

void rdll_FreeModeList(RMode* pModes)
{
	RMode* pCur = pModes;
	while (pCur) 
	{
		RMode* pNext = pCur->m_pNext;
		delete pCur;
		pCur = pNext; 
	}
}

void rdll_RenderDLLSetup(RenderStruct *pStruct)
{
	g_pStruct = pStruct;

	pStruct->Init						= d3d_Init;
	pStruct->Term						= d3d_Term;
	pStruct->GetD3DDevice				= d3d_GetD3DDevice;
	pStruct->BindTexture				= d3d_BindTexture;
	pStruct->UnbindTexture				= d3d_UnbindTexture;
	pStruct->QueryDDSupport				= CTextureManager::QueryDDSupport;
	pStruct->GetTextureDDFormat1		= CTextureManager::QueryDDFormat1;
	pStruct->GetTextureDDFormat2		= CTextureManager::QueryDDFormat2;
	pStruct->ConvertTexDataToDD			= CTextureManager::ConvertTexDataToDD;
	pStruct->DrawPrimSetTexture			= CTextureManager::DrawPrimSetTexture;
	pStruct->DrawPrimDisableTextures	= CTextureManager::DisableTextures;
	pStruct->CreateContext				= d3d_CreateContext;
	pStruct->DeleteContext				= d3d_DeleteContext;
	pStruct->Clear						= d3d_Clear;
	pStruct->Start3D					= CD3D_Device::Start3D;
	pStruct->End3D						= CD3D_Device::End3D;
	pStruct->IsIn3D						= CD3D_Device::IsIn3D;
	pStruct->StartOptimized2D			= d3d_StartOptimized2D;
	pStruct->EndOptimized2D				= d3d_EndOptimized2D;
	pStruct->SetOptimized2DBlend		= d3d_SetOptimized2DBlend;
	pStruct->GetOptimized2DBlend		= d3d_GetOptimized2DBlend;
	pStruct->SetOptimized2DColor		= d3d_SetOptimized2DColor;
	pStruct->GetOptimized2DColor		= d3d_GetOptimized2DColor;
	pStruct->IsInOptimized2D			= d3d_IsInOptimized2D;
	pStruct->OptimizeSurface			= d3d_OptimizeSurface;
	pStruct->UnoptimizeSurface			= d3d_UnoptimizeSurface;
	pStruct->RenderScene				= d3d_RenderScene;
	pStruct->RenderCommand				= d3d_RenderCommand;
	pStruct->SwapBuffers				= d3d_SwapBuffers;
	pStruct->GetScreenFormat			= d3d_GetScreenFormat;
	pStruct->CreateSurface				= d3d_CreateSurface;
	pStruct->DeleteSurface				= d3d_DeleteSurface;
	pStruct->GetSurfaceInfo				= d3d_GetSurfaceInfo;
	pStruct->LockSurface				= d3d_LockSurface;
	pStruct->UnlockSurface				= d3d_UnlockSurface;
	pStruct->LockScreen					= d3d_LockScreen;
	pStruct->UnlockScreen				= d3d_UnlockScreen;
	pStruct->MakeScreenShot				= d3d_MakeScreenShot;
	pStruct->MakeCubicEnvMap			= d3d_MakeCubicEnvMap;
	pStruct->ReadConsoleVariables		= d3d_ReadConsoleVariables;
	pStruct->GetRenderInfo				= d3d_GetRenderInfo;
	pStruct->CreateRenderObject			= CD3D_Device::CreateRenderObject;
	pStruct->DestroyRenderObject		= CD3D_Device::DestroyRenderObject;
	pStruct->BlitToScreen				= d3d_BlitToScreen;
	pStruct->BlitFromScreen				= d3d_BlitFromScreen;
	pStruct->WarpToScreen				= d3d_WarpToScreen;
	pStruct->LoadWorldData				= d3d_LoadWorldData;
	pStruct->SetLightGroupColor			= d3d_SetLightGroupColor;
	pStruct->SetOccluderEnabled			= d3d_SetOccluderEnabled;
	pStruct->GetOccluderEnabled			= d3d_GetOccluderEnabled;
	pStruct->GetTextureEffectVarID		= d3d_GetTextureEffectVarID;
	pStruct->SetTextureEffectVar		= d3d_SetTextureEffectVar;
	pStruct->IsObjectGroupEnabled		= CObjectGroupMgr::IsObjectGroupEnabled;
	pStruct->SetObjectGroupEnabled		= CObjectGroupMgr::SetObjectGroupEnabled;
	pStruct->SetAllObjectGroupEnabled	= CObjectGroupMgr::SetAllObjectGroupEnabled;
	pStruct->AddGlowRenderStyleMapping	= d3d_AddGlowRenderStyleMapping;
	pStruct->SetGlowDefaultRenderStyle	= d3d_SetGlowDefaultRenderStyle;
	pStruct->SetNoGlowRenderStyle		= d3d_SetNoGlowRenderStyle;

	//initialize our object group list
	CObjectGroupMgr::InitObjectGroups();
}

