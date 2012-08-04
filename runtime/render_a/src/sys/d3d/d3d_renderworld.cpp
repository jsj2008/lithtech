//////////////////////////////////////////////////////////////////////////////
// D3D main world rendering implementation

#include "precompile.h"

#include "d3d_renderworld.h"
#include "d3d_convar.h"
#include "d3d_renderblock.h"
#include "d3d_rendershader.h"
#include "d3d_rendershader_dynamiclight.h"
#include "d3d_rendershader_gouraud.h"
#include "d3d_rendershader_gouraud_fullbright.h"
#include "d3d_rendershader_lightmap.h"
#include "modelshadowshader.h"
#include "d3d_viewparams.h"
#include "common_draw.h"
#include "drawsky.h"
#include "counter.h"
#include "iaggregateshader.h"
#include "rendererframestats.h"
#include "d3d_texture.h"

#include "strtools.h"

#include <vector>
#include <deque>
#include <stack>

CD3D_RenderWorld::CD3D_RenderWorld() :
	m_pRenderBlocks(0),
	m_nRenderBlockCount(0),
	m_nOccludeCollectTime(0),
	m_bBlocksDirty(true)
{
	for (uint32 nShaderLoop = 0; nShaderLoop < k_eShader_Num + 2; ++nShaderLoop)
		m_aShaders[nShaderLoop] = 0;
}

CD3D_RenderWorld::~CD3D_RenderWorld()
{
	// Take the Load/Render critical section so we don't crash if we're deleting
	// before a load
	CSAccess cLoadRenderCSLock(&g_Device.GetLoadRenderCS());

	Release();
	delete[] m_pRenderBlocks;

	TWorldModelMap::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
	{
		delete iCurWorldModel->second;
	}
}

void CD3D_RenderWorld::Release()
{
	if (m_bBlocksDirty)
		return;

	m_cTotalMemStats.Clear();

	// Release the renderblocks
	for (uint32 nCurRenderBlock = 0; nCurRenderBlock < m_nRenderBlockCount; ++nCurRenderBlock)
		m_pRenderBlocks[nCurRenderBlock].Release();

	// Delete the shaders
	for (uint32 nShaderLoop = 0; nShaderLoop < k_eShader_Num; ++nShaderLoop)
	{
		delete m_aShaders[nShaderLoop];
		m_aShaders[nShaderLoop] = 0;
	}

	// Release the worldmodels
	TWorldModelMap::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
	{
		iCurWorldModel->second->Release();
	}

	// Release the static shaders
	CRenderShader_DynamicLight::GetSingleton()->Release();
	CModelShadowShader::GetSingleton()->Release();

	m_bBlocksDirty = true;
}

uint32 CD3D_RenderWorld::GetWMNameHash(const char *pName)
{
	return st_GetHashCode(pName);
}

CD3D_RenderWorld *CD3D_RenderWorld::FindWorldModel(const char *pName)
{
	TWorldModelMap::iterator iWorldModel = m_aWorldModels.find(GetWMNameHash(pName));
	if (iWorldModel == m_aWorldModels.end())
		return 0;

	return iWorldModel->second;
}

bool CD3D_RenderWorld::Load(ILTStream *pStream)
{
	*pStream >> m_nRenderBlockCount;

	if (m_nRenderBlockCount)
	{
		LT_MEM_TRACK_ALLOC(m_pRenderBlocks = new CD3D_RenderBlock[m_nRenderBlockCount],LT_MEM_TYPE_RENDER_WORLD);
		for (uint32 nCurRenderBlock = 0; nCurRenderBlock < m_nRenderBlockCount; ++nCurRenderBlock)
		{
			m_pRenderBlocks[nCurRenderBlock].SetWorld(this);
		}
	}

	bool bResult = true;

	// Load the blocks
	for (uint32 nReadLoop = 0; nReadLoop < m_nRenderBlockCount; ++nReadLoop)
	{
		if (!m_pRenderBlocks[nReadLoop].Load(pStream))
		{
			// Cut off the array and remember that we failed
			m_nRenderBlockCount = nReadLoop;
			bResult = false;
			break;
		}
	}

	// Fix-up their child pointers
	for (uint32 nFixupLoop = 0; nFixupLoop < m_nRenderBlockCount; ++nFixupLoop)
	{
		m_pRenderBlocks[nFixupLoop].FixupChildren(m_pRenderBlocks);
	}

	m_bBlocksDirty = true;

	// Load the worldmodels
	uint32 nNumWorldModels;
	*pStream >> (uint32)nNumWorldModels;

	m_aWorldModels.clear();

	for (uint32 nCurWorldModel = 0; nCurWorldModel < nNumWorldModels; ++nCurWorldModel)
	{
		char sWMName[MAX_WORLDNAME_LEN+1];
		pStream->ReadString(sWMName, sizeof(sWMName));

		CD3D_RenderWorld *pNewWorldModel;
		LT_MEM_TRACK_ALLOC(pNewWorldModel = new CD3D_RenderWorld, LT_MEM_TYPE_RENDER_WORLD);
		if (!pNewWorldModel->Load(pStream))
		{
			ASSERT(!"Error loading world model");
			return false;
		}

		uint32 nNameHash = GetWMNameHash(sWMName);
		TWorldModelMap::iterator iFindWMNameHash = m_aWorldModels.find(nNameHash);
		if (iFindWMNameHash != m_aWorldModels.end())
		{
			ASSERT(!"World model name hash conflict detected!");
			dsi_ConsolePrint("Error loading world model %s due to name hash conflict", sWMName);
			delete pNewWorldModel;
			continue;
		}

		LT_MEM_TRACK_ALLOC(m_aWorldModels[nNameHash] = pNewWorldModel, LT_MEM_TYPE_RENDER_WORLD);
	}

	return bResult;
}

void CD3D_RenderWorld::GetRBChildrenSorted(const ViewParams& Params, CD3D_RenderBlock *pCurBlock, CD3D_RenderBlock *&pChild1, CD3D_RenderBlock *&pChild2)
{
	ASSERT(CD3D_RenderBlock::GetNumChildren() == 2);
	pChild1 = pCurBlock->GetChild(0);
	pChild2 = pCurBlock->GetChild(1);
	if (!pChild1 || !pChild2)
		return;
	// Sort the children in rough front-to-back order 
	float fForward1 = Params.m_Forward.Dot(pChild1->GetCenter() - Params.m_Pos);
	float fForward2 = Params.m_Forward.Dot(pChild2->GetCenter() - Params.m_Pos);
	if (fForward1 > fForward2)
	{
		CD3D_RenderBlock *pTemp = pChild2;
		pChild2 = pChild1;
		pChild1 = pTemp;
	}
}

void CD3D_RenderWorld::GetFrustumRBList(const ViewParams& Params, TRBList &cList)
{
	typedef std::stack<CD3D_RenderBlock*> TRBQueue;
	static TRBQueue cRBQueue;

	cRBQueue.push(m_pRenderBlocks);
	while (!cRBQueue.empty())
	{
		CD3D_RenderBlock *pCurBlock = cRBQueue.top();
		cRBQueue.pop();
		if (!Params.ViewAABBIntersect(pCurBlock->GetBoundsMin(), pCurBlock->GetBoundsMax()))
			continue;
		cList.push_back(pCurBlock);
		CD3D_RenderBlock *pChild1, *pChild2;
		GetRBChildrenSorted(Params, pCurBlock, pChild1, pChild2);
		if (pChild2)
			cRBQueue.push(pChild2);
		if (pChild1)
			cRBQueue.push(pChild1);
	}
}

void CD3D_RenderWorld::DrawOccluder(const COccludee::COutline &cOutline)
{
	StateSet NoWireframe(D3DRS_FILLMODE, D3DFILL_SOLID);

	StateSet Alpha(D3DRS_ALPHABLENDENABLE, TRUE);
	StateSet SrcBlend(D3DRS_SRCBLEND, D3DBLEND_ONE);
	StateSet DestBlend(D3DRS_DESTBLEND, D3DBLEND_ONE);
	StateSet ZWrite(D3DRS_ZWRITEENABLE, FALSE);
	StateSet ZFunc(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	StateSet CullMode(D3DRS_CULLMODE, D3DCULL_NONE);

	StageStateSet tss00(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	StageStateSet tss01(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	StageStateSet tss02(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	StageStateSet tss03(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	struct SOccluderVert
	{
		LTVector m_vPos;
		uint32 m_nColor;
	};
	static SOccluderVert aOccluderBuffer[16];
	ASSERT(cOutline.size() <= 16);

	for (uint32 nFillLoop = 0; nFillLoop < cOutline.size(); ++nFillLoop)
	{
		aOccluderBuffer[nFillLoop].m_vPos = cOutline[nFillLoop];
		aOccluderBuffer[nFillLoop].m_nColor = 0xFF202020;
	}

	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, cOutline.size() - 2, aOccluderBuffer, sizeof(SOccluderVert));
}

void CD3D_RenderWorld::StartFrame(const ViewParams& Params)
{
	m_aOccluderOutlines.clear();
	m_nOccludeCollectTime = 0;

	// Don't do any occluder stuff if we're fiddling with console variables
	if (g_CV_DebugRBAll.m_Val || g_CV_DebugRBCur.m_Val || g_CV_DebugRBFrustum.m_Val)
		return;

	// Build a special occluder for the view frustum
	m_cFrameFrustumOccluder.InitFrustum(Params);

	// Get the rest of the occluders
	{
		CountAdder cOccludeCollect(&m_nOccludeCollectTime);

		GetFrameOccluders(Params);
	}
}

void CD3D_RenderWorld::GetFrameOccluders(const ViewParams& Params)
{
	m_aFrameOccluders.clear();

	if (g_CV_DebugRBNoOccluders.m_Val != 0.0f)
		return;

	typedef std::vector<CD3D_RenderBlock*> TRBQueue;
	static TRBQueue cRBQueue;

	// A static occludee to use for finding occluded occluders
	static COccludee cOccludee;
	cOccludee.Init();

	m_fFrameOccluders_ScreenArea = 0.0f;

	// Get the occluders from the RB's in the frustum
	cRBQueue.push_back(m_pRenderBlocks);
	while (!cRBQueue.empty())
	{
		CD3D_RenderBlock *pCurBlock = cRBQueue.back();
		cRBQueue.pop_back();
		if (!Params.ViewAABBIntersect(pCurBlock->GetBoundsMin(), pCurBlock->GetBoundsMax()))
			continue;

		CD3D_RenderBlock *pChild1, *pChild2;
		pChild1 = pCurBlock->GetChild(0);
		pChild2 = pCurBlock->GetChild(1);
		if (pChild1)
			cRBQueue.push_back(pChild1);
		if (pChild2)
			cRBQueue.push_back(pChild2);
		
		uint32 nOccluderCount = pCurBlock->GetNumOccluders();
		for (uint32 nCurOccluder = 0; nCurOccluder < nOccluderCount; ++nCurOccluder)
		{
			SRBOccluder &sOccluderPoly = pCurBlock->GetOccluder(nCurOccluder);

			// Skip occluders that aren't enabled
			if (!sOccluderPoly.m_bEnabled)
				continue;

			cOccludee.InitPolyWorld(sOccluderPoly, true);

			// Skip it if it's outside the frustum
			if (m_cFrameFrustumOccluder.Classify(cOccludee) == BackSide)
				continue;

			// Clip the occluder to the near plane
			if (m_cFrameFrustumOccluder.ClipNear(cOccludee) == BackSide)
				continue;

			if (g_CV_DebugRBDrawOccluders.m_Val)
				m_aOccluderOutlines.push_back(cOccludee.m_aVisible[0]);

			ASSERT(cOccludee.m_aVisible.size() == 1);
			
			// Grow the occluder list and initialize directly into the new one
			// Instead of making a new one and copying it onto the end
			LT_MEM_TRACK_ALLOC(m_aFrameOccluders.resize(m_aFrameOccluders.size() + 1), LT_MEM_TYPE_RENDERER);
			COccluder_2D &cNewOccluder = m_aFrameOccluders.back();
			cNewOccluder.InitOutline(Params, cOccludee.m_aVisible[0], sOccluderPoly.m_cPlane);
			m_fFrameOccluders_ScreenArea += cNewOccluder.m_fScreenArea;
		}
	}

	// Sort the occluders by screen area
	if (g_CV_DebugRBSortOccluders.m_Val && (m_aFrameOccluders.size() > 1))
	{
		// Hooray for bubble sort!
		TOccluderList::iterator iFirst = m_aFrameOccluders.begin();
		TOccluderList::iterator iCompare;
		for (; (iCompare = iFirst + 1) != m_aFrameOccluders.end(); ++iFirst)
		{
			TOccluderList::iterator iBiggest = iFirst;
			for (; iCompare != m_aFrameOccluders.end(); ++iCompare)
			{
				if (iCompare->m_fScreenArea > iBiggest->m_fScreenArea)
					iBiggest = iCompare;
			}
			if (iBiggest != iFirst)
				iBiggest->Swap(*iFirst);
		}
	}
}

bool CD3D_RenderWorld::IsAABBVisible(const ViewParams& Params, const LTVector &vMin, const LTVector &vMax, bool bUseOccluders) const
{
	if (!m_pRenderBlocks)
		return true;

	// Check for a box which actually contains the viewing position
	if ((vMin.x <= Params.m_Pos.x) && (vMin.y <= Params.m_Pos.y) && (vMin.z <= Params.m_Pos.z) &&
		(vMax.x >= Params.m_Pos.x) && (vMax.y >= Params.m_Pos.y) && (vMax.z >= Params.m_Pos.z))
	{
		IncFrameStat(eFS_InsideBoxCullCount, 1);
		return true;
	}

	// Use a static occludee
	static COccludee cOccludee;
	cOccludee.Init();

	PolySide nBoxSide = m_cFrameFrustumOccluder.ClassifyAABB(vMin, vMax, Params.m_FarZ);
	if (nBoxSide == BackSide)
	{
		IncFrameStat(eFS_FrustumCulledCount, 1);
		return false;
	}

	// Short-cut out if there aren't any visible occluders
	if (!bUseOccluders || m_aFrameOccluders.empty())
		return true;

	cOccludee.InitAABB(Params, vMin, vMax, true);
	if (nBoxSide == Intersect)
	{
		// Clip it to the frustum
		if (m_cFrameFrustumOccluder.Occlude2D(cOccludee) == BackSide)
		{
			// This generally means we're not going to be able to properly occlude this AABB, so consider it visible..
			//ASSERT(!"Frustum ClassifyAABB/Occlude2D result mismatch encountered");
			return true;
		}
	}

	// Find out if it's visible
	bool bVisible = true;
	
	float fOccludeeArea = cOccludee.CalcArea2D(true);
	float fOccluderAreaRemaining = m_fFrameOccluders_ScreenArea;

	TOccluderList::const_iterator iCurOccluder = m_aFrameOccluders.begin();
	for (; bVisible && (fOccluderAreaRemaining >= fOccludeeArea) && (iCurOccluder != m_aFrameOccluders.end()); ++iCurOccluder)
	{
		// Move the visible portions into the occluded bucket
		cOccludee.m_aOccluded.clear();
		cOccludee.m_aOccluded.swap(cOccludee.m_aVisible);
		PolySide nResult = iCurOccluder->Occlude2D(cOccludee);
		bVisible = nResult != BackSide;

		// Early-out based on remaining screen area...
		if (nResult == Intersect)
		{
			// Note : It's faster to remove the occluded area than to calculate 
			// the visible area because the occluded area is always going to be 
			// a single convex polygon at this point.
			// Also note : It's quite possible that this re-calculation isn't
			// making a very significant difference.  In my testing (based on
			// adding some variables to track the percentage of occluders that
			// get skipped based on remaining occluder area), it looks like
			// the difference between doing this re-calculation and not doing this
			// re-calculation changes the percentage of skipped occluders by < 1%.
			// However, as this is guaranteed to make the early-out happen earlier
			// (if it changes anything), and it wasn't slowing the framerate down
			// in any appreciable fashion, I'm leaving it in.
			fOccludeeArea -= cOccludee.CalcArea2D(false);
		}
		// Adjust the occluder area remaining
		fOccluderAreaRemaining -= iCurOccluder->m_fScreenArea;

		ASSERT((nResult != FrontSide) || (cOccludee.m_aOccluded.empty()));
	}

	if(!bVisible)
		IncFrameStat(eFS_OccluderCulledCount, 1);	

	return bVisible;
}

void CD3D_RenderWorld::BindAll()
{
	if (!m_bBlocksDirty)
		return;

	m_cTotalMemStats.Clear();

	// Make sure the static shaders are bound
	CRenderShader_DynamicLight::GetSingleton()->Bind();
	CModelShadowShader::GetSingleton()->Bind();

	// Pre-bind the renderblocks
	for (uint32 nRBBindLoop = 0; nRBBindLoop < m_nRenderBlockCount; ++nRBBindLoop)
	{
		m_pRenderBlocks[nRBBindLoop].PreBind();
	}

	// Bind the renderblocks
	for (uint32 nRBBindLoop = 0; nRBBindLoop < m_nRenderBlockCount; ++nRBBindLoop)
	{
		m_pRenderBlocks[nRBBindLoop].Bind();
	}

	//Build up the memory stats
	for(uint32 nCurrShader = 0; nCurrShader < k_eShader_Num; nCurrShader++)
	{
		if(m_aShaders[nCurrShader])
			m_aShaders[nCurrShader]->GetMemStats(m_cTotalMemStats);
	}

	// Bind all the worldmodels, too....
	TWorldModelMap::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
	{
		// Inherit the texture usage
		iCurWorldModel->second->m_cTotalMemStats.FilterTextures(m_cTotalMemStats);

		iCurWorldModel->second->BindAll();
	}

	m_bBlocksDirty = false;
}


// Support for aggregate shaders. This allows for the passing in of view parameters and
// then specifying a shader that should be used for rendering all render blocks that
// intersect the frustum. 
bool CD3D_RenderWorld::DrawAggregateShader(uint32 nNumFrustumPlanes, const LTPlane* pFrustum, const ViewParams& Params, IAggregateShader *pShader)
{
	// Make sure parameters are valid
	if (((nNumFrustumPlanes > 0) && !pFrustum) || !pShader || !m_pRenderBlocks)
		return false;

	// Try and initialize the shader
	if (!pShader->BeginRendering())
		return false;

	// Now we need to find all intersecting render blocks
	static TRBList aVisibleRBs;
	aVisibleRBs.clear();

	GetFrustumRBList(Params, aVisibleRBs);

	EAABBCorner *aCorners = (EAABBCorner*)alloca(nNumFrustumPlanes * sizeof(EAABBCorner));
	for (uint32 nCurCorner = 0; nCurCorner < nNumFrustumPlanes; ++nCurCorner)
		aCorners[nCurCorner] = GetAABBPlaneCorner(pFrustum[nCurCorner].m_Normal);

	// Now we need to render each and every shader
	TRBList::iterator iCurRB = aVisibleRBs.begin();

	for (; iCurRB != aVisibleRBs.end(); ++iCurRB)
	{
		// We know that this render block is in the visible frustum, but now we need to make
		// Sure that it is in the aggreagte's frustum (so we don't waste time rendering blocks
		// That would either be a waste of time, or potentially cause artifacts rendering)
		bool bOccluded = false;

		const LTVector& vMin = (*iCurRB)->GetBoundsMin();
		const LTVector& vMax = (*iCurRB)->GetBoundsMax();

		for (uint32 nCurrPlane = 0; nCurrPlane < nNumFrustumPlanes; nCurrPlane++)
		{
			// See if this plane separates the bounding box
			const LTPlane& Plane = pFrustum[nCurrPlane];			

			if (GetAABBPlaneSideBack(aCorners[nCurrPlane], Plane, vMin, vMax))
			{
				bOccluded = true;
				break;
			}
		}

		if (!bOccluded)
		{
			(*iCurRB)->DrawAggregateShader(pShader);
		}
	}

	// Now we end the renderer and return the status
	bool bResult = pShader->EndRendering();

	return bResult;
}


void CD3D_RenderWorld::Draw(const ViewParams& Params, bool bTransformed)
{
	if (!m_pRenderBlocks)
		return;

	if (m_bBlocksDirty)
	{
		BindAll();
	}

	LT_MEM_TRACK_ALLOC(m_aVisibleRBs.reserve(m_nRenderBlockCount), LT_MEM_TYPE_RENDERER);
	m_aVisibleRBs.clear();

	uint32 nOccludeTime = 0;
	uint32 nDrawTime = 0;

	// Implementation note: Transformed world rendering doesn't do any occlusion or frustum checking
	if (g_CV_DebugRBAll.m_Val || bTransformed)
	{
		for (uint32 nRBDrawLoop = 0; nRBDrawLoop < m_nRenderBlockCount; ++nRBDrawLoop)
		{
			m_aVisibleRBs.push_back(&m_pRenderBlocks[nRBDrawLoop]);
		}
	}
	else if (g_CV_DebugRBCur.m_Val != 0)
	{
		// Draw the blocks we're inside of

		CD3D_RenderBlock *pInsideBlock = m_pRenderBlocks;
		if (pInsideBlock->IsPtInside(Params.m_Pos))
		{
			while (pInsideBlock)
			{
				m_aVisibleRBs.push_back(pInsideBlock);
				CD3D_RenderBlock *pChild = LTNULL;
				uint32 nChildIndex = 0;
				for(; !pChild && (nChildIndex < CD3D_RenderBlock::GetNumChildren()); ++nChildIndex)
				{
					pChild = pInsideBlock->GetChild(nChildIndex);
					if (pChild && !pChild->IsPtInside(Params.m_Pos))
						pChild = LTNULL;
				}
				pInsideBlock = pChild;
			}
		}	
		else
		{
			for (uint32 nRBDrawLoop = 0; nRBDrawLoop < m_nRenderBlockCount; ++nRBDrawLoop)
			{
				m_aVisibleRBs.push_back(&m_pRenderBlocks[nRBDrawLoop]);
			}
		}
	}
	else if (g_CV_DebugRBFrustum.m_Val)
	{
		// Get the list of render blocks inside the frustum
		GetFrustumRBList(Params, m_aVisibleRBs);
	}
	else
	{
		// Only do the full occlusion if our occluder set isn't empty 
		// (i.e. use the frustum set if we can)
		if (m_aFrameOccluders.empty() && (g_CV_DebugRBNoOccluders.m_Val == 0.0f))
			GetFrustumRBList(Params, m_aVisibleRBs);
		else
		{
			IncFrameStat(eFS_Occluders, m_aFrameOccluders.size());

			CountAdder cOccludeCollect(&nOccludeTime);

			// Forget which ones are visible if we're going to use occlusion
			m_aVisibleRBs.clear();

			// Get the visible RB set
			typedef std::stack<CD3D_RenderBlock*> TRBQueue;
			static TRBQueue cRBQueue;

			cRBQueue.push(m_pRenderBlocks);
			while (!cRBQueue.empty())
			{
				CD3D_RenderBlock *pCurBlock = cRBQueue.top();
				cRBQueue.pop();

				IncFrameStat(eFS_WorldBlocksCullTested, 1);
				if (!IsAABBVisible(Params, pCurBlock->GetBoundsMin(), pCurBlock->GetBoundsMax(), true))
				{
					IncFrameStat(eFS_WorldBlocksCulled, 1);
					continue;
				}

				// Add it to the list and recurse into its children
				m_aVisibleRBs.push_back(pCurBlock);
				CD3D_RenderBlock *pChild1, *pChild2;
				GetRBChildrenSorted(Params, pCurBlock, pChild1, pChild2);
				if (pChild2)
					cRBQueue.push(pChild2);
				if (pChild1)
					cRBQueue.push(pChild1);
			}
		}
	}

	{
		CountAdder cDrawTime(&nDrawTime);

		// Extend the sky bounds, as long as we're not drawing a transformed world
		if (!bTransformed)
		{
			float fSkyMinX = FLT_MAX, fSkyMinY = FLT_MAX, fSkyMaxX = -FLT_MAX, fSkyMaxY = -FLT_MAX;

			// Get the sky portals from the RBs
			TRBList::iterator iCurRB = m_aVisibleRBs.begin();
			for (; iCurRB != m_aVisibleRBs.end(); ++iCurRB)
			{
				(*iCurRB)->ExtendSkyBounds(Params, fSkyMinX, fSkyMinY, fSkyMaxX, fSkyMaxY);
			}

			// Draw the sky
			if (((fSkyMaxX - fSkyMinX) > 0.9f) && ((fSkyMaxY - fSkyMinY) > 0.9f))
				d3d_DrawSkyExtents(Params, fSkyMinX, fSkyMinY, fSkyMaxX, fSkyMaxY);
		}

		// Set up some default states
		StageStateSet state00(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0);
		StageStateSet state10(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1);
		StageStateSet state20(2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 2);

		PD3DDEVICE->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		PD3DDEVICE->SetSoftwareVertexProcessing(((g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) ? 1 : 0);
		PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

		if (g_CV_DebugRBDraw.m_Val)
		{
			// Tell the render blocks they're going to get drawn
			TRBList::iterator iCurRB = m_aVisibleRBs.begin();
			for (; iCurRB != m_aVisibleRBs.end(); ++iCurRB)
			{
				(*iCurRB)->PreDraw(Params);
			}

			// Draw the render blocks
			iCurRB = m_aVisibleRBs.begin();
			for (; iCurRB != m_aVisibleRBs.end(); ++iCurRB)
			{
				(*iCurRB)->Draw(Params);
			}

			// Flush the shaders 
			for (uint32 nCurShader = 0; nCurShader < k_eShader_Num; ++nCurShader)
			{
				if (m_aShaders[nCurShader])
					m_aShaders[nCurShader]->Flush();
			}
		}
	}

	if (g_CV_DebugRBDrawOccluders.m_Val)
	{
		COccludee::TOutlineList::iterator iCurOccluder = m_aOccluderOutlines.begin();
		for (; iCurOccluder != m_aOccluderOutlines.end(); ++iCurOccluder)
			DrawOccluder(*iCurOccluder);
	}
					
	if (g_CV_DebugRBTri.m_Val)
		DebugTri(Params);

	if (g_CV_DebugRBTime.m_Val)
		dsi_ConsolePrint("RB Time - Collect : %d  Occlude : %d  Draw : %d", m_nOccludeCollectTime, nOccludeTime, nDrawTime);

	if (g_CV_ShowMemStats_Render.m_Val)
	{
		if (!bTransformed)
		{
			dsi_ConsolePrint("****************************************************");
			dsi_ConsolePrint("Main World Data : ");
			m_cTotalMemStats.Report("    ");

			m_cVisibleMemStats.Clear();

			TRBList::iterator iCurRB = m_aVisibleRBs.begin();
			for (; iCurRB != m_aVisibleRBs.end(); ++iCurRB)
			{
				(*iCurRB)->GetMemStats(m_cVisibleMemStats);
			}

			CMemStats_World cWMStats;
			if (!m_aWorldModels.empty())
			{
				TWorldModelMap::const_iterator iCurWorldModel = m_aWorldModels.begin();
				for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
				{
					cWMStats += iCurWorldModel->second->m_cTotalMemStats;
				}
				dsi_ConsolePrint("WorldModel Data : ");
				cWMStats.Report("    ");
			}

			if (g_CV_ShowMemStats_Render.m_Val > 1)
			{
				dsi_ConsolePrint("  Visible Data : ");
				m_cVisibleMemStats.Report("    ");
			}

			dsi_ConsolePrint("****************************************************");
			dsi_ConsolePrint(" ");
		}
		else
		{
			if (g_CV_ShowMemStats_Render.m_Val > 1)
			{
				dsi_ConsolePrint("    Visible WorldModel Data : ");
				m_cTotalMemStats.Report("      ");
			}
		}
	}
}

void CD3D_RenderWorld::SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
	// Filter the call into the renderblocks
	for (uint32 nCurRB = 0; nCurRB < m_nRenderBlockCount; ++nCurRB)
	{
		m_pRenderBlocks[nCurRB].SetLightGroupColor(nID, vColor);
	}

	// Filter it into the worldmodels
	TWorldModelMap::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
	{
		iCurWorldModel->second->SetLightGroupColor(nID, vColor);
	}
}

bool CD3D_RenderWorld::SetOccluderEnabled(uint32 nID, bool bEnabled)
{
	bool bResult = false;

	// Filter the call into the renderblocks
	for (uint32 nCurRB = 0; nCurRB < m_nRenderBlockCount; ++nCurRB)
	{
		bResult |= m_pRenderBlocks[nCurRB].SetOccluderEnabled(nID, bEnabled);
	}

	// Filter it into the worldmodels
	TWorldModelMap::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; iCurWorldModel != m_aWorldModels.end(); ++iCurWorldModel)
	{
		bResult |= iCurWorldModel->second->SetOccluderEnabled(nID, bEnabled);
	}

	return bResult;
}

bool CD3D_RenderWorld::GetOccluderEnabled(uint32 nID, bool *pResult)
{
	bool bResult = false;

	// Filter the call into the renderblocks
	for (uint32 nCurRB = 0; (nCurRB < m_nRenderBlockCount) && (!bResult); ++nCurRB)
	{
		bResult |= m_pRenderBlocks[nCurRB].GetOccluderEnabled(nID, pResult);
	}

	// Filter it into the worldmodels
	TWorldModelMap::iterator iCurWorldModel = m_aWorldModels.begin();
	for (; (iCurWorldModel != m_aWorldModels.end()) && (!bResult); ++iCurWorldModel)
	{
		bResult |= iCurWorldModel->second->GetOccluderEnabled(nID, pResult);
	}

	return bResult;
}

CRenderShader *CD3D_RenderWorld::AllocShader(const CRBSection &cSection)
{
	RTexture *pRTexture[CRBSection::kNumTextures];
	
	for (uint32 nCurrTex = 0; nCurrTex < CRBSection::kNumTextures; nCurrTex++)
	{
		pRTexture[nCurrTex] = (cSection.m_pTexture[nCurrTex]) ? (RTexture*)cSection.m_pTexture[nCurrTex]->m_pRenderData : 0;
	}

	// The base texture (the one that should be used for getting properties)
	SharedTexture *pBaseTexture = cSection.m_pTexture[0];

	ERenderShader eResult;
	switch (cSection.m_eShader)
	{
		case eShader_Gouraud_DualTexture :
			eResult = eShader_Gouraud_DualTexture;
			break;
		case eShader_Lightmap_DualTexture:
			eResult = eShader_Lightmap_DualTexture;
			break;
		case eShader_Gouraud :
		case eShader_Lightmap_Texture:
			if (g_CV_DrawFlat)
				eResult = eShader_Gouraud;
			else if ((!g_CV_LightMap) || (cSection.m_eShader == eShader_Gouraud))
			{
				if (pBaseTexture)
				{
					eResult = eShader_Gouraud_Texture;
					if (pBaseTexture->m_eTexType == eSharedTexType_Detail)
					{
						if (g_CV_DetailTextures.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Detail_Fullbright;
							else
								eResult = eShader_Gouraud_Detail;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Texture_Fullbright;
							else
								eResult = eShader_Gouraud_Texture;
						}
					}
					else if (pBaseTexture->m_eTexType == eSharedTexType_EnvMap)
					{
						if (g_CV_EnvMapEnable.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_EnvMap;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Texture_Fullbright;
							else
								eResult = eShader_Gouraud_Texture;
						}
					}
					else if (pBaseTexture->m_eTexType == eSharedTexType_EnvMapAlpha)
					{
						if (g_CV_EnvMapEnable.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								// Don't have an appropriate fullbright, so just default to
								// The normal
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_Alpha_EnvMap;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Texture_Fullbright;
							else
								eResult = eShader_Gouraud_Texture;
						}
					}
					else if (pBaseTexture->m_eTexType == eSharedTexType_EnvBumpMap_NoFallback)
					{
						if (g_CV_EnvBumpMap.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_EnvBumpMap_NoFallback;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Texture_Fullbright;
							else
								eResult = eShader_Gouraud_Texture;
						}
					}
					else if (pBaseTexture->m_eTexType == eSharedTexType_EnvBumpMap)
					{
						if (g_CV_EnvBumpMap.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_EnvBumpMap;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_EnvMap;
						}
					}

					else if (pBaseTexture->m_eTexType == eSharedTexType_DOT3BumpMap)
					{
						if (g_CV_DOT3BumpMap.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Texture_Fullbright;  
							else
								eResult = eShader_Gouraud_DOT3BumpMap;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_Texture_Fullbright;
							else
								eResult = eShader_Gouraud_Texture;
						}
					}
					else if (pBaseTexture->m_eTexType == eSharedTexType_DOT3EnvBumpMap)
					{
						if (g_CV_DOT3EnvBumpMap.m_Val)
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_DOT3EnvBumpMap;
						}
						else
						{
							if (pRTexture[0] && pRTexture[0]->IsFullbrite())
								eResult = eShader_Gouraud_EnvMap_Fullbright;
							else
								eResult = eShader_Gouraud_EnvMap;
						}
					}
					else if (pBaseTexture->m_eTexType == eSharedTexType_Effect)
					{
						if (pRTexture[0] && pRTexture[0]->IsFullbrite())
						{
							eResult = eShader_Gouraud_Texture_Fullbright;  
						}
						else
						{
							eResult = eShader_Gouraud_Effect;
						}
					}
					else
					{
						if (pRTexture[0] && pRTexture[0]->IsFullbrite())
							eResult = eShader_Gouraud_Texture_Fullbright;
						else
							eResult = eShader_Gouraud_Texture;
					}
				}
				else
					eResult = eShader_Gouraud;
			}
			else if (g_CV_LightmapsOnly || !pBaseTexture)
				return 0;
			else
			{
				if (pBaseTexture->m_eTexType == eSharedTexType_Detail)
				{
					 if (g_CV_DetailTextures.m_Val)
						 eResult = eShader_Lightmap_Texture_Detail;
					 else
						 eResult = eShader_Lightmap_Texture;
				}
				else if (pBaseTexture->m_eTexType == eSharedTexType_EnvMap)
				{
					if (g_CV_EnvMapEnable.m_Val)
						eResult = eShader_Lightmap_Texture_EnvMap;
					else
						eResult = eShader_Lightmap_Texture;
				}
				else if (pBaseTexture->m_eTexType == eSharedTexType_EnvBumpMap_NoFallback)
				{
					if (g_CV_EnvBumpMap.m_Val)
						eResult = eShader_Lightmap_Texture_EnvBumpMap_NoFallback;
					else
						eResult = eShader_Lightmap_Texture;
				}
				else if (pBaseTexture->m_eTexType == eSharedTexType_EnvBumpMap)
				{
					if (g_CV_EnvBumpMap.m_Val)
						eResult = eShader_Lightmap_Texture_EnvBumpMap;
					else
					{
						if (g_CV_EnvMapEnable.m_Val)
							eResult = eShader_Lightmap_Texture_EnvMap;
						else
							eResult = eShader_Lightmap_Texture;
					}
				}
				else if (pBaseTexture->m_eTexType == eSharedTexType_DOT3BumpMap)
				{
					if (g_CV_DOT3BumpMap.m_Val)
						eResult = eShader_Lightmap_Texture_DOT3BumpMap;
					else
						eResult = eShader_Lightmap_Texture;
				}
				else if (pBaseTexture->m_eTexType == eSharedTexType_DOT3EnvBumpMap)
				{
					if (g_CV_DOT3EnvBumpMap.m_Val)
						eResult = eShader_Lightmap_Texture_DOT3EnvBumpMap;
					else
					{
						if (g_CV_EnvMapEnable.m_Val)
							eResult = eShader_Lightmap_Texture_EnvMap;
						else
							eResult = eShader_Lightmap_Texture;
					}
				}
				else
					eResult = eShader_Lightmap_Texture;
			}
			break;
		case eShader_Lightmap:
			if (g_CV_DrawFlat)
				return 0;
			else if (!g_CV_LightMap)
				return 0;
			else
				eResult = eShader_Lightmap;
			break;
		// Skip invalid shaders
		case eShader_Invalid :
			return 0;
		// You've got a shader that shouldn't be parsed..
		default : 
			eResult = cSection.m_eShader;
			break;
	}

	CRenderShader *pShader = 0;
	do 
	{
		pShader = m_aShaders[eResult];
		if (!pShader)
		{
			switch (eResult)
			{
				case eShader_Gouraud :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_Texture :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_Texture,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_Detail :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_Detail,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_EnvMap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_EnvMap(false),LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_Alpha_EnvMap:
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_EnvMap(true),LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_EnvBumpMap:
				case eShader_Gouraud_EnvBumpMap_NoFallback:
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_EnvBumpMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_DualTexture:
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_DualTexture,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_Texture_Fullbright :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_Texture_Fullbright,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_Detail_Fullbright :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_Detail_Fullbright,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_EnvMap_Fullbright :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_EnvMap_Fullbright,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_DOT3BumpMap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_DOT3BumpMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_DOT3EnvBumpMap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_DOT3EnvBumpMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Gouraud_Effect :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Gouraud_Effect,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_Texture :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_Texture_Detail :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture_Detail,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_Texture_EnvMap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture_EnvMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_Texture_EnvBumpMap :
				case eShader_Lightmap_Texture_EnvBumpMap_NoFallback :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture_EnvBumpMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_DualTexture :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture_DualTexture,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_Texture_DOT3BumpMap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture_DOT3BumpMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				case eShader_Lightmap_Texture_DOT3EnvBumpMap :
					LT_MEM_TRACK_ALLOC(pShader = new CRenderShader_Lightmap_Texture_DOT3EnvBumpMap,LT_MEM_TYPE_RENDER_SHADER);
					break;
				default :
					ASSERT(!"This shader is not yet implemented");
					pShader = 0;
					break;
			}
		}
		if (!pShader || !pShader->ValidateShader(cSection))
		{
			if (!m_aShaders[eResult])
				delete pShader;
			pShader = 0;
			// Choose a more stable shader.. (Would rather this happened automatically...)
			switch (eResult)
			{
				case eShader_Gouraud :
					ASSERT(!"Gouraud shader returned not validated");
					break;
				case eShader_Gouraud_Texture :
				case eShader_Lightmap :
					eResult = eShader_Gouraud;
					break;
				case eShader_Gouraud_Detail :
				case eShader_Gouraud_EnvMap :
				case eShader_Gouraud_Alpha_EnvMap:
				case eShader_Gouraud_DualTexture:
				case eShader_Gouraud_Texture_Fullbright :
				case eShader_Gouraud_EnvBumpMap_NoFallback :
				case eShader_Gouraud_DOT3BumpMap :
				case eShader_Gouraud_Effect :
					eResult = eShader_Gouraud_Texture;
					break;
				case eShader_Gouraud_Detail_Fullbright :
					eResult = eShader_Gouraud_Detail;
					break;
				case eShader_Gouraud_EnvMap_Fullbright :
				case eShader_Gouraud_EnvBumpMap:
				case eShader_Gouraud_DOT3EnvBumpMap :
					eResult = eShader_Gouraud_EnvMap;
					break;
				case eShader_Lightmap_Texture_EnvBumpMap :
				case eShader_Lightmap_Texture_DOT3EnvBumpMap :
					eResult = eShader_Lightmap_Texture_EnvMap;
					break;
				case eShader_Lightmap_Texture_Detail :
				case eShader_Lightmap_Texture_EnvMap :
				case eShader_Lightmap_DualTexture:
				case eShader_Lightmap_Texture_EnvBumpMap_NoFallback :
				case eShader_Lightmap_Texture_DOT3BumpMap :
					eResult = eShader_Lightmap_Texture;
					break;
				case eShader_Lightmap_Texture :
					// Note : This will cause overdraw since the lightmap texture
					// shader is an additive pass.
					eResult = eShader_Gouraud_Texture;
					break;
				default :
					eResult = eShader_Gouraud;
					break;
			}
		}
	} while (!pShader);

	m_aShaders[eResult] = pShader;

	return pShader;
}

void CD3D_RenderWorld::DebugTri(const ViewParams& Params)
{
	CD3D_RenderBlock::SRay sRay(Params.m_Pos, Params.m_Forward);
	CD3D_RenderBlock::TIntersectionList aIntersections;
	uint32 nIntersectionBlock = 0;

	for (uint32 nIntersectLoop = 0; nIntersectLoop < m_nRenderBlockCount; ++nIntersectLoop)
	{
		CD3D_RenderBlock::TIntersectionList aBlockIntersections;
		if (!m_pRenderBlocks[nIntersectLoop].IntersectRay(sRay, aBlockIntersections))
			continue;

		// Remember that the intersections are in this block
		nIntersectionBlock = nIntersectLoop;

		// Clear the current intersection list
		aIntersections.clear();

		// Find the closest intersection distance
		float fClosestDist = aBlockIntersections[0].m_fDistance;
		for (uint32 nClosestLoop = 1; nClosestLoop < aBlockIntersections.size(); ++nClosestLoop)
		{
			fClosestDist = LTMIN(fClosestDist, aBlockIntersections[nClosestLoop].m_fDistance);
		}

		// Update the maximum raycast distance
		sRay.m_fMax = fClosestDist;

		// Add the triangles which are similarly close to the intersection list
		for (uint32 nAddLoop = 0; nAddLoop < aBlockIntersections.size(); ++nAddLoop)
		{
			if (fabs(aBlockIntersections[nAddLoop].m_fDistance - fClosestDist) < 0.01f)
				aIntersections.push_back(aBlockIntersections[nAddLoop]);
		}
	}

	float fSizeX = Params.m_fScreenWidth * 0.25f, fSizeY = Params.m_fScreenHeight * 0.25f;
	float fCurX = 0.0f, fCurY = 0.0f;
	// Display the results
	for (uint32 nDrawLoop = 0; nDrawLoop < aIntersections.size(); ++nDrawLoop)
	{
		m_pRenderBlocks[nIntersectionBlock].DebugTri(aIntersections[nDrawLoop], 
			fCurX, fCurY, fSizeX, fSizeY);
		fCurX += fSizeX;
		if (fCurX > (Params.m_fScreenWidth - fSizeX))
		{
			fCurX = 0.0f;
			fCurY += fSizeY;
		}
	}
}

bool CD3D_RenderWorld::CastRay(
	const LTVector &vOrigin, const LTVector &vDir, float fMinDist, float fMaxDist,
	bool bFirstIntersect,
	LTVector *pResult_Position, float *pResult_Distance,
	LTRGB *pResult_LightingColor, 
	SharedTexture **pResult_Texture, LTRGB *pResult_TextureColor,
	LTVector *pResult_InterpNormal, LTVector *pResult_TriNormal)
{
	CD3D_RenderBlock::SRay sRay(vOrigin, vDir, fMinDist, fMaxDist);
	static CD3D_RenderBlock::TIntersectionList aBlockIntersections;
	static CD3D_RenderBlock::TIntersectionList aFinalIntersections;
	aFinalIntersections.clear();
	CD3D_RenderBlock::EIntersectionType eIntersection;
	if (bFirstIntersect)
		eIntersection = CD3D_RenderBlock::eIntersectionType_First;
	else 
		eIntersection = CD3D_RenderBlock::eIntersectionType_Any;

	uint32 nIntersectionBlock = 0;
	for (uint32 nIntersectLoop = 0; nIntersectLoop < m_nRenderBlockCount; ++nIntersectLoop)
	{
		aBlockIntersections.clear();

		uint32 nBlockTrisChecked;
		if (!m_pRenderBlocks[nIntersectLoop].IntersectRay(sRay, aBlockIntersections, eIntersection, &nBlockTrisChecked))
			continue;

		// Remember which renderblock we hit
		nIntersectionBlock = nIntersectLoop;

		// Update the maximum ray distance
		sRay.m_fMax = aBlockIntersections.front().m_fDistance;

		// Remember the results
		aFinalIntersections.swap(aBlockIntersections);
	}

	// Did we hit anything?
	if (aFinalIntersections.empty())
		return false;

	// Look up the results

	// Position
	if (pResult_Position)
		*pResult_Position = sRay.m_vOrigin + sRay.m_vDir * sRay.m_fMax;

	// Distance
	if (pResult_Distance)
		*pResult_Distance = sRay.m_fMax;

	// Ask the renderblock for anything else we need
	CD3D_RenderBlock::TIntersectionList::iterator iCurIntersection = aFinalIntersections.begin();
	for (; iCurIntersection != aFinalIntersections.end(); ++iCurIntersection)
	{
		m_pRenderBlocks[nIntersectionBlock].GetIntersectInfo(*iCurIntersection, 
			pResult_LightingColor, 
			pResult_Texture, 
			pResult_TextureColor, 
			pResult_InterpNormal,
			pResult_TriNormal);
	}

	// Don't leave anything lying around
	aFinalIntersections.clear();

	return true;
}

