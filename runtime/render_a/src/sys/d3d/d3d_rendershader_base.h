//////////////////////////////////////////////////////////////////////////////
// D3D CRenderShader_Base templatized default rendershader implementation

#ifndef __D3D_RENDERSHADER_BASE_H__
#define __D3D_RENDERSHADER_BASE_H__

#include "d3d_rendershader.h"
#include "d3d_renderblock.h"
#include "d3d_device.h"
#include "d3d_draw.h"
#include "d3d_rendershader_glow.h"
#include "iaggregateshader.h"
#include "texturescriptinstance.h"
#include "texturescriptmgr.h"
#include "rendererframestats.h"
#include "renderstruct.h"
#include "sprite.h"
#include <algorithm>

// Base templatized shader implementation
template <typename SVertex, typename CShaderSection, uint32 nFVF>
class CRenderShader_Base : public CRenderShader
{
public:

	CRenderShader_Base() :
		m_pIB(NULL),
		m_aVBs(0),
		m_nTotalTriCount(0),
		m_nTotalVertexCount(0),
		m_nCurSection(0),
		m_nCurRB(0),
		m_bLocked(false),
		m_pLockedIndices(0),
		m_nCurVB(0),
		m_nQueuedSections(0)
	{
	}

	virtual ~CRenderShader_Base()
	{
		// Forget the VB & IB
		if (!m_aVBs.empty())
		{
			TVBList::iterator iCurVB = m_aVBs.begin();
			for (; iCurVB != m_aVBs.end(); ++iCurVB)
			{
				(*iCurVB)->Release();
			}

			m_aVBs.clear();
		}

		if (m_pIB)
		{
			m_pIB->Release();
		}

		m_pIB = 0;
	}

	// Preview a renderblock
	// Returns the renderblock's index
	virtual uint32 PreviewRenderBlock(const CD3D_RenderBlock &cRenderBlock)
	{
		// Remember where this renderblock starts
		m_aPreviewRenderBlocks.push_back(m_aSections.size());

		// Tell them what their index is
		return m_aPreviewRenderBlocks.size() - 1;
	}

	// Preview a section (for growing internal buffers and etc)
	virtual void PreviewSection(const CRBSection &cSection)
	{
		CInternalSection cInternalSection;
		cInternalSection.m_nSectionIndex = cSection.m_nIndex;
		cInternalSection.m_nStartVertex = 0;
		cInternalSection.m_nEndVertex = 0;
		cInternalSection.m_nStartIndex = 0;
		cInternalSection.m_nEndIndex = 0;
		cInternalSection.m_nTriCount = cSection.m_nTriCount;
		cInternalSection.m_nVertexCount = cSection.m_nVertexCount;

		cInternalSection.m_pTextureEffect = cSection.m_pTextureEffect;
		if(cInternalSection.m_pTextureEffect)
		{
			cInternalSection.m_pTextureEffect->AddRef();
		}

		FillSection(cInternalSection, cSection);
		LT_MEM_TRACK_ALLOC(m_aSections.push_back(cInternalSection), LT_MEM_TYPE_RENDERER);
	}

	// Lock (AddSection is going to be called for each of the sections)
	virtual void Lock(uint32 nRenderBlock)
	{
		if (m_bLocked)
		{
			ASSERT(!"Double lock encountered");
			return;
		}

		if (m_aVBs.empty() || !m_pIB)
		{
			PrepareSections();

			const uint32 k_nVerticesPerVB = 65536;

			// Count the vertices & indices and adjust the starting positions
			m_nTotalVertexCount = 0;
			m_nTotalTriCount = 0;

			uint32 nStartVertex = 65536;
			uint32 nNumVBs = 0;

			typedef std::vector<uint32> TSizeList;
			TSizeList aVBSizes;

			aVBSizes.reserve(16);

			TSectionList::iterator iFinger = m_aSections.begin();
			for (; iFinger != m_aSections.end(); ++iFinger)
			{
				CInternalSection &cCurSection = *iFinger;

				// Move to the next VB if necessary
				if ((nStartVertex + cCurSection.m_nVertexCount) >= k_nVerticesPerVB)
				{
					aVBSizes.push_back(0);
					nStartVertex = 0;
					++nNumVBs;
				}

				cCurSection.m_nStartVertex = nStartVertex;
				cCurSection.m_nStartIndex = m_nTotalTriCount * 3;
				cCurSection.m_nEndVertex = nStartVertex + cCurSection.m_nVertexCount;
				cCurSection.m_nEndIndex = cCurSection.m_nStartIndex + cCurSection.m_nTriCount * 3;
				cCurSection.m_nVBIndex = nNumVBs - 1;

				// Keep track of how big the VBs should be...
				aVBSizes[nNumVBs - 1] = cCurSection.m_nEndVertex + 1;

				nStartVertex += cCurSection.m_nVertexCount;
				m_nTotalVertexCount += cCurSection.m_nVertexCount;
				m_nTotalTriCount += cCurSection.m_nTriCount;
			}

			if (m_nTotalVertexCount && m_nTotalTriCount)
			{
				if (m_aVBs.empty())
				{
					ASSERT(aVBSizes.size() == nNumVBs);
					m_aVBs.resize(nNumVBs);
					TVBList::iterator iCurVB = m_aVBs.begin();
					TSizeList::iterator iCurVBSize = aVBSizes.begin();
					for (; iCurVB != m_aVBs.end(); ++iCurVB, ++iCurVBSize)
					{
						// Create the VB
						if (!SUCCEEDED(PD3DDEVICE->CreateVertexBuffer(*iCurVBSize * sizeof(SVertex),
							D3DUSAGE_WRITEONLY, nFVF, D3DPOOL_DEFAULT, &(*iCurVB))))
						{
							ASSERT(!"Unable to create vertex buffer!");
							return;
						}
					}
				}
				else
				{
					ASSERT(nNumVBs <= m_aVBs.size());
				}

				if (!m_pIB)
				{
					// Create the IB
					if (!SUCCEEDED(PD3DDEVICE->CreateIndexBuffer(m_nTotalTriCount * 3 * sizeof(uint16),
						D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB)))
					{
						ASSERT(!"Unable to create index buffer!");
						return;
					}
				}
			}
			else
			{
				ASSERT(!"Empty lock encountered");
				m_nTotalVertexCount = 0;
				m_nTotalTriCount = 0;
			}
		}

		if (!m_pLockedIndices)
		{
			ASSERT(m_aLockedVertices.empty()); // If this isn't empty, something very bad will happen no matter what we do.

			m_aLockedVertices.reserve(m_aVBs.size());
			TVBList::iterator iCurVB = m_aVBs.begin();
			void *plv = 0;

			for (; iCurVB != m_aVBs.end(); ++iCurVB)
			{
				// Lock it
				m_aLockedVertices.push_back(plv);
				if (!SUCCEEDED((*iCurVB)->Lock(0, 0, (void **)&m_aLockedVertices.back(), 0)))
				{
					ASSERT(!"Unable to lock vertex buffer!");
					return;
				}
			}

			// Lock the IB
			if (!SUCCEEDED(m_pIB->Lock(0, 0, (void **)&m_pLockedIndices, 0)))
			{
				ASSERT(!"Unable to lock index buffer!");
				return;
			}
		}

		if (nRenderBlock >= m_aRenderBlocks.size())
		{
			ASSERT(!"Invalid lock encountered");
			return;
		}

		m_nCurSection = 0;
		m_nCurRB = nRenderBlock;

		m_bLocked = true;
	}

	// Add a section to the rendering list
	virtual void AddSection(const CRBSection &cSection, const uint16 *aIndices, const SRBVertex *aVertices, const SRBVertex *aSrcVertices)
	{
		// Don't add if we're not locked
		if (!m_bLocked)
		{
			ASSERT(!"Tried to add section when not locked");
			return;
		}

		CRenderBlockData &cRB = m_aRenderBlocks[m_nCurRB];

		// Make sure we're not past the end
		if (m_nCurSection >= cRB.m_aSections.size())
		{
			ASSERT(!"Invalid AddSection encountered");
			return;
		}

		// Which section are we on?
		CInternalSection &cISection = m_aSections[cRB.m_aSections[m_nCurSection]];

		if(cISection.m_nSectionIndex != cSection.m_nIndex)
		{
			assert(!"Found section index mismatch");
			return;
		}

		// Remember where we came from
		cISection.m_pOriginalIndices = &aIndices[cSection.m_nStartIndex];
		cISection.m_pOriginalVertices = aSrcVertices;
		cISection.m_nOriginalVertexOffset = -(int32)(cSection.m_nStartVertex - cISection.m_nStartVertex);

		// Fill the VB
		ASSERT(cSection.m_nVertexCount == cISection.m_nVertexCount);
		TranslateVertices((SVertex*)m_aLockedVertices[cISection.m_nVBIndex] + cISection.m_nStartVertex, &aVertices[cSection.m_nStartVertex], cSection.m_nVertexCount);

		// Fill the IB
		ASSERT(cSection.m_nTriCount == cISection.m_nTriCount);
		uint16 *pCurOut = m_pLockedIndices + cISection.m_nStartIndex;
		uint16 *pEndOut = &pCurOut[cISection.m_nTriCount * 3];
		const uint16 *pCurIn = &aIndices[cSection.m_nStartIndex];
		const uint16 *pEndIn = &pCurIn[cSection.m_nTriCount * 3];
		uint16 nOffset = cSection.m_nStartVertex - cISection.m_nStartVertex;
		while (pCurOut != pEndOut)
		{
			*pCurOut = *pCurIn - nOffset;
			++pCurIn;
			++pCurOut;
		}

		// Copy over the texture effect
		if (cSection.m_pTextureEffect)
		{
			cISection.m_pTextureEffect = cSection.m_pTextureEffect;
			cISection.m_pTextureEffect->AddRef();
		}

		// Copy over the sprite info
		for (uint32 nCurrSpr = 0; nCurrSpr < CRBSection::kNumTextures; nCurrSpr++)
		{
			if(cSection.m_pSpriteData[nCurrSpr])
			{
				LT_MEM_TRACK_ALLOC(cISection.m_pSpriteData[nCurrSpr] = new CRBSection::CSpriteData(*cSection.m_pSpriteData[nCurrSpr]),LT_MEM_TYPE_RENDERER);
			}
		}

		// Remember that we've added this section
		++m_nCurSection;
	}

	// Unlock (AddSection is done being called)
	virtual void Unlock()
	{
		// Don't unlock if we're not locked
		if (!m_bLocked)
		{
			ASSERT(!"Tried to unlock when not locked");
			return;
		}

		ASSERT("Some sections were not filled!" && m_aRenderBlocks[m_nCurRB].m_aSections.size() == m_nCurSection);

		m_bLocked = false;
	}

	// Draw the sections
	virtual void Draw(const DrawState &cState, uint32 nRenderBlock)
	{
		// Make sure we can
		if (m_aVBs.empty() || !m_pIB || !m_nTotalVertexCount || !m_nTotalTriCount)
		{
			return;
		}

		// Get ready to draw...
		FinalizeUnlock();

		// Clear the VB index
		m_nCurVB = m_aVBs.size();

		// Glow instead if that's what's needed
		if (cState.m_pParams->m_eRenderMode == ViewParams::eRenderMode_Glow)
		{
			DrawGlow(nRenderBlock);
		}
		else
		{
			// Update the textures for this renderblock
			CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
			CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
			for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
			{
				m_aSections[*iCurSection].UpdateTexture();
			}

			DrawNormal(cState, nRenderBlock);
		}
	}

	// Renders an aggreagate shader on all of its child shader blocks
	virtual bool DrawAggregateShader(IAggregateShader *pShader, uint32 nRenderBlock)
	{
		// Get ready to draw...
		FinalizeUnlock();

		if (!pShader->BeginShader(	sizeof(SVertex),
									nFVF,
									NULL,
									0,
									m_pIB,
									GetShaderID()))
		{
			return false;
		}

		// Clear the VB index
		m_nCurVB = m_aVBs.size();

		CRenderBlockData &cRenderBlock = m_aRenderBlocks[nRenderBlock];
		for (uint32 nFindSection = 0; nFindSection < cRenderBlock.m_aSections.size(); ++nFindSection)
		{
			CInternalSection *pSection = &m_aSections[cRenderBlock.m_aSections[nFindSection]];
			ChangeSection(*pSection);

			pShader->RenderSection(	D3DPT_TRIANGLELIST,
									pSection->m_pOriginalIndices,
									pSection->m_pOriginalVertices,
									pSection->m_nTriCount * 3,
									pSection->m_nStartVertex,
									pSection->m_nVertexCount,
									pSection->m_nOriginalVertexOffset,
									pSection->m_nStartIndex);
		}

		// Don't leak the VB or IB
		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);

		return true;
	}

	// Flush any pending rendering
	virtual void Flush()
	{
		// Jump out if we don't want to do this
		if (!m_nQueuedSections || m_aVBs.empty() || !m_pIB)
		{
			return;
		}

		// Get ready to draw...
		FinalizeUnlock();

		PreFlush();

		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(nFVF);
		PD3DDEVICE->SetIndices(m_pIB);

		m_nCurVB = m_aVBs.size();

		bool bChangeSection = true;

		TSectionList::iterator iCurSection = m_aSections.begin();

		for (;;)
		{
			// Skip any un-rendered sections
			for (; iCurSection != m_aSections.end(); ++iCurSection)
			{
				bChangeSection |= iCurSection->m_bSectionBreak;
				if (iCurSection->m_bQueuedForRender)
				{
					break;
				}
			}

			if (iCurSection == m_aSections.end())
			{
				break;
			}

			// Mark this section as rendered
			iCurSection->m_bQueuedForRender = false;

			if (bChangeSection)
			{
				// Select the right VB
				ChangeSection(*iCurSection);

				FlushChangeSection(*iCurSection);

				bChangeSection = false;
			}

			PreFlushBlock(*iCurSection);

			uint32 nStartIndex = iCurSection->m_nStartIndex;
			uint32 nEndIndex = iCurSection->m_nEndIndex;
			uint32 nStartVertex = iCurSection->m_nStartVertex;
			uint32 nEndVertex = iCurSection->m_nEndVertex;

			// Queue up any continguous sections
			TSectionList::iterator iFirstSection = iCurSection;
			for (++iCurSection; iCurSection != m_aSections.end(); ++iCurSection)
			{
				if ((!iCurSection->m_bQueuedForRender) ||
					(iCurSection->m_bSectionBreak))
				{
					bChangeSection |= iCurSection->m_bSectionBreak;
					break;
				}

				ASSERT(nStartVertex <= iCurSection->m_nStartVertex);
				ASSERT(nEndVertex <= iCurSection->m_nEndVertex);
				nEndVertex = iCurSection->m_nEndVertex;
				nEndIndex = iCurSection->m_nEndIndex;
				iCurSection->m_bQueuedForRender = false;

				IncFrameStat(eFS_ShaderBatched, 1);
			}

			uint32 nNumTris = (nEndIndex - nStartIndex) / 3;
			uint32 nNumVerts = nEndVertex - nStartVertex;

			IncFrameStat(eFS_ShaderTris, nNumTris);
			IncFrameStat(eFS_ShaderVerts, nNumVerts);
			IncFrameStat(eFS_ShaderDrawPrim, 1);

			// Draw
			if(HasEffect(*iFirstSection))
			{
				FlushWithEffect(*iFirstSection, nStartIndex, nEndIndex, nStartVertex, nEndVertex);
			}
			else
			{
				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
					0, nStartVertex, nNumVerts,
					nStartIndex, nNumTris);
				DEBUG_PRINT(2,("Brush Section: (%p) Start: (%d) Verts: (%d) sIdx: (%d) Tris: (%d)", &(*iFirstSection), nStartVertex, nNumVerts, nStartIndex, nNumTris));
			}

			IncFrameStat(eFS_WorldTriangles, nNumTris);

			PostFlushBlock(*iFirstSection, nStartIndex, nEndIndex, nStartVertex, nEndVertex);
		}

		m_nQueuedSections = 0;

		PostFlush();

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}

protected:

	struct CInternalSection;

	// Draw the sections of a RenderBlock normally
	virtual void DrawNormal(const DrawState &cState, uint32 nRenderBlock) = 0;

	// Call this function to do a standard fog pass (used frequently during dynamic lighting handling)
	virtual void DrawFogPass(uint32 nRenderBlock)
	{
		// Use a 0 T-Factor w/ white alpha
		StateSet ssTFactor(D3DRS_TEXTUREFACTOR, 0xFF000000);

		// Set up an alpha blend
		StateSet blend0(D3DRS_ALPHABLENDENABLE, TRUE);
		StateSet blend1(D3DRS_SRCBLEND, D3DBLEND_ONE);
		StateSet blend2(D3DRS_DESTBLEND, D3DBLEND_ONE);

		StateSet ssZWrite(D3DRS_ZWRITEENABLE, FALSE);
		StateSet ssZFUNC(D3DRS_ZFUNC, D3DCMP_EQUAL);

		// TSS w/ just TFactor color
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// Draw more triangles.
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
			IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
		}
	}

	// Call this function to do the standard dynamic lighting, including standard diffuse & z-buffer
	virtual void DrawLights(const DrawState &cState, uint32 nRenderBlock)
	{
		{
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			uint32 nOldAlphaTest;
			PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&nOldAlphaTest);
			bool bCurAlphaTest = (nOldAlphaTest != 0);

			// Draw the diffuse color & fill in the Z-Buffer
			CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
			CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
			for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
			{
				CInternalSection &cSection = m_aSections[*iCurSection];

				ChangeSection(cSection);

				// Change the texture
				d3d_SetTexture(cSection.GetTexture(), 0, eFS_WorldBaseTexMemory);

				// Set up the alpha test state
				if (bCurAlphaTest != cSection.ShouldAlphaTest())
				{
					PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.ShouldAlphaTest());
					bCurAlphaTest = cSection.ShouldAlphaTest();
				}

				if(cSection.m_pTextureEffect)
				{
					cSection.m_pTextureEffect->Install(1, TSChannel_Base);
				}

				IncFrameStat(eFS_ShaderTris, cSection.m_nTriCount);
				IncFrameStat(eFS_ShaderVerts, cSection.m_nVertexCount);
				IncFrameStat(eFS_ShaderDrawPrim, 1);

				// Draw it
				PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
					0, cSection.m_nStartVertex, cSection.m_nVertexCount,
					cSection.m_nStartIndex, cSection.m_nTriCount);

				// Count it
				IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);

				if(cSection.m_pTextureEffect)
				{
					cSection.m_pTextureEffect->Uninstall(1, TSChannel_Base);
				}
			}

			if (bCurAlphaTest != (nOldAlphaTest != 0))
			{
				PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)nOldAlphaTest);
			}
		}

		DrawLightList(cState, nRenderBlock);
	}

	// Call this function to do a standard pass on the dynamic lights
	virtual void DrawLightList(const DrawState &cState, uint32 nRenderBlock)
	{
		DrawState::TLightListIterator iEndLight = &cState.m_pLightList[cState.m_nNumLights];
		DrawState::TLightListIterator iCurLight = cState.m_pLightList;
		for (; iCurLight != iEndLight; ++iCurLight)
		{
			CRenderShader_DynamicLight::GetSingleton()->SetupLight(cState.m_pParams, *iCurLight);

			bool bLightBackfacing = g_CV_DynamicLight_Backfacing.m_Val || (((*iCurLight)->m_Flags & FLAG_DONTLIGHTBACKFACING) == 0);

			if (bLightBackfacing)
			{
				PD3DDEVICE->SetIndices(m_pIB);
			}

			CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
			CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
			for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
			{
				CInternalSection &cSection = m_aSections[*iCurSection];

				ChangeSection(cSection);

				// Draw it
				if (bLightBackfacing)
				{
					PD3DDEVICE->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
						0, cSection.m_nStartVertex, cSection.m_nVertexCount,
						cSection.m_nStartIndex, cSection.m_nTriCount);
					IncFrameStat(eFS_DynamicLightTriangles, cSection.m_nTriCount);
				}
				else
				{
					uint32 nNumTris = CRenderShader_DynamicLight::GetSingleton()->DrawTris(
						cSection.m_nStartVertex, cSection.m_nVertexCount,
						cSection.m_pOriginalIndices, cSection.m_nTriCount,
						cSection.m_pOriginalVertices,
						sizeof(SRBVertex), cSection.m_nOriginalVertexOffset);

					IncFrameStat(eFS_WorldTriangles, nNumTris);
				}
			}
		}

		CRenderShader_DynamicLight::GetSingleton()->ResetStates();
	}

	// Called before flushing
	virtual void PreFlush()
	{
	}

	// Called when changing sections
	virtual void FlushChangeSection(CInternalSection &cSection)
	{
	}

	// Called before flushing a new block of sections
	virtual void PreFlushBlock(CInternalSection &cSection)
	{
	}

	// Called after flushing a block of sections
	virtual void PostFlushBlock(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
	{
	}

	// Called after flushing
	virtual void PostFlush()
	{
	}

	// C
	virtual bool FlushWithEffect(CInternalSection &cSection,
		uint32 nStartIndex, uint32 nEndIndex,
		uint32 nStartVertex, uint32 nEndVertex)
	{
		return false;
	}

	// C
	virtual bool HasEffect(CInternalSection &cSection)
	{
		return false;
	}

	// Draw the sections with glow (default behavior is to draw in black)
	virtual void DrawGlow(uint32 nRenderBlock)
	{
		// Get ready to draw...
		FinalizeUnlock();

		// Start me up...
		if (!CRenderShader_Glow::BeginNewShader(nFVF, sizeof(SVertex), m_pIB, 0))
		{
			return;
		}

		uint32 nOldAlphaTest;
		PD3DDEVICE->GetRenderState(D3DRS_ALPHATESTENABLE, (DWORD*)&nOldAlphaTest);
		bool bCurAlphaTest = nOldAlphaTest != 0;

		CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
		CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
		for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
		{
			CInternalSection &cSection = m_aSections[*iCurSection];
			ChangeSection(cSection);

			// Set up the alpha test state
			if (bCurAlphaTest != cSection.ShouldAlphaTest())
			{
				PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)cSection.ShouldAlphaTest());
				bCurAlphaTest = cSection.ShouldAlphaTest();
			}

			// Fullbright it
			if (cSection.ShouldGlow())
			{
				CRenderShader_Glow::DrawGlow(	cSection.GetTexture(), cSection.m_pTextureEffect, TSChannel_Base,
												cSection.m_nStartVertex, cSection.m_nVertexCount,
												cSection.m_nStartIndex, cSection.m_nTriCount);
			}
			else
			{
				CRenderShader_Glow::DrawBlack(	cSection.m_nStartVertex, cSection.m_nVertexCount,
												cSection.m_nStartIndex, cSection.m_nTriCount);
			}

			IncFrameStat(eFS_WorldTriangles, cSection.m_nTriCount);
		}

		if (bCurAlphaTest != (nOldAlphaTest != 0))
		{
			PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)nOldAlphaTest);
		}

		PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
		PD3DDEVICE->SetIndices(0);
	}

	// Queue a renderblock for later rendering
	void QueueRenderBlock(uint32 nRenderBlock)
	{
		CRenderBlockData &cRB = m_aRenderBlocks[nRenderBlock];
		CRenderBlockData::TSectionIndexList::iterator iCurSection = cRB.m_aSections.begin();
		for (; iCurSection != cRB.m_aSections.end(); ++iCurSection)
		{
			m_aSections[*iCurSection].m_bQueuedForRender = true;
			++m_nQueuedSections;
		}
	}

	// Translate a block of vertices from SRBVertex to SVertex
	virtual void TranslateVertices(SVertex *pOut, const SRBVertex *pIn, uint32 nCount) = 0;

	// Fill in an internal section from a CRBSection
	virtual void FillSection(CShaderSection &cInternalSection, const CRBSection &cSection) = 0;

	// Find the internal section associated with a CRBSection's index
	virtual CInternalSection *FindSection(uint32 nRenderBlock, uint32 nIndex)
	{
		// Find it differently if we're in the preview phase
		if (nRenderBlock >= m_aRenderBlocks.size())
		{
			if (nRenderBlock >= m_aPreviewRenderBlocks.size())
			{
				return NULL;
			}

			TSectionList::iterator iCurSection = m_aSections.begin() + m_aPreviewRenderBlocks[nRenderBlock];
			TSectionList::iterator iEndSection = (nRenderBlock < (m_aPreviewRenderBlocks.size() - 1))
				? (m_aSections.begin() + m_aPreviewRenderBlocks[nRenderBlock + 1]) : m_aSections.end();
			for (; iCurSection != iEndSection; ++iCurSection)
			{
				CInternalSection &cCurSection = *iCurSection;
				if (cCurSection.m_nSectionIndex == nIndex)
				{
					return &cCurSection;
				}
			}

			return NULL;
		}

		CRenderBlockData &cRenderBlock = m_aRenderBlocks[nRenderBlock];
		for (uint32 nFindSection = 0; nFindSection < cRenderBlock.m_aSections.size(); ++nFindSection)
		{
			CInternalSection &cCurSection = m_aSections[cRenderBlock.m_aSections[nFindSection]];
			if (cCurSection.m_nSectionIndex == nIndex)
			{
				return &cCurSection;
			}
		}

		// Didn't find it
		return NULL;
	}

	// Convenience function for drawing a textured quad on the screen for debugging
	virtual void DebugDrawTextureQuad(IDirect3DTexture9 *pTexture,
									  float fMinU, float fMinV, float fMaxU, float fMaxV,
									  float fX, float fY, float fSizeX, float fSizeY)
	{
		struct SQuadVert
		{
			LTVector m_vPos;
			float m_fRHW;
			float m_fU, m_fV;
		};

		SQuadVert aQuad[4];
		aQuad[0].m_vPos.Init(fX, fY, 0.0f);
		aQuad[0].m_fRHW = 1.0f;
		aQuad[0].m_fU = fMinU;
		aQuad[0].m_fV = fMinV;
		aQuad[1].m_vPos.Init(fX + fSizeX, fY, 0.0f);
		aQuad[1].m_fRHW = 1.0f;
		aQuad[1].m_fU = fMaxU;
		aQuad[1].m_fV = fMinV;
		aQuad[2].m_vPos.Init(fX + fSizeX, fY + fSizeY, 0.0f);
		aQuad[2].m_fRHW = 1.0f;
		aQuad[2].m_fU = fMaxU;
		aQuad[2].m_fV = fMaxV;
		aQuad[3].m_vPos.Init(fX, fY + fSizeY, 0.0f);
		aQuad[3].m_fRHW = 1.0f;
		aQuad[3].m_fU = fMinU;
		aQuad[3].m_fV = fMaxV;

		if (pTexture)
		{
			d3d_SetTextureDirect(pTexture, 0);
			d3d_SetTextureDirect(NULL, 1);
		}

		StageStateSet stateTSS00(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		StageStateSet stateTSS01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		StageStateSet stateTSS02(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		StageStateSet stateTSS03(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		StageStateSet stateTSS04(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		StageStateSet stateTSS05(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		StageStateSet stateTSS16(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		StageStateSet stateTSS17(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		SamplerStateSet stateWrap0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		SamplerStateSet stateWrap1(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

		PD3DDEVICE->SetVertexShader(NULL);
		PD3DDEVICE->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

		PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, aQuad, sizeof(SQuadVert));
	}

	// Call when changing sections to select the right VB
	void ChangeSection(const CInternalSection &cSection)
	{
		if (m_nCurVB != cSection.m_nVBIndex)
		{
			m_nCurVB = cSection.m_nVBIndex;
			PD3DDEVICE->SetStreamSource(0, m_aVBs[m_nCurVB], 0, sizeof(SVertex));
		}
	}

	// Mark the section breaks
	virtual void MarkSectionBreaks()
	{
		TSectionList::iterator iCurSection = m_aSections.begin();
		TSectionList::iterator iPrevSection = m_aSections.end();
		for (; iCurSection != m_aSections.end(); ++iCurSection)
		{
			iCurSection->m_bSectionBreak =
				(iPrevSection == m_aSections.end()) ||
				(*iCurSection != *iPrevSection) ||
				(iCurSection->m_nVBIndex != iPrevSection->m_nVBIndex);
			iPrevSection = iCurSection;
		}
	}

private:

	// Prepare the sections for moving into the VB's
	void PrepareSections()
	{
		// Init the renderblock section indices
		InitRenderBlocks();

		// Sort the sections
		SortSections();
	}

	// Init the renderblock list
	void InitRenderBlocks()
	{
		ASSERT(m_aRenderBlocks.empty());
		m_aRenderBlocks.clear();

		// Make some room
		m_aRenderBlocks.reserve(m_aPreviewRenderBlocks.size());

		// Go through the starting points...
		TRenderBlockPreviewList::iterator iCurPreviewRB = m_aPreviewRenderBlocks.begin();
		CRenderBlockData rbd;
		for (; iCurPreviewRB != m_aPreviewRenderBlocks.end(); ++iCurPreviewRB)
		{
			// Add a new renderblock
			m_aRenderBlocks.push_back(rbd);
			CRenderBlockData &cRB = m_aRenderBlocks.back();

			// Figure out how many sections are in this block
			TRenderBlockPreviewList::iterator iNextRB = iCurPreviewRB + 1;
			uint32 nNumSections = (iNextRB != m_aPreviewRenderBlocks.end()) ? (*iNextRB - *iCurPreviewRB) : (m_aSections.size() - *iCurPreviewRB);

			// Make room
			cRB.m_aSections.reserve(nNumSections);

			// Fill it with contiguous indices
			for (uint32 nIndex = 0; nIndex < nNumSections; ++nIndex)
			{
				cRB.m_aSections.push_back(nIndex + *iCurPreviewRB);
			}
		}
	}

	// Sort the section list
	void SortSections()
	{
		typedef std::vector<SSectionRBIndex> TIndexList;
		TIndexList aSortIndices;

		// Fill the index list with the indices in order
		aSortIndices.reserve(m_aSections.size());
		TRenderBlockPreviewList::iterator iCurRBStart = m_aPreviewRenderBlocks.begin();
		if (iCurRBStart != m_aPreviewRenderBlocks.end())
		{
			++iCurRBStart;
		}
		else
		{
			ASSERT(m_aSections.empty());
		}

		uint32 nCurRB = 0;
		for (uint32 nIndex = 0; nIndex < m_aSections.size(); ++nIndex)
		{
			while ((iCurRBStart != m_aPreviewRenderBlocks.end()) && (*iCurRBStart <= nIndex))
			{
				++iCurRBStart;
				++nCurRB;
			}

			SSectionRBIndex sIndex;
			sIndex.m_nSection = nIndex;
			sIndex.m_nRB = nCurRB;
			ASSERT(nCurRB < m_aRenderBlocks.size());
			ASSERT((nIndex - m_aPreviewRenderBlocks[nCurRB]) < m_aRenderBlocks[nCurRB].m_aSections.size());
			aSortIndices.push_back(sIndex);
		}

		// Sort!
		std::sort(aSortIndices.begin(), aSortIndices.end(), FSortSections(m_aSections));

		// Re-order into a new section list
		TSectionList aSortedSections;
		aSortedSections.reserve(m_aSections.size());

		// Put them in order
		TIndexList::iterator iCurIndex = aSortIndices.begin();
		CInternalSection is;
		for (; iCurIndex != aSortIndices.end(); ++iCurIndex)
		{
			// Move the section
			aSortedSections.push_back(is);
			aSortedSections.back() = m_aSections[iCurIndex->m_nSection];

			// Adjust the RB section index
			uint32 nRBSection = iCurIndex->m_nSection - m_aPreviewRenderBlocks[iCurIndex->m_nRB];
			ASSERT(iCurIndex->m_nRB < m_aRenderBlocks.size());
			ASSERT(nRBSection < m_aRenderBlocks[iCurIndex->m_nRB].m_aSections.size());
			m_aRenderBlocks[iCurIndex->m_nRB].m_aSections[nRBSection] = iCurIndex - aSortIndices.begin();
		}

		// Swap!
		m_aSections.swap(aSortedSections);

		// We don't need you any more
		m_aPreviewRenderBlocks.swap(TRenderBlockPreviewList());
	}

	// Actually unlock the IB & VB's, which is delayed until it actually starts rendering
	void FinalizeUnlock()
	{
		if (!m_pLockedIndices)
		{
			ASSERT(m_aLockedVertices.empty());
			return;
		}

		// Unlock the VB's
		TVBList::iterator iCurVB = m_aVBs.begin();
		for (; iCurVB != m_aVBs.end(); ++iCurVB)
		{
			(*iCurVB)->Unlock();
		}

		m_aLockedVertices.swap(TLockedVertexList());

		// Unlock the IB
		m_pIB->Unlock();
		m_pLockedIndices = 0;
	}

protected:

	struct CInternalSection : public CShaderSection
	{
		CInternalSection() :
			CShaderSection(),
			m_pTextureEffect(NULL),
			m_bQueuedForRender(false),
			m_bSectionBreak(false)
		{
			for(uint32 nCurrSpr = 0; nCurrSpr < CRBSection::kNumTextures; nCurrSpr++)
			{
				m_pSpriteData[nCurrSpr] = NULL;
			}
		}

		CInternalSection(const CInternalSection &cOther) :
			CShaderSection(cOther),
			m_nSectionIndex(cOther.m_nSectionIndex),
			m_nStartIndex(cOther.m_nStartIndex),
			m_nEndIndex(cOther.m_nEndIndex),
			m_nTriCount(cOther.m_nTriCount),
			m_nStartVertex(cOther.m_nStartVertex),
			m_nEndVertex(cOther.m_nEndVertex),
			m_nVertexCount(cOther.m_nVertexCount),
			m_pTextureEffect(cOther.m_pTextureEffect),
			m_bQueuedForRender(false),
			m_bSectionBreak(false)
		{
			if (m_pTextureEffect)
			{
				m_pTextureEffect->AddRef();
			}

			for (uint32 nCurrSpr = 0; nCurrSpr < CRBSection::kNumTextures; nCurrSpr++)
			{
				// Copy over the sprite if necessary
				if (cOther.m_pSpriteData[nCurrSpr])
				{
					LT_MEM_TRACK_ALLOC(m_pSpriteData[nCurrSpr] = new CRBSection::CSpriteData(*cOther.m_pSpriteData[nCurrSpr]),LT_MEM_TYPE_SPRITE);
				}
				else
				{
					m_pSpriteData[nCurrSpr] = NULL;
				}
			}
		}

		~CInternalSection()
		{
			if (m_pTextureEffect)
			{
				CTextureScriptMgr::GetSingleton().ReleaseInstance(m_pTextureEffect);
			}

			// Free our sprite information
			for (uint32 nCurrSpr = 0; nCurrSpr < CRBSection::kNumTextures; nCurrSpr++)
			{
				delete m_pSpriteData[nCurrSpr];
			}
		}

 		bool operator<(const CInternalSection &cOther) const
 		{
 			//see if the base class is less than ours...
 			if(CShaderSection::operator<(static_cast<const CShaderSection&>(cOther)))
			{
 				return true;
			}

 			if(CShaderSection::operator!=(static_cast<const CShaderSection&>(cOther)))
			{
 				return false;
			}

 			return m_pTextureEffect < cOther.m_pTextureEffect;
 		}

 		bool operator!=(const CInternalSection &cOther) const
 		{
			return !(*this == cOther);
 		}

 		bool operator==(const CInternalSection &cOther) const
 		{
 			if(	CShaderSection::operator==(static_cast<const CShaderSection&>(cOther)) &&
 				(m_pTextureEffect == cOther.m_pTextureEffect))
 			{
 				return true;
 			}

 			return false;
 		}

		CInternalSection &operator=(const CInternalSection &cOther)
		{
			if(&cOther == this)
			{
				return *this;
			}

			if (m_pTextureEffect)
			{
				CTextureScriptMgr::GetSingleton().ReleaseInstance(m_pTextureEffect);
			}

			m_pTextureEffect = cOther.m_pTextureEffect;
			if (m_pTextureEffect)
			{
				m_pTextureEffect->AddRef();
			}

			// Copy over the sprite, cleaning up ours
			for (uint32 nCurrSpr = 0; nCurrSpr < CRBSection::kNumTextures; nCurrSpr++)
			{
				delete m_pSpriteData[nCurrSpr];
				m_pSpriteData[nCurrSpr] = NULL;

				if (cOther.m_pSpriteData[nCurrSpr])
				{
					LT_MEM_TRACK_ALLOC(m_pSpriteData[nCurrSpr] = new CRBSection::CSpriteData(*cOther.m_pSpriteData[nCurrSpr]),LT_MEM_TYPE_SPRITE);
				}
			}

			CShaderSection::operator=(cOther);
			m_nSectionIndex = cOther.m_nSectionIndex;
			m_nStartIndex = cOther.m_nStartIndex;
			m_nEndIndex = cOther.m_nEndIndex;
			m_nTriCount = cOther.m_nTriCount;
			m_nStartVertex = cOther.m_nStartVertex;
			m_nEndVertex = cOther.m_nEndVertex;
			m_nVertexCount = cOther.m_nVertexCount;
			return *this;
		}

		// Called to update the texture. This will handle sprite updating and should be
		// called before the texture is used while rendering
		void UpdateTexture()
		{
			for (uint32 nCurrSpr = 0; nCurrSpr < CRBSection::kNumTextures; nCurrSpr++)
			{
				// See if we even have a sprite
				if (!m_pSpriteData[nCurrSpr])
				{
					continue;
				}

				// Get the time delta, mul by 1000 since it is in seconds and we want
				// Milliseconds
				uint32 nDeltaTimeMS = (uint32)(g_pSceneDesc->m_FrameTime * 1000.0f);

				// We have a sprite, so let us update it, and select the new texture
				spr_UpdateTracker(&m_pSpriteData[nCurrSpr]->m_SpriteTracker, nDeltaTimeMS);
				if (m_pSpriteData[nCurrSpr]->m_SpriteTracker.m_pCurFrame)
				{
					SetTexture(m_pSpriteData[nCurrSpr]->m_SpriteTracker.m_pCurFrame->m_pTex);
				}
				else
				{
					SetTexture(LTNULL);
				}
			}
		}

		uint32 m_nSectionIndex;
		uint32 m_nStartIndex;
		uint32 m_nEndIndex;
		uint32 m_nTriCount;
		uint32 m_nStartVertex;
		uint32 m_nEndVertex;
		uint32 m_nVertexCount;
		uint32 m_nVBIndex;

		const uint16 *m_pOriginalIndices;
		const SRBVertex *m_pOriginalVertices;
		int32 m_nOriginalVertexOffset;

		// The sprite data if applicable
		CRBSection::CSpriteData *m_pSpriteData[CRBSection::kNumTextures];

		// Reference to the texture effect on this surface
		CTextureScriptInstance *m_pTextureEffect;

		bool m_bQueuedForRender;
		bool m_bSectionBreak;
	};

	typedef std::vector<CInternalSection> TSectionList;
	TSectionList m_aSections;

	struct CRenderBlockData
	{
		typedef std::vector<uint32> TSectionIndexList;
		TSectionIndexList m_aSections;
	};

	typedef std::vector<CRenderBlockData> TRenderBlockList;
	TRenderBlockList m_aRenderBlocks;

	typedef std::vector<uint32> TRenderBlockPreviewList;
	TRenderBlockPreviewList m_aPreviewRenderBlocks;

	// Classes for sorting.  (Not local to the function to help VC figure out what I mean)
	// Index for associating a section with a renderblock
	struct SSectionRBIndex
	{
		uint32 m_nSection;
		uint32 m_nRB;
	};

	// Functor class for sorting sections
	struct FSortSections
	{
		FSortSections(TSectionList &cSectionList) : m_aSections(cSectionList) {}
		bool operator()(const SSectionRBIndex &sLHS, const SSectionRBIndex &sRHS) const
		{
			return m_aSections[sLHS.m_nSection] < m_aSections[sRHS.m_nSection];
		}
		TSectionList &m_aSections;
	};

	IDirect3DIndexBuffer9 *m_pIB;
	uint32 m_nTotalTriCount;
	typedef std::vector<IDirect3DVertexBuffer9*> TVBList;
	TVBList m_aVBs;
	uint32 m_nTotalVertexCount;

	// The currently active VB
	uint32 m_nCurVB;
	// The number of currently queued sections
	uint32 m_nQueuedSections;

	bool m_bLocked;
	uint16 *m_pLockedIndices;
	// Note : This is supposed to be a vector of SVertex*, but the MS compiler runs out of heap space if you do that.
	typedef std::vector<void*> TLockedVertexList;
	TLockedVertexList m_aLockedVertices;

	// The internal section iterator
	uint32 m_nCurSection;
	uint32 m_nCurRB;
};



#endif //__D3D_RENDERSHADER_BASE_H__
