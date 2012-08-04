//////////////////////////////////////////////////////////////////////////////
// Dynamic lighting sub-shader

/*

How this works:

	Set up 2 textures.
	The first is square, and filled with the value (1 - (x^2 + y^2)), where x & y range [-1,1] across the texture.
	The other is one-dimensional, and filled with the value z^2, where z also ranges [-1,1].
	Set up the texture coordinate generation to map the camera space vector back into
	world space, offset by the origin of the light, and scaled by the radius.
	Set up the TSS to subtract texture 2 from texture 1 and modulate by the light color.
		Note : This sets up the TSS for Color=TFACTOR, and Alpha=T2-T1.  The multiply is then performed using the 
			frame buffer alpha blend.  That way it validates more readily on older hardware.
	See various per-pixel lighting documents for more information.  (nVidia's web page is a good place to look.)

Major limitations:
	
	No angular attenuation

*/

#pragma warning(disable : 4786)

#include "precompile.h"

#include "d3d_rendershader_dynamiclight.h"

#include "common_draw.h"

#include "d3d_texture.h"

#include "memstats_world.h"

#include "d3d_renderblock.h"

#include "rendererframestats.h"

// Console variables

// The singleton
CRenderShader_DynamicLight g_RS_DynamicLight;

CRenderShader_DynamicLight *CRenderShader_DynamicLight::GetSingleton()
{
	return &g_RS_DynamicLight;
}

CRenderShader_DynamicLight::CRenderShader_DynamicLight() :
	m_bBound(false),
	m_pTextureXY(0), m_pTextureZ(0),
	m_bStatesSaved(false),
	m_pSavedIB(0),
	m_nSavedIBBase(0),
	m_pTempIB(0),
	m_nTempIBIndex(0),
	m_pCurLight(0)
{
}

CRenderShader_DynamicLight::~CRenderShader_DynamicLight()
{
	ASSERT(!m_bBound && "Dynamic light sub-shader bound at destruction time!");
}

void CRenderShader_DynamicLight::Bind()
{
	if (m_bBound)
		return;

	// Don't try to bind again, even if we fail
	m_bBound = true;

	// Default to the failure case in the lighting method
	m_eLightingMethod = eLight_Unsupported;

	HRESULT hr;

	// Init the dynamic lighting textures
	ASSERT(!m_pTextureXY);
	hr = PD3DDEVICE->CreateTexture(g_CV_DynamicLight_TextureSize.m_Val,g_CV_DynamicLight_TextureSize.m_Val,1, 
		0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTextureXY);
	if (FAILED(hr))
	{
		dsi_ConsolePrint("Unable to create dynamic lighting XY texture.");
		return;
	}

	ASSERT(!m_pTextureZ);
	hr = PD3DDEVICE->CreateTexture(g_CV_DynamicLight_TextureSize.m_Val,1,1, 
		0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTextureZ);
	if (FAILED(hr))
	{
		dsi_ConsolePrint("Unable to create dynamic lighting Z texture.");
		Release();
		m_bBound = true;
		return;
	}

	// Create the index buffer
	hr = PD3DDEVICE->CreateIndexBuffer(k_nTempIBSize * sizeof(uint16), 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pTempIB);
	if (FAILED(hr))
	{
		dsi_ConsolePrint("Unable to create dynamic lighting index buffer.");
		Release();
		m_bBound = true;
		return;
	}
	// Point at the beginning of the buffer
	m_nTempIBIndex = 0;

	DWORD nNumPasses;

	// Try the accurate lighting method
	m_eLightingMethod = eLight_Ambient;
	SetStates();
	// Note : We can't set the textures during validation, since we just created the textures, and 
	// they might not have made it to the video card yet.
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);
	hr = PD3DDEVICE->ValidateDevice(&nNumPasses);
	ResetStates();
	if (SUCCEEDED(hr))
	{
		FillTextures();
		return;
	}

	// No dynamic light support for you!
	m_eLightingMethod = eLight_Unsupported;
	
	// Release the textures so we're not wasting memory..
	m_pTextureXY->Release();
	m_pTextureXY = 0;
	m_pTextureZ->Release();
	m_pTextureZ = 0;
}

void CRenderShader_DynamicLight::Release()
{
	if (!m_bBound)
		return;

	if (m_pTextureXY)
	{
		m_pTextureXY->Release();
		m_pTextureXY = 0;
	}

	if (m_pTextureZ)
	{
		m_pTextureZ->Release();
		m_pTextureZ = 0;
	}

	if (m_pTempIB)
	{
		m_pTempIB->Release();
		m_pTempIB = 0;
	}

	m_bBound = false;
}

void CRenderShader_DynamicLight::SetStates()
{
	if (m_eLightingMethod == eLight_Unsupported)
		return;

	SaveState();

	switch (m_eLightingMethod)
	{
		case eLight_Ambient :
		{
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 0);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
			PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SUBTRACT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
			PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		
			break;
		}

		// Note : No other lighting methods are currently supported...
	}

	// Set the states we need no matter what type of lighting we're doing
	PD3DDEVICE->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF, 0x00);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	d3d_SetTextureDirect(m_pTextureZ, 0);
	d3d_SetTextureDirect(m_pTextureXY, 1);

	PD3DDEVICE->SetIndices(m_pTempIB);
}

void CRenderShader_DynamicLight::SaveState()
{
	if (m_eLightingMethod == eLight_Unsupported)
		return;

	if (m_bStatesSaved)
		return;

	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, &m_aSavedStates[eRS_AlphaBlendEnable]);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, &m_aSavedStates[eRS_SrcBlend]);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, &m_aSavedStates[eRS_DestBlend]);
	PD3DDEVICE->GetRenderState(D3DRS_ZFUNC, &m_aSavedStates[eRS_ZFunc]);
	PD3DDEVICE->GetRenderState(D3DRS_ZWRITEENABLE, &m_aSavedStates[eRS_ZWrite]);
	PD3DDEVICE->GetRenderState(D3DRS_TEXTUREFACTOR, &m_aSavedStates[eRS_TextureFactor]);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, &m_aSavedStates[eRS_AlphaTestEnable]);
	PD3DDEVICE->GetRenderState(D3DRS_ALPHAREF, &m_aSavedStates[eRS_AlphaRef]);
	PD3DDEVICE->GetRenderState(D3DRS_ALPHAFUNC, &m_aSavedStates[eRS_AlphaFunc]);

	PD3DDEVICE->GetTextureStageState(0, D3DTSS_COLORARG1, &m_aSavedStates[eRS_TSS0_CArg1]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_COLORARG2, &m_aSavedStates[eRS_TSS0_CArg2]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_COLOROP, &m_aSavedStates[eRS_TSS0_COp]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_ALPHAARG1, &m_aSavedStates[eRS_TSS0_AArg1]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_ALPHAARG2, &m_aSavedStates[eRS_TSS0_AArg2]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_ALPHAOP, &m_aSavedStates[eRS_TSS0_AOp]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_TEXCOORDINDEX, &m_aSavedStates[eRS_TSS0_TCI]);
	PD3DDEVICE->GetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, &m_aSavedStates[eRS_TSS0_TTF]);
	PD3DDEVICE->GetSamplerState(0, D3DSAMP_ADDRESSU, &m_aSavedStates[eRS_TSS0_AddrU]);
	PD3DDEVICE->GetSamplerState(0, D3DSAMP_ADDRESSV, &m_aSavedStates[eRS_TSS0_AddrV]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_COLORARG1, &m_aSavedStates[eRS_TSS1_CArg1]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_COLORARG2, &m_aSavedStates[eRS_TSS1_CArg2]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_COLOROP, &m_aSavedStates[eRS_TSS1_COp]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_ALPHAARG1, &m_aSavedStates[eRS_TSS1_AArg1]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_ALPHAARG2, &m_aSavedStates[eRS_TSS1_AArg2]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_ALPHAOP, &m_aSavedStates[eRS_TSS1_AOp]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_TEXCOORDINDEX, &m_aSavedStates[eRS_TSS1_TCI]);
	PD3DDEVICE->GetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, &m_aSavedStates[eRS_TSS1_TTF]);
	PD3DDEVICE->GetSamplerState(1, D3DSAMP_ADDRESSU, &m_aSavedStates[eRS_TSS1_AddrU]);
	PD3DDEVICE->GetSamplerState(1, D3DSAMP_ADDRESSV, &m_aSavedStates[eRS_TSS1_AddrV]);
	PD3DDEVICE->GetTextureStageState(2, D3DTSS_COLOROP, &m_aSavedStates[eRS_TSS2_COp]);
	PD3DDEVICE->GetTextureStageState(2, D3DTSS_ALPHAOP, &m_aSavedStates[eRS_TSS2_AOp]);

	PD3DDEVICE->GetIndices(&m_pSavedIB);

	m_bStatesSaved = true;
}

void CRenderShader_DynamicLight::ResetStates()
{
	if (m_eLightingMethod == eLight_Unsupported)
		return;

	if (!m_bStatesSaved)
		return;

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, m_aSavedStates[eRS_TSS0_CArg1]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, m_aSavedStates[eRS_TSS0_CArg2]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, m_aSavedStates[eRS_TSS0_COp]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, m_aSavedStates[eRS_TSS0_AArg1]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, m_aSavedStates[eRS_TSS0_AArg2]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, m_aSavedStates[eRS_TSS0_AOp]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, m_aSavedStates[eRS_TSS0_TCI]);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, m_aSavedStates[eRS_TSS0_TTF]);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, m_aSavedStates[eRS_TSS0_AddrU]);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, m_aSavedStates[eRS_TSS0_AddrV]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, m_aSavedStates[eRS_TSS1_CArg1]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, m_aSavedStates[eRS_TSS1_CArg2]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, m_aSavedStates[eRS_TSS1_COp]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, m_aSavedStates[eRS_TSS1_AArg1]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, m_aSavedStates[eRS_TSS1_AArg2]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, m_aSavedStates[eRS_TSS1_AOp]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, m_aSavedStates[eRS_TSS1_TCI]);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, m_aSavedStates[eRS_TSS1_TTF]);
	PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSU, m_aSavedStates[eRS_TSS1_AddrU]);
	PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSV, m_aSavedStates[eRS_TSS1_AddrV]);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, m_aSavedStates[eRS_TSS2_COp]);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, m_aSavedStates[eRS_TSS2_AOp]);

	PD3DDEVICE->SetRenderState(D3DRS_ZFUNC, m_aSavedStates[eRS_ZFunc]);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, m_aSavedStates[eRS_ZWrite]);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_aSavedStates[eRS_AlphaBlendEnable]);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_aSavedStates[eRS_SrcBlend]);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_aSavedStates[eRS_DestBlend]);

	PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_aSavedStates[eRS_AlphaTestEnable]);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF, m_aSavedStates[eRS_AlphaRef]);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC, m_aSavedStates[eRS_AlphaFunc]);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	PD3DDEVICE->SetIndices(m_pSavedIB);

	m_pCurLight = 0;

	m_bStatesSaved = false;
}

void CRenderShader_DynamicLight::FillTextures()
{
	if (m_eLightingMethod == eLight_Unsupported)
		return;

	HRESULT hr;

	D3DLOCKED_RECT sXYRect;
	hr = m_pTextureXY->LockRect(0, &sXYRect, 0, 0);
	ASSERT(SUCCEEDED(hr));
	ASSERT(sXYRect.pBits);
	ASSERT(sXYRect.Pitch);
	uint32 *pCurTexel = (uint32*)sXYRect.pBits;
	const uint32 &nTextureSize = g_CV_DynamicLight_TextureSize.m_Val;
	float fInvHalfValue = 2.0f / (float)(nTextureSize - 1);
	for (uint32 nYLoop = 0; nYLoop < nTextureSize; ++nYLoop)
	{
		float fYValue = (float)(nYLoop) * fInvHalfValue - 1.0f;
		fYValue = fYValue * fYValue;

		for (uint32 nXLoop = 0; nXLoop < nTextureSize; ++nXLoop)
		{
			float fXValue =  (float)(nXLoop) * fInvHalfValue - 1.0f;
			fXValue = fXValue * fXValue;

			float fFinalValue = (fXValue + fYValue);
			fFinalValue = 1.0f - LTCLAMP(fFinalValue, 0.0f, 1.0f);
			pCurTexel[nXLoop] = D3DRGBA(fFinalValue, fFinalValue, fFinalValue, fFinalValue);
		}
		pCurTexel += sXYRect.Pitch / sizeof(uint32);
	}
	m_pTextureXY->UnlockRect(0);

	D3DLOCKED_RECT sZRect;
	m_pTextureZ->LockRect(0, &sZRect, 0, 0);
	ASSERT(SUCCEEDED(hr));
	ASSERT(sZRect.pBits);
	ASSERT(sZRect.Pitch);
	pCurTexel = (uint32*)sZRect.pBits;
	for (uint32 nZLoop = 0; nZLoop < nTextureSize; ++nZLoop)
	{
		float fValue = (float)(nZLoop) * fInvHalfValue - 1.0f;
		float fFinalValue = fValue * fValue;
		pCurTexel[nZLoop] = D3DRGBA(fFinalValue, fFinalValue, fFinalValue, fFinalValue);
	}
	m_pTextureZ->UnlockRect(0);
}

bool CRenderShader_DynamicLight::SetupLight(const ViewParams *pParams, const DynamicLight *pLight)
{
	if (!m_bBound)
	{
		ASSERT(!"SetupLight called with un-bound sub-shader");
		Bind();
	}

	if (m_eLightingMethod == eLight_Unsupported)
		return false;
	
	// Set up the generic states
	SetStates();

	// Remember the current light
	m_pCurLight = pLight;
	// Get the light position in the space of the current model
	MatVMul_H(&m_vCurLightPos, &pParams->m_mInvWorld, &pLight->GetPos());

	// Set up the non-generic states
	PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR, RGB_MAKE(pLight->m_ColorR, pLight->m_ColorG, pLight->m_ColorB));

	// Set up the lighting matrices
	float fRadiusScale = 1.0f / (pLight->m_LightRadius * 2.0f);
	LTMatrix mXYTransform;
	mXYTransform.Init(
		fRadiusScale, 0.0f, 0.0f, -pLight->GetPos().x * fRadiusScale + 0.5f,
		0.0f, fRadiusScale, 0.0f, -pLight->GetPos().y * fRadiusScale + 0.5f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	mXYTransform = mXYTransform * pParams->m_mInvView;

	D3DXMATRIX mD3DXYTransform;
	D3DXMatrixTranspose(&mD3DXYTransform, (D3DXMATRIX*)&mXYTransform);
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &mD3DXYTransform);

	LTMatrix mZTransform;
	mZTransform.Init(
		0.0f, 0.0f, fRadiusScale, -pLight->GetPos().z * fRadiusScale + 0.5f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	mZTransform = mZTransform * pParams->m_mInvView;

	D3DXMATRIX mD3DZTransform;
	D3DXMatrixTranspose(&mD3DZTransform, (D3DXMATRIX*)&mZTransform);
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE0, &mD3DZTransform);

	// Ok, your turn..  Render them triangles...
	return true;
}

void CRenderShader_DynamicLight::GetMemStats(CMemStats_World &cMemStats) const
{
	static uint32 s_nLastMemStatFrame = 0;
	// Make sure we only get counted once per frame
	if (s_nLastMemStatFrame == g_CurFrameCode)
		return;
	s_nLastMemStatFrame = g_CurFrameCode;

	if (!m_bBound)
		return;

	if (m_eLightingMethod == eLight_Unsupported)
		return;

	// Add in the size of the lighting textures
	cMemStats.m_nDynamicTextureCount += 2;
	cMemStats.m_nDynamicTextureData += g_CV_DynamicLight_TextureSize.m_Val * g_CV_DynamicLight_TextureSize.m_Val * 4;
	cMemStats.m_nDynamicTextureData += g_CV_DynamicLight_TextureSize.m_Val * 4;
}

static const SRBVertex *GetVertexIndex(const SRBVertex*pVertices, uint32 nStride, int32 nIndex)
{
	return reinterpret_cast<const SRBVertex*>(&reinterpret_cast<const uint8*>(pVertices)[nIndex * (int32)nStride]);
}

uint32 CRenderShader_DynamicLight::DrawTris(
		uint32 nVBStart, uint32 nVBCount,
		const uint16 *pIndices,
		uint32 nNumTris,
		const SRBVertex *pVertices,
		uint32 nVertexStride,
		int32 nVertexOffset
	)
{
	if (!m_pCurLight)
	{
		ASSERT(!"Light drawing request encountered without active light!");
		return 0;
	}

	uint32 nResult = 0;

	// Adjust for the vertex offset
	//pVertices = GetVertexIndex(pVertices, nVertexStride, nVertexOffset);

	uint16 *pTempIndices;
	HRESULT hr = m_pTempIB->Lock(m_nTempIBIndex * sizeof(uint16), (k_nTempIBSize - m_nTempIBIndex) * sizeof(uint16), 
		(void **)&pTempIndices, m_nTempIBIndex ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);
	if (FAILED(hr))
	{
		ASSERT(!"Failed on dynamic lighting index buffer lock!");
		return 0;
	}

	uint32 nBaseIndex = m_nTempIBIndex;

	const uint16 *pEndIndex = &pIndices[nNumTris * 3];
	for (; pIndices != pEndIndex; pIndices += 3)
	{
		// Get the vertex positions
		const SRBVertex &v0 = *GetVertexIndex(pVertices, nVertexStride, pIndices[0]);
		const SRBVertex &v1 = *GetVertexIndex(pVertices, nVertexStride, pIndices[1]);
		const SRBVertex &v2 = *GetVertexIndex(pVertices, nVertexStride, pIndices[2]);
		
		// Is it a back-facing tri?
		LTVector vFacing = (v2.m_vPos - v0.m_vPos).Cross(v1.m_vPos - v0.m_vPos);
		LTVector vOfs = m_vCurLightPos - ((v0.m_vPos + v1.m_vPos + v2.m_vPos) / 3.0f);
		bool bBackFacing = 
			(vOfs.Dot(vFacing) < -1.0f) &&
			(v0.m_vNormal.Dot(m_vCurLightPos - v0.m_vPos) < -g_CV_DynamicLight_Backfacing_Error.m_Val) &&
			(v1.m_vNormal.Dot(m_vCurLightPos - v1.m_vPos) < -g_CV_DynamicLight_Backfacing_Error.m_Val) &&
			(v2.m_vNormal.Dot(m_vCurLightPos - v2.m_vPos) < -g_CV_DynamicLight_Backfacing_Error.m_Val);
		if (bBackFacing)
			continue;

		// Do we need to make space?
		if (m_nTempIBIndex == k_nTempIBSize)
		{
			m_pTempIB->Unlock();
			PD3DDEVICE->SetIndices(m_pTempIB);
			PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, nVBStart, nVBCount, nBaseIndex, (m_nTempIBIndex - nBaseIndex) / 3);
			m_nTempIBIndex = 0;
			nBaseIndex = 0;
			m_pTempIB->Lock(0, 0, (void **)&pTempIndices, D3DLOCK_DISCARD);

			//update our triangle count
			IncFrameStat(eFS_DynamicLightTriangles, (m_nTempIBIndex - nBaseIndex) / 3);
		}

		// Write out the indices
		pTempIndices[0] = (uint16)(pIndices[0] + nVertexOffset);
		pTempIndices[1] = (uint16)(pIndices[1] + nVertexOffset);
		pTempIndices[2] = (uint16)(pIndices[2] + nVertexOffset);

		// Move ahead in the output
		pTempIndices += 3;
		m_nTempIBIndex += 3;

		// Count it
		++nResult;
	}

	m_pTempIB->Unlock();
	
	if (nBaseIndex != m_nTempIBIndex)
	{
		PD3DDEVICE->SetIndices(m_pTempIB);
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, nVBStart, nVBCount, nBaseIndex, (m_nTempIBIndex - nBaseIndex) / 3);
		if (m_nTempIBIndex == k_nTempIBSize)
			m_nTempIBIndex = 0;

		//update our triangle count
		IncFrameStat(eFS_DynamicLightTriangles, (m_nTempIBIndex - nBaseIndex) / 3);
	}

	return nResult;
}
