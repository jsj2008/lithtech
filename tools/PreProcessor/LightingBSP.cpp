#include "lightingbsp.h"
#include "de_world.h"

//-----------------------------
//defines
//-----------------------------

//constants used for plane side determination. Please note that the values
//set up the way they are for performance reasons
#define PLANE_ON				0x0
#define PLANE_FRONT				0x1
#define PLANE_BACK				0x2
#define PLANE_SPAN				(PLANE_FRONT | PLANE_BACK)

//used as an epsilon for determining coplanar planes
#define PLANAR_EPSILON			((PReal)0.01)

//used as an epsilon for determining if a point lies inside of a poly edge
#define POLY_EDGE_EPSILON		((PReal)0.01)

//this is added onto node radius to avoid having it be too small and have light leak
#define NODE_RADIUS_PADDING		((PReal)5.0)


//different types of nodes, used for information gathering
#define NODETYPE_LEAF			0
#define NODETYPE_SINGLE			1
#define NODETYPE_DOUBLE			2

//structure used to hold plane information for a polygon edge
class CLightBSPPolyEdge
{
public:
	PVector		m_vNormal;
	PReal		m_fDist;
};

//class used to represent polygons for lighting. They are represented
//differently in order to optimize segment intersection tests
class CLightBSPPoly
{
public:

	//the center for the polygon bounding sphere
	PVector				m_vBSphereCenter;

	//the radius squared of the bounding sphere
	PReal				m_fBSphereRadiusSqr;

	//the number of edges of the polygon
	uint16				m_nNumEdges;

	//the flags for this polygon
	uint16				m_nFlags;

	//the edge list. This must come last since polygons are
	//created through overallocation of memory
	CLightBSPPolyEdge	m_Edges[1];
	//					...
};


//class used to represent a node in the BSP tree
class CLightBSPNode
{
public:
	CLightBSPNode();
	~CLightBSPNode();

	//used to free the children of a node
	void FreeChildren();

	//determines the depth of the tree
	uint32 GetDepth() const;

	//determines the number of nodes
	uint32 NumNodes() const;

	//determines the minimal depth of the tree
	uint32 GetMinDepth() const;

	//gets the depth of the first node that does not have two children. The higher
	//this number the better.
	uint32 GetDepthOfFirstNullChild() const;

	//determines the longest chain in the tree (a chain is a series of connected
	//nodes that only have 1 child. 0 is optimal)
	uint32 GetLongestChainLen() const;

	//finds the length of chain that this node is in (see above)
	uint32 GetChainLen() const;

	//counts nodes of a specified type
	uint32 CountNodesOfType(uint32 nType);

public:
	
	//the children of this node
	CLightBSPNode*		m_pFront;
	CLightBSPNode*		m_pBack;

	//the plane information
	PVector				m_vPlaneNorm;
	PReal				m_fPlaneDist;

	//the bounding sphere that holds all the polygons that lie on this node
	PVector				m_vBSphereCenter;
	PReal				m_fBSphereRadSqr;

	//The list of polygons that this node contains. Note that this must come last since
	//memory is allocated past the end of the object to support variable sized
	//lists
	uint32				m_nNumPolies;
	CLightBSPPoly*		m_pPolyList[1];
	//                  ...
};

//class used as a function stack in intersection calculations
class CLightBSPStack
{
public:
	PVector			m_vEnd;
	CLightBSPNode*	m_pNode;
	CLightBSPNode*	m_pTest;
};


//-----------------------------------
// Polygon allocators
//-----------------------------------
static inline CLightBSPPoly* AllocateBSPPoly(CPrePoly* pPoly)
{
	//find out how many edges we need
	uint32 nNumEdges = pPoly->NumVerts();

	//sanity check
	ASSERT(nNumEdges >= 3);

	//allocate it
	uint32 size = sizeof(CLightBSPPoly) + sizeof(CLightBSPPolyEdge) * (nNumEdges - 1);
	CLightBSPPoly *pRV = (CLightBSPPoly*)malloc(size);

	if(pRV == NULL)
	{
		return NULL;
	}

	//trigger the constructor
	::new((DummyThingForConstructor*)NULL, pRV) CLightBSPPoly;
	
	//now we need to set up the polygon
	pRV->m_vBSphereCenter		= pPoly->GetCenter();
	pRV->m_fBSphereRadiusSqr	= pPoly->GetRadius() * pPoly->GetRadius();

	//setup the flags
	pRV->m_nFlags = 0;

	if(pPoly->GetSurfaceFlags() & SURF_SKY)
		pRV->m_nFlags |= LMPOLY_SKY;

	//set up the edge information
	pRV->m_nNumEdges = nNumEdges;

	uint32 nPrevVert = nNumEdges - 1;
	for(uint32 nCurrVert = 0; nCurrVert < nNumEdges; nPrevVert = nCurrVert, nCurrVert++)
	{
		//find the edge normal
		pRV->m_Edges[nCurrVert].m_vNormal = pPoly->Normal().Cross(pPoly->Pt(nCurrVert) - pPoly->Pt(nPrevVert));
		pRV->m_Edges[nCurrVert].m_vNormal.Norm();

		//now find the edge distance
		pRV->m_Edges[nCurrVert].m_fDist = pRV->m_Edges[nCurrVert].m_vNormal.Dot(pPoly->Pt(nCurrVert));
	}

	//success
	return pRV;
}

static inline void FreeBSPPoly(CLightBSPPoly* pPoly)
{
	delete pPoly;
}

//-----------------------------------
// Node allocators
//-----------------------------------

//called to allocate a light bsp node with the specified number of polygons
static inline CLightBSPNode* AllocateBSPNode(uint32 nNumPolies)
{
	//sanity check
	ASSERT(nNumPolies >= 1);

	//first allocate the block of memory
	uint32 size = sizeof(CLightBSPNode) + sizeof(CLightBSPPoly*) * (nNumPolies - 1);
	CLightBSPNode *pRV = (CLightBSPNode*)malloc(size);

	if(pRV)
	{
		//trigger the constructor
		::new((DummyThingForConstructor*)NULL, pRV) CLightBSPNode;
		
		//save the poly count
		pRV->m_nNumPolies = nNumPolies;
	}

	return pRV;
}

//called to free a light bsp node
static inline void FreeBSPNode(CLightBSPNode* pNode)
{
	delete pNode;
}

//-----------------------------------
// CLightBSPNode
//-----------------------------------

CLightBSPNode::CLightBSPNode() :
	m_pFront(NULL),
	m_pBack(NULL)
{
}

CLightBSPNode::~CLightBSPNode()
{
	//don't free the polygons, they are stored in a global list, but
	//do free the children
	FreeChildren();
}

void CLightBSPNode::FreeChildren()
{
	if(m_pFront)
	{
		m_pFront->FreeChildren();
		FreeBSPNode(m_pFront);
		m_pFront = NULL;
	}
	if(m_pBack)
	{
		m_pBack->FreeChildren();
		FreeBSPNode(m_pBack);
		m_pBack = NULL;
	}
}

uint32 CLightBSPNode::GetDepth() const
{
	//get the size of the two branches
	uint32 nFront	= 1 + ((m_pFront) ? m_pFront->GetDepth() : 0);
	uint32 nBack	= 1 + ((m_pBack) ? m_pBack->GetDepth() : 0);

	//return the larger branch
	if(nFront > nBack)
		return nFront;

	return nBack;
}

//determines the minimal depth of the tree
uint32 CLightBSPNode::GetMinDepth() const
{
	//get the size of the two branches
	uint32 nFront	= 1 + ((m_pFront) ? m_pFront->GetDepth() : 0);
	uint32 nBack	= 1 + ((m_pBack) ? m_pBack->GetDepth() : 0);

	//return the smaller branch
	if(nFront > nBack)
		return nBack;

	return nFront;
}

//counts nodes of a specified type
uint32 CLightBSPNode::CountNodesOfType(uint32 nType)
{
	uint32 nRV = 0;

	if(m_pFront)
		nRV += m_pFront->CountNodesOfType(nType);
	if(m_pBack)
		nRV += m_pBack->CountNodesOfType(nType);

	//see if we contribute
	switch(nType)
	{
	case NODETYPE_LEAF:		
		if((m_pFront == NULL) && (m_pBack == NULL))
			nRV++;
		break;
	case NODETYPE_SINGLE:
		if(	((m_pFront != NULL) && (m_pBack == NULL)) ||
			((m_pFront == NULL) && (m_pBack != NULL)))
			nRV++;
		break;
	case NODETYPE_DOUBLE:
		if(m_pFront && m_pBack)
			nRV++;
		break;
	}

	return nRV;
}

//finds the length of chain that this node is in (see above)
uint32 CLightBSPNode::GetChainLen() const
{
	if((m_pFront == NULL) || (m_pBack == NULL))
	{
		//we are part of a chain
		if(m_pFront)
			return 1 + m_pFront->GetChainLen();
		if(m_pBack)
			return 1 + m_pBack->GetChainLen();
	}

	//not part of a chain
	return 0;
}

//determines the longest chain in the tree (a chain is a series of connected
//nodes that only have 1 child. 0 is optimal)
uint32 CLightBSPNode::GetLongestChainLen() const
{
	//get the longest chains for the left and right sides
	uint32 nFront = (m_pFront) ? m_pFront->GetLongestChainLen() : 0;
	uint32 nBack = (m_pBack) ? m_pBack->GetLongestChainLen() : 0;

	//find the length of chain for this node
	uint32 nCurr = GetChainLen();

	//return the longest
	if((nCurr > nFront) && (nCurr > nBack))
		return nCurr;
	if(nFront > nBack)
		return nFront;

	return nBack;
}


//gets the depth of the first node that does not have two children. The higher
//this number the better.
uint32 CLightBSPNode::GetDepthOfFirstNullChild() const
{
	//see if we are the node with only one child
	if((m_pFront == NULL) || (m_pBack == NULL))
	{
		return 1;
	}

	//find the minimum one of our children
	uint32 nFront = m_pFront->GetDepthOfFirstNullChild();
	uint32 nBack  = m_pBack->GetDepthOfFirstNullChild();

	//return the min
	return ((nFront < nBack) ? nFront : nBack) + 1;
}



//determines the number of nodes
uint32 CLightBSPNode::NumNodes() const
{
	uint32 nNum = 1;
	if(m_pFront)
		nNum += m_pFront->NumNodes();
	if(m_pBack)
		nNum += m_pBack->NumNodes();

	return nNum;
}

//-----------------------------------
// CLightingBSP
//-----------------------------------
CLightingBSP::CLightingBSP() :
	m_pHead(NULL),
	m_nNumPolies(0),
	m_ppPolyList(NULL)
{
}

CLightingBSP::~CLightingBSP()
{
	FreeBSP();
}

void CLightingBSP::FreeBSP()
{
	FreeBSPNode(m_pHead);
	m_pHead = NULL;

	//now delete our polygons
	if(m_ppPolyList)
	{
		for(uint32 nCurrPoly = 0; nCurrPoly < m_nNumPolies; nCurrPoly++)
		{
			FreeBSPPoly(m_ppPolyList[nCurrPoly]);
			m_ppPolyList[nCurrPoly] = NULL;
		}

		//free the list itself
		delete [] m_ppPolyList;
		m_ppPolyList = NULL;
	}
	m_nNumPolies = 0;
}

//---------------------------------------------------------------------------------------------
// BSP Generation
//---------------------------------------------------------------------------------------------

//classifies a polygon with regards to the plane. It will return one of the PLANE_XXX values
//indicating the classification of the polygon
static uint32 ClassifyPoly(const PVector& vNormal, PReal fPlaneDist, CPrePoly* pPoly)
{
	//sanity check
	ASSERT(pPoly);

	//first classify the poly sphere
	PReal fDot = vNormal.Dot(pPoly->m_BasePoly.GetCenterDirect()) - fPlaneDist;

	//see if it lies entirely on one side
	PReal fPolyRad = pPoly->m_BasePoly.GetRadiusDirect();

	if(fDot > fPolyRad)
	{
		//it is entirely on the front side
		return PLANE_FRONT;
	}
	else if(fDot < -fPolyRad)
	{
		//it is entirely on the back side
		return PLANE_BACK;
	}

	uint32 nRV = PLANE_ON;

	uint32 nNumVerts = pPoly->NumVerts();

	//the polygon could span, so now we have to do the full every vert check
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		fDot = vNormal.Dot(pPoly->Pt(nCurrVert)) - fPlaneDist;

		if(fDot < -PLANAR_EPSILON)
		{
			//check for front side intersection
			for(nCurrVert++; nCurrVert < nNumVerts; nCurrVert++)
			{
				if(vNormal.Dot(pPoly->Pt(nCurrVert)) - fPlaneDist > PLANAR_EPSILON)
				{
					return PLANE_SPAN;
				}
			}
			return PLANE_BACK;
		}

		if(fDot > PLANAR_EPSILON)
		{
			//check for front side intersection
			for(nCurrVert++; nCurrVert < nNumVerts; nCurrVert++)
			{
				if(vNormal.Dot(pPoly->Pt(nCurrVert)) - fPlaneDist < -PLANAR_EPSILON)
				{
					return PLANE_SPAN;
				}
			}
			return PLANE_FRONT;
		}
	}

	return nRV;
}

//calculates the score for a split. The lower the better
static uint32 CalcSplitScore(uint32 nSplits, uint32 nFront, uint32 nBack)
{
	//splits are bad because they add nodes, and unbalancing is also bad
	return nSplits * 20 + abs((long)(nFront - nBack)) * 10;
}

//determines which polygon would be the best to split. Returns the integer index of that
//polygon. Note that it assumes that the list contain at least one element. In addition,
//it will return the number of polygons that lie on the same plane to make allocation
//of nodes easier, along with how many will end up on the front and back
static uint32 FindBestSplitPlane(PrePolyArray& PolyList, 
								 uint32& nNumLieOn, uint32& nNumFront, uint32& nNumBack)
{
	ASSERT(PolyList.GetSize() > 0);

	uint32 nBestIndex = 0;
	uint32 nBestScore = 0xFFFFFFFF;

	//plane information
	PVector vNormal;
	PReal fPlaneDist;

	//counts
	uint32 nFront, nBack, nSplit, nOn;

	//the poly classification
	uint32 nType;

	//the number of polygons to test
	uint32 nNumPolies = PolyList.GetSize();

	//the amount of polygons to skip over. Currently just do 2 * sqrt(n)
	//samples, so 20 polygons will be sampled for 100 nodes, etx. This
	//speeds up BSP generation time significantly.
	uint32 nPolyInc = LTMAX(1, (uint32)(nNumPolies / sqrt(4.0 * nNumPolies)));

	for(uint32 nCurrPoly = 0; nCurrPoly < nNumPolies; nCurrPoly += nPolyInc)
	{
		fPlaneDist	= PolyList[nCurrPoly]->Dist();
		vNormal		= PolyList[nCurrPoly]->Normal();

		//reset the info
		nFront = nBack = nSplit = nOn = 0;

		//now need to count up the split information
		for(uint32 nTestPoly = 0; nTestPoly < PolyList.GetSize(); nTestPoly++)
		{
			//skip over the current poly
			if(nTestPoly == nCurrPoly)
			{
				nOn++;
				continue;
			}

			nType = ClassifyPoly(vNormal, fPlaneDist, PolyList[nTestPoly]);

			if(nType == PLANE_SPAN)
				nSplit++;
			else if(nType == PLANE_FRONT)
				nFront++;
			else if(nType == PLANE_BACK)
				nBack++;
			else
				nOn++;
		}

		//ok, figure out the score
		uint32 nScore = CalcSplitScore(nSplit, nFront, nBack);

		if(nScore < nBestScore)
		{
			nBestScore	= nScore;
			nBestIndex	= nCurrPoly;
			nNumLieOn	= nOn;
			nNumFront	= nFront + nSplit;
			nNumBack	= nBack + nSplit;
		}
	}

	return nBestIndex;
}

static bool RecursivelyBuildBSP(CLightBSPNode** ppNode, PrePolyArray& PolyList, CLightBSPPoly** ppBSPPolyList)
{
	//sanity check
	ASSERT(ppNode);
	ASSERT(PolyList.GetSize() > 0);

	//number of polies that lie on the plane
	uint32 nNumLieOn, nFront, nBack;

	//first off, find the best splitting plane
	uint32 nSplitPlane = FindBestSplitPlane(PolyList, nNumLieOn, nFront, nBack);

	//allocate the new node
	*ppNode = AllocateBSPNode(nNumLieOn);

	//check for memory failure
	if(*ppNode == NULL)
		return false;

	//setup the node
	(*ppNode)->m_vPlaneNorm = PolyList[nSplitPlane]->Normal();
	(*ppNode)->m_fPlaneDist = PolyList[nSplitPlane]->Dist();

	//create the front, back, and on arrays
	PrePolyArray	Front, Back, On;
	Front.SetSize(nFront);
	Back.SetSize(nBack);
	On.SetSize(nNumLieOn);

	//now fill up all the lists
	PVector vNormal = PolyList[nSplitPlane]->Normal();
	float	fDist	= PolyList[nSplitPlane]->Dist();

	//index to the coplanar poly offset
	uint32 nPolyIndex = 0;

	//indices for the lists
	uint32 nFrontIndex	= 0;
	uint32 nBackIndex	= 0;
	uint32 nOnIndex		= 0;

	uint32 nType;

	//now need to count up the split information
	for(uint32 nTestPoly = 0; nTestPoly < PolyList.GetSize(); nTestPoly++)
	{
		if(nTestPoly == nSplitPlane)
		{
			On[nOnIndex++] = PolyList[nTestPoly];
			(*ppNode)->m_pPolyList[nPolyIndex++] = ppBSPPolyList[PolyList[nTestPoly]->m_Index];
			continue;
		}

		nType = ClassifyPoly(vNormal, fDist, PolyList[nTestPoly]);

		if(nType & PLANE_FRONT)
			Front[nFrontIndex++] = PolyList[nTestPoly];
		if(nType & PLANE_BACK)
			Back[nBackIndex++] = PolyList[nTestPoly];
		if(nType == PLANE_ON)
		{
			On[nOnIndex++] = PolyList[nTestPoly];
			(*ppNode)->m_pPolyList[nPolyIndex++] = ppBSPPolyList[PolyList[nTestPoly]->m_Index];
		}
	}

	//sanity check
	ASSERT(nPolyIndex == (*ppNode)->m_nNumPolies);
	ASSERT(nFrontIndex == nFront);
	ASSERT(nBackIndex == nBack);
	ASSERT(nOnIndex == nNumLieOn);

	//build up the child lists
	bool bSuccess = true;

	if(Front.GetSize() > 0)
	{
		bSuccess = RecursivelyBuildBSP(&(*ppNode)->m_pFront, Front, ppBSPPolyList);
	}
	if(bSuccess && (Back.GetSize() > 0))
	{
		bSuccess = RecursivelyBuildBSP(&(*ppNode)->m_pBack, Back, ppBSPPolyList);
	}

	//see if we need to clean up
	if(!bSuccess)
	{
		FreeBSPNode(*ppNode);
		*ppNode = NULL;
		return false;
	}

	//we need to update the node's bounding sphere. This is done by running through all
	//the polygons on the plane and generating a bounding box, finding its center, and then
	//the maximum distance to the points
	PVector vMin((PReal)MAX_CREAL, (PReal)MAX_CREAL, (PReal)MAX_CREAL);
	PVector vMax(-vMin);

	uint32 nCurrPoly;
	for(nCurrPoly = 0; nCurrPoly < nNumLieOn; nCurrPoly++)
	{
		CPrePoly* pPoly = On[nCurrPoly];
		for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nCurrVert++)
		{
			VEC_MIN(vMin, vMin, pPoly->Pt(nCurrVert));
			VEC_MAX(vMax, vMax, pPoly->Pt(nCurrVert));
		}
	}

	//found the center
	(*ppNode)->m_vBSphereCenter = ((vMin + vMax) / 2);

	//now update the radius
	(*ppNode)->m_fBSphereRadSqr = 0;

	PReal fDistSqr;

	for(nCurrPoly = 0; nCurrPoly < nNumLieOn; nCurrPoly++)
	{
		CPrePoly* pPoly = On[nCurrPoly];
		for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nCurrVert++)
		{
			fDistSqr = (pPoly->Pt(nCurrVert) - (*ppNode)->m_vBSphereCenter).MagSqr();

			if(fDistSqr > (*ppNode)->m_fBSphereRadSqr)
			{
				(*ppNode)->m_fBSphereRadSqr = fDistSqr;
			}
		}
	}
	
	//add some padding to the radius to compensate for inaccuracy
	(*ppNode)->m_fBSphereRadSqr += NODE_RADIUS_PADDING;

	return bSuccess;	
}

//given a list of polygons, this will generate a non splitting BSP
//of those polygons. Note that it maintains pointers to the polygons
//passed in, so they cannot be deleted unless this lighting bsp is
//freed first
bool CLightingBSP::BuildBSP(PrePolyArray& PolyList)
{
	//free any old BSP
	FreeBSP();

	//first create our list to hold our internal polygons
	m_ppPolyList = new CLightBSPPoly* [PolyList.GetSize()];

	if(m_ppPolyList == NULL)
	{
		return false;
	}

	m_nNumPolies = PolyList.GetSize();

	//we want to run through all polygons and ensure that they have their
	//bounding spheres up to date
	for(uint32 nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		PolyList[nCurrPoly]->FlushCenter();
		PolyList[nCurrPoly]->m_Index = nCurrPoly;

		//create our polygon
		m_ppPolyList[nCurrPoly] = AllocateBSPPoly(PolyList[nCurrPoly]);
	}

	//sanity check
	if(PolyList.GetSize() == 0)
	{
		return false;
	}

	//now we need to build the tree
	return RecursivelyBuildBSP(&m_pHead, PolyList, m_ppPolyList);
}

//---------------------------------------------------------------------------------------------
// Segment Intersection
//---------------------------------------------------------------------------------------------

//determines if the point is intersecting any of the polygons in the specified node. If
//it is, it returns the pointer to the intersected polygon
static inline CLightBSPPoly* CheckHitNode(CLightBSPNode* pNode, const PVector& vPt)
{
	//sanity check
	ASSERT(pNode);

	//first off do a radius check on all the polygons in the node
	if((vPt - pNode->m_vBSphereCenter).MagSqr() >= pNode->m_fBSphereRadSqr)
	{
		//we missed all polygons
		return NULL;
	}

	CLightBSPPoly* pPoly;

	//now we need to check all polygons in the list
	for(uint32 nCurrPoly = 0; nCurrPoly < pNode->m_nNumPolies; nCurrPoly++)
	{
		pPoly = pNode->m_pPolyList[nCurrPoly];

		//first off do a radius check
		if((vPt - pPoly->m_vBSphereCenter).MagSqr() >= pPoly->m_fBSphereRadiusSqr)
		{
			//outside of this radius, don't bother checking the polygon
			continue;
		}

		//now we actually need to do the full polygon level test
		for(uint32 nCurrEdge = 0; nCurrEdge < pPoly->m_nNumEdges; nCurrEdge++)
		{
			if(pPoly->m_Edges[nCurrEdge].m_vNormal.Dot(vPt) - pPoly->m_Edges[nCurrEdge].m_fDist > POLY_EDGE_EPSILON)
			{
				//we are on the outside
				
				//break with nCurrVert being set to 0 to indicate that no hit occurred
				nCurrEdge = 0;
				break;
			}
		}

		//if nCurrVert is 0, that is the flag that it was outside
		if(nCurrEdge > 0)
		{
			return pNode->m_pPolyList[nCurrPoly];
		}
	}

	//nothing was hit
	return NULL;
}

//determines if the specified segment intersects a polygon. It fills out
//passed in datastructure appropriately.
bool CLightingBSP::IntersectSegment(const PVector& vSegStart, const PVector& vSegEnd, 
									CLightBSPStack* pStack, CIntersectInfo* pInfo) const
{
	ASSERT(pStack);

	uint32 nStackOff = 0;

	//the working endpoints
	PVector vStart(vSegStart);
	PVector vEnd(vSegEnd);

	//the polygon hit
	CLightBSPPoly* pHitPoly;

	//the working node
	CLightBSPNode* pNode = m_pHead;

	while(1)
	{
		//classify the points
		PReal fStartDot = vStart.Dot(pNode->m_vPlaneNorm) - pNode->m_fPlaneDist;
		PReal fEndDot	= vEnd.Dot(pNode->m_vPlaneNorm) - pNode->m_fPlaneDist;

		//check for the same side
		if((fStartDot >= 0) && (fEndDot >= 0))
		{
			//we are in the front
			pNode = pNode->m_pFront;
		}
		else if((fStartDot < 0) && (fEndDot < 0))
		{
			//we are in the back
			pNode = pNode->m_pBack;
		}
		else
		{
			//save the node that we want to test first
			pStack[nStackOff].m_pTest	= pNode;
			pStack[nStackOff].m_vEnd	= vEnd;

			//we have intersected
			if(fStartDot < 0)
			{
				//setup our stack information
				pStack[nStackOff].m_pNode	= pNode->m_pFront;

				//we do the back first
				pNode = pNode->m_pBack;
			}
			else
			{
				//setup our stack information
				pStack[nStackOff].m_pNode	= pNode->m_pBack;

				//we do the front first
				pNode = pNode->m_pFront;
			}

			//calculate the new working endpoint
			vEnd = vStart + (vEnd - vStart) * -fStartDot / (fEndDot - fStartDot);

			//next stack item
			nStackOff++;
		}

		//if we hit a null node, we need to bubble out of the stack, or bail
		while(pNode == NULL)
		{
			//if the stack is empty, no intersection occurred
			if(nStackOff == 0)
				return false;

			//pop the stack
			nStackOff--;

			//the stack is not empty, we need to test against the specified node,
			//and then check behind it if no hit occurred
			if(pHitPoly = CheckHitNode(pStack[nStackOff].m_pTest, vEnd))
			{
				if(pInfo)
				{
					pInfo->m_vNormal		= pStack[nStackOff].m_pTest->m_vPlaneNorm;
					pInfo->m_vIntersectPt	= vEnd;
					pInfo->m_nFlags			= pHitPoly->m_nFlags;
				}
				return true;
			}

			//no intersection, so we now need to process the back half
			vStart = vEnd;
			vEnd = pStack[nStackOff].m_vEnd;
			pNode = pStack[nStackOff].m_pNode;
		}

	} //infinite loop. Exits when stack is popped off completely, or when poly is hit
}

//------------------------------------------------------------------------------------------
// Stack Creation
//------------------------------------------------------------------------------------------

//this should be called once per thread to create a stack that the thread can use for
//intersection queries
CLightBSPStack* CLightingBSP::CreateStack() const
{
	//if there is no BSP we can't allocate a stack
	if(m_pHead == NULL)
	{
		return NULL;
	}

	//determine the deepest possible stack
	uint32 nMaxDepth = m_pHead->GetDepth();

	//now allocate the array based upon that depth
	return new CLightBSPStack [nMaxDepth];
}

//this needs to be used to free the allocated stack
void CLightingBSP::FreeStack(CLightBSPStack* pStack) const
{
	//just clean it up
	delete [] pStack;
}

//------------------------------------------------------------------------------------------
// Information
//------------------------------------------------------------------------------------------
//determines the depth of this tree
uint32 CLightingBSP::GetTreeDepth() const
{
	return (m_pHead) ? m_pHead->GetDepth() : 0;
}

//determines the number of nodes in the tree
uint32 CLightingBSP::GetNumNodes() const
{
	return (m_pHead) ? m_pHead->NumNodes() : 0;
}

//gets the minimal depth of the tree
uint32 CLightingBSP::GetMinTreeDepth() const
{
	return (m_pHead) ? m_pHead->GetMinDepth() : 0;
}

//gets the depth of the first node that does not have two children. The higher
//this number the better.
uint32 CLightingBSP::GetDepthOfFirstNullChild() const
{
	return (m_pHead) ? m_pHead->GetDepthOfFirstNullChild() : 0;
}

//determines the longest chain in the tree (a chain is a series of connected
//nodes that only have 1 child. 0 is optimal)
uint32 CLightingBSP::GetLongestChainLen() const
{
	return (m_pHead) ? m_pHead->GetLongestChainLen() : 0;
}

//gets the number of leaves (no children)
uint32 CLightingBSP::GetLeafCount() const
{
	return (m_pHead) ? m_pHead->CountNodesOfType(NODETYPE_LEAF) : 0;
}

//gets the number of nodes with only one child
uint32 CLightingBSP::GetSingleChildrenCount() const
{
	return (m_pHead) ? m_pHead->CountNodesOfType(NODETYPE_SINGLE) : 0;
}

//gets the number of nodes with two children
uint32 CLightingBSP::GetTwoChildrenCount() const
{
	return (m_pHead) ? m_pHead->CountNodesOfType(NODETYPE_DOUBLE) : 0;
}
