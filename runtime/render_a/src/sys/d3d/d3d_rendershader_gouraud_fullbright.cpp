//////////////////////////////////////////////////////////////////////////////
// Gouraud+fullbright single-pass render shader implementations

#include "precompile.h"

#include "d3d_rendershader_gouraud_fullbright.h"

#include "d3d_device.h"
#include "d3d_draw.h"
#include "common_draw.h"
#include "common_stuff.h"
#include "d3d_texture.h"
#include "renderstruct.h"

#include "memstats_world.h"

#include "d3d_rendershader_dynamiclight.h"

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_Texture_Fullbright implementation

bool CRenderShader_Gouraud_Texture_Fullbright::s_bValidateRequired = true;
bool CRenderShader_Gouraud_Texture_Fullbright::s_bValidateResult = false;

bool CRenderShader_Gouraud_Texture_Fullbright::ValidateShader(const CRBSection &cSection)
{
	if (!s_bValidateRequired)
		return s_bValidateResult;

	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		return false;
	}

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);

	RTexture *pRenderTexture = (cSection.m_pTexture[0]) ? (RTexture*)cSection.m_pTexture[0]->m_pRenderData : 0;
	bool bAlphaTest = (pRenderTexture) ? pRenderTexture->m_AlphaRef != ALPHAREF_NONE : false;

	// Set up the alpha states
	StateSet state1(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	// Set up the texture stages
	StageStateSet state00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet state02(0, D3DTSS_COLOROP, (g_CV_Saturate) ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	StageStateSet state03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet state05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet state10(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet state11(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet state12(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
	StageStateSet state13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet state14(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet state15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	StageStateSet state16(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	StageStateSet state20(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet state21(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);
	
	d3d_DisableTexture(0);

	s_bValidateRequired = false;
	s_bValidateResult = !FAILED(hr);
	return s_bValidateResult;
}

void CRenderShader_Gouraud_Texture_Fullbright::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	bool bDrawingLights = cState.m_nNumLights != 0;

	uint32 nOldAlphaBlendEnable;
	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&nOldAlphaBlendEnable);
	bDrawingLights &= (nOldAlphaBlendEnable == 0);

	if (bDrawingLights)
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

			StageStateSet state0(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			StageStateSet state1(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			StageStateSet state2(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			StageStateSet state3(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			StageStateSet state4(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			StageStateSet state5(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			StageStateSet state6(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			StageStateSet state7(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
			CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
			for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
			{
				CInternalSection &cSection = m_aSections[*iCurSection];

				ChangeSection(cSection);

				// Change the texture
				d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);

				if(cSection.m_pTextureEffect)
					cSection.m_pTextureEffect->Install(1, TSChannel_Base);

				// Draw it
				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
					0, cSection.m_nStartVertex, cSection.m_nVertexCount,
					cSection.m_nStartIndex, cSection.m_nTriCount);

				// Count the tris
				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				// Draw the fullbright
				PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
					0, cSection.m_nStartVertex, cSection.m_nVertexCount,
					cSection.m_nStartIndex, cSection.m_nTriCount);

				// Count the extra fullbright pass
				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
				PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, (g_CV_Saturate) ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

				if(cSection.m_pTextureEffect)
					cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base);

				d3d_DisableTexture(0);
			}
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

void CRenderShader_Gouraud_Texture_Fullbright::PreFlush()
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, (g_CV_Saturate) ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;
}

void CRenderShader_Gouraud_Texture_Fullbright::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture, 1, eFS_WorldBaseTexMemory);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_Texture_Fullbright::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_Base);
}

void CRenderShader_Gouraud_Texture_Fullbright::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Clear texture effects
	if (cSection.m_pTextureEffect)
	{
		cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_Base);

		//we also have to reset our odd texture referencing since the script will set it back to the
		//default value
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
}
}

void CRenderShader_Gouraud_Texture_Fullbright::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_Detail_Fullbright implementation

bool CRenderShader_Gouraud_Detail_Fullbright::s_bValidateRequired = true;
bool CRenderShader_Gouraud_Detail_Fullbright::s_bValidateResult = false;

bool CRenderShader_Gouraud_Detail_Fullbright::ValidateShader(const CRBSection &cSection)
{
	if (!s_bValidateRequired)
		return s_bValidateResult;

	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		return false;
	}

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0], 3, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_Detail), 4, eFS_WorldDetailTexMemory);

	RTexture *pRenderTexture = (cSection.m_pTexture) ? (RTexture*)cSection.m_pTexture[0]->m_pRenderData : 0;
	bool bAlphaTest = (pRenderTexture) ? pRenderTexture->m_AlphaRef != ALPHAREF_NONE : false;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	// Set up the texture stages
	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet tss02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet tss03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet tss05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
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
	StageStateSet tss30(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss31(3, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss32(3, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
	StageStateSet tss33(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss34(3, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss35(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet tss36(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	StageStateSet tss40(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss41(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);
	d3d_DisableTexture(3);
	d3d_DisableTexture(4);

	s_bValidateRequired = false;
	s_bValidateResult = true;
	if (FAILED(hr))
	{
		// Looks like this won't work 
		s_bValidateResult = false;
	}

	return s_bValidateResult;
}

void CRenderShader_Gouraud_Detail_Fullbright::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	bool bDrawingLights = cState.m_nNumLights != 0;

	uint32 nOldAlphaBlendEnable;
	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&nOldAlphaBlendEnable);
	bDrawingLights &= (nOldAlphaBlendEnable == 0);

	if (bDrawingLights)
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

			CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
			CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
			for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
			{
				CInternalSection &cSection = m_aSections[*iCurSection];

				ChangeSection(cSection);

				// Change the texture
				d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
				d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);

				if (cSection.m_pTextureEffect)
					cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_Detail);

				// Draw it
				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
					0, cSection.m_nStartVertex, cSection.m_nVertexCount,
					cSection.m_nStartIndex, cSection.m_nTriCount);

				// Count it
				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				// Fullbright it
				RTexture *pRTexture = (cSection.m_pTexture) ? (RTexture*)cSection.m_pTexture->m_pRenderData : 0;
				if (pRTexture && pRTexture->IsFullbrite())
				{
					PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

					// Draw it
					PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
						0, cSection.m_nStartVertex, cSection.m_nVertexCount,
						cSection.m_nStartIndex, cSection.m_nTriCount);

					PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
					PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

					IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
				}

				if (cSection.m_pTextureEffect)
					cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_Detail);

				d3d_DisableTexture(0);
				d3d_DisableTexture(1);
			}
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

void CRenderShader_Gouraud_Detail_Fullbright::PreFlush()
{
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
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	PD3DDEVICE->SetTextureStageState(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;
}

void CRenderShader_Gouraud_Detail_Fullbright::FlushChangeSection(CInternalSection &cSection)
{
	// Change the texture
	d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_Detail), 1, eFS_WorldDetailTexMemory);
	d3d_SetTexture(cSection.m_pTexture, 3, eFS_WorldBaseTexMemory);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_Detail_Fullbright::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(4, TSChannel_Base, TSChannel_Detail, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Gouraud_Detail_Fullbright::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Clear texture effects
	if (cSection.m_pTextureEffect)
	{
		cSection.m_pTextureEffect->Uninstall(4, TSChannel_Base, TSChannel_Detail, TSChannel_Null, TSChannel_Base);

		//we also have to reset our odd texture referencing since the script will set it back to the
		//default value
		PD3DDEVICE->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	}

}

void CRenderShader_Gouraud_Detail_Fullbright::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);
	d3d_DisableTexture(3);

	PD3DDEVICE->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 3);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

//////////////////////////////////////////////////////////////////////////////
// CRenderShader_Gouraud_EnvMap_Fullbright implementation

bool CRenderShader_Gouraud_EnvMap_Fullbright::s_bValidateRequired = true;
bool CRenderShader_Gouraud_EnvMap_Fullbright::s_bValidateResult = false;

bool CRenderShader_Gouraud_EnvMap_Fullbright::ValidateShader(const CRBSection &cSection)
{
	if (!s_bValidateRequired)
		return s_bValidateResult;

	if (!PD3DDEVICE)
	{
		ASSERT(!"Device not yet initialized");
		return false;
	}

	// Load up the textures for validation
	d3d_SetTexture(cSection.m_pTexture[0], 0, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), 1, eFS_WorldEnvMapTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0],3, eFS_WorldBaseTexMemory);
	d3d_SetTexture(cSection.m_pTexture[0]->GetLinkedTexture(eLinkedTex_EnvMap), 4, eFS_WorldEnvMapTexMemory);

	RTexture *pRenderTexture = (cSection.m_pTexture) ? (RTexture*)cSection.m_pTexture[0]->m_pRenderData : 0;
	bool bAlphaTest = (pRenderTexture) ? pRenderTexture->m_AlphaRef != ALPHAREF_NONE : false;

	// Set up the alpha states
	StateSet stateAlphaTest(D3DRS_ALPHATESTENABLE, (DWORD)bAlphaTest);

	D3DMATRIX mat;
    mat._11 =-0.5f; mat._12 = 0.0f; mat._13 = 0.0f; mat._14 = 0.0f; 
    mat._21 = 0.0f; mat._22 =-0.5f; mat._23 = 0.0f; mat._24 = 0.0f; 
    mat._31 = 0.0f; mat._32 = 0.0f; mat._33 = 0.0f; mat._34 = 0.0f; 
    mat._41 = 0.5f; mat._42 = 0.5f; mat._43 = 0.0f; mat._44 = 1.0f; 
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &mat);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss01(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	StageStateSet tss02(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet tss03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss04(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	StageStateSet tss05(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet tss10(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss11(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss12(1, D3DTSS_COLOROP, g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	StageStateSet tss13(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss14(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss15(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	StageStateSet tss20(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	StageStateSet tss21(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss22(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	StageStateSet tss23(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss24(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss25(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	StageStateSet tss30(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	StageStateSet tss31(3, D3DTSS_COLORARG2, D3DTA_CURRENT);
	StageStateSet tss32(3, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
	StageStateSet tss33(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	StageStateSet tss34(3, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	StageStateSet tss35(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	StageStateSet tss36(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	StageStateSet tss40(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StageStateSet tss41(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	HRESULT hr;
	uint32 nNumPasses;
	hr = PD3DDEVICE->ValidateDevice((DWORD*)&nNumPasses);

	d3d_DisableTexture(0);
	d3d_DisableTexture(1);
	d3d_DisableTexture(3);
	d3d_DisableTexture(4);

	s_bValidateRequired = false;
	s_bValidateResult = true;
	if (FAILED(hr))
	{
		// Looks like this won't work
		s_bValidateResult = false;
	}

	return s_bValidateResult;
}

void CRenderShader_Gouraud_EnvMap_Fullbright::DrawNormal(const DrawState &cState, uint32 nRenderBlock)
{
	bool bDrawingLights = cState.m_nNumLights != 0;

	uint32 nOldAlphaBlendEnable;
	PD3DDEVICE->GetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD*)&nOldAlphaBlendEnable);
	bDrawingLights &= (nOldAlphaBlendEnable == 0);

	if (bDrawingLights)
	{
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
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
			PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			RTexture* pPrevEnvMap = NULL;

			CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
			CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
			for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
			{
				CInternalSection &cSection = m_aSections[*iCurSection];

				ChangeSection(cSection);

				// See if we need to setup the environment map texture
				RTexture* pEnvMap = (RTexture*)cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap)->m_pRenderData;
				if (pPrevEnvMap != pEnvMap)
				{
					d3d_SetEnvMapTransform(pEnvMap, 1);
					pPrevEnvMap = pEnvMap;
				}

				// Change the texture
				d3d_SetTexture(cSection.m_pTexture, 0, eFS_WorldBaseTexMemory);
				d3d_SetTexture(cSection.m_pTexture->GetLinkedTexture(eLinkedTex_EnvMap), 1, eFS_WorldEnvMapTexMemory);

				if (cSection.m_pTextureEffect)
					cSection.m_pTextureEffect->Install(2, TSChannel_Base, TSChannel_EnvMap);

				// Draw it
				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
					0, cSection.m_nStartVertex, cSection.m_nVertexCount,
					cSection.m_nStartIndex, cSection.m_nTriCount);

				// Count it
				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				// Fullbright it
				RTexture *pRTexture = (cSection.m_pTexture) ? (RTexture*)cSection.m_pTexture->m_pRenderData : 0;
				if (pRTexture && pRTexture->IsFullbrite())
				{
					PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

					// Draw it
					PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
						0, cSection.m_nStartVertex, cSection.m_nVertexCount,
						cSection.m_nStartIndex, cSection.m_nTriCount);

					PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
					PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, g_CV_Saturate ? D3DBLEND_SRCCOLOR : D3DBLEND_ZERO);

					IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
				}

				if (cSection.m_pTextureEffect)
					cSection.m_pTextureEffect->Uninstall(2, TSChannel_Base, TSChannel_EnvMap);

				d3d_DisableTexture(0);
				d3d_DisableTexture(1);
			}
		}

		if (nFogEnabled)
			DrawFogPass(nRenderBlock);

		// Make sure to turn off the transform
		d3d_UnsetEnvMapTransform(1);

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}
	else
	{
		QueueRenderBlock(nRenderBlock);
	}
}

void CRenderShader_Gouraud_EnvMap_Fullbright::PreFlush()
{
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, g_CV_EnvMapAdd.m_Val ? D3DTOP_ADDSIGNED : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, g_CV_Saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLORARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	PD3DDEVICE->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	PD3DDEVICE->SetTextureStageState(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&m_nOldAlphaTest);
	m_nCurAlphaTest = m_nOldAlphaTest;

	m_pCurEnvMap = 0;
}

void CRenderShader_Gouraud_EnvMap_Fullbright::FlushChangeSection(CInternalSection &cSection)
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
	d3d_SetTexture(cSection.m_pTexture, 3, eFS_WorldBaseTexMemory);

   	// Set up the alpha test state
	if (m_nCurAlphaTest != cSection.m_nAlphaTest)
   	{
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.m_nAlphaTest);
		m_nCurAlphaTest = cSection.m_nAlphaTest;
   	}
}

void CRenderShader_Gouraud_EnvMap_Fullbright::PreFlushBlock(CInternalSection &cSection)
{
	// Setup texture effects
	if (cSection.m_pTextureEffect)
		cSection.m_pTextureEffect->Install(4, TSChannel_Base, TSChannel_EnvMap, TSChannel_Null, TSChannel_Base);
}

void CRenderShader_Gouraud_EnvMap_Fullbright::PostFlushBlock(CInternalSection &cSection, 
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
{
	// Clear texture effects
	if (cSection.m_pTextureEffect)
	{
		cSection.m_pTextureEffect->Uninstall(4, TSChannel_Base, TSChannel_EnvMap, TSChannel_Null, TSChannel_Base);

		//we also have to reset our odd texture referencing since the script will set it back to the
		//default value
		PD3DDEVICE->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
	}
}

void CRenderShader_Gouraud_EnvMap_Fullbright::PostFlush()
{
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);
	d3d_DisableTexture(3);

	// Make sure to turn off the transform
	d3d_UnsetEnvMapTransform(1);

	PD3DDEVICE->SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 3);

	if (m_nCurAlphaTest != m_nOldAlphaTest)
		PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)m_nOldAlphaTest);
}

