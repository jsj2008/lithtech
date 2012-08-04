
// This module implements the initialization and DLL export functions 
// for the Direct3D renderer.

#include "precompile.h"

#include "d3d_convar.h"
#include "d3d_device.h"
#include "common_stuff.h"
#include "d3d_utils.h"
#include "renderstruct.h"
#include "d3d_init.h"
#include "d3d_texture.h"
#include "d3d_draw.h"
#include "tagnodes.h"
#include "drawobjects.h"
#include "d3d_shell.h"
#include "rendererconsolevars.h"

uint32				g_NormalTextureStage;
bool				g_bInOptimized2D = false;

// ---------------------------------------------------------------- //
// Main functions.
// ---------------------------------------------------------------- //
void d3d_ReadExtraConsoleVariables()
{
	if (!PD3DDEVICE) 
	{ 
		return; 
	}

	// Check for some kind of fog support...
	if ((g_Device.GetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_FOGTABLE)  && ((g_Device.GetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_ZFOG) || (g_Device.GetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_WFOG)) || 
		(g_Device.GetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_FOGVERTEX) && ((g_Device.GetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_ZFOG) || (g_Device.GetDeviceCaps()->RasterCaps & D3DPRASTERCAPS_WFOG))) 
	{ 
	}
	else 
	{ 
		g_CV_FogEnable = false;
		g_CV_ScreenGlowFogEnable = false;
	} 

	// This handles a TNT bug if the near and far Z are the same.
	if (g_CV_FogNearZ == g_CV_FogFarZ) 
		g_CV_FogEnable = false;

	if (g_CV_ScreenGlowFogNearZ == g_CV_ScreenGlowFogFarZ)
		g_CV_ScreenGlowFogEnable = false;

	//setup the fog based upon the current console variables
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, D3DRGB_255(g_CV_FogColorR, g_CV_FogColorG, g_CV_FogColorB));
	PD3DDEVICE->SetRenderState(D3DRS_FOGSTART, *((uint32*)&g_CV_FogNearZ.m_Val));
	PD3DDEVICE->SetRenderState(D3DRS_FOGEND, *((uint32*)&g_CV_FogFarZ.m_Val));
	PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, g_CV_FogEnable.m_Val ? TRUE : FALSE);

	//setup dither on the device
	PD3DDEVICE->SetRenderState(D3DRS_DITHERENABLE, (uint32)g_CV_Dither);

	for(uint32 nCurrStage = 0; nCurrStage < 4; nCurrStage++)
	{
		D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MAXANISOTROPY, g_CV_Anisotropic.m_Val ? g_CV_Anisotropic.m_Val : 1));

		// check for device caps anisotropic filter for min filter
		if ((g_Device.GetDeviceCaps()->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) && (g_CV_Anisotropic.m_Val))
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
		}
		else
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MINFILTER, g_CV_Bilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		}

		// check for device cpas anistropic filter for mag filter
		if ((g_Device.GetDeviceCaps()->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) && (g_CV_Anisotropic.m_Val))
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC));
		}
		else 
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MAGFILTER, g_CV_Bilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		}


		D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MIPFILTER, g_CV_Trilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
	}
}

 
bool d3d_PostInitializeDevice(RenderStructInit *pInit,bool bFullInit)
{

	// Setup the viewport.
	if (!pInit) 
		return false;

	g_Device.GetExtraDevCaps()->m_bLightAddPolyCapable		= (g_Device.GetDeviceCaps()->SrcBlendCaps & D3DPBLENDCAPS_ONE)  && (g_Device.GetDeviceCaps()->DestBlendCaps & D3DPBLENDCAPS_ONE);
	
	g_Device.CheckSpecialCards();							// Check for special cards so we can disable behaviour if necessary.

	// Update the gamma values
	d3d_SetGamma(LTVector(g_CV_GammaR.m_Val, g_CV_GammaG.m_Val, g_CV_GammaB.m_Val));

	// Try to initialize the texture manager.
	if (!g_TextureManager.Init(bFullInit)) 
	{ 
		return false; 
	} 

	d3d_InitObjectModules();
	
	return true;
}

// ---------------------------------------------------------------- //
// RenderStruct functions implemented in this module.
// ---------------------------------------------------------------- //
HRENDERCONTEXT d3d_CreateContext()
{
	RenderContext* pContext;
	LT_MEM_TRACK_ALLOC(pContext = (RenderContext*)dalloc_z(sizeof(RenderContext)),LT_MEM_TYPE_RENDERER);
	if (!pContext) return NULL;

	pContext->m_CurFrameCode = 0xFFFF;

	return (HRENDERCONTEXT)pContext;
}

void d3d_DeleteContext(HRENDERCONTEXT hContext)
{
	if (hContext) 
	{
		RenderContext* pContext = (RenderContext*)hContext;
		dfree(hContext); 
	}
}

void d3d_RenderCommand(int argc, char **argv)
{
	if (argc > 0) 
	{
		if (stricmp(argv[0], "LISTDEVICES") == 0) 
		{
			g_D3DShell.ListDevices(); 
		}
		else if (stricmp(argv[0], "LISTTEXTUREFORMATS") == 0) 
		{
			g_TextureManager.ListTextureFormats(); 
		}
		else if (stricmp(argv[0], "LISTDEVICECAPS") == 0) 
		{
			g_Device.ListDeviceCaps(); 
		}
		else if (stricmp(argv[0], "FREETEXTURES") == 0) 
		{
			g_TextureManager.FreeAllTextures(); 
		}
    }
}





