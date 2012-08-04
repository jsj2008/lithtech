
#include "bdefs.h"
#include "world_tree.h"


#include "worldtreehelper.h"


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


static bool IntersectSegment_R(WorldTreeNode *pNode, ISInfo *pInfo);

// -------------------------------------------------------------------------------- //
// WorldTree internal helpers.
// -------------------------------------------------------------------------------- //

inline void FilterBox(	const LTVector *pMin, const LTVector *pMax, 
						WorldTreeNode *pNode, FilterFn_R fn, void *pFnData)
{
	if(pMin->x < pNode->GetCenterX())
	{
		if(pMin->z < pNode->GetCenterZ())
		{
			fn(pNode->GetChild(0, 0), pFnData);
		}

		if(pMax->z > pNode->GetCenterZ())
		{
			fn(pNode->GetChild(0, 1), pFnData);
		}
	}

	if(pMax->x > pNode->GetCenterX())
	{
		if(pMin->z < pNode->GetCenterZ())
		{
			fn(pNode->GetChild(1, 0), pFnData);
		}

		if(pMax->z > pNode->GetCenterZ())
		{
			fn(pNode->GetChild(1, 1), pFnData);
		}
	}
}
	

static void FilterObj_R(WorldTreeNode *pNode, FilterObjInfo *pInfo)
{
	if(pInfo->m_MaxSize >= (pNode->GetSmallestDim() * 0.5f) || !pNode->HasChildren())
	{
		// This shouldn't ever happen.  If it does, the object won't be
		// located correctly.
		if(pInfo->m_iCurLink >= MAX_OBJ_NODE_LINKS)
		{
			ASSERT(false);
			return;
		}
	
		// Ok, it's likely to cover the space of all the nodes below us anyways, so add it
		// to this node and stop recursing.
		WTObjLink *pLink = &pInfo->m_pObj->m_Links[pInfo->m_iCurLink];
		ASSERT(pLink->m_Link.IsTiedOff());

		pNode->AddObjectToList(pLink, pInfo->m_iObjArray);

		pInfo->m_iCurLink++;
	}
	else
	{
		FilterBox(&pInfo->m_Min, &pInfo->m_Max, pNode, (FilterFn_R)FilterObj_R, pInfo);
	}
}


// Returns true if the boxes intersect or touch.
inline bool DoBoxesTouch(	const LTVector& vMin1, const LTVector& vMax1, 
							const LTVector& vMin2, const LTVector& vMax2)
{
	return !(vMin1.x > vMax2.x || vMin1.y > vMax2.y || vMin1.z > vMax2.z ||
			vMax1.x < vMin2.x || vMax1.y < vMin2.y || vMax1.z < vMin2.z);
}


// Returns true if the line segment from pt0 to pt1 is outside of the bounding
// box in the dimension specified by iDim.
inline bool IsSegmentDimOutsideBox(	const LTVector &pt0, const LTVector &pt1, 
									const LTVector &boxMin, const LTVector &boxMax, 
									uint32 iDim)
{
	return (pt0[iDim] < boxMin[iDim] && pt1[iDim] < boxMin[iDim]) ||
		(pt0[iDim] > boxMax[iDim] && pt1[iDim] > boxMax[iDim]);
}


// Returns BackSide if segment is behind boxMin, FrontSide if the segment
// is in front of boxMax, and Intersect otherwise.
inline PolySide GetDimBoxStatus(const LTVector &pt0, const LTVector &pt1, 
								const LTVector &boxMin, const LTVector &boxMax, 
								uint32 iDim)
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
	// Check objects sitting on this node.	
	if(pNode->GetNumObjectsOnOrBelow() == 0)
		return;

	LTLink *pCur, *pListHead;
	WorldTreeObj *pObj;

	pListHead = pNode->m_Objects[pInfo->m_iObjArray].AsLTLink();
	for(pCur=pListHead->m_pNext; pCur != pListHead;)
	{
		pObj = (WorldTreeObj*)pCur->m_pData;

		// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
		pCur = pCur->m_pNext;
	
		// Check the frame code.
		if(pObj->m_WTFrameCode == pInfo->m_pTree->GetTempFrameCode())
			continue;

		pObj->m_WTFrameCode = pInfo->m_pTree->GetTempFrameCode();

		// Do the boxes intersect?
		if(DoBoxesTouch(pObj->GetBBoxMin(), pObj->GetBBoxMax(), pInfo->m_Min, pInfo->m_Max))
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
inline bool TestBoxPlane(	const LTVector &pt1, 
							const LTVector &pt2, 
							float fPlane,
							const LTVector &boxMin, 
							const LTVector &boxMax, 
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
		return false;
	}

	vTest = pt1 + (pt2 - pt1) * t;
	return vTest[iOtherDim1] >= boxMin[iOtherDim1] && vTest[iOtherDim1] <= boxMax[iOtherDim1];
}


// Intersects the line segment with the 2 sides of the box on dimension iDim, then sees if the
// intersection point is inside the box on iOtherDim1 and iOtherDim2.
static bool TestBoxBothSides(	const LTVector &pt1, 
								const LTVector &pt2, 
								const LTVector &boxMin, 
								const LTVector &boxMax, 
								uint32 iDim, 
								uint32 iOtherDim1)
{
	if(TestBoxPlane(pt1, pt2, boxMin[iDim], boxMin, boxMax, iDim, iOtherDim1))
		return true;

	if(TestBoxPlane(pt1, pt2, boxMax[iDim], boxMin, boxMax, iDim, iOtherDim1))
		return true;

	return false;
}


// Does tests to see if the segment intersects the node.  If so, calls IntersectSegment_R
// on it and returns the value.
static bool TestNode(WorldTreeNode *pNode, ISInfo *pInfo)
{
	PolySide outStatus;

	// Trivial accept.
	if(base_IsPtInBoxXZ(&pInfo->m_Pts[0], &pNode->GetBBoxMin(), &pNode->GetBBoxMax()) ||
		base_IsPtInBoxXZ(&pInfo->m_Pts[1], &pNode->GetBBoxMin(), &pNode->GetBBoxMax()))
	{
		return IntersectSegment_R(pNode, pInfo);
	}
	else
	{
		// If both points are outside on the same side, then the line is outside.
		outStatus = GetDimBoxStatus(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->GetBBoxMin(), pNode->GetBBoxMax(), 0);
		if(outStatus != Intersect)
		{
			if(outStatus == GetDimBoxStatus(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->GetBBoxMin(), pNode->GetBBoxMax(), 2))
			{
				// Trivial reject.
				return false;
			}
		}
		
		// Allllllllllll-righty, we'll do the extensive test!
		if(TestBoxBothSides(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->GetBBoxMin(), pNode->GetBBoxMax(), 0, 2) ||
			TestBoxBothSides(pInfo->m_Pts[0], pInfo->m_Pts[1], pNode->GetBBoxMin(), pNode->GetBBoxMax(), 2, 0))
		{
			return IntersectSegment_R(pNode, pInfo);
		}
	}

	return false;
}


// Filters the line segment down the tree and calls the callback for objects
// in nodes that the line segment intersects.

// Returns true if it tested all subnodes and an object was hit (ie: it should
// return true up the stack and terminate early thus avoid tons of tests).
static bool IntersectSegment_R(WorldTreeNode *pNode, ISInfo *pInfo)
{
	LTLink *pCur, *pListHead;
	LTVector vMin, vMax;
	WorldTreeObj *pObj;
	bool bIntersected;
	int iX, iZ;


	bIntersected = false;

	// Visit objects in this node.
	pListHead = pNode->m_Objects[pInfo->m_iObjArray].AsLTLink();
	for(pCur=pListHead->m_pNext; pCur != pListHead;)
	{
		pObj = (WorldTreeObj*)pCur->m_pData;

		// KEF - 04/03/00 - Increment the link pointer here in case this link goes away in the callback
		pCur = pCur->m_pNext;
	
		// Check the frame code.
		if(pObj->m_WTFrameCode == pInfo->m_pTree->GetTempFrameCode())
			continue;

		pObj->m_WTFrameCode = pInfo->m_pTree->GetTempFrameCode();
		bIntersected |= pInfo->m_CB(pObj, pInfo->m_pCBUser);
	}

	// Recurse into child nodes.
	if(pNode->HasChildren())
	{
		// Test in front to back order.  Node: it should be possible to remove one of these
		// calls because the segment can only cross 3 of the nodes so
		// we could test the front one, the diagonal one it intersects, and the back one.
		iX = pInfo->m_Pts[0].x > pNode->GetCenterX();
		iZ = pInfo->m_Pts[0].z > pNode->GetCenterZ();
		
		bool bResult = TestNode(pNode->GetChild(iX, iZ), pInfo);
		bResult |= TestNode(pNode->GetChild(!iX, iZ), pInfo);
		bResult |= TestNode(pNode->GetChild(iX, !iZ), pInfo);
		bResult |= TestNode(pNode->GetChild(!iX, !iZ), pInfo);

		if (bResult)
			return true;
	}

	return bIntersected;
}

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
		pLink->m_pNode = NULL;
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
	m_WTFrameCode = FRAMECODE_NOTINTREE;
	for(uint32 i=0; i < MAX_OBJ_NODE_LINKS; i++)
	{
		WTObjLink *pLink = &m_Links[i];
		WorldTreeNode::RemoveLink(pLink);
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
	m_pParent	= LTNULL;
	m_pChildren = LTNULL;

	for(uint32 i=0; i < NUM_NODEOBJ_ARRAYS; i++)
	{
		m_Objects[i].TieOff();
	}

	m_nObjectsOnOrBelow = 0;

	m_fCenterX			= 0.0f;
	m_fCenterZ			= 0.0f;

	m_fSmallestDim		= 0.0f;

	m_vBBoxMin.Init();
	m_vBBoxMax.Init();
}


void WorldTreeNode::SetBBox(const LTVector& boxMin, const LTVector& boxMax)
{
	m_vBBoxMin	= boxMin;
	m_vBBoxMax	= boxMax;

	m_fCenterX  = (boxMax.x + boxMin.x) * 0.5f;
	m_fCenterZ  = (boxMax.z + boxMin.z) * 0.5f;

	m_fSmallestDim = LTMIN(boxMax.x - boxMin.x, boxMax.z - boxMin.z);
}

//static member to handle the cleanup of a world tree link so that it can remove any dependancies
void WorldTreeNode::RemoveLink(WTObjLink* pLink)
{
	pLink->m_Link.Remove();
	pLink->m_Link.TieOff();
		
	// Remove references from this node all the way up the tree
	while(pLink->m_pNode)
	{
		//make sure that that node acutally has objects
		assert(pLink->m_pNode->GetNumObjectsOnOrBelow());

		//reduce the object count
		pLink->m_pNode->m_nObjectsOnOrBelow--;

		//remove the node from the list
		pLink->m_pNode = pLink->m_pNode->m_pParent;
	}
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
	m_pChildren = NULL;
}

void WorldTreeNode::LoadLayout(ILTStream *pStream, uint8 &curByte, uint8 &curBit, WorldTreeNode* pNodeList, uint32& nCurrOffset)
{
	TermChildren();
	
	// Read the next bit.
	if(curBit == 8)
	{
		*pStream >> curByte;
		curBit = 0;
	}

	bool bSubdivide = !!(curByte & (1<<curBit));
	++curBit;

	if(bSubdivide)
	{
		Subdivide(pNodeList, nCurrOffset);

		for(uint32 i = 0; i < MAX_WTNODE_CHILDREN; i++)
		{
			GetChild(i)->LoadLayout(pStream, curByte, curBit, pNodeList, nCurrOffset);
		}
	}
}

void WorldTreeNode::Subdivide(WorldTreeNode* pNodes, uint32& nOffset)
{
	//get our children out of the list of nodes
	m_pChildren = &pNodes[nOffset];

	//update our current offset in the list of nodes
	nOffset += MAX_WTNODE_CHILDREN;

	// Setup the child pointers..
	for(uint32 i = 0; i < MAX_WTNODE_CHILDREN; i++)
	{
		GetChild(i)->m_pParent = this;
	}

	// -x -z
	GetChild(0, 0)->SetBBox(
		LTVector(GetBBoxMin().x, GetBBoxMin().y, GetBBoxMin().z),
		LTVector(GetCenterX(), GetBBoxMax().y, GetCenterZ()));

	// -x +z
	GetChild(0, 1)->SetBBox(
		LTVector(GetBBoxMin().x, GetBBoxMin().y, GetCenterZ()),
		LTVector(GetCenterX(), GetBBoxMax().y, GetBBoxMax().z));

	// +x -z
	GetChild(1, 0)->SetBBox(
		LTVector(GetCenterX(), GetBBoxMin().y, GetBBoxMin().z),
		LTVector(GetBBoxMax().x, GetBBoxMax().y, GetCenterZ()));

	// +x +z
	GetChild(1, 1)->SetBBox(
		LTVector(GetCenterX(), GetBBoxMin().y, GetCenterZ()),
		LTVector(GetBBoxMax().x, GetBBoxMax().y, GetBBoxMax().z));
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

WorldTree::WorldTree() :
	m_pHelper(NULL),
	m_pNodes(NULL),
	m_nNumNodes(1)
{
	m_AlwaysVisObjects.TieOff();
}

WorldTree::~WorldTree()
{
	Term();
}

void WorldTree::InitWorldTree(WorldTreeHelper *pHelper)
{
	//make sure that we have a helper
	assert(pHelper);
	m_pHelper = pHelper;

	m_AlwaysVisObjects.Init();
}

void WorldTree::Term()
{
	//make sure to release our memory
	delete [] m_pNodes;
	m_pNodes = NULL;

	//clear out the number of nodes
	m_nNumNodes = 1;

	//flush out existing lists
	m_RootNode.Term();
	m_AlwaysVisObjects.Term();
}

void WorldTree::InsertObject(WorldTreeObj *pObj, NodeObjArray iArray)
{
	InsertObject2(pObj, pObj->GetBBoxMin(), pObj->GetBBoxMax(), iArray);
}


void WorldTree::InsertObject2(WorldTreeObj *pObj, 
	const LTVector& vMin, 
	const LTVector& vMax,
	NodeObjArray iArray)
{
	FilterObjInfo foInfo;
	LTVector vDiff;

	pObj->RemoveFromWorldTree();

	vDiff = vMax - vMin;
	
	foInfo.m_pObj = pObj;
	foInfo.m_iObjArray = iArray;
	foInfo.m_Min = vMin;
	foInfo.m_Max = vMax;
	foInfo.m_MaxSize = LTMAX(vDiff.x, vDiff.z);
	foInfo.m_iCurLink = 0;

	if(!pObj->InsertSpecial(this))
	{
		FilterObj_R(&m_RootNode, &foInfo);
	}
}

void WorldTree::FindObjectsInBox(const LTVector *pMin, const LTVector *pMax, 
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
	m_nTempFrameCode = m_pHelper->IncFrameCode();

	pInfo->m_pTree = this;
	FindObjectsInBox_R(&m_RootNode, pInfo);
}


void WorldTree::FindObjectsOnPoint(const LTVector *pPoint,
	WTObjCallback cb, void *pCBUser, NodeObjArray iArray)
{
	FindObjectsInBox(pPoint, pPoint, cb, pCBUser, iArray);
}


void WorldTree::IntersectSegment(const LTVector *pPt1, const LTVector *pPt2, 
	ISCallback cb, void *pCBUser, NodeObjArray iArray)
{
	ISInfo isInfo;

	
	ASSERT(m_pHelper);
	m_nTempFrameCode = m_pHelper->IncFrameCode();

	isInfo.m_pTree = this;
	isInfo.m_iObjArray = iArray;
	isInfo.m_Pts[0] = *pPt1;
	isInfo.m_Pts[1] = *pPt2;
	isInfo.m_CB = cb;
	isInfo.m_pCBUser = pCBUser;

	IntersectSegment_R(&m_RootNode, &isInfo);
}

bool WorldTree::Inherit(const WorldTree *pOther) 
{
	//clear out any old data
	Term();

	//sanity check, there should always be at least 1 node
	assert(pOther->m_nNumNodes > 0);

	//allocate our list of nodes, excluding the first one
	LT_MEM_TRACK_ALLOC(m_pNodes = new WorldTreeNode[pOther->m_nNumNodes - 1], LT_MEM_TYPE_WORLDTREE);

	//make sure the allocation worked
	if(!m_pNodes)
	{
		Term();
		return false;
	}

	uint32 nCurrOffset = 0;

	//copy over the number of nodes into our own value
	m_nNumNodes = pOther->m_nNumNodes;

	// Create an identical node layout.
	m_RootNode.SetBBox(pOther->m_RootNode.GetBBoxMin(), pOther->m_RootNode.GetBBoxMax());
	CopyNodeLayout_R(&m_RootNode, &pOther->m_RootNode, m_pNodes, nCurrOffset);

	//sanity check that we didn't use more nodes than we allocated
	assert(nCurrOffset == (m_nNumNodes - 1));
			
	return true;
}

bool WorldTree::LoadLayout(ILTStream *pStream)
{
	//clear out any old data
	Term();

	//load in the dimensions and number of nodes
	LTVector boxMin, boxMax;

	*pStream >> boxMin >> boxMax;
	*pStream >> m_nNumNodes;

	uint32 nDummyTerrainDepth;
	*pStream >> nDummyTerrainDepth;

	//sanity check, there should always be at least 1 node
	assert(m_nNumNodes > 0);

	//allocate our list of nodes, excluding the first one
	LT_MEM_TRACK_ALLOC(m_pNodes = new WorldTreeNode[m_nNumNodes - 1], LT_MEM_TYPE_WORLDTREE);

	//check the allocation
	if(!m_pNodes)
	{
		Term();
		return false;
	}

	uint8 curByte = 0;
	uint8 curBit = 8;
	m_RootNode.SetBBox(boxMin, boxMax);

	uint32 nCurrOffset = 0;
	m_RootNode.LoadLayout(pStream, curByte, curBit, m_pNodes, nCurrOffset);

	//make sure that the file was valid
	if(pStream->ErrorStatus() != LT_OK)
	{
		//it wasn't. Clean up.
		Term();
		return false;
	}

	//sanity check that we didn't use more nodes than we allocated
	assert(nCurrOffset == (m_nNumNodes - 1));
			
	return true;
}

void WorldTree::CopyNodeLayout_R(WorldTreeNode *pDest, const WorldTreeNode *pSrc, WorldTreeNode* pNodeList, uint32& nCurrOffset)
{
	if(pSrc->HasChildren())
	{
		pDest->Subdivide(pNodeList, nCurrOffset);

		for(uint32 i=0; i < MAX_WTNODE_CHILDREN; i++)
		{
			CopyNodeLayout_R(pDest->GetChild(i), pSrc->GetChild(i), pNodeList, nCurrOffset);
		}
	}
}

// Insert an object into the always-visible list
void WorldTree::InsertAlwaysVisObject(WorldTreeObj *pObj)
{
	WTObjLink *pLink = &pObj->m_Links[OBJ_NODE_LINK_ALWAYSVIS];
	dl_Insert(&m_AlwaysVisObjects, &pLink->m_Link);
	pLink->m_pNode = NULL;
}

// Remove an object from the always-visible list
void WorldTree::RemoveAlwaysVisObject(WorldTreeObj *pObj)
{
	WTObjLink *pLink = &pObj->m_Links[OBJ_NODE_LINK_ALWAYSVIS];
	pLink->m_Link.Remove();
	pLink->m_Link.TieOff();
}



