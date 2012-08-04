//------------------------------------------------------------------
//
//	FILE	  : BspGen.h
//
//	PURPOSE	  : Implements the CBspGen class.
//
//	CREATED	  : 2nd May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------



// Includes....
#include "bdefs.h"
#include "bspgen.h"
#include "threads.h"
#include "pregeometry.h"
#include "splitpoly.h"
#include "processing.h"
#include "preworld.h"


#define POLYTYPE_SKY				0
#define POLYTYPE_NORMAL				1
#define NUM_POLYTYPES				2
#define POLYTYPE_OCCLUDER			2 // Note : These don't ever go in the BSP
#define POLYTYPE_NONEXISTENT		3
#define POLYTYPE_BLOCKER			4
#define POLYTYPE_PARTICLEBLOCKER	5

// Mask to determine if a poly shouldn't be removed, ever..)
#define MASK_NOREMOVE (SURF_VISBLOCKER | SURF_NONEXISTENT | SURF_PHYSICSBLOCKER)

//determines the type of the polygon
inline uint32 GetPolyType(CPrePoly *pPoly)
{
	if (pPoly->GetSurfaceFlags() & SURF_VISBLOCKER)
		return POLYTYPE_OCCLUDER;
	else if (pPoly->GetSurfaceFlags() & SURF_NONEXISTENT)
		return POLYTYPE_NONEXISTENT;
	else if (pPoly->GetSurfaceFlags() & SURF_PHYSICSBLOCKER)
		return POLYTYPE_BLOCKER;
	else if (pPoly->GetSurfaceFlags() & SURF_PARTICLEBLOCKER)
		return POLYTYPE_PARTICLEBLOCKER;
	else if(pPoly->GetSurfaceFlags() & SURF_SKY)
		return POLYTYPE_SKY;
	else
		return POLYTYPE_NORMAL;
}

//maintains a list of polygon types
class CPolyList
{
	public:

		void		AddTail(CPrePoly *pPoly)
		{
			m_PolyLists[GetPolyType(pPoly)].AddTail(pPoly);
		}

		uint32		GetSize()
		{
			uint32 i, total;

			total=0;
			for(i=0; i < NUM_POLYTYPES; i++)
			{
				total += m_PolyLists[i].GetSize();
			}
			return total;
		}

		CLinkedList<CPrePoly*>*	GetFirstList()
		{
			uint32 i;

			for(i=0; i < NUM_POLYTYPES; i++)
			{
				if(m_PolyLists[i].GetSize() != 0)
					return &m_PolyLists[i];
			}
			
			return NULL;
		}

		CLinkedList<CPrePoly*>	m_PolyLists[NUM_POLYTYPES];
};



// Defines....
class CSplitThreadData
{
public:
	
	CSplitThreadData() : m_bDone(FALSE) {}

public:

	bool		m_bDone;

	CBspGen		*m_pBspGen;
	CPolyList	*m_pPolies;	
	NODEREF		*m_pRootNode;

	uint32		m_nLevel;
};


#define NUM_POLIES_TO_CREATE_THREAD	10


// Magic number for skipping through a BSP looking for good splitting planes
#define BSP_BALANCE_SKIP  13
// Other magic number for restricting the search length
#define BSP_BALANCE_MAXSEARCH (BSP_BALANCE_SKIP * BSP_BALANCE_SKIP)

// ----------------------------------------------------------------------- //
//      Routine:        SplitThreadFn
//      Purpose:        This is the multithreaded Routine: that calls 
//                      CBspGen::RecurseOnList()...
// ----------------------------------------------------------------------- //

static void SplitThreadFn( void *in )
{
	CSplitThreadData *pData = (CSplitThreadData*)in;

	pData->m_pBspGen->RecurseOnList( *pData->m_pPolies, pData->m_pRootNode, pData->m_nLevel );
	pData->m_bDone = TRUE;

	--pData->m_pBspGen->m_nThreadsBeingUsed;
	thd_EndThread();
}

 
static PReal CountNodes(CNode *pRoot)
{
	if (!IsValidNode(pRoot))
		return 0.0f;

	return (PReal)(1.0) + CountNodes(pRoot->m_Sides[0]) + CountNodes(pRoot->m_Sides[1]);
}

static PReal CalcBalance(CNode *pRoot)
{
	if (!IsValidNode(pRoot))
		return (PReal)1.0;

	// Get the count on either side
	PReal fLeft = CountNodes(pRoot->m_Sides[0]);
	PReal fRight = CountNodes(pRoot->m_Sides[1]);

	// This is definately a balanced node
	if ((fLeft + fRight) <= 1.0f)
		return (PReal)1.0;

	// Get a number [0.5..1] describing the balancing of the number of nodes
	PReal fBalance = 1.0f - ((PReal)fabs(fRight - fLeft) / ((fLeft + fRight + (PReal)1.0f) * (PReal)2.0));

	// Get the balance of the children
	PReal fLeftBalance = CalcBalance(pRoot->m_Sides[0]);
	PReal fRightBalance = CalcBalance(pRoot->m_Sides[1]);

	// Return the node count balance * the average child balance
	return fBalance * (PReal)0.5 + (fLeftBalance + fRightBalance) * (PReal)0.25;
}


CNode* AddNode(CNodeList &nodeList, const CPrePlane *pPlane, CPrePoly *pPoly)
{
	CNode	*pNode = new CNode;

	memset( pNode, 0, sizeof(CNode) );
	pNode->m_pPlane = pPlane;
	pNode->m_pPoly = pPoly;
	pNode->m_bPostRemove = FALSE;

	nodeList.AddTail(pNode);

	return pNode;
}


inline void ParentizeNode(CPreWorld *pWorld, CNode *pParent, NODEREF child, int childSide)
{
	pParent->m_Sides[childSide] = child;
	if(IsValidNode(child))
		pWorld->GetNode(child)->m_pParentNode = pParent;
}


static void GetNodesWithPoly(CMoArray<CNode*> &nodeList, CPrePoly *pPoly, CNode *pRoot)
{
	while (IsValidNode(pRoot))
	{
		if(pRoot->m_pPoly == pPoly)
			nodeList.Append(pRoot);

		GetNodesWithPoly(nodeList, pPoly, pRoot->m_Sides[0]);

		pRoot = pRoot->m_Sides[1];
	}
}


// Filters the poly into the tree.  Returns TRUE if it hits pNode.
static bool IsPolyInNodeSpace(CNode *pRoot, CNode *pNode, CPrePoly *pPoly)
{
	static SplitStruct splitStruct;
	PolySide side;

	if(!IsValidNode(pRoot))
		return false;
	
	if(pRoot == pNode)
		return true;

	side = GetPolySide(pRoot->m_pPoly->GetPlane(), pPoly, &splitStruct);
	if(side == Intersect)
	{
		if(IsPolyInNodeSpace(pRoot->m_Sides[0], pNode, pPoly))
			return true;

		return IsPolyInNodeSpace(pRoot->m_Sides[1], pNode, pPoly);
	}
	else
	{
		return IsPolyInNodeSpace(pRoot->m_Sides[side], pNode, pPoly);
	}
}


// ----------------------------------------------------------------------- //
// Returns what side pNode is on of it's parent.  If pNode is the root node,
// bRoot is set to TRUE and Intersect is returned.
// ----------------------------------------------------------------------- //
static PolySide GetParentSide(CPreWorld *pWorld, CNode *pNode, bool &bRoot)
{
	CNode *pParent;

	bRoot = false;

	if(pNode == pWorld->m_RootNode)
	{
		bRoot = true;
		return Intersect;
	}
	else
	{
		pParent = pNode->m_pParentNode;
		ASSERT(pParent);
		if(pParent->m_Sides[0] == pNode)
		{
			return (PolySide)0;
		}
		else if(pParent->m_Sides[1] == pNode)
		{
			return (PolySide)1;
		}
		else
		{
			ASSERT(false);
			return (PolySide)0;
		}
	}
}


// ----------------------------------------------------------------------- //
//      Routine:        Constructor
//      Purpose:        
// ----------------------------------------------------------------------- //

CBspGen::CBspGen()
{
	m_pWorld = NULL;

	m_bRemoveIntersectingPolies = FALSE;
}



// ----------------------------------------------------------------------- //
//      Routine:        Destructor
//      Purpose:        
// ----------------------------------------------------------------------- //

CBspGen::~CBspGen()
{
}


bool CBspGen::GenerateBspTree(CBspGenOptions *pOptions)
{
	CPolyList				polyList;
	CGLinkedList<CPrePoly*> *pPolies;
	CPrePoly				*pPoly;
	GPOS					pos;
	uint32					type;

	
	// Set our 'global' to model.
	m_Options = *pOptions;
	m_pWorld = m_Options.m_pWorld;
	
	// Get our poly list.
	if(pOptions->m_pCustomPolyList)
		pPolies = pOptions->m_pCustomPolyList;
	else
		pPolies = &m_pWorld->m_Polies;


	m_pWorld->RemoveAllNodes();

	
	if( pPolies->GetSize() == 0 )
		return false;


	// Set all the stats to defaults.
	m_nSplitPolyArrayCalls = 0;
	m_nPolySplits = 0;
	m_nThreadsBeingUsed = 0;
	

	DoPreProcessing();
	m_AppendCS = thd_CreateCriticalSection();


	// Create the initial poly list.
	for(pos=pPolies->GetHeadPosition(); pos; )
	{
		pPoly = pPolies->GetNext(pos);

		type = GetPolyType(pPoly);
		if (type >= NUM_POLYTYPES)
			continue;

		polyList.m_PolyLists[type].AddTail(pPoly);
	}

	if(polyList.GetSize() == 0)
		return false;

	// Loop thru and make the tree.
	RecurseOnList(polyList, &m_pWorld->m_RootNode, 1);

	// Wait until all threads are done.
	while(m_nThreadsBeingUsed > 0)
	{
		thd_Sleep(100);
	}

	thd_DeleteCriticalSection( m_AppendCS );

	// Finish up.
	if(m_Options.m_bPostProcess)
	{
		DoPostProcessing();
	}

	// Calculate the balancing
	DrawStatusTextIfMainWorld(eST_Normal, "BSP tree %0.2f%% balanced.",
		CalcBalance(m_pWorld->m_RootNode) * 100.0f);

	return true;
}



// ----------------------------------------------------------------------- //
//      Routine:        CBspGen::MinimizePolySize
//      Purpose:        Splits the polygon into pieces of the given size if necessary.
// ----------------------------------------------------------------------- //

bool CBspGen::MinimizeNodePolySize(NODEREF inNode, CReal maxX, CReal maxY)
{
	CNode				*pNewNode;
	bool				bRoot;
	CPrePlane			splitPlane;
	PolySide			side, parentSide;
	PReal				dot;
	bool				bSplit;
	uint32				i;
	int32				outType;
	CPrePoly			*sides[2];
	CNode				*pNodes[2];
	SplitStruct			splitStruct;
	CMoArray<CNode*>	nodeList;
	bool				bIsIn[2];
	CPrePoly			*pInsidePoly;


	CNode*			pNode = m_pWorld->GetNode(inNode);
	CPrePoly*		pPoly = pNode->m_pPoly;
	CPreSurface*	pSurface = pPoly->GetSurface();

	bSplit = false;
	for( i=0; i < pPoly->NumVerts(); i++ )
	{
		// If any of the points go over maxX, split on a plane made from P*maxX
		dot = pSurface->P.Dot( pPoly->Pt(i) - pPoly->PolyO() );
		if( dot > (maxX+POINT_SIDE_EPSILON) )
		{
			outType = 0;

			bSplit = true;
			splitPlane.m_Normal = pPoly->P();
			splitPlane.m_Normal.Norm();
			splitPlane.m_Dist = splitPlane.m_Normal.Dot( pPoly->PolyO() + (pPoly->InverseP() * (maxX - 1.0f)) );
			break;
		}

		// If any of the points go over maxY, split on a plane made from Q*maxY
		dot = pSurface->Q.Dot( pPoly->Pt(i) - pPoly->PolyO() );
		if( dot > (maxY+POINT_SIDE_EPSILON) )
		{
			outType = 1;

			bSplit = true;
			splitPlane.m_Normal = pPoly->Q();
			splitPlane.m_Normal.Norm();
			splitPlane.m_Dist = splitPlane.m_Normal.Dot( pPoly->PolyO() + (pPoly->InverseQ() * (maxY - 1.0f)) );
			break;
		}
	}
	
	
	if(!bSplit)
	{
		return false;
	}
	

	side = GetPolySide(&splitPlane, pPoly, &splitStruct);

	// It REALLY should be intersect, but if it's not checked in release versions, it'll
	// crash the processor.
	ASSERT( side == Intersect );
	if(side != Intersect)
	{
		return false;
	}		

	if(!::SplitPoly(&splitPlane, pPoly, sides, &splitStruct))
	{
		DeletePoly(sides[0]);
		DeletePoly(sides[1]);
		return false;
	}

	sides[0]->FindTextureOrigin(NULL, TRUE, (float)m_pWorld->GetLMGridSize(pPoly));
	sides[1]->FindTextureOrigin(NULL, TRUE, (float)m_pWorld->GetLMGridSize(pPoly));


	// Update all the nodes with this poly.
	GetNodesWithPoly(nodeList, pPoly, m_pWorld->m_RootNode);
	for(i=0; i < nodeList; i++)
	{
		pNode = nodeList[i];

		bIsIn[0] = IsPolyInNodeSpace(m_pWorld->m_RootNode, pNode, sides[0]);
		bIsIn[1] = IsPolyInNodeSpace(m_pWorld->m_RootNode, pNode, sides[1]);

		if((bIsIn[0] + bIsIn[1]) < 2)
		{
			pInsidePoly = bIsIn[0] ? sides[0] : sides[1];

			pNewNode = AddNode(m_pWorld->m_Nodes, pInsidePoly->GetPlane(), pInsidePoly);
			ParentizeNode(m_pWorld, pNewNode, pNode->m_Sides[FrontSide], FrontSide);
			ParentizeNode(m_pWorld, pNewNode, pNode->m_Sides[BackSide], BackSide);

			// Replace this node..  note: it doesn't just replace m_pPoly because the
			// loop over MinimizeNodePolySize assumes that new nodes will go on the end
			// so it doesn't have to keep reiterating.
			parentSide = GetParentSide(m_pWorld, pNode, bRoot);
			if(bRoot)
			{
				m_pWorld->m_RootNode = pNewNode;
				pNewNode->m_pParentNode = NULL;
			}
			else
			{
				ParentizeNode(m_pWorld, pNode->m_pParentNode, pNewNode, parentSide);
			}
		}
		else
		{
			// Ok, they both lie in the node's space.
			pNodes[0] = AddNode(m_pWorld->m_Nodes, sides[0]->GetPlane(), sides[0]);
			pNodes[1] = AddNode(m_pWorld->m_Nodes, sides[1]->GetPlane(), sides[1]);

			// Insert the two new ones into the BSP correctly.
			parentSide = GetParentSide(m_pWorld, pNode, bRoot);
			if(bRoot)
			{
				m_pWorld->m_RootNode = pNodes[0];
				pNodes[0]->m_pParentNode = NULL;
			}
			else
			{
				ParentizeNode(m_pWorld, pNode->m_pParentNode, pNodes[0], parentSide);
			}

			ParentizeNode(m_pWorld, pNodes[0], pNodes[1], FrontSide);
			ParentizeNode(m_pWorld, pNodes[0], pNode->m_Sides[BackSide], BackSide);
			ParentizeNode(m_pWorld, pNodes[1], pNode->m_Sides[FrontSide], FrontSide);
			ParentizeNode(m_pWorld, pNodes[1], NODE_OUT, BackSide);
		}
	}

	
	// Add the 2 new polies.
	m_pWorld->m_Polies.Append( sides[0] );
	m_pWorld->m_Polies.Append( sides[1] );

	return true;
}



// ----------------------------------------------------------------------- //
//      Routine:        CBspGen::RecurseOnList
//      Purpose:        The recursive Routine: that generates a subtree
//                      of the BSP given a big list of polygons.
// ----------------------------------------------------------------------- //

void CBspGen::RecurseOnList(CPolyList &polies, NODEREF *pRootNode, uint32 nLevel)
{
	CPolyList *frontList, *backList;
	CPolyList *sideArrays[2];
	CPrePoly *pSplitPoly;
	CNode *pSplitNode;



	frontList = new CPolyList;
	backList = new CPolyList;
	sideArrays[FrontSide] = frontList;
	sideArrays[BackSide] = backList;


	// Find the best polygon to split everything on.
	pSplitPoly = GetBestSplit(polies, nLevel);
	pSplitNode = *pRootNode = AddNode(m_pWorld->m_Nodes, pSplitPoly->GetPlane(), pSplitPoly);

	// Split all the polygons.
	SplitPolyArray(polies, pSplitPoly, sideArrays);

	// Go into the back side.
	if(backList->GetSize() == 0)
		pSplitNode->m_Sides[BackSide] = NODE_OUT;
	else
		ThreadedRecurseOnList(*backList, &pSplitNode->m_Sides[BackSide], nLevel + 1);

	delete backList;

	
	// Go into the front side.
	if(frontList->GetSize() == 0)
		pSplitNode->m_Sides[FrontSide] = NODE_IN;
	else
		ThreadedRecurseOnList(*frontList, &pSplitNode->m_Sides[FrontSide], nLevel + 1);

	delete frontList;


	// Make sure the child nodes point at the parent node.
	ParentizeNode(m_pWorld, pSplitNode, pSplitNode->m_Sides[FrontSide], FrontSide);
	ParentizeNode(m_pWorld, pSplitNode, pSplitNode->m_Sides[BackSide], BackSide);
}



// ----------------------------------------------------------------------- //
//      Routine:        CBspGen::ThreadedRecurseOnList
//      Purpose:        Recurses into the list of polies to generate the BSP tree.
// ----------------------------------------------------------------------- //
											  
void CBspGen::ThreadedRecurseOnList(CPolyList &polies, NODEREF *pRootNode, uint32 nLevel)
{
	CSplitThreadData		data;
	THREAD_ID				threadID;

	if( (m_nThreadsBeingUsed < m_Options.m_nThreadsToUse) && (polies.GetSize() > NUM_POLIES_TO_CREATE_THREAD) )
	{
		++m_nThreadsBeingUsed;

		data.m_pBspGen = this;
		data.m_pPolies = &polies;
		data.m_pRootNode = pRootNode;
		data.m_nLevel = nLevel;

		threadID = thd_BeginThread( SplitThreadFn, &data );
		thd_WaitForFinish(threadID);
	}
	else
	{
		RecurseOnList( polies, pRootNode, nLevel );
	}
}



// ----------------------------------------------------------------------- //
// GetBestSplit
// Does some testing and returns which polygon is the best one to split 
// the rest of them by.  It determines this by minimizing the (weighted)
// functions of number of splits and tree balance.
// ----------------------------------------------------------------------- //

typedef struct
{
	int		m_nAxisAligned;
	int		m_nAddedPolies;
	int		m_MaxSide, m_SideDiff;
	PReal	m_Weight; 
	DWORD	m_Index;
	CPrePoly *m_pPoly;
} SplitSelector;

static inline BOOL IsSplitBetter(SplitSelector *pTest, SplitSelector *pToBeat)
{
	return pTest->m_Weight < pToBeat->m_Weight;
}

static inline PReal GetSelectorWeight(SplitSelector *pSplit, uint32 nLevel)
{
	PReal fBalance = (PReal)pow((double)g_pGlobs->m_BalanceWeight, (double)nLevel);

	PReal addedPolyFactor, diffFactor;

	addedPolyFactor = (PReal)pSplit->m_nAddedPolies * (1.0f - fBalance); //g_pGlobs->m_SplitWeight;
	diffFactor = (PReal)pSplit->m_SideDiff * fBalance; //g_pGlobs->m_BalanceWeight;

	return addedPolyFactor + diffFactor;
}

static inline int GetQuickSide(const CPrePlane *pPlane, const CPrePoly *pPoly)
{
	// Handle an obviously front/back side polygon
	PReal centerDist = pPlane->DistTo(pPoly->GetCenter());
	if (centerDist > pPoly->GetRadius())
		return FrontSide;
	else if (centerDist < -pPoly->GetRadius())
		return BackSide;

	DWORD i;
	PReal dot;
	int nFront=0, nBack=0;
	
	for(i=0; i < pPoly->NumVerts(); i++)
	{
		dot = pPlane->m_Normal.Dot(pPoly->Pt(i)) - pPlane->m_Dist;
		if(dot > POINT_SIDE_EPSILON)
		{
			if(nBack)
				return Intersect;

			++nFront;
		}
		else if(dot < -POINT_SIDE_EPSILON)
		{
			if(nFront)
				return Intersect;

			++nBack;
		}
	}

	if(nBack == 0)
	{
		// Deal with co-planar polygons
		if (nFront == 0)
		{
			if (pPlane->m_Normal.Dot(pPoly->GetPlane()->m_Normal) >= 0.0f)
				return FrontSide;
			else
				return BackSide;
		}
		else
			return FrontSide;
	}
	else if(nFront == 0)
		return BackSide;
	else
		return Intersect;
}

LTBOOL CalcSplitSelector(CPolyList &polies, CPrePoly *pPoly, SplitSelector &result, const SplitSelector *pBest, uint32 nLevel)
{
	result.m_nAddedPolies = result.m_nAxisAligned = 0;
	result.m_MaxSide = result.m_SideDiff = 0;
	result.m_pPoly = pPoly;
		
	CLinkedList<CPrePoly*> *pList = polies.GetFirstList();
	if (!pList)
		return LTFALSE;

	int nFrontSide = 0;
	int nBackSide = 0;

	// Don't search throught the whole list...
	uint32 nSearchCount = (pList->GetSize() > (BSP_BALANCE_SKIP * BSP_BALANCE_SKIP)) ? (pList->GetSize()  / BSP_BALANCE_SKIP) : pList->GetSize();
	nSearchCount = LTMIN(nSearchCount, BSP_BALANCE_MAXSEARCH);

	for(LPOS listPos = pList->GetHeadPosition(); nSearchCount && listPos; --nSearchCount)
	{
		CPrePoly *pComparePoly = pList->GetNext(listPos);

		if(pPoly == pComparePoly)
			continue;
	
		int side = GetQuickSide(pPoly->GetPlane(), pComparePoly);
		if( side == Intersect )
		{
			++result.m_nAddedPolies;
		}
		else
		{
			if( side == FrontSide )
				++nFrontSide;
			else if( side == BackSide )
				++nBackSide;
		}
	}

	// Figure out how bad the balancing was
	result.m_MaxSide = LTMAX(nFrontSide, nBackSide);
	result.m_SideDiff = abs(nFrontSide - nBackSide);

	// Figure out how good it is.
	result.m_Weight = GetSelectorWeight(&result, nLevel);

	return LTTRUE;
}

											  
CPrePoly* CBspGen::GetBestSplit(CPolyList &polies, uint32 nLevel)
{
	// Get the next available poly list
	CLinkedList<CPrePoly*> *pList = polies.GetFirstList();

	// Go to the beginning
	LPOS listPos = pList->GetHeadPosition();

	SplitSelector bestSplit;

	if (listPos)
	{
		// Choose the first poly
		CalcSplitSelector(polies, pList->GetNext(listPos), bestSplit, NULL, nLevel);
		bestSplit.m_Index = 0;
	}

	// Loop through the rest of the list looking for a better splitter
	DWORD curIteration = 0;
	while(listPos)
	{
		SplitSelector testSplit;

		++curIteration;
		// Remember the iteration (??)
		testSplit.m_Index = curIteration;

		// Calculate the SplitSelector for the next polygon
		if (!CalcSplitSelector(polies, pList->GetNext(listPos), testSplit, &bestSplit, nLevel))
			// Skip this poly if the current one's definately better
			continue;

		// See if it's better than the current best one.
		// Priorities go in this order:
		//	1. Normal brushes always above detail brushes.
		//  2. Least number of splits.
		//  3. Best balanced tree.
		//  4. Axis-aligned splitters.
		if(IsSplitBetter(&testSplit, &bestSplit))
			bestSplit = testSplit;
	}

	ASSERT(bestSplit.m_pPoly);
	return bestSplit.m_pPoly;
}



// ----------------------------------------------------------------------- //
//      Routine:        CBspGen::SplitPolyArray
//      Purpose:        Splits all the polygons in polies against iSplitOn.
//                      Fills in the lists of left and right polygons.
// ----------------------------------------------------------------------- //

void CBspGen::SplitPolyArray(CPolyList &polies, CPrePoly *pSplitOn, CPolyList *sideArrays[2])
{
	LPOS pos;
	CPrePoly *pPoly;
	DWORD i;


	++m_nSplitPolyArrayCalls;
	SetProgressBar( (CReal)(m_nSplitPolyArrayCalls+1) / m_pWorld->m_Polies.GetSize() );
	
	for(i=0; i < NUM_POLYTYPES; i++)
	{
		for(pos=polies.m_PolyLists[i]; pos; )
		{
			pPoly = polies.m_PolyLists[i].GetNext(pos);

			if(pPoly == pSplitOn)
				continue;

			SplitPoly(pSplitOn, pPoly, sideArrays); 
		}
	}
}




// ----------------------------------------------------------------------- //
//      Routine:        CBspGen::SplitPoly
//      Purpose:        Splits the polygon pToSplit on the polygon
//                      pSplitOn.  Fills in pLeft and pRight based on
//                      where pToSplit lies in relation to pSplitOn.
//                      Returns FALSE if there is no more memory.
// ----------------------------------------------------------------------- //

void CBspGen::SplitPoly(CPrePoly *pSplitOn, CPrePoly *pToSplit, CPolyList *sideArrays[2])
{
	CPrePoly *sides[2], *pReplacement;
	PolySide side;
	DWORD i;
	SplitStruct splitStruct;


	sides[0] = sides[1] = NULL;

	// Test for trivial cases.
	side = GetPolySide(pSplitOn->GetPlane(), pToSplit, &splitStruct);


	// Test for the trivial cases...
	switch( side )
	{
		case BackSide:
			sideArrays[BackSide]->AddTail(pToSplit);
			return;

		case FrontSide:
			sideArrays[FrontSide]->AddTail(pToSplit);
			return;
	
		default:
			++m_nPolySplits;
			break;
	}


	// Allocate 2 new polygons.
	::SplitPoly(pSplitOn->GetPlane(), pToSplit, sides, &splitStruct);


	// Fix up and append the new polygons to the correct arrays..
	for(i=0; i < 2; i++)
	{
		ASSERT(sides[i]->NumVerts() >= 3);

		thd_EnterCriticalSection( m_AppendCS );
		m_pWorld->m_Polies.Append( sides[i] );
		thd_LeaveCriticalSection( m_AppendCS );
		
		sideArrays[i]->AddTail(sides[i]);
	}


	// This is where it works differently in hardware-only mode.  It needs to split it during 
	// BSP generation so it doesn't wind up with nodes it doesn't need but afterwards, it 
	// replaces all the new fragments with the original polies.
	pReplacement = pToSplit->m_pReplacement ? pToSplit->m_pReplacement : pToSplit;
	sides[0]->m_pReplacement = pReplacement;
	sides[1]->m_pReplacement = pReplacement;

	// Recalc the sides' centers/radii
	sides[0]->FlushCenter();
	sides[1]->FlushCenter();
}


// ----------------------------------------------------------------------- //
//      Routine:        CBspGen::DoPreProcessing
//      Purpose:        Does any preprocessing on all the geomety.
// ----------------------------------------------------------------------- //

void CBspGen::DoPreProcessing()
{
}



void RemoveUnusedPolygons( CPreWorld *pWorld )
{
	GPOS pos;
	CPrePoly *pPoly;

	
	// First mark them all.
	for(pos=pWorld->m_Polies; pos; )
	{
		pWorld->m_Polies.GetNext(pos)->SetPostRemove(TRUE);
	}

	// Tag all the polies in the BSP.
	pWorld->m_RootNode->ClearPolyPostRemove();


	// Remove untagged ones.	
	for(pos=pWorld->m_Polies; pos; )
	{
		pPoly = pWorld->m_Polies.GetNext(pos);
		
		if(pPoly->GetPostRemove() && ((pPoly->m_pSurface->m_Flags & MASK_NOREMOVE) == 0))
		{
			pWorld->m_Polies.RemoveAt(pPoly);
			DeletePoly(pPoly);
		}
	}
}


void RemoveUnusedNodes( CPreWorld *pWorld )
{
	GPOS pos;
	CNode *pNode;

	
	// First mark them all.
	for(pos=pWorld->m_Nodes; pos; )
	{
		pWorld->m_Nodes.GetNext(pos)->m_bPostRemove = TRUE;
	}

	// Tag all the polies in the BSP.
	pWorld->m_RootNode->ClearNodePostRemove();


	// Remove untagged ones.	
	for(pos=pWorld->m_Nodes; pos; )
	{
		pNode = pWorld->m_Nodes.GetNext(pos);
		
		if(pNode->m_bPostRemove)
		{
			pWorld->m_Nodes.RemoveAt(pNode);
			delete pNode;
		}
	}
}


CNode* NodeWithPoly(CPreWorld *pWorld, CPrePoly *pPoly)
{
	GPOS pos;
	CNode *pNode;

	for(pos=pWorld->m_Nodes; pos; )
	{
		pNode = pWorld->m_Nodes.GetNext(pos);

		if(pNode->m_pPoly == pPoly)
			return pNode;
	}

	return NULL;
}


void FilterPolyIntoBSP(
	CPreWorld			*pWorld, 
	NODEREF				pRoot, 
	CPrePoly			*pPoly, 
	uint32				flags,
	CMoArray<CPrePoly*> *pInsideFragments)
{
	SplitStruct splitStruct;
	PolySide side;
	DWORD i;
	CPrePoly *sides[2];
	CPrePoly *pHullMaker, *pNewPoly;
	CPreSurface *pSurface;
	CNode *pNode;

	sides[0] = sides[1] = NULL;

	side = GetPolySide(pRoot->m_pPlane, pPoly, &splitStruct);
	if(side == Intersect)
	{
		// Split it.
		::SplitPoly(pRoot->m_pPlane, pPoly, sides, &splitStruct);
		ASSERT(sides[0]->NumVerts() >= 3);
		ASSERT(sides[1]->NumVerts() >= 3);
		
		pWorld->m_Polies.RemoveAt(pPoly);
		DeletePoly(pPoly);

		pWorld->m_Polies.Append(sides[0]);
		pWorld->m_Polies.Append(sides[1]);
	}
	else
	{
		sides[side] = pPoly;
	}

	// Add nodes or recurse.
	for(i=0; i < 2; i++)
	{
		if(!sides[i])
			continue;

		if(pRoot->m_Sides[i] == NODE_IN || pRoot->m_Sides[i] == NODE_OUT)
		{
			if((i == FrontSide) || (flags & FILTER_ALWAYSADD))
			{
				// If they only want a list, just add this fragment to the list.
				if(pInsideFragments)
				{
					pInsideFragments->Append(sides[i]);
				}
				else
				{
					pHullMaker = sides[i];
					pSurface = pHullMaker->m_pSurface;

					// Add a BSP node for the hullmaker.
					pNode = AddNode(pWorld->m_Nodes, pHullMaker->GetPlane(), pHullMaker);
					pRoot->m_Sides[i] = pNode;
					pNode->m_Sides[FrontSide] = NODE_IN;
					pNode->m_Sides[BackSide] = NODE_OUT;

					if(flags & FILTER_ADDREVERSE)
					{
						// Add a new, reversed node, for its backside so the space on both sides is still valid.
						pNewPoly = AddReversedPoly(pWorld, pHullMaker);

						// Point the original hullmaker's node at the correct stuff on each side.
						pNode->m_Sides[BackSide] = AddNode(pWorld->m_Nodes, pNewPoly->GetPlane(), pNewPoly);
						
						// Mark front and back as in and out for the new node.
						pNode->m_Sides[BackSide]->m_Sides[FrontSide] = NODE_IN;
						pNode->m_Sides[BackSide]->m_Sides[BackSide] = NODE_OUT;
					}
				}
			}
			else
			{
				pWorld->m_Polies.RemoveAt(sides[i]);
				DeletePoly(sides[i]);
			}
		}
		else
		{
			FilterPolyIntoBSP(
				pWorld, 
				pRoot->m_Sides[i], 
				sides[i], 
				flags,
				pInsideFragments);
		}
	}

	ASSERT(pRoot->m_Sides[FrontSide] != NODE_OUT);
	ASSERT(pRoot->m_Sides[BackSide] != NODE_IN);
}


CPrePoly* AddReversedPoly(
	CPreWorld *pWorld,
	const CPrePoly *pPoly)
{
	CPreSurface *pNewSurface;
	DWORD i;
	CPrePoly *pNewPoly;


	pNewSurface = new CPreSurface;
	*pNewSurface = *pPoly->m_pSurface;
	pNewSurface->m_pPlane = pWorld->FindOrAddPlane(
		-pPoly->m_pSurface->m_pPlane->m_Normal, 
		-pPoly->m_pSurface->m_pPlane->m_Dist);

	pWorld->m_Surfaces.AddTail(pNewSurface);

	pNewPoly = CreatePoly(CPrePoly, pPoly->NumVerts(), false);

	pNewPoly->m_pSurface = pNewSurface;
	for(i=0; i < pPoly->NumVerts(); i++)
	{
		pNewPoly->AddVert(pPoly->Pt(pPoly->NumVerts()-i-1));
		pNewPoly->Alpha(i) = pPoly->Alpha(pPoly->NumVerts()-i-1);
	}

	pWorld->m_Polies.Append(pNewPoly);
	return pNewPoly;
}


CNode* AddBacksideNode(CPreWorld *pWorld, CNode *pNode)
{
	CPrePoly *pNewPoly;
	CNode *pNewNode;


	// Add a new, reversed node, for its backside so the space on both sides is still valid.
	pNewPoly = AddReversedPoly(pWorld, pNode->m_pPoly);

	pNewNode = AddNode(pWorld->m_Nodes, pNewPoly->GetPlane(), pNewPoly);

	pNewNode->m_Sides[FrontSide] = pNode->m_Sides[BackSide];
	if(pNewNode->m_Sides[FrontSide] == NODE_OUT) // Make sure we don't screw the BSP up.
		pNewNode->m_Sides[FrontSide] = NODE_IN;

	pNewNode->m_Sides[BackSide] = NODE_OUT;
	pNode->m_Sides[BackSide] = pNewNode;

	return pNewNode;
}

static void UpdatePolyReplacements(CNode *pRoot)
{
	if(!IsValidNode(pRoot))
		return;

	if(pRoot->m_pPoly->m_pReplacement)
		pRoot->m_pPoly = pRoot->m_pPoly->m_pReplacement;

	UpdatePolyReplacements(pRoot->m_Sides[0]);
	UpdatePolyReplacements(pRoot->m_Sides[1]);
}


// Replaces polies with their m_pOriginalBrushPoly in the BSP tree.
static void ReplaceWithOriginalPolies(CPreWorld *pWorld, CNode *pRoot)
{
	if(!IsValidNode(pRoot))
		return;

	if(pRoot->m_pPoly->m_pOriginalBrushPoly)
	{
		ASSERT(pWorld->m_OriginalBrushPolies.FindElement(pRoot->m_pPoly->m_pOriginalBrushPoly) != BAD_INDEX);
		pRoot->m_pPoly = pRoot->m_pPoly->m_pOriginalBrushPoly;
	}

	ReplaceWithOriginalPolies(pWorld, pRoot->m_Sides[0]);
	ReplaceWithOriginalPolies(pWorld, pRoot->m_Sides[1]);
}


// Moves the m_OriginalBrushPolies list into the m_Polies list.
static void AddOriginalPoliesToWorld(CPreWorld *pWorld)
{
	GPOS pos;
	CPrePoly *pPoly;

	for(pos=pWorld->m_OriginalBrushPolies; pos; )
	{
		pPoly = pWorld->m_OriginalBrushPolies.GetNext(pos);

		if ((pPoly->m_pSurface->m_Flags & SURF_NONEXISTENT) == 0)
			pWorld->m_Polies.AddTail(pPoly);
	}

	pWorld->m_OriginalBrushPolies.RemoveAll();
}


bool CBspGen::DoPostProcessing()
{
	uint32		iCurNode;
	CPreSurface *pSurface;
	CNode		*pNode;
	GPOS		pos;
	uint32		nStartingPolies, nStartingNodes;
	PReal		fClampU, fClampV;


	// Update the replacement polies (ie: replace fragments with the original unsplit polies).
	UpdatePolyReplacements(m_pWorld->m_RootNode);

	// Use the original brush polies (removes the splits from BrushToWorld).
	ReplaceWithOriginalPolies(m_pWorld, m_pWorld->m_RootNode);
	AddOriginalPoliesToWorld(m_pWorld);

	// Remove polygons marked to be removed.	
	RemoveUnusedPolygons( m_pWorld );

	m_pWorld->FindTextureOrigins();

	nStartingPolies = m_pWorld->m_Polies.GetSize();
	nStartingNodes = m_pWorld->m_Nodes.GetSize();


	// Minimize polygon sizes.
	iCurNode = 0;
	for(pos=m_pWorld->m_Nodes; pos; )
	{
		// It does GetAt/GetNext here so it'll work if new nodes get added to the
		// list in between GetAt and GetNext.
		pNode = m_pWorld->m_Nodes.GetAt(pos);
		pSurface = pNode->m_pPoly->GetSurface();

		if(pSurface->ShouldLightmap())
		{
			// We use LIGHTMAP_MAX_PIXELS-1 because all lightmaps need one extra pixel
			// for the last interpolation values.
			fClampU = (m_pWorld->GetLMGridSize(pSurface) * (g_pGlobs->m_MaxLMSize - 1.0f)) - 0.2f;
			fClampV = (m_pWorld->GetLMGridSize(pSurface) * (g_pGlobs->m_MaxLMSize - 1.0f)) - 0.2f;
			MinimizeNodePolySize(pNode, fClampU, fClampV);
		}

		// It does GetAt/GetNext here so it'll work if new nodes get added to the
		// list in between GetAt and GetNext.
		m_pWorld->m_Nodes.GetNext(pos);
		++iCurNode;
	}


	// Remove all the nodes that got removed during minimization.
	RemoveUnusedNodes(m_pWorld);
	RemoveUnusedPolygons(m_pWorld);


	// Display output (after removing unused nodes).
	DrawStatusTextIfMainWorld(eST_Normal, "%d polies (%d nodes) added for lightmapping.", 
		m_pWorld->m_Polies.GetSize() - nStartingPolies, m_pWorld->m_Nodes.GetSize() - nStartingNodes);

	m_pWorld->InitPolyVertexIndices();
	
	m_pWorld->FindTextureOrigins();
	return true;
}



