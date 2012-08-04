#include "precompile.h"
#include "d3d_device.h"
#include "d3d_shell.h"
#include "renderstruct.h"
#include "screenglowmgr.h"
#include "common_draw.h"
#include "d3d_renderworld.h"
#include "d3d_texture.h"
#include "rendererconsolevars.h"
#include "tagnodes.h"
#include "setupmodel.h"
#include "drawobjects.h"
#include "ltpixelshadermgr.h"


//Interface for the client file manager
#include "client_filemgr.h"
static IClientFileMgr* g_pIClientFileMgr;
define_holder(IClientFileMgr, g_pIClientFileMgr);



#define MIN_GLOW_TEXTURE_SIZE 16
#define MAX_GLOW_TEXTURE_SIZE 512

//------------------------
// Local console variables
static ConVar<int> g_CV_ScreenGlow_DrawCanvases("ScreenGlow_DrawCanvases", 1);

extern void d3d_DrawSolidCanvases(const ViewParams& Params);
extern void d3d_DrawGlowModels(const ViewParams& Params);
extern void d3d_DrawSolidWorldModels(const ViewParams& Params);
extern void d3d_QueueTranslucentGlowModels(const ViewParams& Params, ObjectDrawList& DrawList);
extern void d3d_QueueTranslucentWorldModels(const ViewParams& Params, ObjectDrawList& DrawList);

//--------------------------------------------------------------------------------------------------
// Utilities
//

//given a number, this will truncate to a power of two
static uint32 TruncateToPowerOf2(uint32 nVal)
{
	//round it down to the nearest power of 2
	uint32 nNearestPower = 0x80000000;
	while(nNearestPower > nVal)
		nNearestPower >>= 1;

	return nNearestPower;
}

//given a texture and the number of channels to install to, this will set that texture on channels
//0..N
static void InstallTexture(LPDIRECT3DTEXTURE9 pTexture, uint32 nNumChannels)
{
	for(uint32 nCurrChannel = 0; nCurrChannel < nNumChannels; nCurrChannel++)
		d3d_SetTextureDirect(pTexture, nCurrChannel);
}

//installs the specified texture as a render target as well as an optional depth buffer
static bool SetRenderTarget(LPDIRECT3DTEXTURE9 pTarget, LPDIRECT3DSURFACE9 pDepthBuffer)
{
	//get the first surface from the texture
	LPDIRECT3DSURFACE9 pRenderSurface = NULL;
	pTarget->GetSurfaceLevel(0, &pRenderSurface);

	//now try and install that
	HRESULT hr = PD3DDEVICE->SetRenderTarget(pRenderSurface, pDepthBuffer);

	//make sure we release our references
	if(pRenderSurface)
	{
		pRenderSurface->Release();
		pRenderSurface = NULL;
	}

	//now return the result
	return SUCCEEDED(hr);
}

//sets up the vertices of a rectangle to the specified dimensions and UV's
static void SetupRectVerts(float fLeft, float fTop, float fRight, float fBottom, float fU0, float fV0, float fU1, float fV1, uint32 nColor, TLVertex Verts[4])
{
	Verts[0].m_Vec.Init(fLeft, fTop);
	Verts[0].tu		= fU0;
	Verts[0].tv		= fV0;
	Verts[0].color	= nColor;
	Verts[0].rhw	= 1.0f;

	Verts[1].m_Vec.Init(fRight, fTop);
	Verts[1].tu		= fU1;
	Verts[1].tv		= fV0;
	Verts[1].color	= nColor;
	Verts[1].rhw	= 1.0f;

	Verts[2].m_Vec.Init(fRight, fBottom);
	Verts[2].tu		= fU1;
	Verts[2].tv		= fV1;
	Verts[2].color	= nColor;
	Verts[2].rhw	= 1.0f;

	Verts[3].m_Vec.Init(fLeft, fBottom);
	Verts[3].tu		= fU0;
	Verts[3].tv		= fV1;
	Verts[3].color	= nColor;
	Verts[3].rhw	= 1.0f;
}

//----------------------------------------------------------------------------
// Glow4Texture Vertex format
//   Uses: No HW TnL, 4 texture channels

#define GLOW4TEXTUREVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_TEX4)

class CGlow4TextureVertex
{
public:

	struct SUVSet
	{
		float	m_fU, m_fV;
	};

	//position
	LTVector	m_Vec;
	float		m_RHW;

	SUVSet		m_UVs[4];
};

//----------------------------------------------------------------------------
// Glow2Texture Vertex format
//   Uses: No HW TnL, 2 texture channels, color

#define GLOW2TEXTUREVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_DIFFUSE)

class CGlow2TextureVertex
{
public:

	struct SUVSet
	{
		float	m_fU, m_fV;
	};

	//position
	LTVector	m_Vec;
	float		m_RHW;

	uint32		m_nColor;

	SUVSet		m_UVs[2];
};

//--------------------------------------------------------------------------------------------------
// RenderStruct interface
//

bool d3d_AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo)
{
	return CScreenGlowMgr::GetSingleton().GetRenderStyleMap().AddRenderStyle(pszSource, pszMapTo);
}

bool d3d_SetGlowDefaultRenderStyle(const char* pszFilename)
{
	return CScreenGlowMgr::GetSingleton().GetRenderStyleMap().SetDefaultRenderStyle(pszFilename);
}

bool d3d_SetNoGlowRenderStyle(const char* pszFilename)
{
	return CScreenGlowMgr::GetSingleton().GetRenderStyleMap().SetNoGlowRenderStyle(pszFilename);
}


//--------------------------------------------------------------------------------------------------
// CScreenGlowMgr
//

CScreenGlowMgr::CScreenGlowMgr() :
	m_nFilterSize(0),
	m_pGlowTexture(NULL),
	m_pDepthBuffer(NULL),
	m_pBlendTexture(NULL),
	m_nTextureWidth(0),
	m_nTextureHeight(0),
	m_bFailedLoadingShader(false)
{
}

//singleton access
CScreenGlowMgr& CScreenGlowMgr::GetSingleton()
{
	static CScreenGlowMgr s_Singleton;
	return s_Singleton;
}

//determines if the device supports the glow shader or not
bool CScreenGlowMgr::SupportsGlowShader()
{
	//make sure that the shader is valid
	if(!UpdateShader())
		return false;

	return true;
}

//renders a screen glow over the screen with the specified texture dimensions
bool CScreenGlowMgr::RenderScreenGlow(uint32 nGlowTexWidth, uint32 nGlowTexHeight, const SceneDesc& Scene)
{
	//handle the shader
	if(!UpdateShader())
		return false;

	//make sure the texture is updated
	if(!UpdateGlowTexture(nGlowTexWidth, nGlowTexHeight))
		return false;

	//now render the scene to the texture
	if(!RenderSceneToTexture(Scene))
		return false;

	//and finally apply the texture
	if(!DisplayBlendedTexture())
		return false;

	//draw the debug texture if necessary
	if(g_CV_ScreenGlowShowTexture.m_Val)
		DrawDebugTexture();

	//draw the gaussian filter if desired
	if(g_CV_ScreenGlowShowFilter.m_Val)
		DrawDebugFilter();

	return true;
}

//called to release all textures associated with this
void CScreenGlowMgr::FreeTextures()
{
	//free our textures
	if(m_pGlowTexture)
	{
		m_pGlowTexture->Release();
		m_pGlowTexture = NULL;
	}
	if(m_pBlendTexture)
	{
		m_pBlendTexture->Release();
		m_pBlendTexture = NULL;
	}
	if(m_pDepthBuffer)
	{
		m_pDepthBuffer->Release();
		m_pDepthBuffer = NULL;
	}
}

//called when the device goes away to free device objects
void CScreenGlowMgr::FreeDeviceObjects()
{
	FreeTextures();
}

//handles loading the shader if necessary or failing if the device doesn't support it
bool CScreenGlowMgr::UpdateShader()
{
	// See if we already tried and failed.
	if (m_bFailedLoadingShader)
	{
		return true;
	}

	// See if our shader is valid.
	LTPixelShader *pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SCREENGLOW);
	if (NULL == pPixelShader)
	{
		FileRef ref;
		ref.m_FileType 	= FILE_ANYFILE;
		ref.m_pFilename = "ps\\screenglow.psh";

		// Try to load it.
		ILTStream *pStream = g_pIClientFileMgr->OpenFile(&ref);
		if (NULL != pStream)
		{
			if (LTPixelShaderMgr::GetSingleton().AddPixelShader(pStream, ref.m_pFilename,
																LTPixelShader::PIXELSHADER_SCREENGLOW, true))
			{
				pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SCREENGLOW);
			}

			// Close the file.
			pStream->Release();
		}
	}

	// Check the shader.
	if (NULL != pPixelShader && pPixelShader->IsValidShader())
	{
		return true;
	}

	// The shader is no good.
	m_bFailedLoadingShader = true;
	return false;
}


//this will make sure that the glow texture is allocated and meets the specified size
bool CScreenGlowMgr::UpdateGlowTexture(uint32 nWidth, uint32 nHeight)
{
	//get the dimensions of the texture
	nWidth = TruncateToPowerOf2(nWidth);
	nHeight = TruncateToPowerOf2(nHeight);

	//look for invalid textures
	if((nWidth == 0) || (nHeight == 0))
		return false;

	//clamp these
	nWidth	= LTCLAMP(nWidth, MIN_GLOW_TEXTURE_SIZE, MAX_GLOW_TEXTURE_SIZE);
	nHeight = LTCLAMP(nHeight, MIN_GLOW_TEXTURE_SIZE, MAX_GLOW_TEXTURE_SIZE);

	//now see if these differ from our actual texture dimensions or if we don't have a texture
	if(!m_pGlowTexture || !m_pDepthBuffer || (nWidth != m_nTextureWidth) || (nHeight != m_nTextureHeight))
	{
		//we need to regenerate our texture

		//out with the old
		FreeTextures();

		//determine which format we should use
		D3DFORMAT iTargetFormat = g_Device.GetModeInfo()->Format;
		// for DX9 we need to switch this to render target?
		if(FAILED(PD3DDEVICE->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_RENDERTARGET, iTargetFormat, D3DPOOL_DEFAULT, &m_pGlowTexture)))
		{
			dsi_ConsolePrint("ScreenGlow: Error creating %dx%d glow texture", nWidth, nHeight);
			FreeTextures();
			return false;
		}

		if(FAILED(PD3DDEVICE->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_RENDERTARGET, iTargetFormat, D3DPOOL_DEFAULT, &m_pBlendTexture)))
		{
			dsi_ConsolePrint("ScreenGlow: Error creating %dx%d blend texture", nWidth, nHeight);
			FreeTextures();
			return false;
		}

		//now for the Z buffer
		D3DFORMAT iDepthFormat = g_Device.GetDefaultDepthStencilFormat(g_CV_ZBitDepth,g_CV_StencilBitDepth);
		if(FAILED(PD3DDEVICE->CreateDepthStencilSurface(nWidth, nHeight, iDepthFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pDepthBuffer, NULL)))
		{
			dsi_ConsolePrint("ScreenGlow: Error creating %dx%d depth buffer", nWidth, nHeight);
			FreeTextures();
			return false;
		}

		//texture successfully allocated, update our dimensions
		m_nTextureWidth  = nWidth;
		m_nTextureHeight = nHeight;

		dsi_ConsolePrint("ScreenGlow: Recreated glow textures with resolution %dx%d", nWidth, nHeight);
	}

	//success
	return true;

}

//handles rendering the scene to the texture
bool CScreenGlowMgr::RenderSceneToTexture(const SceneDesc& Scene)
{
	//make sure our texture is valid
	assert(m_pGlowTexture);

	//it is, so now we need to install it as the render target

	//save our old targets so we can restore them
	LPDIRECT3DSURFACE9 pDepthStencilBuffer	= NULL;
	LPDIRECT3DSURFACE9 pPrevRenderTarget	= NULL;

	PD3DDEVICE->GetDepthStencilSurface(&pDepthStencilBuffer);
	PD3DDEVICE->GetRenderTarget(&pPrevRenderTarget);

	D3DVIEWPORT9 oldViewportData;
	PD3DDEVICE->GetViewport(&oldViewportData);

	//install our target
	if (!SetRenderTarget(m_pGlowTexture, m_pDepthBuffer))
	{
		//we can't proceed
		if(pDepthStencilBuffer)
			pDepthStencilBuffer->Release();
		if(pPrevRenderTarget)
			pPrevRenderTarget->Release();

		return false;
	}

	D3DVIEWPORT9 viewportData;
	viewportData.X		= 0;
	viewportData.Y		= 0;
	viewportData.Width	= m_nTextureWidth;
	viewportData.Height = m_nTextureHeight;
	viewportData.MinZ	= 0.1f;
	viewportData.MaxZ	= 1.0f;
	HRESULT hResult = PD3DDEVICE->SetViewport(&viewportData);

	//clear out the render target to black
	if(FAILED(PD3DDEVICE->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DRGBA_255(0, 0, 0, 0), 1.0f, 0)))
		PD3DDEVICE->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DRGBA_255(0, 0, 0, 0), 1.0f, 0);

	viewportData.X		= 1;
	viewportData.Y		= 1;
	viewportData.Width	= m_nTextureWidth - 2;
	viewportData.Height = m_nTextureHeight - 2;
	hResult = PD3DDEVICE->SetViewport(&viewportData);


	//setup the fog for the screen glow, always assuming a black fog
	StateSet ssFogEnable(D3DRS_FOGENABLE, g_CV_ScreenGlowFogEnable ? TRUE : FALSE);
	StateSet ssFogNearZ(D3DRS_FOGSTART, *((uint32*)&g_CV_ScreenGlowFogNearZ.m_Val));
	StateSet ssFogFarZ(D3DRS_FOGEND, *((uint32*)&g_CV_ScreenGlowFogFarZ.m_Val));
	StateSet ssFogColor(D3DRS_FOGCOLOR, D3DRGB_255(0, 0, 0));

	// get the far clip distance for rendering glow geometry
	float newFarZ = g_CV_ScreenGlowFogEnable ? g_CV_ScreenGlowFogFarZ : g_CV_FarZ;
	if( newFarZ > g_CV_FarZ )
		newFarZ = g_CV_FarZ;

	//alright, now we have our scene description, setup the frame and render away!
	ViewParams NewViewParams;
	d3d_InitFrustum(&NewViewParams, Scene.m_xFov, Scene.m_yFov, NEARZ, newFarZ,
					0, 0, (float)m_nTextureWidth, (float)m_nTextureHeight,
					&Scene.m_Pos, &Scene.m_Rotation, ViewParams::eRenderMode_Glow);
	d3d_SetD3DTransformStates(NewViewParams);

	//don't render the skybox during the glow pass
	int32 nNumOldSkyObjects = g_pSceneDesc->m_nSkyObjects;
	g_pSceneDesc->m_nSkyObjects = 0;

	//first draw the main world
	if (g_Device.m_pRenderWorld && g_CV_DrawWorld && g_have_world)
	{
		g_Device.m_pRenderWorld->StartFrame(NewViewParams);
		g_Device.m_pRenderWorld->Draw(NewViewParams, false);
	}

	//now we need to draw all the objects. This includes models and world models
	d3d_DrawSolidWorldModels(NewViewParams);
	d3d_DrawGlowModels(NewViewParams);

	if(g_CV_ScreenGlow_DrawCanvases.m_Val)
		d3d_DrawSolidCanvases(NewViewParams);


	//now queue up all the translucent objects
	static ObjectDrawList s_TransDrawList;
	d3d_QueueTranslucentWorldModels(NewViewParams, s_TransDrawList);
	d3d_QueueTranslucentGlowModels(NewViewParams, s_TransDrawList);

	//alright, setup the translucent states and render the list
	d3d_SetTranslucentObjectStates();
		s_TransDrawList.Draw(NewViewParams);
	d3d_UnsetTranslucentObjectStates(true);

	//reset the number of skybox objects
	g_pSceneDesc->m_nSkyObjects = nNumOldSkyObjects;

	//handle the blurring of the texture (we cache the result to ensure that the render target
	//will be properly restored)
	bool bBlendedOK = RenderBlendedTexture(g_CV_ScreenGlowEnablePS && !m_bFailedLoadingShader);

	//restore our render target now
	PD3DDEVICE->SetRenderTarget(pPrevRenderTarget, pDepthStencilBuffer);

	//release our references to them
	pPrevRenderTarget->Release();
	pDepthStencilBuffer->Release();

	//restore our viewport
	PD3DDEVICE->SetViewport(&oldViewportData);

	//success
	return bBlendedOK;
}

//renders the blended texture blended along a specific axis controlled by the weights assigned
//to each direction using a pixel shader
bool CScreenGlowMgr::RenderBlendedTextureDirectionPS(float fUVScale, float fUWeight, float fVWeight)
{
	//we don't want blending on the first pass, this saves us a clear
	StateSet ss0(D3DRS_ALPHABLENDENABLE, FALSE);
	bool bShouldEnableAlpha = true;

	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static CGlow4TextureVertex Verts[4];

	//get the dimensions of the texture
	float fWidth	= (float)m_nTextureWidth;
	float fHeight	= (float)m_nTextureHeight;

	//setup the positions of the four vertices
	Verts[0].m_Vec.Init(0.0f, 0.0f);
	Verts[0].m_RHW		= 1.0f;

	Verts[1].m_Vec.Init(fWidth, 0.0f);
	Verts[1].m_RHW		= 1.0f;

	Verts[2].m_Vec.Init(fWidth, fHeight);
	Verts[2].m_RHW		= 1.0f;

	Verts[3].m_Vec.Init(0.0f, fHeight);
	Verts[3].m_RHW		= 1.0f;

	// Get the shader.
	LTPixelShader *pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SCREENGLOW);
	if (NULL == pPixelShader)
	{
		return false;
	}

	// Get the constants.
	float *pConstants = pPixelShader->GetConstants();
	float *pCurrConstant = pConstants;

	//pixel shader constants
	for(uint32 nCurrElement = 0; nCurrElement < m_nFilterSize; nCurrElement += 4)
	{
		//first we need to generate our UV sets and render it to the texture
		for(uint32 nCurrStage = 0; nCurrStage < 4; nCurrStage++)
		{
			const SGaussianElement& Element = m_GaussianFilter[nCurrElement + nCurrStage];

			//find out what our pixel offset is for this stage
			float fUOffset = fUVScale * Element.m_fTexOffset * fUWeight / fWidth;
			float fVOffset = fUVScale * Element.m_fTexOffset * fVWeight / fHeight;

			//setup all the UVs to be offset by that amount
			Verts[0].m_UVs[nCurrStage].m_fU = 0.0f + fUOffset;
			Verts[0].m_UVs[nCurrStage].m_fV = 0.0f + fVOffset;
			Verts[1].m_UVs[nCurrStage].m_fU = 1.0f + fUOffset;
			Verts[1].m_UVs[nCurrStage].m_fV = 0.0f + fVOffset;
			Verts[2].m_UVs[nCurrStage].m_fU = 1.0f + fUOffset;
			Verts[2].m_UVs[nCurrStage].m_fV = 1.0f + fVOffset;
			Verts[3].m_UVs[nCurrStage].m_fU = 0.0f + fUOffset;
			Verts[3].m_UVs[nCurrStage].m_fV = 1.0f + fVOffset;

			//setup our pixel shader constants so it can weight the pixels by the right amount
			float fPixelWeight = Element.m_fWeight;
			*pCurrConstant++ = fPixelWeight;
			*pCurrConstant++ = fPixelWeight;
			*pCurrConstant++ = fPixelWeight;
			*pCurrConstant++ = fPixelWeight;
		}

		// Set the constants.
		LTPixelShaderMgr::GetSingleton().SetPixelShaderConstants(pPixelShader);

		//now render the quad
		PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(CGlow4TextureVertex));

		//see if this was the first pass and we now need to enable alpha
		if(bShouldEnableAlpha)
		{
			PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			bShouldEnableAlpha = false;
		}
	}

	return true;
}

//this will calculate the two integer versions of the weights, and will return how much was
//lost due to truncation
float CalcWeightValues(float fWeight1, float fWeight2, uint32& nOutWeight1, uint32& nOutWeight2)
{
	//now create our integer forms
	nOutWeight1 = (uint32)LTMAX(0.0f, fWeight1 * 255.0f + 0.5f);

	//see how much we will lose and needs to be given to the next weight, and apply that
	fWeight2 += ((fWeight1 * 255.0f) - (float)nOutWeight1) / 255.0f;

	nOutWeight2 = (uint32)LTMAX(0.0f, fWeight2 * 255.0f + 0.5f);

	//and determine the remainder
	return ((fWeight2 * 255.0f) - (float)nOutWeight2) / 255.0f;
}

//renders the blended texture blended along a specific axis controlled by the weights assigned
//to each direction using a 2 texture approach for older hardware
bool CScreenGlowMgr::RenderBlendedTextureDirection2Texture(float fUVScale, float fUWeight, float fVWeight)
{
	//we don't want blending on the first pass, this saves us a clear
	StateSet ss0(D3DRS_ALPHABLENDENABLE, FALSE);
	bool bShouldEnableAlpha = true;

	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static CGlow2TextureVertex Verts[4];

	//get the dimensions of the texture
	float fWidth	= (float)m_nTextureWidth;
	float fHeight	= (float)m_nTextureHeight;

	//setup the positions of the four vertices
	Verts[0].m_Vec.Init(0.0f, 0.0f);
	Verts[0].m_RHW		= 1.0f;

	Verts[1].m_Vec.Init(fWidth, 0.0f);
	Verts[1].m_RHW		= 1.0f;

	Verts[2].m_Vec.Init(fWidth, fHeight);
	Verts[2].m_RHW		= 1.0f;

	Verts[3].m_Vec.Init(0.0f, fHeight);
	Verts[3].m_RHW		= 1.0f;

	//the amount of glow that has been lost due to numerical accuracy on the last weight. This
	//should be applied to the next weight.
	float fLostWeightToApply = 0.0f;

	for(uint32 nCurrElement = 0; nCurrElement < m_nFilterSize; nCurrElement += 2)
	{
		//first we need to generate our UV sets and render it to the texture
		for(uint32 nCurrStage = 0; nCurrStage < 2; nCurrStage++)
		{
			const SGaussianElement& Element = m_GaussianFilter[nCurrElement + nCurrStage];

			//find out what our pixel offset is for this stage
			float fUOffset = fUVScale * Element.m_fTexOffset * fUWeight / fWidth;
			float fVOffset = fUVScale * Element.m_fTexOffset * fVWeight / fHeight;

			//setup all the UVs to be offset by that amount
			Verts[0].m_UVs[nCurrStage].m_fU = 0.0f + fUOffset;
			Verts[0].m_UVs[nCurrStage].m_fV = 0.0f + fVOffset;
			Verts[1].m_UVs[nCurrStage].m_fU = 1.0f + fUOffset;
			Verts[1].m_UVs[nCurrStage].m_fV = 0.0f + fVOffset;
			Verts[2].m_UVs[nCurrStage].m_fU = 1.0f + fUOffset;
			Verts[2].m_UVs[nCurrStage].m_fV = 1.0f + fVOffset;
			Verts[3].m_UVs[nCurrStage].m_fU = 0.0f + fUOffset;
			Verts[3].m_UVs[nCurrStage].m_fV = 1.0f + fVOffset;
		}

		//we now need to setup the vertex color so that RGB can be used to get the weight of the
		//first texture, and A for the weight of the second texture
		uint32 nWeight1, nWeight2;
		fLostWeightToApply = CalcWeightValues(	m_GaussianFilter[nCurrElement].m_fWeight + fLostWeightToApply,
												m_GaussianFilter[nCurrElement + 1].m_fWeight,
												nWeight1, nWeight2);

		uint32 nColor = D3DRGBA_255(nWeight1, nWeight1, nWeight1, nWeight2);

		for(uint32 nCurrVert = 0; nCurrVert < 4; nCurrVert++)
		{
			Verts[nCurrVert].m_nColor = nColor;
		}

		//now render the quad
		PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(CGlow2TextureVertex));

		//see if this was the first pass and we now need to enable alpha
		if(bShouldEnableAlpha)
		{
			PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			bShouldEnableAlpha = false;
		}
	}

	return true;
}

//renders the glow texture to the blended texture
bool CScreenGlowMgr::RenderBlendedTexture(bool bUsePixelShader)
{
	//update our filter
	if(!UpdateGaussianFilter())
		return false;

	//set the blend mode so that it is opaque on the first pass (avoids the need for a clear) and then
	//set it up so when it is enabled it will be an additive blend
	StateSet ss1(D3DRS_SRCBLEND, D3DBLEND_ONE);
	StateSet ss2(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//we can also turn off Z operations
	StateSet ss3(D3DRS_ZENABLE, FALSE);
	StateSet ss4(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ss5(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	StateSet ss6(D3DRS_ALPHATESTENABLE, FALSE);

	//setup texture clamping
	SamplerStateSet ssWrapU0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SamplerStateSet ssWrapV0(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	SamplerStateSet ssWrapU1(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SamplerStateSet ssWrapV1(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	//amount to adjust UVs by to group them together to avoid blocking artifacts
	float fUVScale		= g_CV_ScreenGlowUVScale;

	if(bUsePixelShader)
	{
		//enable clamping on the remaining stages
		SamplerStateSet ssWrapU2(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		SamplerStateSet ssWrapV2(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		SamplerStateSet ssWrapU3(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		SamplerStateSet ssWrapV3(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


		// Get the shader.
		LTPixelShader *pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_SCREENGLOW);
		if (NULL == pPixelShader)
		{
			return false;
		}

		// Install the pixel shader.
		LTPixelShaderMgr::GetSingleton().InstallPixelShader(pPixelShader);

		//setup our device
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(GLOW4TEXTUREVERTEX_FORMAT);

		//put our texture into each channel
		InstallTexture(m_pGlowTexture, 4);

		//first render it blended in the U direction
		if(!SetRenderTarget(m_pBlendTexture, NULL))
			return false;

		RenderBlendedTextureDirectionPS(fUVScale, 1.0f, 0.0f);

		//now render it blended in the V direction
		InstallTexture(NULL, 4);

		if(!SetRenderTarget(m_pGlowTexture, NULL))
			return false;

		InstallTexture(m_pBlendTexture, 4);

		RenderBlendedTextureDirectionPS(fUVScale, 0.0f, 1.0f);

		InstallTexture(NULL, 4);

		// Uninstall the pixel shader.
		LTPixelShaderMgr::GetSingleton().UninstallPixelShader();
	}
	else
	{
		//use the 2 texture pipeline in order to support older hardware

		//setup our device
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(GLOW2TEXTUREVERTEX_FORMAT);

		//setup the texture stages so we can store our two weights in the RGB for the
		//first, and then A for the second
		StageStateSet ss00(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		StageStateSet ss01(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		StageStateSet ss02(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		StageStateSet ss03(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		StageStateSet ss04(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		StageStateSet ss10(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		StageStateSet ss11(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		StageStateSet ss12(1, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR );

		StageStateSet ss13(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		StageStateSet ss14(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		StageStateSet ss20(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet ss21(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		//put our texture into each channel
		InstallTexture(m_pGlowTexture, 2);

		//first render it blended in the U direction
		if(!SetRenderTarget(m_pBlendTexture, NULL))
			return false;

		RenderBlendedTextureDirection2Texture(fUVScale, 1.0f, 0.0f);

		//now render it blended in the V direction
		InstallTexture(NULL, 2);

		if(!SetRenderTarget(m_pGlowTexture, NULL))
			return false;

		InstallTexture(m_pBlendTexture, 2);

		RenderBlendedTextureDirection2Texture(fUVScale, 0.0f, 1.0f);

		InstallTexture(NULL, 2);
	}

	return true;
}

//handles rendering the blurred texture to the screen
bool CScreenGlowMgr::DisplayBlendedTexture()
{
	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static TLVertex Verts[4];

	float fX1	= (float)g_ViewParams.m_Rect.left;
	float fY1	= (float)g_ViewParams.m_Rect.top;
	float fX2	= (float)g_ViewParams.m_Rect.right;
	float fY2	= (float)g_ViewParams.m_Rect.bottom;

	//offset the UL by one texel in order to compensate for the rendering bias towards right pixels
	//and the UL .5 texel which we don't want
	float fU0		= 1.0f / m_nTextureWidth;
	float fU1		= 1.0f;
	float fV0		= 1.0f / m_nTextureHeight;
	float fV1		= 1.0f;

	// Set up the vertices
	SetupRectVerts(fX1, fY1, fX2, fY2, fU0, fV0, fU1, fV1, 0xFFFFFFFF, Verts);

	d3d_SetTextureDirect(m_pGlowTexture, 0);

	// Set all the states the way we want. (No Z, no fog, alpha blend, blend dest+src)
	StateSet ssFogEnable(D3DRS_FOGENABLE, FALSE);
	StateSet ssZEnable(D3DRS_ZENABLE, FALSE);
	StateSet ssZWriteEnable(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssZCmp(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	StateSet ssAlphaTestEnable(D3DRS_ALPHATESTENABLE, FALSE);

	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_ONE);
	StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_ONE);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet tss02(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss03(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet tss04(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss05(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(TLVertex)));

	d3d_SetTextureDirect(NULL, 0);

	return true;
}

//renders the debug texture for debugging purposes
bool CScreenGlowMgr::DrawDebugTexture()
{
	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static TLVertex Verts[4];

	float fSizeScale = LTCLAMP(g_CV_ScreenGlowShowTextureScale.m_Val, 0.01f, 1.0f);

	float fWidth	= g_ViewParams.m_fScreenWidth * fSizeScale;
	float fHeight	= g_ViewParams.m_fScreenHeight * fSizeScale;

	// Set up the vertices
	SetupRectVerts(0, 0, fWidth, fHeight, 0, 0, 1.0f, 1.0f, 0xFFFFFFFF, Verts);

	d3d_SetTextureDirect(m_pGlowTexture, 0);

	// Set all the states the way we want. (No Z, no fog, no blend)
	StateSet ssFogEnable(D3DRS_FOGENABLE, FALSE);
	StateSet ssZEnable(D3DRS_ZENABLE, FALSE);
	StateSet ssZWriteEnable(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssAlphaTestEnable(D3DRS_ALPHATESTENABLE, FALSE);
	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, FALSE);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet tss02(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss03(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet tss04(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss05(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(TLVertex)));

	d3d_SetTextureDirect(NULL, 0);

	return true;
}

//updates the gaussian element list
bool CScreenGlowMgr::UpdateGaussianFilter()
{
	//the minimum weight an element must have to contribute
	static const float kfMinElementWeight = 0.01f;

	//the gaussian information
	static const uint32	knNumGaussians	= 2;

	uint32	nFilterSize		= LTCLAMP(g_CV_ScreenGlowFilterSize.m_Val, 0, MAX_FILTER_SIZE);
	float	fPixelShift		= g_CV_ScreenGlowPixelShift.m_Val;

	float	fGausseAmplitude[knNumGaussians];
	float	fGausseRadius[knNumGaussians];

	fGausseRadius[0]		= g_CV_ScreenGlowGaussRadius0;
	fGausseAmplitude[0]		= g_CV_ScreenGlowGaussAmp0;

	fGausseRadius[1]		= g_CV_ScreenGlowGaussRadius1;
	fGausseAmplitude[1]		= g_CV_ScreenGlowGaussAmp1;


	//precalculate some data

	//the center element of the filter
	float fFilterCenter = (((float)nFilterSize) - 1.0f ) / 2.0f;

	//to avoid a divide and multiply per element
	float fInvFilterCenterSqr = 1.0f / (fFilterCenter * fFilterCenter);

	//reset our filter
	SGaussianElement* pCurrElement = m_GaussianFilter;
	m_nFilterSize = 0;

	//variables used in the loop
	float	fRadius;
	uint32	nCurrGaussian;

	for(uint32 nCurrElem = 0; nCurrElem < nFilterSize; nCurrElem++ )
	{
		//find the pixel offset
		pCurrElement->m_fTexOffset = ((float)nCurrElem) - fFilterCenter + fPixelShift;

		fRadius = pCurrElement->m_fTexOffset * pCurrElement->m_fTexOffset * fInvFilterCenterSqr;

		//reset the weight
		pCurrElement->m_fWeight = 0.0f;

		//now add up the different gaussians to get our final weight
		for( nCurrGaussian = 0; nCurrGaussian < knNumGaussians; nCurrGaussian++ )
		{
			pCurrElement->m_fWeight += fGausseAmplitude[nCurrGaussian] / expf(fRadius * fGausseRadius[nCurrGaussian]);
		}

		//ignore them if they are below a certain weight
		if(pCurrElement->m_fWeight > kfMinElementWeight)
		{
			pCurrElement++;
			m_nFilterSize++;
		}
	}

	//fill it out to a full pass of elements
	while(m_nFilterSize % MAX_TEXTURES_PER_PASS)
	{
		pCurrElement->m_fTexOffset	= 0.0f;
		pCurrElement->m_fWeight		= 0.0f;

		pCurrElement++;
		m_nFilterSize++;
	}

	return true;
}

//renders a graph of the filter being used
bool CScreenGlowMgr::DrawDebugFilter()
{
	//make sure we have a valid filter first
	if(m_nFilterSize <= 0)
		return false;

	// Note : This has to be static so that broken drivers can DMA the vertices after the DrawPrimitive call
	static TLVertex Verts[4];

	float fSizeScale = LTCLAMP(g_CV_ScreenGlowShowFilterScale.m_Val, 0.01f, 1.0f);

	float fScreenWidth	= g_ViewParams.m_fScreenWidth;
	float fScreenHeight	= g_ViewParams.m_fScreenHeight;

	//we need to determine our actual rectangle
	float fLeft		= 0.0f;
	float fTop		= 0.0f;
	float fRight	= fScreenWidth * fSizeScale;
	float fBottom	= fScreenHeight * fSizeScale;

	//see if they have the glow texture up, in which case we need to shift this
	if(g_CV_ScreenGlowShowTexture.m_Val)
	{
		//they do, shift us
		float fTexWidth = LTCLAMP(g_CV_ScreenGlowShowTextureScale.m_Val, 0.01f, 1.0f) * fScreenWidth;
		fLeft	= LTMIN(fLeft + fTexWidth, fScreenWidth);
		fRight	= LTMIN(fRight + fTexWidth, fScreenWidth);

		//see if we got pushed off
		if(fLeft >= fRight)
			return false;
	}

	//setup our device to just do screen aligned untextured rendering
	d3d_DisableTexture(0);

	// Set all the states the way we want. (No Z, no fog, no blend)
	StateSet ssFogEnable(D3DRS_FOGENABLE, FALSE);
	StateSet ssZEnable(D3DRS_ZENABLE, FALSE);
	StateSet ssZWriteEnable(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ssAlphaTestEnable(D3DRS_ALPHATESTENABLE, FALSE);
	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, FALSE);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	StageStateSet tss01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet tss02(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss03(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet tss04(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss05(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(TLVERTEX_FORMAT);

	//draw our background rectangle
	SetupRectVerts(fLeft, fTop, fRight, fBottom, 0, 0, 0, 0, 0x22222222, Verts);
	D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(TLVertex)));

	//figure out the X dimensions of each element

	//the spacing between each element
	static const float kfElementGutter = 1.0f;

	uint32 nNumGutters = m_nFilterSize - 1;

	//make sure we have at least one pixel for each element
	if((fRight - fLeft) < (m_nFilterSize + nNumGutters * kfElementGutter))
		return false;

	float fElementWidth = ((fRight - fLeft) - (nNumGutters * kfElementGutter)) / m_nFilterSize;

	//alright, now find the highest element so we can scale
	float fMaxElement = g_CV_ScreenGlowShowFilterRange.m_Val;

	//alright, now draw each and every element
	float fCurrX = fLeft;
	for(uint32 nCurrElement = 0; nCurrElement < m_nFilterSize; nCurrElement++)
	{
		float fHeight = LTCLAMP((m_GaussianFilter[nCurrElement].m_fWeight) / fMaxElement, 0.0f, 1.0f);
		fHeight = fTop * fHeight + fBottom * (1.0f - fHeight);

		SetupRectVerts(fCurrX, fHeight, fCurrX + fElementWidth, fBottom, 0, 0, 0, 0, 0xFF00CC00, Verts);
		D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(TLVertex)));

		//move the position along
		fCurrX += fElementWidth + kfElementGutter;
	}

	return true;
}
