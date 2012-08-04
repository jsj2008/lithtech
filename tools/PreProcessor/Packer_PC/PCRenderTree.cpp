//////////////////////////////////////////////////////////////////////////////
// PC-specific octree representation of the world

#pragma warning(disable:4786)

#include "bdefs.h"

#include "preworld.h"
#include "prepoly.h"
#include "pcrendertree.h"
#include "pcrendertreenode.h"
#include "pcrendershaders.h"
#include "pcfileio.h"
#include <stack>

#include "processing.h"

//////////////////////////////////////////////////////////////////////////////
// CPCRenderTree implementation

CPCRenderTree::CPCRenderTree() :
	m_pRoot(0)
{
}

CPCRenderTree::CPCRenderTree(const CPCRenderTree &cOther) :
	m_cLMData(cOther.m_cLMData)
{
	if (cOther.m_pRoot)
		m_pRoot = cOther.m_pRoot->Clone(this);
	else
		m_pRoot = 0;
}

CPCRenderTree &CPCRenderTree::operator=(const CPCRenderTree &cOther)
{
	delete m_pRoot;
	if (cOther.m_pRoot)
		m_pRoot = cOther.m_pRoot->Clone(this);
	else
		m_pRoot = 0;
	m_cLMData = cOther.m_cLMData;
	return *this;
}

CPCRenderTree::~CPCRenderTree()
{
	delete m_pRoot;
}

bool CPCRenderTree::ReadWorld(const CPreMainWorld *pMainWorld, const CPreWorld *pWorld)
{
	// Make sure the world's polys have bounding boxes
	for (GPOS iBoundsPoly = pWorld->m_Polies.GetHeadPosition(); iBoundsPoly; )
	{
		const CPrePoly *pCurPoly = pWorld->m_Polies.GetNext(iBoundsPoly);
		pCurPoly->CalcExtents(0.01f);
	}

	delete m_pRoot;

	// Set up the root node
	m_pRoot = new CPCRenderTreeNode(this);
	LTVector vHalfDims = (pWorld->m_PosMax - pWorld->m_PosMin) * 0.5f;
	m_pRoot->SetCenter(vHalfDims + pWorld->m_PosMin);
	m_pRoot->SetHalfDims(vHalfDims);

	uint32 nWorldIndex = pMainWorld->GetWorldIndex(pWorld);

	// Add the polys from the lightmaps
	if (pMainWorld->m_LightAnims.GetSize() > 0)
	{
		CPreLightAnim *pCurLMAnim = pMainWorld->m_LightAnims[0];
		m_pRoot->SetCurLightmap(pCurLMAnim);
		// Get the per-poly lightmap data map
		TPolyLMDataMap &cPolyLMData = m_cLMData[pCurLMAnim];

		// Add the polys to the root node
		for (uint32 nLMPoly = 0; nLMPoly < pCurLMAnim->m_Polies.GetSize(); ++nLMPoly)
		{
			CPrePolyRef cPolyRef = pCurLMAnim->m_Polies[nLMPoly];
			// Skip over lightmaps that aren't on the main world
			if (cPolyRef.m_iWorld != nWorldIndex)
				continue;
			const CPrePoly *pPoly = pMainWorld->GetLMPoly(&cPolyRef);
			if (!pPoly)
			{
				ASSERT(!"Invalid poly ref found in lightmap animation");
				continue;
			}
			// Add the poly to the lightmap data
			// Note : Only first frame of the animation is supported at this point.  :(
			cPolyLMData[pPoly] = pCurLMAnim->m_Frames[0]->m_PolyMaps[nLMPoly];

			// Add the poly to the root node
			m_pRoot->AddPoly(pPoly, false);
		}
	}

	m_pRoot->SetCurLightmap(LTNULL);
	// Add the polys for the texturing
	for (GPOS iAddPoly = pWorld->m_Polies.GetHeadPosition(); iAddPoly; )
	{
		m_pRoot->AddPoly(pWorld->m_Polies.GetNext(iAddPoly), true);
	}

	// Copy the light group list
	m_aLightGroupList.clear();
	m_aLightGroupList.insert(m_aLightGroupList.end(), pMainWorld->m_aLightGroups.begin(), pMainWorld->m_aLightGroups.end());

	return true;
}

uint32 CPCRenderTree::CountNodes()
{
	if (!m_pRoot)
		return 0;

	uint32 nResult = 0;

	std::stack<CPCRenderTreeNode*> cNodeStack;
	cNodeStack.push(m_pRoot);
	while (!cNodeStack.empty())
	{
		// Get the current node off the top
		CPCRenderTreeNode *pCurNode = cNodeStack.top();
		cNodeStack.pop();

		// Set the node's index
		pCurNode->SetIndex(nResult);

		// Count it
		++nResult;

		// Push the children on the stack
		for (uint32 nChildLoop = 0; nChildLoop < CPCRenderTreeNode::k_NumChildren; ++nChildLoop)
		{
			CPCRenderTreeNode *pChild = pCurNode->GetChild(nChildLoop);
			if (pChild)
				cNodeStack.push(pChild);
		}
	}

	return nResult;
}

bool CPCRenderTree::Write(CAbstractIO &file)
{
	// Count the nodes

	file << (uint32)CountNodes();

	// Write out the tree
	std::stack<CPCRenderTreeNode*> cNodeStack;
	cNodeStack.push(m_pRoot);
	while (!cNodeStack.empty())
	{
		// Get the current node off the top
		CPCRenderTreeNode *pCurNode = cNodeStack.top();
		cNodeStack.pop();

		// Write it
		if (!pCurNode->Write(file))
			return false;

		// Push the children on the stack
		for (uint32 nChildLoop = 0; nChildLoop < CPCRenderTreeNode::k_NumChildren; ++nChildLoop)
		{
			CPCRenderTreeNode *pChild = pCurNode->GetChild(nChildLoop);
			if (pChild)
				cNodeStack.push(pChild);
		}
	}

	return true;
}

bool CPCRenderTree::Optimize(bool bDisplayStats)
{
	CPCRenderTreeNode::SOptimizeStats sOptStats;

	m_pRoot->ReduceTriCount();

	std::stack<CPCRenderTreeNode*> cNodeStack;
	cNodeStack.push(m_pRoot);
	while (!cNodeStack.empty())
	{
		// Get the current node off the top
		CPCRenderTreeNode *pCurNode = cNodeStack.top();
		cNodeStack.pop();

		// Optimize it
		pCurNode->Optimize(&sOptStats);

		// Add the light group data
		pCurNode->ReadLightGroups( &(*(m_aLightGroupList.begin())), m_aLightGroupList.size());

		// Push the children on the stack
		for (uint32 nChildLoop = 0; nChildLoop < CPCRenderTreeNode::k_NumChildren; ++nChildLoop)
		{
			CPCRenderTreeNode *pChild = pCurNode->GetChild(nChildLoop);
			if (pChild)
				cNodeStack.push(pChild);
		}

	}

	if (sOptStats.m_nLMPages && bDisplayStats)
	{
		float fWastedSpace = ((float)sOptStats.m_nLMPageArea - (float)sOptStats.m_nLMArea) / (float)sOptStats.m_nLMPageArea;
		DrawStatusText(eST_Normal, "    LM Data: %d pages, %d bytes, %d%% wasted",
			sOptStats.m_nLMPages, sOptStats.m_nLMPageArea * 4, (uint32)((fWastedSpace * 100.0f) + 0.5f));
	}

	return true;
}

const CPreLightMap *CPCRenderTree::GetPolyLMData(const CPreLightAnim *pLMAnim, const CPrePoly *pPoly) const
{
	// Get the lightmap animation map
	TLMDataMap::const_iterator iLMAnim = m_cLMData.find(pLMAnim);
	// Cut out if we didn't find that lightmap animation
	if (iLMAnim == m_cLMData.end())
		return 0;
	// Find the poly
	TPolyLMDataMap::const_iterator iPolyLM = iLMAnim->second.find(pPoly);
	// Cut out if that poly's not in this lightmap animation
	if (iPolyLM == iLMAnim->second.end())
		return 0;
	// Return the lightmap
	return iPolyLM->second;
}
