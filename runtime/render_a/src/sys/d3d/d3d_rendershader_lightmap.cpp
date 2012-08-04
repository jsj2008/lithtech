//////////////////////////////////////////////////////////////////////////////
// Lightmapped render shader implementation

#include "precompile.h"

#include "d3d_rendershader_lightmap.h"
#include "d3d_device.h"
#include "d3d_draw.h"
#include "common_draw.h"
#include "d3d_texture.h"
#include "lightmapdefs.h"
#include "lightmap_compress.h"
#include "common_stuff.h"
#include "renderstruct.h"

#include "memstats_world.h"

#include "d3d_rendershader_dynamiclight.h"

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap implementation

bool CRenderShader_Lightmap::s_bValidateRequired = true;
bool CRenderShader_Lightmap::s_bValidateResult = false;

bool CRenderShader_Lightmap::ValidateShader(const CRBSection &cSection)
{
	if (!s_bValidateRequired)
		return s_bValidateResult;

	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		return false;
	}

	// Create a temporary testing texture
	TextureFormat* pFormat = g_TextureManager.GetTextureFormat((uint8)CTextureManager::FORMAT_LIGHTMAP);
	if (!pFormat)
		return false;

	IDirect3DTexture9 *pTempTexture;
	if (!SUCCEEDED(PD3DDEVICE->CreateTexture(256, 256, 1, 0,
		pFormat->m_PF, D3DPOOL_MANAGED, &pTempTexture)))
		return false;

	// Try out the states
	StageStateSet state00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet state03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet state05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet state10(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state11(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	SamplerStateSet stateWrap0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SamplerStateSet stateWrap1(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	d3d_SetTextureDirect(pTempTexture, 0);

	// Are we kosher?
	uint32 nNumPasses;
	HRESULT hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
	pTempTexture->Release();

	s_bValidateRequired = false;
	s_bValidateResult = SUCCEEDED(hr);

	return s_bValidateResult;
}

void CRenderShader_Lightmap::TranslateVertices(SVertex_Lightmap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap *pCurOut = pOut;
	SVertex_Lightmap *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_fU = pCurIn->m_fU0;
		pCurOut->m_fV = pCurIn->m_fV0;
        pCurOut->m_fW = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

CSection_Lightmap::CSection_Lightmap(const CSection_Lightmap &cOther) :
	m_pLMTexture(cOther.m_pLMTexture),
	m_TextureMem(cOther.m_TextureMem)
{
	if (m_pLMTexture)
		m_pLMTexture->AddRef();
}

CSection_Lightmap::~CSection_Lightmap()
{
	if (m_pLMTexture)
		m_pLMTexture->Release();
}

CSection_Lightmap &CSection_Lightmap::operator=(const CSection_Lightmap &cOther)
{
	if (m_pLMTexture)
		m_pLMTexture->Release();

	m_pLMTexture = cOther.m_pLMTexture;

	if (m_pLMTexture)
		m_pLMTexture->AddRef();

	m_TextureMem = cOther.m_TextureMem;

	return *this;
}

static uint32 GetNextPow2(uint32 nValue)
{
	// Strip off the set bits from low to high, and remember the number right
	// before it became 0
	uint32 nPrevValue = nValue;
	uint32 nCount = 0;
	while (nValue)
	{
		nPrevValue = nValue;
		nValue &= (nValue - 1);
		++nCount;
	}

	switch (nCount)
	{
		case 0 :
			return 1;
		case 1 :
			return nPrevValue;
		default :
			return nPrevValue << 1;
	}
}

void CRenderShader_Lightmap::FillSection(CSection_Lightmap &cInternalSection, const CRBSection &cSection)
{
	// Release any textures we might accidentally have
	if (cInternalSection.m_pLMTexture)
	{
		cInternalSection.m_pLMTexture->Release();
		cInternalSection.m_pLMTexture = 0;
	}

	// Handle no data..
	if (!cSection.m_pLightmapData)
		return;

	// Get the texture format
	TextureFormat* pFormat = g_TextureManager.GetTextureFormat((uint8)CTextureManager::FORMAT_LIGHTMAP);
	if (!pFormat)
		return;

	// Create a lightmapping texture
	uint32 nRealWidth = GetNextPow2(cSection.m_nLightmapWidth);
	uint32 nRealHeight = GetNextPow2(cSection.m_nLightmapHeight);
	if (!SUCCEEDED(PD3DDEVICE->CreateTexture(nRealWidth, nRealHeight, 1, 0,
		pFormat->m_PF, D3DPOOL_MANAGED, &cInternalSection.m_pLMTexture)))
	{
		ASSERT(!"Error creating lightmapping texture!");
		return;
	}

	// Decompress the lightmap into a temporary buffer
	std::vector<uint8> aLMData;
	LT_MEM_TRACK_ALLOC(aLMData.resize(LIGHTMAP_MAX_TOTAL_PIXELS * 3), LT_MEM_TYPE_RENDER_LIGHTMAP);
	DecompressLMData(cSection.m_pLightmapData, cSection.m_nLightmapSize, static_cast<uint8*>(&(*aLMData.begin())));

	// Fill the lightmap
	UpdateLightmap_Internal(cSection, static_cast<uint8*>(&(*aLMData.begin())), pFormat, &cInternalSection);

	// Remember how much memory this is taking up
	cInternalSection.m_TextureMem.SetMemory(nRealWidth * nRealHeight * pFormat->m_BytesPP);
}

void CRenderShader_Lightmap::UpdateLightmap(uint32 nRenderBlock, const CRBSection &cSection, const uint8 *pLMData)
{
	// Get the texture format
	TextureFormat* pFormat = g_TextureManager.GetTextureFormat((uint8)CTextureManager::FORMAT_LIGHTMAP);
	if (!pFormat)
	{
		ASSERT(!"Lightmap update unavailable due to invalid lightmap format");
		return;
	}

	// Get the internal section pointer
	CSection_Lightmap *pInternalSection = FindSection(nRenderBlock, cSection.m_nIndex);
	if (!pInternalSection)
	{
		ASSERT(!"Lightmap update unavailable due to missing section");
		return;
	}

	if (!pInternalSection->m_pLMTexture)
	{
		ASSERT(!"Lightmap update unavailable due to missing texture");
		return;
	}

	// Update the lightmap
	UpdateLightmap_Internal(cSection, pLMData, pFormat, pInternalSection);
}

void CRenderShader_Lightmap::UpdateLightmap_Internal(const CRBSection &cSection, const uint8 *pLMData, TextureFormat* pFormat, CSection_Lightmap *pInternalSection)
{
	// Lock the texture
	D3DLOCKED_RECT cRect;
	if (!SUCCEEDED(pInternalSection->m_pLMTexture->LockRect(0, &cRect, NULL, 0)))
	{
		ASSERT(!"Lightmap update unavailable due to failed lock");
		return;
	}

	// Convert the data
	FMConvertRequest	cRequest;
	cRequest.m_pSrcFormat->Init(BPP_24, 0, 0x00FF0000, 0x0000FF00, 0x000000FF);
	cRequest.m_pSrc			= const_cast<uint8*>(pLMData);
	cRequest.m_SrcPitch		= cSection.m_nLightmapWidth * sizeof(uint8) * 3;
	pFormat->SetupPFormat(cRequest.m_pDestFormat);
	cRequest.m_pDest		= (uint8*)cRect.pBits;
	cRequest.m_DestPitch	= cRect.Pitch;
	cRequest.m_Width		= cSection.m_nLightmapWidth;
	cRequest.m_Height		= cSection.m_nLightmapHeight;
	g_FormatMgr.ConvertPixels(&cRequest);

	// Unlock the texture
	pInternalSection->m_pLMTexture->UnlockRect(0);
}

void CRenderShader_Lightmap::DrawLights(const DrawState &cState, uint32 nRenderBlock)
{
	// Set up our call
	PD3DDEVICE->SetIndices(m_pIB);
	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(k_SVertex_Lightmap_FVF);

	PreFlush();

	CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
	CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
	for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
	{
		CInternalSection &cSection = m_aSections[*iCurSection];

		ChangeSection(cSection);

		FlushChangeSection(cSection);

		PreFlushBlock(cSection);

		IncFrameStat(eFS_ShaderTris, cSection.m_nTriCount);
		IncFrameStat(eFS_ShaderVerts, cSection.m_nVertexCount);
		IncFrameStat(eFS_ShaderDrawPrim, 1);

		// Draw it
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, cSection.m_nStartVertex, cSection.m_nVertexCount,
			cSection.m_nStartIndex, cSection.m_nTriCount);

		// Count it
		IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

		PostFlushBlock(cSection,
			cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
			cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
	}

	DrawLightList(cState, nRenderBlock);

	PostFlush();

	// Don't leak the VB or IB
	PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
	PD3DDEVICE->SetIndices(0);
}

void CRenderShader_Lightmap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	bool bDrawingLights = cState.m_nNumLights != 0;

	if (bDrawingLights)
	{
		DrawLights(cState, nRenderBlock);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Lightmap::PreFlush()
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CRenderShader_Lightmap::FlushChangeSection(CInternalSection &cSection)
{
	// Select the lightmap texture
	d3d_SetTextureDirect(cSection.m_pLMTexture, 0, cSection.m_TextureMem, eFS_WorldLightMapTexMemory);
}

void CRenderShader_Lightmap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(1, TSChannel_LightMap);
}

void CRenderShader_Lightmap::PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(1, TSChannel_LightMap);
}

void CRenderShader_Lightmap::PostFlush()
{
	d3d_DisableTexture(0);

	// Restore the wrapping mode
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

void CRenderShader_Lightmap::DebugTri(
	uint32 nRenderBlock,
	const CRBSection &cSection,
	uint32 nTriIndex,
	const uint16 *aIndices,
	const SRBVertex *aVertices,
	float fX, float fY,
	float fSizeX, float fSizeY)
{
	// Find the section in our internal list
	CInternalSection *pISection = FindSection(nRenderBlock, cSection.m_nIndex);
	if (!pISection)
		return;

	float fVert0U, fVert0V;
	float fVert1U, fVert1V;
	float fVert2U, fVert2V;

	fVert0U = aVertices[aIndices[nTriIndex]].m_fU0;
	fVert0V = aVertices[aIndices[nTriIndex]].m_fV0;
	fVert1U = aVertices[aIndices[nTriIndex + 1]].m_fU0;
	fVert1V = aVertices[aIndices[nTriIndex + 1]].m_fV0;
	fVert2U = aVertices[aIndices[nTriIndex + 2]].m_fU0;
	fVert2V = aVertices[aIndices[nTriIndex + 2]].m_fV0;

	// Draw the lightmap
	DebugDrawTextureQuad(pISection->m_pLMTexture,
		0, 0, 1, 1,
		fX, fY, fSizeX, fSizeY);

	// Draw the tri
	struct STriVert
	{
		LTVector m_vPos;
		float m_fRHW;
	};
	STriVert aTriVerts[3];
	aTriVerts[0].m_vPos.x = fVert0U * fSizeX + fX;
	aTriVerts[0].m_vPos.y = fVert0V * fSizeY + fY;
	aTriVerts[0].m_vPos.z = 0.0f;
	aTriVerts[0].m_fRHW = 0.0f;
	aTriVerts[1].m_vPos.x = fVert1U * fSizeX + fX;
	aTriVerts[1].m_vPos.y = fVert1V * fSizeY + fY;
	aTriVerts[1].m_vPos.z = 0.0f;
	aTriVerts[1].m_fRHW = 0.0f;
	aTriVerts[2].m_vPos.x = fVert2U * fSizeX + fX;
	aTriVerts[2].m_vPos.y = fVert2V * fSizeY + fY;
	aTriVerts[2].m_vPos.z = 0.0f;
	aTriVerts[2].m_fRHW = 0.0f;

	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(D3DFVF_XYZRHW);

	StateSet stateAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet stateSrcBlend(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
	StateSet stateDestBlend(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	StateSet stateLighting(D3DRS_LIGHTING, FALSE);

	d3d_DisableTexture(0);

	PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, aTriVerts, sizeof(STriVert));
}

void CRenderShader_Lightmap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		++cMemStats.m_nLightmapCount;
		cMemStats.m_nLightmapData += cSection.m_TextureMem.m_nMemory;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture implementation

bool CRenderShader_Lightmap_Texture::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData)
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	StageStateSet state0(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state1(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state2(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet state3(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state4(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet state5(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet state6(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state7(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);

	// Are we kosher?
	uint32 nNumPasses;
	HRESULT hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);

	s_bValidateRequired = false;
	s_bValidateResult = SUCCEEDED(hr);

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture::TranslateVertices(SVertex_Lightmap_Texture *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap_Texture *pCurOut = pOut;
	SVertex_Lightmap_Texture *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_fU = pCurIn->m_fU0;
		pCurOut->m_fV = pCurIn->m_fV0;
        pCurOut->m_fW = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Lightmap_Texture::FillSection(CSection_Lightmap_Texture &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cInternalSection.m_pTexture->m_pRenderData;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Lightmap_Texture::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Lightmap_Texture::PreFlush()
{

//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(-1.0f));

	// Set up our call
	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CRenderShader_Lightmap_Texture::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
}

void CRenderShader_Lightmap_Texture::PreFlushBlock(CInternalSection &cSection)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(1, TSChannel_Base);
}

void CRenderShader_Lightmap_Texture::PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Draw it
		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);
	}

	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base);
}

void CRenderShader_Lightmap_Texture::PostFlush()
{
	d3d_DisableTexture(0);

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);
//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(0.0f));
}

void CRenderShader_Lightmap_Texture::DebugTri(
	uint32 nRenderBlock,
	const CRBSection &cSection,
	uint32 nTriIndex,
	const uint16 *aIndices,
	const SRBVertex *aVertices,
	float fX, float fY,
	float fSizeX, float fSizeY)
{
	// Find the section in our internal list
	CInternalSection *pISection = FindSection(nRenderBlock, cSection.m_nIndex);
	if (!pISection)
		return;

	float fVert0U, fVert0V;
	float fVert1U, fVert1V;
	float fVert2U, fVert2V;

	fVert0U = aVertices[aIndices[nTriIndex]].m_fU0;
	fVert0V = aVertices[aIndices[nTriIndex]].m_fV0;
	fVert1U = aVertices[aIndices[nTriIndex + 1]].m_fU0;
	fVert1V = aVertices[aIndices[nTriIndex + 1]].m_fV0;
	fVert2U = aVertices[aIndices[nTriIndex + 2]].m_fU0;
	fVert2V = aVertices[aIndices[nTriIndex + 2]].m_fV0;

	float fMinU = LTMIN(fVert0U, LTMIN(fVert1U, fVert2U));
	float fMinV = LTMIN(fVert0V, LTMIN(fVert1V, fVert2V));
	float fMaxU = LTMAX(fVert0U, LTMAX(fVert1U, fVert2U));
	float fMaxV = LTMAX(fVert0V, LTMAX(fVert1V, fVert2V));

	float fSpanU = fMaxU - fMinU;
	fMinU -= fSpanU * 0.25f;
	fMaxU += fSpanU * 0.25f;
	float fSpanV = fMaxV - fMinV;
	fMinV -= fSpanV * 0.25f;
	fMaxV += fSpanV * 0.25f;

	float fInvUDiff = 1.0f / (fMaxU - fMinU);
	fVert0U = (fVert0U - fMinU) * fInvUDiff;
	fVert1U = (fVert1U - fMinU) * fInvUDiff;
	fVert2U = (fVert2U - fMinU) * fInvUDiff;

	float fInvVDiff = 1.0f / (fMaxV - fMinV);
	fVert0V = (fVert0V - fMinV) * fInvVDiff;
	fVert1V = (fVert1V - fMinV) * fInvVDiff;
	fVert2V = (fVert2V - fMinV) * fInvVDiff;

	// Draw the lightmap
	d3d_SetTexture(pISection->m_pTexture, 0, eFS_WorldBaseTexMemory);
	DebugDrawTextureQuad(0,
		fMinU, fMinV, fMaxU, fMaxV,
		fX, fY, fSizeX, fSizeY);
	d3d_DisableTexture(0);

	// Draw the tri
	struct STriVert
	{
		LTVector m_vPos;
		float m_fRHW;
	};
	STriVert aTriVerts[3];
	aTriVerts[0].m_vPos.x = fVert0U * fSizeX + fX;
	aTriVerts[0].m_vPos.y = fVert0V * fSizeY + fY;
	aTriVerts[0].m_vPos.z = 0.0f;
	aTriVerts[0].m_fRHW = 0.0f;
	aTriVerts[1].m_vPos.x = fVert1U * fSizeX + fX;
	aTriVerts[1].m_vPos.y = fVert1V * fSizeY + fY;
	aTriVerts[1].m_vPos.z = 0.0f;
	aTriVerts[1].m_fRHW = 0.0f;
	aTriVerts[2].m_vPos.x = fVert2U * fSizeX + fX;
	aTriVerts[2].m_vPos.y = fVert2V * fSizeY + fY;
	aTriVerts[2].m_vPos.z = 0.0f;
	aTriVerts[2].m_fRHW = 0.0f;

	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(D3DFVF_XYZRHW);

	StateSet stateAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet stateSrcBlend(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
	StateSet stateDestBlend(D3DRS_DESTBLEND, D3DBLEND_ZERO);

	StateSet stateLighting(D3DRS_LIGHTING, FALSE);

	d3d_DisableTexture(0);

	PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, aTriVerts, sizeof(STriVert));
}

void CRenderShader_Lightmap_Texture::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());
	}
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture_Detail implementation

bool CRenderShader_Lightmap_Texture_Detail::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture_Detail::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture_Detail::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData)
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_Detail))
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	StageStateSet state00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet state03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet state05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet state10(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state11(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet state12(1, D3DTSS_COLOROP, g_CV_DetailTextureAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	StageStateSet state13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state14(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet state15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	StageStateSet state20(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state21(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);

	// Are we kosher?
	uint32 nNumPasses;
	HRESULT hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	s_bValidateRequired = false;
	s_bValidateResult = SUCCEEDED(hr);

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture_Detail::TranslateVertices(SVertex_Lightmap_Texture_Detail *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap_Texture_Detail *pCurOut = pOut;
	SVertex_Lightmap_Texture_Detail *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];

	// Set up a texture transform to get the detail texture coords
	float aTexMat[2][2];

	CRenderBlockData &cRB = m_aRenderBlocks[m_nCurRB];
	CInternalSection &cISection = m_aSections[cRB.m_aSections[m_nCurSection]];

	SharedTexture *pTexture = cISection.m_pTexture;
	RTexture *pRTex = LTNULL;
	if (pTexture)
	{
		pRTex = reinterpret_cast<RTexture*>(pTexture->m_pRenderData);
	}
	if (pRTex)
	{
		float fScale = pRTex->m_DetailTextureScale * g_CV_DetailTextureScale.m_Val;
		aTexMat[0][0] = fScale * pRTex->m_DetailTextureAngleC;
		aTexMat[0][1] = fScale * -pRTex->m_DetailTextureAngleS;
		aTexMat[1][0] = fScale * pRTex->m_DetailTextureAngleS;
		aTexMat[1][1] = fScale * pRTex->m_DetailTextureAngleC;
	}
	else
	{
		ASSERT(!"Error setting detail texture coordinates (bad texture)");
		// Initialize to identity * g_CV_DetailTextureScale in failure cases
		aTexMat[0][0] = g_CV_DetailTextureScale.m_Val;
		aTexMat[0][1] = 0.0f;
		aTexMat[1][0] = 0.0f;
		aTexMat[1][1] = g_CV_DetailTextureScale.m_Val;
	}

	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_fU0 = pCurIn->m_fU0;
		pCurOut->m_fV0 = pCurIn->m_fV0;
        pCurOut->m_fW0 = 1.0f;
		pCurOut->m_fU1 = pCurIn->m_fU0 * aTexMat[0][0] + pCurIn->m_fV0 * aTexMat[0][1];
		pCurOut->m_fV1 = pCurIn->m_fU0 * aTexMat[1][0] + pCurIn->m_fV0 * aTexMat[1][1];
        pCurOut->m_fW1 = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Lightmap_Texture_Detail::FillSection(CSection_Lightmap_Texture_Detail &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cInternalSection.m_pTexture->m_pRenderData;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Lightmap_Texture_Detail::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Lightmap_Texture_Detail::PreFlush()
{
	// Set up our call
//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(-1.0f));

	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_DetailTextureAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CRenderShader_Lightmap_Texture_Detail::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);
}

void CRenderShader_Lightmap_Texture_Detail::PreFlushBlock(CInternalSection &cSection)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_Detail);
}

void CRenderShader_Lightmap_Texture_Detail::PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Draw it
		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);
	}

	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_Detail);
}

void CRenderShader_Lightmap_Texture_Detail::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);

//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(0.0f));
}

void CRenderShader_Lightmap_Texture_Detail::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture_Detail);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());
		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_Detail)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_Detail)), pRTexture->GetMemoryUse());
	}
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture_EnvMap implementation

bool CRenderShader_Lightmap_Texture_EnvMap::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture_EnvMap::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture_EnvMap::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData)
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap))
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	StageStateSet state00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet state03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet state05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet state10(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state11(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet state12(1, D3DTSS_COLOROP, g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	StageStateSet state13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state14(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet state15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	// Note : We're going to assume the TexGen stuff is fine, since the nVidia drivers
	// will fail ValidateDevice for no particular reason
	StageStateSet state20(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state21(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), 1, eFS_WorldEnvMapTexMemory);

	// Are we kosher?
	uint32 nNumPasses;
	HRESULT hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	s_bValidateRequired = false;
	s_bValidateResult = SUCCEEDED(hr);

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture_EnvMap::TranslateVertices(SVertex_Lightmap_Texture_EnvMap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap_Texture_EnvMap *pCurOut = pOut;
	SVertex_Lightmap_Texture_EnvMap *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_fU = pCurIn->m_fU0;
		pCurOut->m_fV = pCurIn->m_fV0;
        pCurOut->m_fW = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Lightmap_Texture_EnvMap::FillSection(CSection_Lightmap_Texture_EnvMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cInternalSection.m_pTexture->m_pRenderData;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Lightmap_Texture_EnvMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Lightmap_Texture_EnvMap::PreFlush()
{
	// Set up our call
//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(-1.0f));

	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pCurEnvMap = 0;
}

void CRenderShader_Lightmap_Texture_EnvMap::FlushChangeSection(CInternalSection &cSection)
{
	// See if we need to setup the environment map texture
	RTexture* pEnvMap = (RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
	if (m_pCurEnvMap != pEnvMap)
	{
		d3d_SetEnvMapTransform(pEnvMap, 1);
		m_pCurEnvMap = pEnvMap;
	}

	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap), 1, eFS_WorldEnvMapTexMemory);
}

void CRenderShader_Lightmap_Texture_EnvMap::PreFlushBlock(CInternalSection &cSection)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_EnvMap);
}

void CRenderShader_Lightmap_Texture_EnvMap::PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Draw it
		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);
	}

	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_EnvMap);
}

void CRenderShader_Lightmap_Texture_EnvMap::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	d3d_UnsetEnvMapTransform(1);

	LTMatrix mIdentity;
	mIdentity.Identity();
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX*)&mIdentity);

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);

//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(0.0f));
}

void CRenderShader_Lightmap_Texture_EnvMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture_EnvMap);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());
		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)), pRTexture->GetMemoryUse());
	}
}


//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture_EnvBumpMap implementation

bool CRenderShader_Lightmap_Texture_EnvBumpMap::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture_EnvBumpMap::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture_EnvBumpMap::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData)
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap))
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap))
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Which stage each approrpriate thing is installed on
	static const uint32 knTexStage		= 2;
	static const uint32 knBumpStage		= 0;
	static const uint32 knEnvMapStage	= 1;

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	// Stage 0: the bump map
	StageStateSet state0co ( 0, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
	StageStateSet state0ca1( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

	StageStateSet state0aa1( 0, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	StageStateSet state0ao ( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 1: a specular environment map.
	StageStateSet state1co ( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	StageStateSet state1ca1( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	StageStateSet state1ca2( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	StageStateSet state1aa1( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	StageStateSet state1ao ( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 2: the base texture.
	StageStateSet state2co ( 2, D3DTSS_COLOROP, nEnvBlendOp );
	StageStateSet state2ca1( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	StageStateSet state2ca2( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
	StageStateSet state2ao ( 2, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	StageStateSet state2ao1( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	StageStateSet state2ao2( 2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	StageStateSet state20  ( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

	// Stage 3: End of the line
	StageStateSet state30  ( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet state31  ( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	// Note : We're going to assume the TexGen stuff is fine, since the nVidia drivers
	// will fail ValidateDevice for no particular reason

	// Change the texture
	d3d_SetTexture(cSection.m_pTexture[0], knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), knEnvMapStage, eFS_WorldEnvMapTexMemory);

	// Are we kosher?
	uint32 nNumPasses;
	HRESULT hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(knTexStage);
	d3d_DisableTexture(knBumpStage);
	d3d_DisableTexture(knEnvMapStage);

	s_bValidateRequired = false;
	s_bValidateResult = SUCCEEDED(hr);

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::TranslateVertices(SVertex_Lightmap_Texture_EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap_Texture_EnvBumpMap *pCurOut = pOut;
	SVertex_Lightmap_Texture_EnvBumpMap *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];

	// Set up a texture transform to get the detail texture coords
	float aTexMat[2][2];

	CRenderBlockData &cRB = m_aRenderBlocks[m_nCurRB];
	CInternalSection &cISection = m_aSections[cRB.m_aSections[m_nCurSection]];

	SharedTexture *pTexture = cISection.m_pTexture;
	RTexture *pRTex = LTNULL;
	if (pTexture)
	{
		pRTex = reinterpret_cast<RTexture*>(pTexture->m_pRenderData);
	}
	if (pRTex)
	{
		float fScale = pRTex->m_DetailTextureScale * g_CV_EnvBumpMapScale.m_Val;
		aTexMat[0][0] = fScale * pRTex->m_DetailTextureAngleC;
		aTexMat[0][1] = fScale * -pRTex->m_DetailTextureAngleS;
		aTexMat[1][0] = fScale * pRTex->m_DetailTextureAngleS;
		aTexMat[1][1] = fScale * pRTex->m_DetailTextureAngleC;
	}
	else
	{
		ASSERT(!"Error setting detail texture coordinates (bad texture)");
		// Initialize to identity * g_CV_DetailTextureScale in failure cases
		aTexMat[0][0] = g_CV_EnvBumpMapScale.m_Val;
		aTexMat[0][1] = 0.0f;
		aTexMat[1][0] = 0.0f;
		aTexMat[1][1] = g_CV_EnvBumpMapScale.m_Val;
	}

	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_fU0 = pCurIn->m_fU0 * aTexMat[0][0] + pCurIn->m_fV0 * aTexMat[0][1];
		pCurOut->m_fV0 = pCurIn->m_fU0 * aTexMat[1][0] + pCurIn->m_fV0 * aTexMat[1][1];
        pCurOut->m_fW0 = 1.0f;
		pCurOut->m_fU1 = pCurIn->m_fU0;
		pCurOut->m_fV1 = pCurIn->m_fV0;
        pCurOut->m_fW1 = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::FillSection(CSection_Lightmap_Texture_EnvBumpMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cInternalSection.m_pTexture->m_pRenderData;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::PreFlush()
{
	// Set up our call
//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(-1.0f));

	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	// Stage 0: the bump map
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 1: a specular environment map.
	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 2: the base texture.
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

	// Stage 3: End of the line
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	m_pCurEnvMap = 0;
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::FlushChangeSection(CInternalSection &cSection)
{
	// See if we need to setup the environment map texture
	RTexture* pCurrRTexture = (RTexture*)cSection.m_pTexture->m_pRenderData;
	if (pCurrRTexture != m_pCurEnvMap)
	{
		RTexture* pEnvMap = (RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
		d3d_SetEnvMapTransform(pEnvMap, knEnvMapStage);

		RTexture* pBumpMap = (RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)->m_pRenderData;
		d3d_SetBumpMapTransform(knBumpStage, pBumpMap->m_DetailTextureScale);

		m_pCurEnvMap = pCurrRTexture;
	}

	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap), knEnvMapStage, eFS_WorldEnvMapTexMemory);

}

void CRenderShader_Lightmap_Texture_EnvBumpMap::PreFlushBlock(CInternalSection &cSection)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(3, TSChannel_Base, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Draw it
		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);
	}

	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(3, TSChannel_Base, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::PostFlush()
{
	d3d_DisableTexture(knTexStage);
	d3d_DisableTexture(knBumpStage);
	d3d_DisableTexture(knEnvMapStage);

	d3d_UnsetEnvMapTransform(knEnvMapStage);

	LTMatrix mIdentity;
	mIdentity.Identity();
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX*)&mIdentity);

	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2 );

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);

//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(0.0f));
}

void CRenderShader_Lightmap_Texture_EnvBumpMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture_EnvBumpMap);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());

		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)), pRTexture->GetMemoryUse());

		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)), pRTexture->GetMemoryUse());
	}
}




//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture_DualTexture implementation

bool CRenderShader_Lightmap_Texture_DualTexture::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture_DualTexture::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture_DualTexture::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0] || !cSection.m_pTexture[1])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData || !cSection.m_pTexture[1]->m_pRenderData)
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	StageStateSet state00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	StageStateSet state03(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet state04(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	StageStateSet state05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	StageStateSet tss10(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	StageStateSet tss11(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	StageStateSet state12(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);

	StageStateSet state13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state14(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet state15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

	StageStateSet state20(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state21(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[1], 1, eFS_WorldBaseTexMemory);

	// Are we kosher?
	uint32 nNumPasses;
	HRESULT hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	s_bValidateRequired = false;
	s_bValidateResult = SUCCEEDED(hr);

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture_DualTexture::TranslateVertices(SVertex_Lightmap_Texture_DualTexture *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap_Texture_DualTexture *pCurOut = pOut;
	SVertex_Lightmap_Texture_DualTexture *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];

	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_nColor = pCurIn->m_nColor;
		pCurOut->m_fU0 = pCurIn->m_fU0;
		pCurOut->m_fV0 = pCurIn->m_fV0;
        pCurOut->m_fW0 = 1.0f;
		pCurOut->m_fU1 = pCurIn->m_fU1;
		pCurOut->m_fV1 = pCurIn->m_fV1;
        pCurOut->m_fW1 = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Lightmap_Texture_DualTexture::FillSection(CSection_Lightmap_Texture_DualTexture &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture0 = cSection.m_pTexture[0];
	cInternalSection.m_pTexture1 = cSection.m_pTexture[1];
	cInternalSection.m_bAdditive = ((RTexture*)cSection.m_pTexture[1]->m_pRenderData)->IsFullbrite();
}

void CRenderShader_Lightmap_Texture_DualTexture::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Lightmap_Texture_DualTexture::PreFlush()
{
	// Set up our call
//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(-1.0f));

	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CRenderShader_Lightmap_Texture_DualTexture::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture0, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture1, 1, eFS_WorldBaseTexMemory);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, (cSection.m_bAdditive) ? D3DTOP_MODULATEALPHA_ADDCOLOR : D3DTOP_BLENDCURRENTALPHA);
}

void CRenderShader_Lightmap_Texture_DualTexture::PreFlushBlock(CInternalSection &cSection)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_DualTexture);
}

void CRenderShader_Lightmap_Texture_DualTexture::PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_DualTexture);
}

void CRenderShader_Lightmap_Texture_DualTexture::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);

//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(0.0f));
}

void CRenderShader_Lightmap_Texture_DualTexture::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture_DualTexture);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture0->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture0), pRTexture->GetMemoryUse());

		pRTexture = (const RTexture*)cSection.m_pTexture1->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture1), pRTexture->GetMemoryUse());
	}
}




/***********************************************************************************************************/
//
//
//   DOT3BUMPMAP
//
//
//*********************************************************************************************************



//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture_DOT3BumpMap implementation

bool CRenderShader_Lightmap_Texture_DOT3BumpMap::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture_DOT3BumpMap::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture_DOT3BumpMap::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData)
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap))
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);


	// Stage 0: bump map

	StageStateSet state0co (0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
	StageStateSet state0ca1(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state0ca2(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);

	StageStateSet state0ao(0, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3);
	StageStateSet state0aa1(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state0aa2(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

	// Stage 1: base texture
	StageStateSet state1co ( 1, D3DTSS_COLOROP, g_CV_DOT3Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
	StageStateSet state1ca1( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	StageStateSet state1ca2( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	StageStateSet state1ao ( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	StageStateSet state1aa1( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	StageStateSet state1aa2( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	// Stage 3: End of the line
	StageStateSet state40  ( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet state41  ( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );


	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	s_bValidateRequired = false;
	s_bValidateResult = true;
	if (FAILED(hr))
	{
		s_bValidateResult = false;
	}

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture_DOT3BumpMap::TranslateVertices(SVertex_Lightmap_Texture_DOT3BumpMap *pOut, const SRBVertex *pIn, uint32 nCount)
{

	SVertex_Lightmap_Texture_DOT3BumpMap *pCurOut = pOut;
	SVertex_Lightmap_Texture_DOT3BumpMap *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];

	while (pCurOut != pEndOut)
	{
		// only one set of coords
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_fU0 = pCurIn->m_fU0;
		pCurOut->m_fV0 = pCurIn->m_fV0;
		pCurOut->m_fW0 = 1.0f;

		++pCurOut;
		++pCurIn;
	}


}

void CRenderShader_Lightmap_Texture_DOT3BumpMap::FillSection(CSection_Lightmap_Texture_DOT3BumpMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Lightmap_Texture_DOT3BumpMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{

	QueueRenderBlock(nRenderBlock);

}

void CRenderShader_Lightmap_Texture_DOT3BumpMap::PreFlush()
{

	// Set up our call
	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);


	// save the tfactor before setting it
	PD3DDEVICE->GetRenderState( D3DRS_TEXTUREFACTOR, &m_nOldTFactor);

	// Create a light vector for ambient lighting ( light points straight on the surface ) 
	LTVector vLight(-1.0f,0.0f,1.0f);

	// Set the TFactor to the vector to rgb for DOT3 mapping 
	PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, d3d_VectorToRGB(&vLight) );

	// Stage 1: the bump map.
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );		
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );

	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );

	// Stage 2: base texture
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_DOT3Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );  // ??? DIFFUSE ????

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0 );

	// Stage 4: End of the line
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

}



void CRenderShader_Lightmap_Texture_DOT3BumpMap::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);

}



void CRenderShader_Lightmap_Texture_DOT3BumpMap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(1, TSChannel_Base);
}



void CRenderShader_Lightmap_Texture_DOT3BumpMap::PostFlushBlock(CInternalSection &cSection, uint32 nStartIndex, uint32 nEndIndex,uint32 nStartVertex, uint32 nEndVertex )
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Draw it
		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base );
}




void CRenderShader_Lightmap_Texture_DOT3BumpMap::PostFlush()
{
	d3d_DisableTexture(knTexStage);
	d3d_DisableTexture(knBumpStage);

	// Restore the TFactor changed for bump mapping
	PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, m_nOldTFactor );

	// restore coords index
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);

}



void CRenderShader_Lightmap_Texture_DOT3BumpMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture_DOT3BumpMap);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());

		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)), pRTexture->GetMemoryUse());
	}
}





/***********************************************************************************************************/
//
//
//   DOT3ENVBUMPMAP
//
//
//*********************************************************************************************************



//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Lightmap_Texture_DOT3EnvBumpMap implementation

bool CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::s_bValidateRequired = true;
bool CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::s_bValidateResult = false;

bool CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::ValidateShader(const CRBSection &cSection)
{
	// Check the section first
	if (!cSection.m_pTexture[0])
		return false;

	if (!cSection.m_pTexture[0]->m_pRenderData)
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap))
		return false;

	if (!cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap))
		return false;

	// Make sure we've got a device
	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		s_bValidateRequired = true;
		return false;
	}

	// Use our previous results if we already know about this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), knEnvMapStage, eFS_WorldEnvMapTexMemory);


	//find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

	// Try out the states
	StateSet alpha0(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet alpha1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	StateSet alpha2(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	StateSet ssFixMultipassFog(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);

	// Stage 0: bump map

	StageStateSet state0co (0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
	StageStateSet state0ca1(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state0ca2(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);

	StageStateSet state0ao(0, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3);
	StageStateSet state0aa1(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state0aa2(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

	// Stage 1: base texture
	StageStateSet state1co ( 1, D3DTSS_COLOROP, g_CV_DOT3Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
	StageStateSet state1ca1( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	StageStateSet state1ca2( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	StageStateSet state1ao ( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	StageStateSet state1aa1( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	StageStateSet state1aa2( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	// Stage 2: the environment map
	StageStateSet state2co ( 2, D3DTSS_COLOROP, nEnvBlendOp );
	StageStateSet state2ca1( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	StageStateSet state2ca2( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
	StageStateSet state2ao ( 2, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	StageStateSet state2ao1( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ); 
	StageStateSet state2ao2( 2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	// Stage 4: End of the line
	StageStateSet state40  ( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet state41  ( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );


	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);
	d3d_DisableTexture(2);

	s_bValidateRequired = false;
	s_bValidateResult = true;
	if (FAILED(hr))
	{
		s_bValidateResult = false;
	}

	return s_bValidateResult;
}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::TranslateVertices(SVertex_Lightmap_Texture_DOT3EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Lightmap_Texture_DOT3EnvBumpMap *pCurOut = pOut;
	SVertex_Lightmap_Texture_DOT3EnvBumpMap *pEndOut = &pCurOut[nCount];

	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];

	// Set up a texture transform to get the detail texture coords
	float aTexMat[2][2];

	CRenderBlockData &cRB = m_aRenderBlocks[m_nCurRB];
	CInternalSection &cISection = m_aSections[cRB.m_aSections[m_nCurSection]];

	SharedTexture *pTexture = cISection.m_pTexture;
	RTexture *pRTex = LTNULL;
	if (pTexture)
	{
		pRTex = reinterpret_cast<RTexture*>(pTexture->m_pRenderData);
	}
	if (pRTex)
	{
		float fScale = pRTex->m_DetailTextureScale * g_CV_EnvBumpMapScale.m_Val;
		aTexMat[0][0] = fScale * pRTex->m_DetailTextureAngleC;
		aTexMat[0][1] = fScale * -pRTex->m_DetailTextureAngleS;
		aTexMat[1][0] = fScale * pRTex->m_DetailTextureAngleS;
		aTexMat[1][1] = fScale * pRTex->m_DetailTextureAngleC;
	}
	else
	{
		ASSERT(!"Error setting detail texture coordinates (bad texture)");
		// Initialize to identity * g_CV_DetailTextureScale in failure cases
		aTexMat[0][0] = g_CV_EnvBumpMapScale.m_Val;
		aTexMat[0][1] = 0.0f;
		aTexMat[1][0] = 0.0f;
		aTexMat[1][1] = g_CV_EnvBumpMapScale.m_Val;
	}

	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_fU0 = pCurIn->m_fU0 * aTexMat[0][0] + pCurIn->m_fV0 * aTexMat[0][1];
		pCurOut->m_fV0 = pCurIn->m_fU0 * aTexMat[1][0] + pCurIn->m_fV0 * aTexMat[1][1];
      pCurOut->m_fW0 = 1.0f;

		pCurOut->m_fU1 = pCurIn->m_fU0;
		pCurOut->m_fV1 = pCurIn->m_fV0;
      pCurOut->m_fW1 = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::FillSection(CSection_Lightmap_Texture_DOT3EnvBumpMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{

	QueueRenderBlock(nRenderBlock);

}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::PreFlush()
{
	// Find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

	// save the tfactor before setting it
	PD3DDEVICE->GetRenderState( D3DRS_TEXTUREFACTOR, &m_nOldTFactor);

	// Create a light vector for ambient lighting ( light points straight on the surface ) 
	LTVector vLight(-1.0f,0.0f,1.0f);

	// Set the TFactor to the vector to rgb for DOT3 mapping 
	PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, d3d_VectorToRGB(&vLight) );


	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	PD3DDEVICE->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	PD3DDEVICE->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (DWORD*)&m_nFlushStateFogColor);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, g_CV_Saturate ? 0x808080 : 0xFFFFFF);


	//Stage 1: Bump map
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
		
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3 );
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );

	// Stage 2: base texture
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_DOT3Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,  D3DTSS_TCI_PASSTHRU | 0 );

	// Stage 1: a specular environment map.
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, nEnvBlendOp  );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_ADD );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	
	// Stage 4: End of the line
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	m_pCurEnvMap = 0;
}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::FlushChangeSection(CInternalSection &cSection)
{
	// See if we need to setup the environment map texture
	RTexture* pCurrRTexture = (RTexture*)cSection.m_pTexture->m_pRenderData;
	if (pCurrRTexture != m_pCurEnvMap)
	{
		RTexture* pEnvMap = (RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
		d3d_SetEnvMapTransform(pEnvMap, knEnvMapStage);
		
		m_pCurEnvMap = pCurrRTexture;
	}

	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap), knEnvMapStage, eFS_WorldEnvMapTexMemory);

}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(3, TSChannel_Base, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Draw it
		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);

		PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(3, TSChannel_Base, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::PostFlush()
{
	d3d_DisableTexture(knTexStage);
	d3d_DisableTexture(knBumpStage);
	d3d_DisableTexture(knEnvMapStage);

	// Make sure to turn off the transform
	d3d_UnsetEnvMapTransform(knEnvMapStage);

	// Restore the TFactor changed for bump mapping
	PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, m_nOldTFactor );


	LTMatrix mIdentity;
	mIdentity.Identity();
	PD3DDEVICE->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+knEnvMapStage), (D3DMATRIX*)&mIdentity);

	PD3DDEVICE->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, m_nFlushStateAlpha0);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, m_nFlushStateAlpha1);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, m_nFlushStateAlpha2);
	PD3DDEVICE->SetRenderState(D3DRS_FOGCOLOR, m_nFlushStateFogColor);

}

void CRenderShader_Lightmap_Texture_DOT3EnvBumpMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Lightmap_Texture_DOT3EnvBumpMap);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());

		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)), pRTexture->GetMemoryUse());

		pRTexture = (const RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)), pRTexture->GetMemoryUse());
	}
}






