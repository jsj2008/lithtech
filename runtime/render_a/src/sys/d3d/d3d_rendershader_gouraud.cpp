//////////////////////////////////////////////////////////////////////////////
// Gouraud render shader implementations

#include "precompile.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "d3d_rendershader_gouraud.h"
#include "d3d_device.h"
#include "d3d_draw.h"
#include "common_draw.h"
#include "common_stuff.h"
#include "d3d_texture.h"
#include "renderstruct.h"
#include "3d_ops.h"
#include "memstats_world.h"

#include "d3d_rendershader_dynamiclight.h"

#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
// Utility functions

// Given the render state, this will determine if dynamic lights should be drawn
bool ShouldDrawLights(const CRenderShader::DrawState& cState)
{
	// Determine if we are drawing dynamic lights
	if (cState.m_nNumLights == 0)
		return false;

	uint32 nOldAlphaBlendEnable;
	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&nOldAlphaBlendEnable);
	return (nOldAlphaBlendEnable == 0);
}


//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud implementation

bool CRenderShader_Gouraud::ValidateShader(const CRBSection &cSection)
{
	// Note : This shader ALWAYS has to return true, because it's the base fallback
	return true;
}

void CRenderShader_Gouraud::TranslateVertices(SVertex_Gouraud *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud *pCurOut = pOut;
	SVertex_Gouraud *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_nColor = pCurIn->m_nColor;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Gouraud::DrawLights(const DrawState &cState, uint32 nRenderBlock)
{
	// Set up our call
	PD3DDEVICE->SetIndices(m_pIB);
	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(k_SVertex_Gouraud_FVF);

	d3d_DisableTexture(0);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_ADD : D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Draw it
	CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
	CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
	for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
	{
		CInternalSection &cSection = m_aSections[*iCurSection];

		ChangeSection(cSection);

		// Draw it
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, cSection.m_nStartVertex, cSection.m_nVertexCount, 
			cSection.m_nStartIndex, cSection.m_nTriCount);

		// Count it
		IncFrameStat(eFS_ShaderTris, cSection.m_nTriCount);
		IncFrameStat(eFS_ShaderVerts, cSection.m_nVertexCount);
		IncFrameStat(eFS_ShaderDrawPrim, 1);

		IncFrameStat(eFS_DynamicLightTriangles, cSection.m_nTriCount);
	}

	DrawLightList(cState, nRenderBlock);

	PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
	PD3DDEVICE->SetIndices(0);
}

void CRenderShader_Gouraud::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	// Draw the lights if we have them and we aren't translucent
	if (ShouldDrawLights(cState))
	{
		uint32 nOldAlphaBlendEnable;
		PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&nOldAlphaBlendEnable);
		
		if (nOldAlphaBlendEnable == 0)
		{
			DrawLights(cState, nRenderBlock);
			return;
		}
	}

	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Gouraud::PreFlush()
{
	d3d_DisableTexture(0);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_ADD : D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CRenderShader_Gouraud::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_Texture implementation

bool CRenderShader_Gouraud_Texture::s_bValidateRequired = true;
bool CRenderShader_Gouraud_Texture::s_bValidateResult = false;

bool CRenderShader_Gouraud_Texture::ValidateShader(const CRBSection &cSection)
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

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet state1(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	// Set up the texture stages
	StageStateSet tss0(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss1(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet tss2(0, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	StageStateSet tss3(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss4(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet tss5(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	StageStateSet tss6(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss7(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
	d3d_DisableTexture(0);

	s_bValidateRequired = false;
	s_bValidateResult = !FAILED(hr);
	return s_bValidateResult;
}

void CRenderShader_Gouraud_Texture::TranslateVertices(SVertex_Gouraud_Texture *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_Texture *pCurOut = pOut;
	SVertex_Gouraud_Texture *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_nColor = pCurIn->m_nColor;

		pCurOut->m_fU = pCurIn->m_fU0;
		pCurOut->m_fV = pCurIn->m_fV0;
        pCurOut->m_fW = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Gouraud_Texture::FillSection(CSection_Gouraud_Texture &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_Texture::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	if (ShouldDrawLights(cState))
	{
		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_Texture_FVF);

		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);

		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

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

				// Just Draw it
				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
						0, cSection.m_nStartVertex, cSection.m_nVertexCount,
						cSection.m_nStartIndex, cSection.m_nTriCount);

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}

			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);

			d3d_DisableTexture(0);
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_Texture::PreFlush()
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, (g_CV_Saturate) ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;
}

void CRenderShader_Gouraud_Texture::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_Texture::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(1, TSChannel_Base);					
}

void CRenderShader_Gouraud_Texture::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
 		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state2over(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base);
}

void CRenderShader_Gouraud_Texture::PostFlush()
{
	d3d_DisableTexture(0);

	// disable the textures now if they exist...
	d3d_DisableTexture(1);
	d3d_DisableTexture(2);
	d3d_DisableTexture(3);
	d3d_DisableTexture(4);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_Texture::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_Texture);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());
	}
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_Detail implementation

bool CRenderShader_Gouraud_Detail::s_bValidateRequired = true;
bool CRenderShader_Gouraud_Detail::s_bValidateResult = false;
bool CRenderShader_Gouraud_Detail::s_bDisableAlpha = false;

bool CRenderShader_Gouraud_Detail::ValidateShader(const CRBSection &cSection)
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

	// Shortcut out if we've already tested it on this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	// Try it the legal way
	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet tss02(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	StageStateSet tss03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet tss05(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	StageStateSet tss10(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss11(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss12(1, D3DTSS_COLOROP, g_CV_DetailTextureAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	StageStateSet tss13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss14(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	StageStateSet tss20(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	StageStateSet tss21(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss22(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	StageStateSet tss23(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss24(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss25(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	StageStateSet tss30(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss31(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	s_bValidateRequired = false;
	s_bValidateResult = true;

	if (FAILED(hr))
	{
		bool bValid = true;

		// Try it using the caps approach

		// Check the operations first
		if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTOP_MODULATE))	bValid = false;
		if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTOP_ADDSIGNED))	bValid = false;
		if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTOP_MODULATE2X))	bValid = false;
		if (!(g_Device.GetDeviceCaps()->TextureOpCaps & D3DTOP_SELECTARG2))	bValid = false;

		// Check the number of states
		if (g_Device.GetDeviceCaps()->MaxTextureBlendStages < 3) bValid = false;

		if (!bValid)
		{
			// Looks like this won't work no matter what...
			s_bValidateResult = false;
		}
		s_bDisableAlpha = true;
	}
	else
		s_bDisableAlpha = false;

	return s_bValidateResult;
}

void CRenderShader_Gouraud_Detail::TranslateVertices(SVertex_Gouraud_Detail *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_Detail *pCurOut = pOut;
	SVertex_Gouraud_Detail *pEndOut = &pCurOut[nCount];
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
		pCurOut->m_nColor = pCurIn->m_nColor;
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

void CRenderShader_Gouraud_Detail::FillSection(CSection_Gouraud_Detail &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_Detail::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	if (ShouldDrawLights(cState))
	{
		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_Detail_FVF);

		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);

		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

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
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

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

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}

			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);

			d3d_DisableTexture(0);
			d3d_DisableTexture(1);

		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_Detail::PreFlush()
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_DetailTextureAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, s_bDisableAlpha ? D3DTOP_DISABLE : D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;
}

void CRenderShader_Gouraud_Detail::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_Detail::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_Detail);					
}

void CRenderShader_Gouraud_Detail::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state12over(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet state15over(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_Detail);
}

void CRenderShader_Gouraud_Detail::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_Detail::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_Detail);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);

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
// CRenderShader_Gouraud_EnvMap implementation

bool CRenderShader_Gouraud_EnvMap::s_bValidateRequired = true;
bool CRenderShader_Gouraud_EnvMap::s_bValidateResult = false;
bool CRenderShader_Gouraud_EnvMap::s_bValidAlphaMask = false;
bool CRenderShader_Gouraud_EnvMap::s_bAlphaMaskUseDualPass = false;
bool CRenderShader_Gouraud_EnvMap::s_bUseDualPass = false;

CRenderShader_Gouraud_EnvMap::CRenderShader_Gouraud_EnvMap(bool bAlphaMask) :
	m_bAlphaMask(bAlphaMask)
{
}


bool CRenderShader_Gouraud_EnvMap::ValidateShader(const CRBSection &cSection)
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
	if (!s_bValidateRequired && !s_bValidateResult)
		return false;

	// Load up the textures for validation
	if (!d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory))
		return false;
	if (!d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), 1, eFS_WorldEnvMapTexMemory))
		return false;

	if (!s_bValidateRequired && s_bValidateResult)
		return true;

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	//find the operation we want to use to blend the environment map
	DWORD nStage1Op = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;
	DWORD nStage2Op = g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE;

	// Try it the legal way
	StageStateSet state00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet state02(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state03(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	StageStateSet state10(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	StageStateSet state11(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	StageStateSet state12(1, D3DTSS_COLOROP, nStage1Op); 
	StageStateSet state13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state14(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	
	StageStateSet state20(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	StageStateSet state21(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state22(2, D3DTSS_COLOROP, nStage2Op);
	StageStateSet state23(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet state24(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	StageStateSet state30(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state31(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Don't setup the actual transform info since nVidia cards will fail if this is done...
	// dunno why. Assume it works

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
	
	s_bValidateRequired = false;
	s_bValidateResult = true;


	if (FAILED(hr))
	{
		// Now try setting it up for dual pass if needed
		StageStateSet ssOverride2cop(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet ssOverride2aop(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
		if (FAILED(hr))
		{
			// We can't do this at all
			s_bValidateResult = false;
			return false;
		}
		else
		{
			s_bUseDualPass = true;
		}
	}

	// We also need to do a test for if we can apply alpha masked environment map
	StageStateSet AlphaMaskTSS12(1, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR);
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	if (FAILED(hr))
	{
		// Lets try a dual pass appropach
		// Now try setting it up for dual pass if needed
		StageStateSet ssOverride2cop(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet ssOverride2aop(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
		if (FAILED(hr))
		{
			//we can't do this at all
			s_bValidAlphaMask = false;
			return false;
		}
		else
		{
			s_bAlphaMaskUseDualPass = true;
		}
	}
	else
	{
		s_bValidAlphaMask = true;
	}

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	// Now see which blends we can handle
	if (m_bAlphaMask)
		return s_bValidateResult && s_bValidAlphaMask;
	else
		return s_bValidateResult;
}

void CRenderShader_Gouraud_EnvMap::TranslateVertices(SVertex_Gouraud_EnvMap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_EnvMap *pCurOut = pOut;
	SVertex_Gouraud_EnvMap *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_nColor = pCurIn->m_nColor;
		pCurOut->m_fU = pCurIn->m_fU0;
		pCurOut->m_fV = pCurIn->m_fV0;
        pCurOut->m_fW = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Gouraud_EnvMap::FillSection(CSection_Gouraud_EnvMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_EnvMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	// Determine if we need to do a dual pass
	bool bUseDualPass = ShouldDrawLights(cState);

	if (m_bAlphaMask)
		bUseDualPass |= s_bAlphaMaskUseDualPass;

	if (bUseDualPass)
	{
		// Find the operation we want to use to blend the environment map
		DWORD nStage1Op;

		if (m_bAlphaMask)
			nStage1Op = D3DTOP_MODULATEALPHA_ADDCOLOR;
		else
			nStage1Op = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_EnvMap_FVF);

		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);

		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, nStage1Op); 

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

			m_pCurEnvMap = NULL;
		
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

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}

			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);

			d3d_DisableTexture(0);
			d3d_DisableTexture(1);

			// Make sure to turn off the transform
			d3d_UnsetEnvMapTransform(1);
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		LTMatrix mIdentity;
		mIdentity.Identity();
		PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, (D3DMATRIX*)&mIdentity);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_EnvMap::PreFlush()
{
	// Find the operation we want to use to blend the environment map
	DWORD nStage1Op;

	if (m_bAlphaMask)
		nStage1Op = D3DTOP_MODULATEALPHA_ADDCOLOR;
	else
		nStage1Op = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, nStage1Op); 
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;

	m_pCurEnvMap = 0;
}

void CRenderShader_Gouraud_EnvMap::FlushChangeSection(CInternalSection &cSection)
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

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_EnvMap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_EnvMap);
}

void CRenderShader_Gouraud_EnvMap::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
 		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state12over(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet state15over(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_EnvMap);
}

void CRenderShader_Gouraud_EnvMap::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	// Make sure to turn off the transform
	d3d_UnsetEnvMapTransform(1);

	LTMatrix mIdentity;
	mIdentity.Identity();
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, (D3DMATRIX*)&mIdentity);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_EnvMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_EnvMap);
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
	}
}


//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_EnvBumpMap implementation

bool CRenderShader_Gouraud_EnvBumpMap::s_bValidateRequired = true;
bool CRenderShader_Gouraud_EnvBumpMap::s_bValidateResult = false;

bool CRenderShader_Gouraud_EnvBumpMap::ValidateShader(const CRBSection &cSection)
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
	d3d_SetTexture(cSection.m_pTexture[0], 2, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_BumpMap), 0, eFS_WorldBumpMapTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), 1, eFS_WorldEnvMapTexMemory);

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	//find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

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

	// Stage 3: the diffuse channel.
	StageStateSet state3ca1( 3, D3DTSS_COLORARG1, D3DTA_CURRENT );
	StageStateSet state3ca2( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	StageStateSet state3co ( 3, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );

	StageStateSet state3aa1( 3, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	StageStateSet state3ao ( 3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 4: End of the line
	StageStateSet state40  ( 4, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet state41  ( 4, D3DTSS_ALPHAOP, D3DTOP_DISABLE );


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

void CRenderShader_Gouraud_EnvBumpMap::TranslateVertices(SVertex_Gouraud_EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_EnvBumpMap *pCurOut = pOut;
	SVertex_Gouraud_EnvBumpMap *pEndOut = &pCurOut[nCount];

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
		pCurOut->m_nColor = pCurIn->m_nColor;
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

void CRenderShader_Gouraud_EnvBumpMap::FillSection(CSection_Gouraud_EnvBumpMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_EnvBumpMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	if (ShouldDrawLights(cState))
	{
		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_EnvBumpMap_FVF);

		// Find the operation we want to use to blend the environment map
		DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);

		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

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
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, nEnvBlendOp );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ); 
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

			// Stage 3: End of the line
			PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
			PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

			m_pCurEnvMap = NULL;
		
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

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
				
				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}

			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2 );

			d3d_DisableTexture(knTexStage);
			d3d_DisableTexture(knBumpStage);
			d3d_DisableTexture(knEnvMapStage);
			
			// Make sure to turn off the transform
			d3d_UnsetEnvMapTransform(knEnvMapStage);
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		LTMatrix mIdentity;
		mIdentity.Identity();
		PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, (D3DMATRIX*)&mIdentity);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_EnvBumpMap::PreFlush()
{
	// Find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

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
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, nEnvBlendOp );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ); 
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

	// Stage 3: the diffuse channel.
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );

	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 4: End of the line
	PD3DDEVICE->SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_DISABLE );
	PD3DDEVICE->SetTextureStageState( 4, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;

	m_pCurEnvMap = 0;
}

void CRenderShader_Gouraud_EnvBumpMap::FlushChangeSection(CInternalSection &cSection)
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

	RTexture* pBumpMap = (RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap)->m_pRenderData;
	// Set the blend mode appropriately
	PD3DDEVICE->SetTextureStageState( knBumpStage, D3DTSS_COLOROP, pBumpMap->IsLumBumpMap() ? D3DTOP_BUMPENVMAPLUMINANCE : D3DTOP_BUMPENVMAP);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_EnvBumpMap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(3, TSChannel_Base, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Gouraud_EnvBumpMap::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
 		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state12over(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet state15over(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(3, TSChannel_Base, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Gouraud_EnvBumpMap::PostFlush()
{
	d3d_DisableTexture(knTexStage);
	d3d_DisableTexture(knBumpStage);
	d3d_DisableTexture(knEnvMapStage);

	// Make sure to turn off the transform
	d3d_UnsetEnvMapTransform(knEnvMapStage);

	LTMatrix mIdentity;
	mIdentity.Identity();
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, (D3DMATRIX*)&mIdentity);
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2 );

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_EnvBumpMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_EnvBumpMap);
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


//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_DualTexture implementation

bool CRenderShader_Gouraud_DualTexture::s_bValidateRequired = true;
bool CRenderShader_Gouraud_DualTexture::s_bValidateResult = false;
bool CRenderShader_Gouraud_DualTexture::s_bTwoPass = false;

bool CRenderShader_Gouraud_DualTexture::ValidateShader(const CRBSection &cSection)
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

	// Shortcut out if we've already tested it on this hardware
	if (!s_bValidateRequired)
		return s_bValidateResult;

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[1], 1, eFS_WorldBaseTexMemory);

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	// Try it the legal way
	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet tss02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	StageStateSet tss03(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss04(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	StageStateSet tss05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	StageStateSet tss10(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	StageStateSet tss11(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	StageStateSet tss12(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);

	StageStateSet tss13(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	StageStateSet tss14(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	StageStateSet tss15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	StageStateSet tss20(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	StageStateSet tss21(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss22(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);

	StageStateSet tss23(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss24(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss25(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

	StageStateSet tss30(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss31(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
	s_bValidateRequired = false;
	s_bValidateResult = true;

	if (FAILED(hr))
	{
		bool bValid = true;

		// Try taking the two pass approach
		StageStateSet tss20o(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet tss21o(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		if (SUCCEEDED(PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses)))
		{
			bValid = true;
			s_bTwoPass = true;
		}

		if (!bValid)
		{
			// Looks like this won't work no matter what...
			s_bValidateResult = false;
		}
	}
	else
		s_bTwoPass = false;

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	return s_bValidateResult;
}

void CRenderShader_Gouraud_DualTexture::TranslateVertices(SVertex_Gouraud_DualTexture *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_DualTexture *pCurOut = pOut;
	SVertex_Gouraud_DualTexture *pEndOut = &pCurOut[nCount];
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

void CRenderShader_Gouraud_DualTexture::FillSection(CSection_Gouraud_DualTexture &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture0 = cSection.m_pTexture[0];
	cInternalSection.m_pTexture1 = cSection.m_pTexture[1];
	cInternalSection.m_bAdditive = ((RTexture*)cSection.m_pTexture[1]->m_pRenderData)->IsFullbrite();

	RTexture *pRenderTexture = (RTexture*)cInternalSection.m_pTexture0->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
}

void CRenderShader_Gouraud_DualTexture::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	if (ShouldDrawLights(cState) || s_bTwoPass)
	{
		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_DualTexture_FVF);

		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);

		PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
		m_nCurAlphaTest = m_nOldAlphaTest;

		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

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

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

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

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}

			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);

			d3d_DisableTexture(0);
			d3d_DisableTexture(1);
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_DualTexture::PreFlush()
{
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

	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);

	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;

	PD3DDEVICE->GetTextureStageState(1, D3DTSS_COLOROP, (DWORD*)&m_nOldColorOp1);
}

void CRenderShader_Gouraud_DualTexture::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture0, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture1, 1, eFS_WorldBaseTexMemory);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}

	// Handle overriding of the blend mode
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, (cSection.m_bAdditive) ? D3DTOP_MODULATEALPHA_ADDCOLOR : D3DTOP_BLENDCURRENTALPHA);
	
}

void CRenderShader_Gouraud_DualTexture::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_DualTexture);
}

void CRenderShader_Gouraud_DualTexture::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_DualTexture);
}

void CRenderShader_Gouraud_DualTexture::PostFlush()
{
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, m_nOldColorOp1);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_DualTexture::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_DualTexture);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);

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
// CRenderShader_Gouraud_DOT3BumpMap implementation

bool CRenderShader_Gouraud_DOT3BumpMap::s_bValidateRequired = true;
bool CRenderShader_Gouraud_DOT3BumpMap::s_bValidateResult = false;

bool CRenderShader_Gouraud_DOT3BumpMap::ValidateShader(const CRBSection &cSection)
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

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;


	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

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

	// Stage 2: the diffuse channel.
	StageStateSet state2co ( 2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
	StageStateSet state2ca1( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
	StageStateSet state2ca2( 2, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	StageStateSet state2ao ( 2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	StageStateSet state2aa1( 2, D3DTSS_ALPHAARG1, D3DTA_CURRENT );

	// Stage 3: End of the line
	StageStateSet state40  ( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet state41  ( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );


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

void CRenderShader_Gouraud_DOT3BumpMap::TranslateVertices(SVertex_Gouraud_DOT3BumpMap *pOut, const SRBVertex *pIn, uint32 nCount)
{

	SVertex_Gouraud_DOT3BumpMap *pCurOut = pOut;
	SVertex_Gouraud_DOT3BumpMap *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];

	while (pCurOut != pEndOut)
	{
		// only one set of coords
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_nColor = pCurIn->m_nColor;
		pCurOut->m_fU0 = pCurIn->m_fU0;
		pCurOut->m_fV0 = pCurIn->m_fV0;
		pCurOut->m_fW0 = 1.0f;

		++pCurOut;
		++pCurIn;
	}


}

void CRenderShader_Gouraud_DOT3BumpMap::FillSection(CSection_Gouraud_DOT3BumpMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_DOT3BumpMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	if (ShouldDrawLights(cState))
	{
		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_DOT3BumpMap_FVF);


		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);
		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

			// save the tfactor before setting it
			PD3DDEVICE->GetRenderState( D3DRS_TEXTUREFACTOR, &m_nOldTFactor);

			// NOTE: we are not updating the vectors based on the dynamic lights ... 
			LTVector vLight(-1.0f,0.0f,1.0f);

			// Set the TFactor to the vector to rgb for DOT3 mapping 
			PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, d3d_VectorToRGB(&vLight) );

			// Stage 0: the bump texture
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );		

			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3 );
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );

			PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX,  0 );

			// Stage 1: the texture
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_DOT3Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0 );

			// Stage 3: End of the line
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

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

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
				
				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}


			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

			d3d_DisableTexture(knTexStage);
			d3d_DisableTexture(knBumpStage);

			// Restore the TFactor changed for DOT3 bump mapping
			PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, m_nOldTFactor );
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_DOT3BumpMap::PreFlush()
{

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
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0 );

	// Stage 3: the diffuse channel. ( colors )
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	// Stage 4: End of the line
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;

}



void CRenderShader_Gouraud_DOT3BumpMap::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, knTexStage, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_BumpMap), knBumpStage, eFS_WorldBumpMapTexMemory);


  	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
  	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
  	}
}



void CRenderShader_Gouraud_DOT3BumpMap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(1, TSChannel_Base);
}



void CRenderShader_Gouraud_DOT3BumpMap::PostFlushBlock(CInternalSection &cSection, uint32 nStartIndex, uint32 nEndIndex,uint32 nStartVertex, uint32 nEndVertex )
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
 		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state12over(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet state15over(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 

		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, nStartVertex, nEndVertex - nStartVertex, nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base);
}




void CRenderShader_Gouraud_DOT3BumpMap::PostFlush()
{
	d3d_DisableTexture(knTexStage);
	d3d_DisableTexture(knBumpStage);

	// Restore the TFactor changed for bump mapping
	PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, m_nOldTFactor );

	// restore coords index
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}



void CRenderShader_Gouraud_DOT3BumpMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_DOT3BumpMap);
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
// CRenderShader_Gouraud_DOT3EnvBumpMap implementation

bool CRenderShader_Gouraud_DOT3EnvBumpMap::s_bValidateRequired = true;
bool CRenderShader_Gouraud_DOT3EnvBumpMap::s_bValidateResult = false;

bool CRenderShader_Gouraud_DOT3EnvBumpMap::ValidateShader(const CRBSection &cSection)
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

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	//find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;


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


	// Stage 3: the diffuse channel.
	StageStateSet state3ca1( 3, D3DTSS_COLORARG1, D3DTA_CURRENT );
	StageStateSet state3ca2( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	StageStateSet state3co ( 3, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );

	StageStateSet state3aa1( 3, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	StageStateSet state3ao ( 3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	// Stage 4: End of the line
	StageStateSet state40  ( 4, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet state41  ( 4, D3DTSS_ALPHAOP, D3DTOP_DISABLE );


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

void CRenderShader_Gouraud_DOT3EnvBumpMap::TranslateVertices(SVertex_Gouraud_DOT3EnvBumpMap *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_DOT3EnvBumpMap *pCurOut = pOut;
	SVertex_Gouraud_DOT3EnvBumpMap *pEndOut = &pCurOut[nCount];

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
		pCurOut->m_nColor = pCurIn->m_nColor;
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

void CRenderShader_Gouraud_DOT3EnvBumpMap::FillSection(CSection_Gouraud_DOT3EnvBumpMap &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	if (ShouldDrawLights(cState))
	{
		// Set up our call
		PD3DDEVICE->SetIndices(m_pIB);
		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(k_SVertex_Gouraud_DOT3EnvBumpMap_FVF);

		// Find the operation we want to use to blend the environment map
		DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

		DWORD nFogEnabled;
		PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &nFogEnabled);

		{
			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

			DrawLights(cState, nRenderBlock);

			PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, nFogEnabled);
			StateSet ssFogColor(D3DRS_FOGCOLOR, 0);

			StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
			StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			StateSet blend2(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

			StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
			StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);


			// NOTE: could ? Should probably update based on the dynamic lights ... 


			// save the tfactor before setting it
			PD3DDEVICE->GetRenderState( D3DRS_TEXTUREFACTOR, &m_nOldTFactor);


			// Create a light vector for ambient lighting ( light points straight on the surface ) 
			LTVector vLight(-1.0f,0.0f,1.0f);

			// Set the TFactor to the vector to rgb for DOT3 mapping 
			PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, d3d_VectorToRGB(&vLight) );


			// Stage 0: the bump texture
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );		

			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DOTPRODUCT3 );
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );

			// Stage 1: the base texture
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_DOT3Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

			PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,  D3DTSS_TCI_PASSTHRU | 0 );

			// Stage 2: a specular environment map.
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLOROP, nEnvBlendOp  );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_ADD );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			PD3DDEVICE->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

			// Stage 3: End of the line
			PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE );
			PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
			m_nCurAlphaTest = m_nOldAlphaTest;

			m_pCurEnvMap = NULL;
		
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

				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
				
				PostFlushBlock(cSection, 
					cSection.m_nStartIndex, cSection.m_nStartIndex + cSection.m_nTriCount * 3,
					cSection.m_nStartVertex, cSection.m_nStartVertex + cSection.m_nVertexCount);
			}


			// Restore everything back to the way we found it - Cleanup follows

			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, m_nOldAlphaTest);

			PD3DDEVICE->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1 );

			// Set the TFactor to previous state 
			PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, m_nOldTFactor);

			d3d_DisableTexture(knTexStage);
			d3d_DisableTexture(knBumpStage);
			d3d_DisableTexture(knEnvMapStage);
			
			// Make sure to turn off the transform
			d3d_UnsetEnvMapTransform(knEnvMapStage);
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		LTMatrix mIdentity;
		mIdentity.Identity();

		PD3DDEVICE->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+knEnvMapStage), (D3DMATRIX*)&mIdentity);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::PreFlush()
{
	// Find the operation we want to use to blend the environment map
	DWORD nEnvBlendOp = g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE;

	// save the tfactor before setting it
	PD3DDEVICE->GetRenderState( D3DRS_TEXTUREFACTOR, &m_nOldTFactor);

	// Create a light vector for ambient lighting ( light points straight on the surface ) 
	LTVector vLight(-1.0f,0.0f,1.0f);

	// Set the TFactor to the vector to rgb for DOT3 mapping 
	PD3DDEVICE->SetRenderState( D3DRS_TEXTUREFACTOR, d3d_VectorToRGB(&vLight) );

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
	
	// Stage 3: the diffuse channel.
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );

	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	PD3DDEVICE->SetTextureStageState( 3, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	// Stage 4: End of the line
	PD3DDEVICE->SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_DISABLE );
	PD3DDEVICE->SetTextureStageState( 4, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;

	m_pCurEnvMap = 0;
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::FlushChangeSection(CInternalSection &cSection)
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

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
  	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
  	}
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(3, TSChannel_Base, TSChannel_Null, TSChannel_Base );
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
 		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state12over(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet state15over(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(3, TSChannel_Base, TSChannel_Null, TSChannel_Base );
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::PostFlush()
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

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_DOT3EnvBumpMap::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_DOT3EnvBumpMap);
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

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_Effect implementation

bool CRenderShader_Gouraud_Effect::s_bValidateRequired = true;
bool CRenderShader_Gouraud_Effect::s_bValidateResult = false;

CRenderShader_Gouraud_Effect::CRenderShader_Gouraud_Effect()
{ 
}

CRenderShader_Gouraud_Effect::~CRenderShader_Gouraud_Effect() 
{ 
	s_bValidateRequired = true;
}

bool CRenderShader_Gouraud_Effect::ValidateShader(const CRBSection &cSection)
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

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);

	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	bool bAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;

	// Set up the alpha states
	StateSet state1(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	// Set up the texture stages
	StageStateSet tss0(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss1(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet tss2(0, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	StageStateSet tss3(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss4(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet tss5(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	StageStateSet tss6(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss7(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
	d3d_DisableTexture(0);

	s_bValidateRequired = false;
	s_bValidateResult = !FAILED(hr);

	// If we have an Effect Shader attached to this section
	// Try to find a valid technique for this effect.
	if(cSection.m_pTexture[0])
	{
		if(cSection.m_pTexture[0]->m_nShaderID != 0)
		{
			LTEffectImpl* _pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(cSection.m_pTexture[0]->m_nShaderID);

			bool bPassed = true;
			if(_pEffect)
			{
				ID3DXEffect* pEffect = _pEffect->GetEffect();
				if(pEffect)
				{
					D3DXHANDLE hTechnique;
					HRESULT hr = pEffect->FindNextValidTechnique(NULL, &hTechnique);
					if(FAILED(hr))
					{						
						bPassed = false;
					}
				}
				else
				{
						bPassed = false;
				}
			}
			else
			{
				bPassed = false;
			}

			if(!bPassed)
			{
				dsi_ConsolePrint("Effect ID %d Failed to find a valid Technique for this device!", cSection.m_pTexture[0]->m_nShaderID);
				s_bValidateResult = false;
			}
		}
	}

	return s_bValidateResult;
}

void CRenderShader_Gouraud_Effect::TranslateVertices(SVertex_Gouraud_Effect *pOut, const SRBVertex *pIn, uint32 nCount)
{
	SVertex_Gouraud_Effect *pCurOut = pOut;
	SVertex_Gouraud_Effect *pEndOut = &pCurOut[nCount];
	const SRBVertex *pCurIn = pIn;
	const SRBVertex *pEndIn = &pCurIn[nCount];
	while (pCurOut != pEndOut)
	{
		pCurOut->m_vPos = pCurIn->m_vPos;
		pCurOut->m_nColor = pCurIn->m_nColor;
		pCurOut->m_vNormal = pCurIn->m_vNormal;
		pCurOut->m_vTangent = pCurIn->m_vTangent;
		pCurOut->m_vBinormal = pCurIn->m_vBinormal;
		pCurOut->m_fU = pCurIn->m_fU0;
		pCurOut->m_fV = pCurIn->m_fV0;
		pCurOut->m_fW = 1.0f;
		++pCurOut;
		++pCurIn;
	}
}

void CRenderShader_Gouraud_Effect::FillSection(CSection_Gouraud_Effect &cInternalSection, const CRBSection &cSection)
{
	cInternalSection.m_pTexture = cSection.m_pTexture[0];
	RTexture *pRenderTexture = (RTexture*)cSection.m_pTexture[0]->m_pRenderData;
	cInternalSection.m_nAlphaTest = pRenderTexture->m_AlphaRef != ALPHAREF_NONE;
	cInternalSection.m_bFullbright = pRenderTexture->IsFullbrite();
}

void CRenderShader_Gouraud_Effect::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
#if 0
	if (ShouldDrawLights(cState))
	{
		//TODO: add support for dynamic lights
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
#endif

	QueueRenderBlock(nRenderBlock);
}

void CRenderShader_Gouraud_Effect::PreFlush()
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, (g_CV_Saturate) ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;
}

void CRenderShader_Gouraud_Effect::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);

	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
	}
}

void CRenderShader_Gouraud_Effect::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(1, TSChannel_Base);					
}

bool CRenderShader_Gouraud_Effect::FlushWithEffect(CInternalSection &cSection, 
													uint32 nStartIndex, uint32 nEndIndex,
													uint32 nStartVertex, uint32 nEndVertex)
{
	// If we have an Effect Shader attached to this section
	//if(cSection.m_pTexture && (cSection.m_pTexture->m_nShaderID != 0))
	//{
		LTEffectImpl* _pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(cSection.m_pTexture->m_nShaderID);
		ID3DXEffect* pEffect = _pEffect->GetEffect();

		if(!pEffect)
		{
			//dsi_ConsolePrint("Error! cSection.m_pTexture->m_nShaderID (%d) is not valid!", cSection.m_pTexture->m_nShaderID);
			return false;
		}

		if(!cSection.m_pTexture)
		{
			dsi_ConsolePrint("Error! cSection.m_pTexture is NULL!");
			return false;
		}

		// Send out vertex decl to DirectX
		_pEffect->UploadVertexDeclaration();

		if(cSection.m_pTexture->m_pRenderData)
		{
			RTexture* pRTexture = (RTexture*)cSection.m_pTexture->m_pRenderData;
			if(pRTexture)
			{
				pEffect->SetTexture("texture0", pRTexture->m_pD3DTexture);
			}
		}

		SharedTexture* pTex1 = cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EffectTexture1);
		if(pTex1)
		{
			d3d_SetTexture(pTex1, 1, eFS_WorldBaseTexMemory);
			RTexture* pRTexture = (RTexture*)pTex1->m_pRenderData;
			if(pRTexture)
			{
				pEffect->SetTexture("texture1", pRTexture->m_pD3DTexture);
			}
		}
		SharedTexture* pTex2 = cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EffectTexture2);
		if(pTex2)
		{
			d3d_SetTexture(pTex2, 2, eFS_WorldBaseTexMemory);
			RTexture* pRTexture = (RTexture*)pTex2->m_pRenderData;
			if(pRTexture)
			{
				pEffect->SetTexture("texture2", pRTexture->m_pD3DTexture);
			}
		}
		SharedTexture* pTex3 = cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EffectTexture3);
		if(pTex3)
		{
			d3d_SetTexture(pTex3, 3, eFS_WorldBaseTexMemory);
			RTexture* pRTexture = (RTexture*)pTex3->m_pRenderData;
			if(pRTexture)
			{
				pEffect->SetTexture("texture3", pRTexture->m_pD3DTexture);
			}
		}
		SharedTexture* pTex4 = cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EffectTexture4);
		if(pTex4)
		{
			d3d_SetTexture(pTex4, 4, eFS_WorldBaseTexMemory);
			RTexture* pRTexture = (RTexture*)pTex2->m_pRenderData;
			if(pRTexture)
			{
				pEffect->SetTexture("texture4", pRTexture->m_pD3DTexture);
			}
		}

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
		uint32 nNumVerts = nEndVertex - nStartVertex; 

		//Ask the game for variables...
		i_client_shell->OnEffectShaderSetParams((LTEffectShader*)_pEffect, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

		UINT cPasses;
		pEffect->Begin(&cPasses, 0);
		for(UINT iPass = 0; iPass < cPasses; iPass++)
		{	
			pEffect->BeginPass(iPass);

			// Draw it
			PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
				0, nStartVertex, nNumVerts,
				nStartIndex, nNumTris);

			DEBUG_PRINT(2,("Brush Section: (%p) Start: (%d) Verts: (%d) sIdx: (%d) Tris: (%d)", &cSection, nStartVertex, nNumVerts, nStartIndex, nNumTris));

			pEffect->EndPass();
		}
		pEffect->End();

		return true;
	//}

	//return false;
}

void CRenderShader_Gouraud_Effect::PostFlushBlock(CInternalSection &cSection, 
												   uint32 nStartIndex, uint32 nEndIndex,
												   uint32 nStartVertex, uint32 nEndVertex)
{
	// Fullbright it
	if (cSection.m_bFullbright)
	{
		StateSet ssAlpha(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet ssSrcBlend(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		StateSet ssDestBlend(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		StageStateSet state2over(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		uint32 nNumTris = (nEndIndex - nStartIndex) / 3; 
		PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
			0, nStartVertex, nEndVertex - nStartVertex,
			nStartIndex, nNumTris);

		IncFrameStat(eFS_WorldTriangles, nNumTris);
	}

	// Clear texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base);
}

void CRenderShader_Gouraud_Effect::PostFlush()
{
	d3d_DisableTexture(0);

	// disable the textures now if they exist...
	d3d_DisableTexture(1);
	d3d_DisableTexture(2);
	d3d_DisableTexture(3);
	d3d_DisableTexture(4);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

void CRenderShader_Gouraud_Effect::GetMemStats(CMemStats_World &cMemStats) const
{
	cMemStats.m_nVertexCount += m_nTotalVertexCount;
	cMemStats.m_nVertexData += m_nTotalVertexCount * sizeof(SVertex_Gouraud_Texture);
	cMemStats.m_nTriangleCount += m_nTotalTriCount;
	cMemStats.m_nTriangleData += m_nTotalTriCount * 3 * sizeof(uint16);

	TSectionList::const_iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		const CInternalSection &cSection = *iCurSection;
		const RTexture* pRTexture = (const RTexture*)cSection.m_pTexture->m_pRenderData;
		cMemStats.CountTexture(g_pStruct->GetTextureName(cSection.m_pTexture), pRTexture->GetMemoryUse());
	}
}
