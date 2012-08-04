
#include "bdefs.h"
#include "brushtoworld.h"
#include "prepoly.h"
#include "create_world_tree.h"
#include "editpoly.h"
#include "splitpoly.h"

PReal g_WorldNodeSize;

// Subdivision size for areas with normal world polies.
PReal GetWorldNodeSize(const char *pInfoString)
{
	ConParse	parse;
	PReal		ret;

	parse.Init(pInfoString);
	if(parse.ParseFind("WorldNodeSize", FALSE, 1))
	{
		ret = (PReal)atof(parse.m_Args[1]);
		return DCLAMP(ret, (PReal)1.0, (PReal)20000.0);
	}
	else
	{
		return (PReal)600.0;
	}
}

bool DoesPolyIntersectNode(CEditPoly *pPoly, WorldTreeNode *pNode)
{
	uint32			i, j;
	LTPlane			thePlane;
	bool			bOutside;
	SplitStruct		splitStruct;
	PolySide		side;
	

	// Trivial reject if all points are outside any of the planes..
	for(i=0; i < NUM_WTPLANES; i++)
	{
		pNode->SetupPlane(i, &thePlane);

		bOutside = true;
		for(j=0; j < pPoly->NumVerts(); j++)
		{
			if(thePlane.DistTo(pPoly->Pt(j)) > 0.0f)
			{
				bOutside = false;
				break;
			}
		}

		if(bOutside)
			return false;
	}

	// Trivial accept..
	for(i=0; i < pPoly->NumVerts(); i++)
	{
		if(base_IsPtInBox(&pPoly->Pt(i), &pNode->m_BBoxMin, &pNode->m_BBoxMax))
		{
			return true;
		}
	}

	// Ok, clip it up.
	CPrePoly *pTemp = EditPolyToPrePoly(pPoly);
	if(!pTemp)
		return false;

	for(i=0; i < NUM_WTPLANES; i++)
	{
		pNode->SetupPlane(i, &thePlane);

		side = GetPolySide(&thePlane, pTemp, &splitStruct);
		if(side == BackSide)
		{
			if (pTemp)
				DeletePoly(pTemp);
			return false;
		}
		else if(side == Intersect)
		{
			// Clip.
			CPrePoly *pNewPoly = CreatePoly(CPrePoly, pTemp->NumVerts()+2, false);
			if(!pNewPoly)
				return false;

			SplitPolySide(FrontSide, &thePlane, pTemp, pNewPoly, &splitStruct);
			DeletePoly(pTemp);
			pTemp = pNewPoly;
		}
	}
	DeletePoly(pTemp);
	
	return true;
}


bool CWTCallback(WorldTree *pWorldTree, 
	WorldTreeNode *pNode, 
	CMoArray<CEditPoly*> *pPolies,
	CMoArray<CEditPoly*> *pInsidePolies)
{
	// Find out which polies are inside this node.
	bool bMainWorld = false;

	for(uint32 i=0; i < pPolies->GetSize(); i++)
	{
		if(DoesPolyIntersectNode(pPolies->Get(i), pNode))
		{
			pInsidePolies->Append(pPolies->Get(i));

			// (MainWorld stuff gets subdivide smallest so no need to keep testing polies).
			bMainWorld = true;
		}
	}

	// Either use GetMinWorldBoxSize or GetMinTerrainBoxSize to determine
	// if the node should be divided further.
	if(bMainWorld)
	{
		return pNode->m_MinSize > g_WorldNodeSize;
	}
	else
	{
		// Nothing in here.. definitely don't divide further!
		return false;
	}
}


bool CreateNodes_R(	WorldTree *pWorldTree, WorldTreeNode *pNode,
					PVector *pMin, PVector *pMax, uint32 depth, CMoArray<CEditPoly*> *pPolies)
{
	WorldTreeNode			*pChild;
	uint32					i;
	CMoArray<CEditPoly*>	insidePolies;


	pNode->TermChildren();

	++pWorldTree->m_nNodes;
	if(depth > pWorldTree->m_nDepth)
	{
		pWorldTree->m_nDepth = depth;
	}

	// Setup..
	pNode->SetBBox(*pMin, *pMax);

	// It'll avoid making a node if we're past MAX_NODE_LEVEL, but it would have to
	// be a ridiculously large tree to have > 16 levels (4^16 or 4294967296 nodes 
	// would be in the tree...)
	ASSERT(depth < MAX_NODE_LEVEL);

	// Subdivide?
	insidePolies.SetCacheSize(pPolies->GetSize() / 2); // For speed...
	if(depth < MAX_NODE_LEVEL && 
		CWTCallback(pWorldTree, pNode, pPolies, &insidePolies))
	{
		if(!pNode->Subdivide())
			return false;

		// Recurse		
		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			pChild = pNode->m_ChildrenA[i];

			if(!CreateNodes_R(pWorldTree, pChild,
				&pChild->m_BBoxMin, &pChild->m_BBoxMax, depth+1, &insidePolies))
			{
				pNode->Term();
				return false;
			}
		}
	}
	
	return true;
}


bool CreateWorldTree(WorldTree *pWorldTree, 
	CMoArray<CWorldModelDef*> &worldModels, char *pInfoString)
{
	uint32					i, iPoly, iOutPoly, iBrush;
	CMoArray<CEditPoly*>	rootPolies;
	CEditBrush				*pBrush;
	CWorldModelDef			*pWorldModel;
	PVector					minBox, maxBox;
	CEditPoly				*pPoly;



	// Figure out node sizes.
	g_WorldNodeSize = GetWorldNodeSize(pInfoString);

	pWorldTree->Term();
		
	// Build the initial poly list.
	for(i=0; i < worldModels.GetSize(); i++)
	{
		pWorldModel = worldModels[i];

		if(!(pWorldModel->m_WorldInfoFlags & WIF_MAINWORLD))
			continue;

		// Add all of its polies.
		iOutPoly = rootPolies.GetSize();
		rootPolies.Fast_NiceSetSize(rootPolies.GetSize() + pWorldModel->CalcNumPolies());
		for(iBrush=0; iBrush < pWorldModel->m_Brushes; iBrush++)
		{
			pBrush = pWorldModel->m_Brushes[iBrush];
		
			for(iPoly=0; iPoly < pBrush->m_Polies; iPoly++)
			{
				rootPolies[iOutPoly] = pBrush->m_Polies[iPoly];
				iOutPoly++;
			}
		}
	}

	// Figure out the extents for the root node.
	// (Some messed up worlds have 0 polies on the root so the extents need to be 0).
	if(rootPolies.GetSize() > 0)
	{
		minBox.Init((PReal)MAX_CREAL, (PReal)MAX_CREAL, (PReal)MAX_CREAL);
		maxBox = -minBox;
		
		for(iPoly=0; iPoly < rootPolies; iPoly++)
		{
			pPoly = rootPolies[iPoly];

			for(i=0; i < pPoly->NumVerts(); i++)
			{
				VEC_MIN(minBox, minBox, pPoly->Pt(i));
				VEC_MAX(maxBox, maxBox, pPoly->Pt(i));
			}
		}
	}
	else
	{
		minBox.Init();
		maxBox.Init();
	}

	return CreateNodes_R(pWorldTree, &pWorldTree->m_RootNode,
		&minBox, &maxBox, 0, &rootPolies);
}



