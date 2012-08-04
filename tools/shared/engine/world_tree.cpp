
#include "bdefs.h"
#include "world_tree.h"


// Used by some of the recursive routines.
typedef void (*FilterFn_R)(WorldTreeNode *pNode, void *pData);


class FilterObjInfo
{
public:
	WorldTreeObj		*m_pObj;
	NodeObjArray		m_iObjArray;
	LTVector			m_Min;
	LTVector			m_Max;
	float				m_MaxSize;
	uint32				m_iCurLink;
};


class ISInfo
{
public:
	WorldTree		*m_pTree;
	NodeObjArray	m_iObjArray;
	LTVector		m_Pts[2];
	ISCallback		m_CB;
	void			*m_pCBUser;
};


static LTBOOL IntersectSegment_R(WorldTreeNode *pNode, ISInfo *pInfo);


uint32 g_WorldTreeDims[NUM_WORLDTREE_DIMS] = {0,2};



// -------------------------------------------------------------------------------- //
// WorldTree internal helpers.
// -------------------------------------------------------------------------------- //

static inline void FilterBox(LTVector *pMin, LTVector *pMax, 
	WorldTreeNode *pNode, FilterFn_R fn, void *pFnData)
{
	if(pMin->x < pNode->m_vCenter.x)
	{
		if(pMin->z < pNode->m_vCenter.z)
		{
			fn(pNode->m_Children[0][0], pFnData);
		}

		if(pMax->z > pNode->m_vCenter.z)
		{
			fn(pNode->m_Children[0][1], pFnData);
		}
	}

	if(pMax->x > pNode->m_vCenter.x)
	{
		if(pMin->z < pNode->m_vCenter.z)
		{
			fn(pNode->m_Children[1][0], pFnData);
		}

		if(pMax->z > pNode->m_vCenter.z)
		{
			fn(pNode->m_Children[1][1], pFnData);
		}
	}
}
	

static void FilterObj_R(WorldTreeNode *pNode, FilterObjInfo *pInfo)
{
	WorldTreeObj *pContainer;
	WTObjLink *pLink;
	LTLink *pVisLink;


	// If there are any vis containers here, add it to the first one (shouldn't be a problem
	// if there are multiple vis containers, each vis container should handle it appropriately).
	if(pNode->m_Objects[NOA_VisContainers].m_pNext != &pNode->m_Objects[NOA_VisContainers])
	{
		pContainer = ((WorldTreeObj*)pNode->m_Objects[NOA_VisContainers].m_pNext->m_pData);
		pContainer->AddToVis(pInfo->m_pObj);
	}

	if(pInfo->m_MaxSize >= (pNode->m_MinSize*0.5f) || !pNode->HasChildren())
	{
		// This shouldn't ever happen.  If it does, the object won't be
		// located correctly.
		if(pInfo->m_iCurLink >= MAX_OBJ_NODE_LINKS)
		{
			ASSERT(LTFALSE);
			return;
		}
	
		// Ok, it's likely to cover the space of all the nodes below us anyways, so add it
		// to this node and stop recursing.
		pLink = &pInfo->m_pObj->m_Links[pInfo->m_iCurLink];
		ASSERT(pLink->m_Link.IsTiedOff());

		pNode->AddObjectToList(pLink, pInfo->m_iObjArray);

		// If it's a vis container, add it to the list.
		if(pInfo->m_pObj->IsVisContainer())
		{
			pVisLink = pInfo->m_pObj->GetVisContainerLink(pInfo->m_iCurLink);
			ASSERT(pVisLink->IsTiedOff());
			dl_Insert(&pNode->m_Objects[NOA_VisContainers], pVisLink);
		}

		pInfo->m_iCurLink++;
	}
	else
	{
		FilterBox(&pInfo->m_Min, &pInfo->m_Max, pNode, (FilterFn_R)FilterObj_R, pInfo);
	}
}


// Returns LTTRUE if the boxes intersect or touch.
static inline LTBOOL DoBoxesTouch(LTVector *pMin1, LTVector *pMax1, 
	LTVector *pMin2, LTVector *pMax2)
{
	return !(pMin1->x > pMax2->x || pMin1->y > pMax2->y || pMin1->z > pMax2->z ||
		pMax1->x < pMin2->x || pMax1->y < pMin2->y || pMax1->z < pMin2->z);
}


// Returns LTTRUE if the line segment from pt0 to pt1 is outside of the bounding
// box in the dimension specified by iDim.
static inline LTBOOL IsSegmentDimOutsideBox(LTVector &pt0, LTVector &pt1, 
	LTVector &boxMin, LTVector &boxMax, uint32 iDim)
{
	return (pt0[iDim] < boxMin[iDim] && pt1[iDim] < boxMin[iDim]) ||
		(pt0[iDim] > boxMax[iDim] && pt1[iDim] > boxMax[iDim]);
}


// Returns BackSide if segment is behind boxMin, FrontSide if the segment
// is in front of boxMax, and Intersect otherwise.
static inline PolySide GetDimBoxStatus(LTVector &pt0, LTVector &pt1, 
	LTVector &boxMin, LTVector &boxMax, uint32 iDim)
{
	if(pt0[iDim] < boxMin[iDim] && pt1[iDim] < boxMin[iDim])
		return BackSide;
	else if (pt0[iDim] > boxMax[iDim] && pt1[iDim] > boxMax[iDim])
		return FrontSide;
	else
		return Intersect;
}


// Filters the box down the tree and calls the callback for any objects
// that the box touches.
static void FindObjectsInBox_R(WorldTreeNode *pNode, FindObjInfo *pInfo)
{
	LTLink *pCur, *pListHead;
	LTVector vMin, vMax;
	WorldTreeObj *pObj;


	// Check objects sitting on this node.	
	if(pNode->m_nObjectsOnOrBelow == 0)
		return;

	pListHead = pNode->m_Objects[pInfo->m_iObjArray].AsDLink();
	for(pCur=pListHead->m_pNext; pCur != pListHead;)
	{
		pObj = (WorldTreeObj*)pCur->m_pData;

		// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
		pCur = pCur->m_pNext;
	
		// Check the frame code.
		if(pObj->m_WTFrameCode == pInfo->m_pTree->m_TempFrameCode)
			continue;

		pObj->m_WTFrameCode = pInfo->m_pTree->m_TempFrameCode;

		// Do the boxes intersect?
		pObj->GetBBox(&vMin, &vMax);
		if(DoBoxesTouch(&vMin, &vMax, &pInfo->m_Min, &pInfo->m_Max))
		{
			pInfo->m_CB(pObj, pInfo->m_pCBUser);
		}
	}

	// Recurse into appropriate nodes.
	if(pNode->HasChildren())
	{
		FilterBox(&pInfo->m_Min, &pInfo->m_Max, pNode, (FilterFn_R)FindObjectsInBox_R, pInfo);
	}
}

// Intersects the line segment in the specified dimension on fPlane's plane, 
// then sees if the intersection point is inside the box on iOtherDim1 and iOtherDim2.
static inline LTBOOL TestBoxPlane(
	LTVector &pt1, 
	LTVector &pt2, 
	float fPlane,
	LTVector &boxMin, 
	LTVector &boxMax, 
	uint32 iDim, 
	uint32 iOtherDim1)
{
	float t;
	LTVector vTest;
	
	// Does it cross the minimum line?
	if((pt1[iDim] < fPlane) != (pt2[iDim] <= fPlane))
	{
		if(fabs(pt2[iDim] - pt1[iDim]) < 0.001f)
			t = 0.0f;
		else
			t = (fPlane - pt1[iDim]) / (pt2[iDim] - pt1[iDim]);
	}
	else
	{
		return LTFALSE;
	}

	vTest = pt1 + (pt2 - pt1) * t;
	return vTest[iOtherDim1] >= boxMin[iOtherDim1] && vTest[iOtherDim1] <= boxMax[iOtherDim1];
}


// Intersects the line segment with the 2 sides of the box on dimension iDim, then sees if the
// intersection point is inside the box on iOtherDim1 and iOtherDim2.
static LTBOOL TestBoxBothSides(
	LTVector &pt1, 
	LTVector &pt2, 
	LTVector &boxMin, 
	LTVector &boxMax, 
	uint32 iDim, 
	uint32 iOtherDim1)
{
	if(TestBoxPlane(pt1, pt2, boxMin[iDim], boxMin, boxMax, iDim, iOtherDim1))
		return LTTRUE;

	if(TestBoxPlane(pt1, pt2, boxMax[iDim], boxMin, boxMax, iDim, iOtherDim1))
		return LTTRUE;

	return LTFALSE;
}

// Does tests to see if the segment intersects the node.  If so, calls IntersectSegment_R
// on it and returns the value.
static LTBOOL TestNode(WorldTreeNode *pNode, ISInfo *pInfo)
{
	PolySide outStatus;


	// Trivial accept.
	if(base_IsPtInBoxXZ(&pInfo->m_Pts[0], &pNode->m_BBoxMin, &pNode->m_BBoxMax) ||
		base_IsPtInBoxXZ(&pInfo->m_Pts[1], &pNode->m_BBoxMin, &pNode->m_BBoxMax))
	{
		return IntersectSegment_R(pNode, pInfo);
	}
	else
	{
		// If both points are outside on the same side, then the line is outside.
		outStatus = GetDimBoxStatus(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->m_BBoxMin, pNode->m_BBoxMax, 0);
		if(outStatus != Intersect)
		{
			if(outStatus == GetDimBoxStatus(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->m_BBoxMin, pNode->m_BBoxMax, 2))
			{
				// Trivial reject.
				return LTFALSE;
			}
		}
		
		// Allllllllllll-righty, we'll do the extensive test!
		if(TestBoxBothSides(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->m_BBoxMin, pNode->m_BBoxMax, 0, 2) ||
			TestBoxBothSides(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->m_BBoxMin, pNode->m_BBoxMax, 2, 0))
		{
			return IntersectSegment_R(pNode, pInfo);
		}
	}

	return LTFALSE;
}


// Filters the line segment down the tree and calls the callback for objects
// in nodes that the line segment intersects.

// Returns LTTRUE if it tested all subnodes and an object was hit (ie: it should
// return LTTRUE up the stack and terminate early thus avoid tons of tests).
static LTBOOL IntersectSegment_R(WorldTreeNode *pNode, ISInfo *pInfo)
{
	LTLink *pCur, *pListHead;
	LTVector vMin, vMax;
	WorldTreeObj *pObj;
	LTBOOL bIntersected;
	int iX, iZ;


	bIntersected = LTFALSE;

	// Visit objects in this node.
	pListHead = pNode->m_Objects[pInfo->m_iObjArray].AsDLink();
	for(pCur=pListHead->m_pNext; pCur != pListHead;)
	{
		pObj = (WorldTreeObj*)pCur->m_pData;

		// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
		pCur = pCur->m_pNext;
	
		// Check the frame code.
		if(pObj->m_WTFrameCode == pInfo->m_pTree->m_TempFrameCode)
			continue;

		pObj->m_WTFrameCode = pInfo->m_pTree->m_TempFrameCode;
		bIntersected |= pInfo->m_CB(pObj, pInfo->m_pCBUser);
	}

	// Recurse into child nodes.
	if(pNode->HasChildren())
	{
		// Test in front to back order.  Node: it should be possible to remove one of these
		// calls because the segment can only cross 3 of the nodes so
		// we could test the front one, the diagonal one it intersects, and the back one.
		iX = pInfo->m_Pts[0].x > pNode->m_vCenter.x;
		iZ = pInfo->m_Pts[0].z > pNode->m_vCenter.z;
		
		LTBOOL bResult = TestNode(pNode->m_Children[iX][iZ], pInfo);
		bResult |= TestNode(pNode->m_Children[!iX][iZ], pInfo);
		bResult |= TestNode(pNode->m_Children[iX][!iZ], pInfo);
		bResult |= TestNode(pNode->m_Children[!iX][!iZ], pInfo);

		if (bResult)
			return LTTRUE;
	}

	return bIntersected;
}


/*// Calls the object callback for any terrain sections sitting on this node.
inline void VisQuery_CheckTerrainSections(WorldTreeNode *pNode, VisQueryRequest *pInfo)
{
	LTLink *pCur;


	for(pCur=pNode->m_Objects[NOA_TerrainSections].m_pNext; 
		pCur != &pNode->m_Objects[NOA_TerrainSections];)
	{
		WorldTreeObj *pObj = (WorldTreeObj *)pCur->m_pData;
		// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
		pCur = pCur->m_pNext;
	
		pInfo->m_ObjectCB(pObj, pInfo->m_pUserData);
	}
}
  

// Recurses and looks for terrain sections.
inline void VisQuery_CheckTerrainSections_R(
	WorldTreeNode *pNode, VisQueryRequest *pInfo, uint32 curDepth)
{
	uint32 i;

	if(curDepth > pInfo->m_pTree->m_TerrainDepth)
		return;

	// Allow them to filter out nodes.
	if(!pInfo->m_NodeFilterFn(pNode))
		return;

	VisQuery_CheckTerrainSections(pNode, pInfo);
	if(pNode->HasChildren())
	{
		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
			VisQuery_CheckTerrainSections_R(pNode->m_ChildrenA[i], pInfo, curDepth+1);
	}
}
  */

// Default node filter function..
static LTBOOL DummyNodeFilter(WorldTreeNode *pNode)
{
	return LTTRUE;
}


/*void VisQuery_R(WorldTreeNode *pNode, VisQueryRequest *pInfo, uint32 curDepth)
{
	LTLink *pCur, *pListHead;
	LTVector vMin, vMax;
	WorldTreeObj *pObj;


	// Allow them to filter out nodes.
	if(!pInfo->m_NodeFilterFn(pNode))
		return;

	// Are there any vis containers on this node?  If so, have them do their thing.
	if(pNode->m_Objects[NOA_VisContainers].m_pNext != &pNode->m_Objects[NOA_VisContainers])
	{
		// Have the vis container get objects.
		for(pCur=pNode->m_Objects[NOA_VisContainers].m_pNext; 
			pCur != &pNode->m_Objects[NOA_VisContainers];)
		{
			pObj = (WorldTreeObj*)pCur->m_pData;
			
			// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
			pCur = pCur->m_pNext;
		
			if(pObj->m_WTFrameCode == pInfo->m_pTree->m_TempFrameCode)
				continue;
			
			pObj->m_WTFrameCode = pInfo->m_pTree->m_TempFrameCode;

			ASSERT(pObj->IsVisContainer());
			pObj->DoVisQuery(pInfo);
		}

		// Now we need to look for terrain sections.
		VisQuery_CheckTerrainSections_R(pNode, pInfo, curDepth);
	}
	else
	{
		// Check for terrain sections.
		VisQuery_CheckTerrainSections(pNode, pInfo);

		// Check objects sitting on this node.	
		pListHead = pNode->m_Objects[pInfo->m_iObjArray].AsDLink();
		for(pCur=pListHead->m_pNext; pCur != pListHead;)
		{
			pObj = (WorldTreeObj*)pCur->m_pData;
		
			// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
			pCur = pCur->m_pNext;
		
			// Check the frame code.
			if(pObj->m_WTFrameCode == pInfo->m_pTree->m_TempFrameCode)
				continue;

			pObj->m_WTFrameCode = pInfo->m_pTree->m_TempFrameCode;

			// Do the boxes intersect?
			pObj->GetBBox(&vMin, &vMax);
			if(DoBoxesTouch(&vMin, &vMax, &pInfo->m_Min, &pInfo->m_Max))
			{
				pInfo->m_ObjectCB(pObj, pInfo->m_pUserData);
			}
		}

		// Recurse into appropriate nodes.
		if(pNode->HasChildren())
		{
			if(pInfo->m_Min.x < pNode->m_vCenter.x)
			{
				if(pInfo->m_Min.z < pNode->m_vCenter.z)
				{
					VisQuery_R(pNode->m_Children[0][0], pInfo, curDepth+1);
				}

				if(pInfo->m_Max.z > pNode->m_vCenter.z)
				{
					VisQuery_R(pNode->m_Children[0][1], pInfo, curDepth+1);
				}
			}

			if(pInfo->m_Max.x > pNode->m_vCenter.x)
			{
				if(pInfo->m_Min.z < pNode->m_vCenter.z)
				{
					VisQuery_R(pNode->m_Children[1][0], pInfo, curDepth+1);
				}

				if(pInfo->m_Max.z > pNode->m_vCenter.z)
				{
					VisQuery_R(pNode->m_Children[1][1], pInfo, curDepth+1);
				}
			}
		}
	}
}
*/

// -------------------------------------------------------------------------------- //
// WorldTreeObj.
// -------------------------------------------------------------------------------- //

WorldTreeObj::WorldTreeObj(WTObjType objType)
{
	uint32 i;
	WTObjLink *pLink;

	for(i=0; i < MAX_OBJ_NODE_LINKS; i++)
	{
		pLink = &m_Links[i];

		pLink->m_Link.TieOff();
		pLink->m_Link.m_pData = this;
		pLink->m_pNode = LTNULL;
	}

	m_ObjType = objType;
	m_WTFrameCode = FRAMECODE_NOTINTREE;
}


WorldTreeObj::~WorldTreeObj()
{
	RemoveFromWorldTree();
}


void WorldTreeObj::RemoveFromWorldTree()
{
	uint32 i;
	WTObjLink *pLink;

	m_WTFrameCode = FRAMECODE_NOTINTREE;
	for(i=0; i < MAX_OBJ_NODE_LINKS; i++)
	{
		pLink = &m_Links[i];

		pLink->m_Link.Remove();
		pLink->m_Link.TieOff();
		
		// Remove references from this node.
		while(pLink->m_pNode)
		{
			ASSERT(pLink->m_pNode->m_nObjectsOnOrBelow);
			pLink->m_pNode->m_nObjectsOnOrBelow--;
			pLink->m_pNode = pLink->m_pNode->m_pParent;
		}
	}
}



// -------------------------------------------------------------------------------- //
// WorldTreeNode.
// -------------------------------------------------------------------------------- //

WorldTreeNode::WorldTreeNode()
{
	Clear();
}


WorldTreeNode::~WorldTreeNode()
{
	Term();
}


void WorldTreeNode::Clear()
{
	uint32 i;

	m_pParent = LTNULL;
	m_nObjectsOnOrBelow = 0;

	for(i=0; i < NUM_NODEOBJ_ARRAYS; i++)
	{
		m_Objects[i].TieOff();
	}

	for(i=0; i < MAX_WTNODE_CHILDREN; i++)
	{
		m_ChildrenA[i] = LTNULL;
	}

	m_vCenter.Init();
	m_BBoxMin.Init();
	m_BBoxMax.Init();

	m_MinSize = 0.0f;
	m_Radius = 0.0f;
}


void WorldTreeNode::GetBBox(LTVector *pBoxMin, LTVector *pBoxMax)
{
	*pBoxMin = m_BBoxMin;
	*pBoxMax = m_BBoxMax;
}


void WorldTreeNode::SetBBox(LTVector boxMin, LTVector boxMax)
{
	LTVector halfBox;

	m_BBoxMin = boxMin;
	m_BBoxMax = boxMax;
	halfBox = (boxMax - boxMin) * 0.5f;
	m_vCenter = boxMin + halfBox;
	m_MinSize = LTMIN(boxMax.x - boxMin.x, boxMax.z - boxMin.z);
	m_Radius = halfBox.Mag();
}


void WorldTreeNode::Term()
{
	uint32 i;
	LTLink *pCur, *pNext;
	WorldTreeObj *pObj;

	// Free child nodes.
	TermChildren();

	// Remove all the objects, so they don't have bad pointers into us.
	for(i=0; i < NUM_NODEOBJ_ARRAYS; i++)
	{
		for(pCur=m_Objects[i].m_pNext; pCur != &m_Objects[i]; pCur=pNext)
		{
			pNext = pCur->m_pNext;
			pObj = ((WorldTreeObj*)pCur->m_pData);
			pObj->RemoveFromWorldTree();
		}
	}

	Clear();
}


void WorldTreeNode::TermChildren()
{
	uint32 i;

	for(i=0; i < MAX_WTNODE_CHILDREN; i++)
	{
		if(m_ChildrenA[i])
		{
			delete m_ChildrenA[i];
			m_ChildrenA[i] = LTNULL;
		}
	}
}


void WorldTreeNode::SetupPlane(uint32 iPlane, LTPlane *pPlane)
{
	if(iPlane == WTPLANE_LEFT)
	{
		pPlane->Init(LTVector(1.0f, 0.0f, 0.0f), m_BBoxMin.x);
	}
	else if(iPlane == WTPLANE_RIGHT)
	{
		pPlane->Init(LTVector(-1.0f, 0.0f, 0.0f), -m_BBoxMax.x);
	}
	else if(iPlane == WTPLANE_BACK)
	{
		pPlane->Init(LTVector(0.0f, 0.0f, 1.0f), m_BBoxMin.z);
	}
	else if(iPlane == WTPLANE_FRONT)
	{
		pPlane->Init(LTVector(0.0f, 0.0f, -1.0f), -m_BBoxMax.z);
	}
}


uint32 WorldTreeNode::NumSubtreeNodes()
{
	uint32 i, nChildren;


	nChildren = 1;
	
	if(HasChildren())
	{
		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			nChildren += m_ChildrenA[i]->NumSubtreeNodes();
		}
	}

	return nChildren;
}


LTBOOL WorldTreeNode::LoadLayout(ILTStream *pStream, uint8 &curByte, uint8 &curBit)
{
	LTBOOL bSubdivide;
	uint32 i;


	TermChildren();
	
	// Read the next bit.
	if(curBit == 8)
	{
		*pStream >> curByte;
		curBit = 0;
	}
	bSubdivide = !!(curByte & (1<<curBit));
	++curBit;

	if(bSubdivide)
	{
		if(!Subdivide())
			return LTFALSE;

		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			if(!m_ChildrenA[i]->LoadLayout(pStream, curByte, curBit))
			{
				TermChildren();
				return LTFALSE;
			}
		}
	}

	return LTTRUE;
}


void WorldTreeNode::SaveLayout(ILTStream *pStream, uint8 &curByte, uint8 &curBit)
{
	uint32 i;

	if(HasChildren())
	{
		curByte |= (1<<curBit);
	}

	curBit++;
	if(curBit == 8)
	{
		*pStream << curByte;
		curByte = curBit = 0;
	}

	if(HasChildren())
	{
		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			m_ChildrenA[i]->SaveLayout(pStream, curByte, curBit);
		}
	}
}


LTBOOL WorldTreeNode::Subdivide()
{
	uint32 i;
	LTVector vSubSize, vSubMin, vSubMax;
	

	// Allocate..
	for(i=0; i < MAX_WTNODE_CHILDREN; i++)
	{
		m_ChildrenA[i] = new WorldTreeNode;
		if(!m_ChildrenA[i])
		{
			Term();
			return LTFALSE;
		}

		m_ChildrenA[i]->m_pParent = this;
	}

	vSubSize = (m_BBoxMax - m_BBoxMin) * 0.5f;

	// -x -z
	m_Children[0][0]->SetBBox(
		LTVector(m_vCenter.x - vSubSize.x, m_BBoxMin.y, m_vCenter.z - vSubSize.z),
		LTVector(m_vCenter.x, m_BBoxMax.y, m_vCenter.z));

	// -x +z
	m_Children[0][1]->SetBBox(
		LTVector(m_vCenter.x - vSubSize.x, m_BBoxMin.y, m_vCenter.z),
		LTVector(m_vCenter.x, m_BBoxMax.y, m_vCenter.z + vSubSize.z));

	// +x -z
	m_Children[1][0]->SetBBox(
		LTVector(m_vCenter.x, m_BBoxMin.y, m_vCenter.z - vSubSize.z),
		LTVector(m_vCenter.x + vSubSize.x, m_BBoxMax.y, m_vCenter.z));

	// +x +z
	m_Children[1][1]->SetBBox(
		LTVector(m_vCenter.x, m_BBoxMin.y, m_vCenter.z),
		LTVector(m_vCenter.x + vSubSize.x, m_BBoxMax.y, m_vCenter.z + vSubSize.z));

	return LTTRUE;
}


LTBOOL WorldTreeNode::GetNodePath(WorldTreeNode *pNode, NodePath *pPath)
{
	uint32 i, iPrevLevel, iBit, iByte;

	if(pNode == this)
	{
		return LTTRUE;
	}

	if(HasChildren())
	{
		iPrevLevel = pPath->m_iLevel;
		iByte = iPrevLevel >> 2;
		iBit = (iPrevLevel & 3) << 1;
		
		++pPath->m_iLevel;
		ASSERT(pPath->m_iLevel < MAX_NODE_LEVEL);

		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			pPath->m_Path[iByte] &= ~(3 << iBit);
			pPath->m_Path[iByte] |= (i << iBit);

			if(m_ChildrenA[i]->GetNodePath(pNode, pPath))
			{
				return LTTRUE;
			}
		}

		--pPath->m_iLevel;
	}

	return LTFALSE;
}


void WorldTreeNode::AddObjectToList(WTObjLink *pLink, NodeObjArray iArray)
{
	WorldTreeNode *pTempNode;


	dl_Insert(&m_Objects[iArray], &pLink->m_Link);
	pLink->m_pNode = this;
	
	// Add a reference to all the nodes above here.
	pTempNode = this;
	while(pTempNode)
	{
		pTempNode->m_nObjectsOnOrBelow++;
		pTempNode = pTempNode->m_pParent;
	}
}



// -------------------------------------------------------------------------------- //
// WorldTree.
// -------------------------------------------------------------------------------- //

WorldTree::WorldTree()
{
	m_pHelper = LTNULL;
	m_nNodes = 0;
	m_nDepth = 0;
	m_TerrainDepth = 0;
	m_AlwaysVisObjects.TieOff();
}


uint32 WorldTree::GetCurFrameCode()
{
	ASSERT(m_pHelper);
	return m_pHelper->GetFrameCode();
}


void WorldTree::Init(WorldTreeHelper *pHelper)
{
	m_pHelper = pHelper;
	m_AlwaysVisObjects.Init();
}


void WorldTree::Term()
{
	m_RootNode.Term();
	m_nNodes = 0;
	m_nDepth = 0;
	m_AlwaysVisObjects.Term();
}


WorldTreeNode* WorldTree::FindNode(NodePath *pPath)
{
	uint32 iCurByte, iCurBit, iCurLevel, nextChoice;
	WorldTreeNode *pNode;
	
	pNode = &m_RootNode;
	iCurByte = iCurBit = iCurLevel = 0;
	while(iCurLevel != pPath->m_iLevel)
	{	
		if(!pNode->HasChildren())
		{
			ASSERT(LTFALSE);
			return LTNULL;
		}

		// Read the next token..
		nextChoice = (pPath->m_Path[iCurByte] >> iCurBit) & 3;
		iCurBit += 2;
		if(iCurBit == 8)
		{
			++iCurByte;
			iCurBit = 0;
			if(iCurByte >= 4)
			{
				ASSERT(LTFALSE);
				return LTNULL;
			}
		}

		// Descend..
		ASSERT(nextChoice < MAX_WTNODE_CHILDREN);
		pNode = pNode->m_ChildrenA[nextChoice];
		++iCurLevel;
	}

	return pNode;
}


LTBOOL WorldTree::GetNodePath(WorldTreeNode *pNode, NodePath *pPath)
{
	pPath->Clear();
	if(m_RootNode.GetNodePath(pNode, pPath))
	{
		ASSERT(FindNode(pPath) == pNode);
		return LTTRUE;
	}
	else
	{
		return LTFALSE;
	}
}


void WorldTree::InsertObject(WorldTreeObj *pObj, NodeObjArray iArray)
{
	LTVector vMin, vMax;

	pObj->GetBBox(&vMin, &vMax);
	InsertObject2(pObj, &vMin, &vMax, iArray);
}


void WorldTree::InsertObject2(WorldTreeObj *pObj, 
	LTVector *pMin, LTVector *pMax,
	NodeObjArray iArray)
{
	FilterObjInfo foInfo;
	LTVector vDiff;

	pObj->RemoveFromWorldTree();

	vDiff = *pMax - *pMin;
	
	foInfo.m_pObj = pObj;
	foInfo.m_iObjArray = iArray;
	foInfo.m_Min = *pMin;
	foInfo.m_Max = *pMax;
	foInfo.m_MaxSize = LTMAX(vDiff.x, vDiff.z);
	foInfo.m_iCurLink = 0;

	if(!pObj->InsertSpecial(this))
	{
		FilterObj_R(&m_RootNode, &foInfo);
	}
}


void WorldTree::RemoveObject(WorldTreeObj *pObj)
{
	pObj->RemoveFromWorldTree();
}


void WorldTree::FindObjectsInBox(LTVector *pMin, LTVector *pMax, 
	WTObjCallback cb, void *pCBUser, NodeObjArray iArray)
{
	FindObjInfo foInfo;

	foInfo.m_iObjArray = iArray;
	foInfo.m_Min = *pMin;
	foInfo.m_Max = *pMax;
	foInfo.m_CB = cb;
	foInfo.m_pCBUser = pCBUser;

	FindObjectsInBox2(&foInfo);
}


void WorldTree::FindObjectsInBox2(FindObjInfo *pInfo)
{
	ASSERT(m_pHelper);
	m_TempFrameCode = m_pHelper->IncFrameCode();

	pInfo->m_pTree = this;
	FindObjectsInBox_R(&m_RootNode, pInfo);
}


void WorldTree::FindObjectsOnPoint(LTVector *pPoint,
	WTObjCallback cb, void *pCBUser, NodeObjArray iArray)
{
	FindObjectsInBox(pPoint, pPoint, cb, pCBUser, iArray);
}


void WorldTree::IntersectSegment(LTVector *pPt1, LTVector *pPt2, 
	ISCallback cb, void *pCBUser, NodeObjArray iArray)
{
	ISInfo isInfo;

	
	ASSERT(m_pHelper);
	m_TempFrameCode = m_pHelper->IncFrameCode();

	isInfo.m_pTree = this;
	isInfo.m_iObjArray = iArray;
	isInfo.m_Pts[0] = *pPt1;
	isInfo.m_Pts[1] = *pPt2;
	isInfo.m_CB = cb;
	isInfo.m_pCBUser = pCBUser;

	IntersectSegment_R(&m_RootNode, &isInfo);
}
/*
// Adds a list of objects to the visibility list via the request's callback
static void AddListToVisibility(LTLink *pListHead, VisQueryRequest *pRequest)
{
	WorldTreeObj *pObj;
	LTLink *pCur;
	for(pCur=pListHead->m_pNext; pCur != pListHead;)
	{
		pObj = (WorldTreeObj*)pCur->m_pData;
	
		// Increment the link pointer here in case this link goes away in the callback
		pCur = pCur->m_pNext;
	
		// Check the frame code.
		if(pObj->m_WTFrameCode == pRequest->m_pTree->m_TempFrameCode)
			continue;

		pObj->m_WTFrameCode = pRequest->m_pTree->m_TempFrameCode;

		pRequest->m_ObjectCB(pObj, pRequest->m_pUserData);
	}
}


void WorldTree::DoVisQuery(VisQueryRequest *pRequest)
{
	ASSERT(m_pHelper);
	m_TempFrameCode = m_pHelper->IncFrameCode();
	
	pRequest->m_pTree = this;
	pRequest->m_Min = pRequest->m_Viewpoint - LTVector(pRequest->m_ViewRadius, pRequest->m_ViewRadius, pRequest->m_ViewRadius);
	pRequest->m_Max = pRequest->m_Viewpoint + LTVector(pRequest->m_ViewRadius, pRequest->m_ViewRadius, pRequest->m_ViewRadius);

	if(!pRequest->m_NodeFilterFn)
		pRequest->m_NodeFilterFn = DummyNodeFilter;

	VisQuery_R(&m_RootNode, pRequest, 0);

	// Add the always-visible list to the visibility list first
	AddListToVisibility(m_AlwaysVisObjects.AsLTLink(), pRequest);
}
*/

LTBOOL WorldTree::Inherit(WorldTree *pOther)
{
	Term();

	m_TerrainDepth = pOther->m_TerrainDepth;
	m_nDepth = pOther->m_nDepth;
	m_nNodes = pOther->m_nNodes;

	// Create an identical node layout.
	m_RootNode.SetBBox(pOther->m_RootNode.m_BBoxMin, pOther->m_RootNode.m_BBoxMax);
	return CopyNodeLayout_R(&m_RootNode, &pOther->m_RootNode);
}


LTBOOL WorldTree::CreateNodes(
	LTVector *pMin, LTVector *pMax,
	CreateNodesCB cb, void *pUser)
{
	LTBOOL bRet;

	Term();

	bRet = RecurseAndCreateNodes(&m_RootNode, pMin, pMax, cb, pUser, 0);

	dsi_ConsolePrint("CreateNodes, depth: %d, nNodes: %d", m_nDepth, m_nNodes);
	return bRet;
}


LTBOOL WorldTree::LoadLayout(ILTStream *pStream)
{
	uint32 nTotalNodes;
	LTVector boxMin, boxMax;
	uint8 curByte, curBit;

	*pStream >> boxMin >> boxMax;
	*pStream >> nTotalNodes;
	*pStream >> m_TerrainDepth;

	curByte = 0;
	curBit = 8;
	m_RootNode.SetBBox(boxMin, boxMax);
	return m_RootNode.LoadLayout(pStream, curByte, curBit) && pStream->ErrorStatus() == LT_OK;
}


LTBOOL WorldTree::SaveLayout(ILTStream *pStream)
{	
	uint8 curByte, curBit;

	// Save root node dims.
	*pStream << m_RootNode.m_BBoxMin << m_RootNode.m_BBoxMax;
	pStream->WriteVal(m_RootNode.NumSubtreeNodes());
	*pStream << m_TerrainDepth;

	// Save a bit for each node telling if it's subdivided.
	curByte = curBit = 0;
	m_RootNode.SaveLayout(pStream, curByte, curBit);
	if(curBit)
		*pStream << curByte;

	return pStream->ErrorStatus() == LT_OK;
}


LTBOOL WorldTree::RecurseAndCreateNodes(WorldTreeNode *pNode,
	LTVector *pMin, LTVector *pMax, CreateNodesCB cb, void *pUser,
	uint32 depth)
{
	WorldTreeNode *pChild;
	uint32 i;


	pNode->TermChildren();

	++m_nNodes;
	if(depth > m_nDepth)
	{
		m_nDepth = depth;
	}

	// Setup..
	pNode->SetBBox(*pMin, *pMax);

	// It'll avoid making a node if we're past MAX_NODE_LEVEL, but it would have to
	// be a ridiculously large tree to have > 16 levels (4^16 or 4294967296 nodes 
	// would be in the tree...)
	ASSERT(depth < MAX_NODE_LEVEL);

	// Subdivide?
	if(depth < MAX_NODE_LEVEL && cb(this, pNode, pUser))
	{
		if(!pNode->Subdivide())
			return LTFALSE;

		// Recurse		
		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			pChild = pNode->m_ChildrenA[i];

			if(!RecurseAndCreateNodes(pChild,
				&pChild->m_BBoxMin, &pChild->m_BBoxMax, cb, pUser, depth+1))
			{
				pNode->Term();
				return LTFALSE;
			}
		}
	}
	
	return LTTRUE;
}


LTBOOL WorldTree::CopyNodeLayout_R(WorldTreeNode *pDest, WorldTreeNode *pSrc)
{
	uint32 i;

	if(pSrc->HasChildren())
	{
		if(!pDest->Subdivide())
			return LTFALSE;

		for(i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			if(!CopyNodeLayout_R(pDest->m_ChildrenA[i], pSrc->m_ChildrenA[i]))
				return LTFALSE;
		}
	}

	return LTTRUE;
}

// Insert an object into the always-visible list
void WorldTree::InsertAlwaysVisObject(WorldTreeObj *pObj)
{
	WTObjLink *pLink = &pObj->m_Links[OBJ_NODE_LINK_ALWAYSVIS];
	dl_Insert(&m_AlwaysVisObjects, &pLink->m_Link);
	pLink->m_pNode = LTNULL;
}

// Remove an object from the always-visible list
void WorldTree::RemoveAlwaysVisObject(WorldTreeObj *pObj)
{
	WTObjLink *pLink = &pObj->m_Links[OBJ_NODE_LINK_ALWAYSVIS];
	pLink->m_Link.Remove();
	pLink->m_Link.TieOff();
}



