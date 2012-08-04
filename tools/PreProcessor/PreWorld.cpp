//------------------------------------------------------------------
//
//	FILE	  : PreWorld.cpp
//
//	PURPOSE	  : Implements the CPreWorld class.
//
//	CREATED	  : 2nd May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

// ------------------------------------------------------------------------- //
// STL include notes:  The STL headers (without modification) cannot
// coexist with MFC.  So this file cannot use MFC.  Specifically,
// BDEFS_MFC, STDLITH_MFC, and _AFXDLL can't be defined.  In addition,
// it must include windows.h because CWorldNodes have HTREEITEMs 
// in DEdit.
//
// To include windows.h, we must define BDEFS_WINDOWS_H since stdlith.h
// has to be included after windows.h (cuz windows.h doesn't like 
// it if someone has already defined DWORD, WORD, etc..).
//
// Also, for some reason this file can NOT use the intel compiler.  It
// doesn't like STL either!
//
// It actually used to be worse when all the libs Lithtech used had
// #pragmas to include their libs because I couldn't have a specific file
// not use the stdlith version of MFC because it would then include an
// EXTRA lib!
// ------------------------------------------------------------------------- // 

#include "list"
#include "queue"

using namespace std;

//------------------------------------------------------------------

//this is neede by DEdit

 
//work around an issue where windows is masking the DrawStatusText function for
//localization purposes
#undef DrawStatusText

#include "preworld.h"
#include "prepoly.h"
#include "pregeometry.h"
#include "lightmap_planes.h"
#include "processing.h"
#include "dtxmgr.h"
#include "lightmapdefs.h"
#include "filemarker.h"
#include "ltamgr.h"
#include "ltasaveutils.h"
#include "prelightmap.h"
#include "bspgen.h"


#define SURFACE_TEXTURE_VARIANCE	0.1f

// Used in CPreWorld::GenerateVertexNormals.
#define MAX_POLYADJACENCY_POLIES	16


class PolyAdjacency
{
public:

				PolyAdjacency()
				{
					m_nPolies = 0;
				}

	CPrePoly	*m_Polies[MAX_POLYADJACENCY_POLIES];
	uint32		m_nPolies;
};


void BuildSurfacePolyLists(CGLinkedList<CPreSurface*> &surfaces, CPrePolyList &polies)
{
	GPOS pos;
	CPrePoly *pPoly;

	TermSurfacePolyLists(surfaces);

	for(pos=polies; pos; )
	{
		pPoly = polies.GetNext(pos);
		pPoly->GetSurface()->m_PolyList.AddTail(pPoly, &pPoly->m_PolyListNode);
	}
}

void TermSurfacePolyLists(CGLinkedList<CPreSurface*> &surfaces)
{
	GPOS pos;

	for(pos=surfaces; pos; )
		surfaces.GetNext(pos)->m_PolyList.RemoveAll();
}


int NodeToIndex(NODEREF node)
{
	if(node == NODE_IN)
		return -1;
	else if(node == NODE_OUT)
		return -2;
	else
		return node->m_Index;
}

void SetNodeIndices(CNodeList &theList)
{
	uint32 curIndex;
	CNode *pNode;
	GPOS pos;

	curIndex = 0;
	for(pos=theList; pos; )
	{
		pNode = theList.GetNext(pos);
		pNode->m_Index = curIndex;
		curIndex++;
	}
}


static CPreWorld *g_pRecursionWorld;
static int g_MaxRecursionDepth;
static void RecurseAndGetTreeDepth(NODEREF iRoot, int depth)
{
	if(!IsValidNode(iRoot))
		return;	

	if(depth > g_MaxRecursionDepth)
		g_MaxRecursionDepth = depth;

	RecurseAndGetTreeDepth(g_pRecursionWorld->GetNode(iRoot)->m_Sides[0], depth+1);
	RecurseAndGetTreeDepth(g_pRecursionWorld->GetNode(iRoot)->m_Sides[1], depth+1);
}


class PointElement
{
	public:

				PointElement() {}
				PointElement(PVector *pVec, uint32 *pIndex)
				{
					m_pPoint = pVec;
					m_pIndex = pIndex;
					m_Mag = m_pPoint->Mag();
				}
		
		bool operator<(const PointElement &other) const
		{
			return m_Mag > other.m_Mag;
		}

		PReal m_Mag;
		PVector	*m_pPoint;
		uint32	*m_pIndex;

};


class PointStackEl
{
	public:

		void	AddToWorld(CPreWorld *pWorld);

		PReal m_Mag;

		PVector *m_pPoint;

		// All the WORD pointers that depend on this coordinate.
		list<uint32*> m_Attachments;

};

void PointStackEl::AddToWorld(CPreWorld *pWorld)
{
	list<uint32*>::iterator iAttachment;
	PVertex vert;

	vert.PVector::operator=(*m_pPoint);
	pWorld->m_Points.Append(vert);

	for(iAttachment=m_Attachments.begin(); 
		iAttachment != m_Attachments.end(); iAttachment++)
	{
		*(*iAttachment) = (uint32)pWorld->m_Points.LastI();
	}
}


				  
// ----------------------------------------------------------------------- //
// PreLightFrame.
// ----------------------------------------------------------------------- //

CPreLightFrame::~CPreLightFrame()
{
	DeleteAndClearArray(m_PolyMaps);
}



// ----------------------------------------------------------------------- //
// PreLightAnim.
// ----------------------------------------------------------------------- //

CPreLightAnim::CPreLightAnim()
{
	m_Name[0] = 0;
}


CPreLightAnim::~CPreLightAnim()
{
	DeleteAndClearArray(m_Frames);
}


uint32 CPreLightAnim::CalcFrameDataSize()
{
	uint32 iFrame, iMap, size;
	CPreLightFrame *pFrame;


	size = 0;
	for(iFrame=0; iFrame < m_Frames; iFrame++)
	{
		pFrame = m_Frames[iFrame];

		for(iMap=0; iMap < pFrame->m_PolyMaps; iMap++)
		{
			size += pFrame->m_PolyMaps[iMap]->m_Data.GetSize();
		}
	}

	return size;
}	

// ----------------------------------------------------------------------- //
// CPreLightGroup
// ----------------------------------------------------------------------- //

CPreLightGroup_PolyData::CPreLightGroup_PolyData() : 
	m_pLightmap(0) 
{
}

CPreLightGroup_PolyData::~CPreLightGroup_PolyData() 
{ 
	delete m_pLightmap; 
}

CPreLightGroup_PolyData::CPreLightGroup_PolyData(const CPreLightGroup_PolyData &cOther) : 
	m_hPoly(cOther.m_hPoly), 
	m_aVertexIntensities(cOther.m_aVertexIntensities) 
{
	if (cOther.m_pLightmap)
	{
		m_pLightmap = new CPreLightMap;
		*m_pLightmap = *cOther.m_pLightmap;
	}
	else
		m_pLightmap = 0;
}

CPreLightGroup_PolyData &CPreLightGroup_PolyData::operator=(const CPreLightGroup_PolyData &cOther) 
{
	if (&cOther == this)
		return *this;

	m_hPoly = cOther.m_hPoly;
	m_aVertexIntensities = cOther.m_aVertexIntensities;
	delete m_pLightmap;
	if (cOther.m_pLightmap)
	{
		m_pLightmap = new CPreLightMap;
		*m_pLightmap = *cOther.m_pLightmap;
	}
	else
		m_pLightmap = 0;

	return *this;
}

// ----------------------------------------------------------------------- //
// CPreMainWorld.
// ----------------------------------------------------------------------- //

CPreMainWorld::CPreMainWorld()
{
	m_pInfoString	= "";
	m_pLightGrid	= NULL;

	m_vWorldOffset.Init();
}


CPreMainWorld::~CPreMainWorld()
{
	Term();
}


void CPreMainWorld::Term()
{
	delete [] m_pLightGrid;
	m_pLightGrid = NULL;

	DeleteAndClearArray(m_LightAnims);

	while (!m_aLightGroups.empty())
	{
		delete m_aLightGroups.back();
		m_aLightGroups.pop_back();
	}

	std::vector<bool> aParentFlag;
	aParentFlag.resize(m_Objects.GetSize(), false);

	// Figure out which objects are parents
	// Must be done before deleting the objects, since some of the objects in the list
	// are actually children and will be deleted by the parents getting deleted.
	for (uint32 nParentLoop = 0; nParentLoop < m_Objects.GetSize(); ++nParentLoop)
		aParentFlag[nParentLoop] = m_Objects[nParentLoop]->GetParent() == 0;

	// Delete only the objects which are parents in their heirarchy
	for (uint32 nObjLoop = 0; nObjLoop < m_Objects.GetSize(); ++nObjLoop)
	{
		if (aParentFlag[nObjLoop])
			delete m_Objects[nObjLoop];
	}
	m_Objects.SetSize(0);

	for( std::vector<CPreBlindData*>::iterator it = m_BlindObjectData.begin(); it != m_BlindObjectData.end(); it++ )
	{
		delete *it;
	}
	m_BlindObjectData.clear();

	m_pInfoString = "";
	m_StringHolder.ClearStrings();
	TermGeometry();
}


void CPreMainWorld::TermGeometry()
{
	for (uint32 i=0;i<m_WorldModels.GetSize();++i){
		m_WorldModels[i]->TermGeometry();
	}
	DeleteAndClearArray(m_WorldModels);

	DeleteAndClearArray(m_BlockerPolys);

	DeleteAndClearArray(m_ParticleBlockerPolys);

	m_WorldTree.Term();
	RemoveAllLightMaps();
}

 

//goes through all world models, and removes all unused geometry
//associated with the world model. This will also update the bounding
//boxes of the world models if bUpdateBoundingBoxes is true;
void CPreMainWorld::RemoveAllUnusedGeometry(bool bUpdateBoundingBoxes)
{

	for(uint32 nCurrWorld = 0; nCurrWorld < m_WorldModels.GetSize(); nCurrWorld++)
	{
		CPreWorld* pModel = m_WorldModels[nCurrWorld];

		//sanity check
		ASSERT(pModel);

		//first off, remove all the unused geometry
		pModel->RemoveUnusedGeometry();

		//now see if the user wants to update the bounding box
		if(bUpdateBoundingBoxes)
		{
			pModel->UpdateBoundingBox();
		}
	}

	CollectBlockerPolys();
	CollectParticleBlockerPolys();
}


//updates the bounding box for the main world model. This assumes that all the 
//preworlds have up to date bounding boxes.
void CPreMainWorld::UpdateBoundingBox()
{
	//the count of how many main world model types we have encountered
	uint32 nNumMainWorldModels = 0;

	//initialize the extents to some outragous value
	m_PosMin.Init(100000.0f, 100000.0f, 100000.0f);
	m_PosMax.Init(-100000.0f, -100000.0f, -100000.0f);

	//run through the worlds and get the extents
	for(uint32 nCurrWorld = 0; nCurrWorld < m_WorldModels.GetSize(); nCurrWorld++)
	{
		CPreWorld* pModel = m_WorldModels[nCurrWorld];

		// Only include the main worlds in our bounding box.
		if(pModel->m_WorldInfoFlags & (WIF_MAINWORLD | WIF_PHYSICSBSP))
		{
			VEC_MIN(m_PosMin, m_PosMin, pModel->m_PosMin);
			VEC_MAX(m_PosMax, m_PosMax, pModel->m_PosMax);
			nNumMainWorldModels++;
		}
	}
	
	// Some worlds don't have main world brushes so dims have to be 0.
	if(nNumMainWorldModels == 0)
	{
		m_PosMin.Init();
		m_PosMax.Init();
	}
}



CPrePoly* CPreMainWorld::GetLMPoly(const CPrePolyRef *pRef)
{
	CPreWorld *pWorld;

	if(pRef->m_iWorld >= m_WorldModels.GetSize())
		return NULL;

	pWorld = m_WorldModels[pRef->m_iWorld];
	if(pRef->m_iPoly >= pWorld->m_Polies.GetSize())
		return NULL;

	return pWorld->FindPolyByIndex(pRef->m_iPoly);
}

const CPrePoly* CPreMainWorld::GetLMPoly(const CPrePolyRef *pRef) const
{
	const CPreWorld *pWorld;

	if(pRef->m_iWorld >= m_WorldModels.GetSize())
		return NULL;

	pWorld = m_WorldModels[pRef->m_iWorld];
	if(pRef->m_iPoly >= pWorld->m_Polies.GetSize())
		return NULL;

	// Note : FindPolyByIndex should probably have a const version, but it updates
	// the poly indices if the poly map isn't initialized.
	return (const_cast<CPreWorld*>(pWorld))->FindPolyByIndex(pRef->m_iPoly);
}

CPreWorld* CPreMainWorld::FindWorldModel(const char *pName)
{
	uint32 i;
	CPreWorld *pModel;

	for(i=0; i < m_WorldModels; i++)
	{
		pModel = m_WorldModels[i];

		if(stricmp(pModel->m_WorldName, pName) == 0)
			return pModel;
	}

	return NULL;
}

void CPreMainWorld::MinimizeSurfaceTCoords()
{
	uint32 i;

	for(i=0; i < m_WorldModels.GetSize(); i++)
	{
		m_WorldModels[i]->MinimizeSurfaceTCoords();
	}
}


uint32 CPreMainWorld::CalcLMDataSize()
{
	uint32 iAnim, count;

	count = 0;
	for(iAnim=0; iAnim < m_LightAnims; iAnim++)
	{
		count += m_LightAnims[iAnim]->CalcFrameDataSize();
	}

	return count;
}


void CPreMainWorld::RemoveAllLightMaps()
{
	DeleteAndClearArray(m_LightAnims);
}

void CPreMainWorld::RemoveLightAnim(CPreLightAnim *pAnim)
{
	uint32 index;

	index = m_LightAnims.FindElement(pAnim);
	if(index < m_LightAnims.GetSize())
		m_LightAnims.Remove(index);

	delete pAnim;
}


CPreWorld* CPreMainWorld::GetPhysicsBSP()
{
	GenListPos pos;
	CPreWorld *pTest;

	for(pos=m_WorldModels.GenBegin(); m_WorldModels.GenIsValid(pos); )
	{
		pTest = m_WorldModels.GenGetNext(pos);

		if(pTest->m_WorldInfoFlags & WIF_PHYSICSBSP)
			return pTest;
	}

	ASSERT(FALSE); // Should always have one.
	return NULL;
}

const CPreWorld* CPreMainWorld::GetPhysicsBSP() const
{
	GenListPos pos;
	const CPreWorld *pTest;

	for(pos=m_WorldModels.GenBegin(); m_WorldModels.GenIsValid(pos); )
	{
		pTest = m_WorldModels.GenGetNext(pos);

		if(pTest->m_WorldInfoFlags & WIF_PHYSICSBSP)
			return pTest;
	}

	ASSERT(FALSE); // Should always have one.
	return NULL;
}

uint32 CPreMainWorld::GetWorldIndex(const CPreWorld *pWorld) const
{
	for (uint32 nWorldLoop = 0; nWorldLoop < m_WorldModels.GetSize(); ++nWorldLoop)
	{
		if (pWorld == m_WorldModels[nWorldLoop])
			return nWorldLoop;
	}

	ASSERT(!"WorldModel not found in list");
	return m_WorldModels.GetSize();
}

uint32 CPreMainWorld::CollectBlockerPolys()
{
	// Move all the blocker polys into the blocker poly list
	for (uint32 nCurWorld = 0; nCurWorld < m_WorldModels.GetSize(); ++nCurWorld)
	{
		CPreWorld *pWorld = m_WorldModels[nCurWorld];
		GPOS pos = pWorld->m_Polies;

		uint32 nNumBlockersFound = 0;
		while (pos)
		{
			GPOS removePos = pos;
			CPrePoly *pPoly = pWorld->m_Polies.GetNext(pos);
			if ((pPoly->GetSurfaceFlags() & SURF_PHYSICSBLOCKER) == 0)
				continue;

			//we now need to convert this polygon to the a blocker polygon
			CPreBlockerPoly* pBlocker = CreatePoly(CPreBlockerPoly, pPoly->NumVerts(), true);

			//check the allocation
			if(pBlocker)
			{
				for(uint32 nCurrVert = 0; nCurrVert < pPoly->NumVerts(); nCurrVert++)
				{
					pBlocker->Vert(nCurrVert) = pPoly->Vert(nCurrVert);
				}
				pBlocker->GetPlane() = *pPoly->GetPlane();
				m_BlockerPolys.Append(pBlocker);
			}

			//clean up the old polygon
			pWorld->m_Polies.RemoveAt(removePos);
			DeletePoly(pPoly);

			nNumBlockersFound++;
		}

		//we should only find blockers in the main world though, if it is under anything but the physics
		//BSP, it is a content error
		if(!(pWorld->m_WorldInfoFlags & WIF_PHYSICSBSP) && (nNumBlockersFound > 0))
		{
			DrawStatusText(eST_Error, "Found %d physics blockers under world model %s. Blockers under world models will cause issues with the world model bounding box. Please remove these blockers.", nNumBlockersFound, pWorld->m_WorldName);
		}
	}

	return m_BlockerPolys.GetSize();
}

// build up the list of particle blocker polygons
uint32 CPreMainWorld::CollectParticleBlockerPolys( void )
{
	// move all the particle blocker polys into the particle blocker poly list
	for( uint32 nCurWorld = 0; nCurWorld < m_WorldModels.GetSize(); ++nCurWorld )
	{
		CPreWorld* pWorld = m_WorldModels[nCurWorld];
		GPOS pos = pWorld->m_Polies;
		while( pos )
		{
			GPOS removePos = pos;
			CPrePoly* pPoly = pWorld->m_Polies.GetNext( pos );
			if( !(pPoly->GetSurfaceFlags() & SURF_PARTICLEBLOCKER) )
				continue;

			CPreBlockerPoly* pBlocker = CreatePoly( CPreBlockerPoly, pPoly->NumVerts(), true );

			if( pBlocker )
			{
				for( uint32 nCurVert = 0; nCurVert < pPoly->NumVerts(); nCurVert++ )
				{
					pBlocker->Vert( nCurVert ) = pPoly->Vert( nCurVert );
				}
				pBlocker->GetPlane() = *pPoly->GetPlane();
				m_ParticleBlockerPolys.Append( pBlocker );
			}

			// clean up the old polygon
			pWorld->m_Polies.RemoveAt( removePos );
			DeletePoly( pPoly );
		}
	}

	return m_ParticleBlockerPolys.GetSize();
}


// ----------------------------------------------------------------------- //
//      ROUTINE:        Constructor
//      PURPOSE:        
// ----------------------------------------------------------------------- //

CPreWorld::CPreWorld()
{
	Construct(NULL);
}


CPreWorld::CPreWorld(CPreMainWorld *pMainWorld)
{
	Construct(pMainWorld);
}


void CPreWorld::Construct(CPreMainWorld *pMainWorld)
{
	m_pMainWorld = pMainWorld;
	m_WorldName[0] = 0;
	m_RootNode = NODE_OUT;
	m_StringHolder.SetAllocSize( 600 );

	m_RootNode = NODE_OUT;
	m_WorldTranslation.Init();

	m_PosMin.Init();
	m_PosMax.Init();

	m_WorldInfoFlags = 0;
}



// ----------------------------------------------------------------------- //
//      ROUTINE:        Destructor
//      PURPOSE:        
// ----------------------------------------------------------------------- //

CPreWorld::~CPreWorld()
{
	Term();
}



// ----------------------------------------------------------------------- //
//      ROUTINE:        CPreWorld::Term
//      PURPOSE:        Clears out any data in the model.
// ----------------------------------------------------------------------- //

void CPreWorld::Term()
{
	m_WorldName[0] = 0;
	TermGeometry();

	m_StringHolder.ClearStrings();
	m_RootNode = NODE_OUT;
}


void CPreWorld::TermGeometry()
{
	GPOS pos;

	m_RootNode = NODE_OUT;

	GDeleteAndRemoveElements(m_Nodes);

	for(pos=m_Polies; pos; )
		DeletePoly(m_Polies.GetNext(pos));

	for(pos=m_OriginalBrushPolies; pos; )
		DeletePoly(m_OriginalBrushPolies.GetNext(pos));

	m_Polies.RemoveAll();
	m_OriginalBrushPolies.RemoveAll();

	GDeleteAndRemoveElements( m_Planes );
	GDeleteAndRemoveElements( m_Surfaces );

	m_Points.SetSize(0);
	m_TextureNames.Term();
	m_WorldTranslation.Init(0, 0, 0);
}


PVertex& CPreWorld::PolyPt(CPrePoly *pPoly, uint32 i)
{
	return m_Points[pPoly->Index(i)];
}


void CPreWorld::InitPolyVertexIndices()
{
	priority_queue<PointElement> theQueue;
	GPOS pos;
	uint32 i, nPointsInQueue, nPointsProcessed;
	CPrePoly *pPoly;
	PointElement testElement;
	bool bAddToStack;
	PointStackEl *pNewStackEl, *pStackEl;

	list<PointStackEl*> theStack;
	list<PointStackEl*>::iterator iStack;
	PReal tolerance, diff, dist;



	tolerance = (PReal)0.0001;
	
	m_Points.Term();
	m_Points.SetCacheSize(1000);


	// Sort all the poly points into the queue.
	nPointsInQueue = 0;
	for(pos=m_Polies; pos; )
	{
		pPoly = m_Polies.GetNext(pos);

		for(i=0; i < pPoly->NumVerts(); i++)
		{
			pPoly->Index(i) = 0xFFFFFFFF;
			theQueue.push(PointElement(&pPoly->Pt(i), (uint32 *)&pPoly->Index(i)));
			++nPointsInQueue;
		}
	}

	// Go over the sorted list, adding necessary ones.
	nPointsProcessed = 0;
	while(!theQueue.empty())
	{
		testElement = theQueue.top();
		
		bAddToStack = true;
		for(iStack=theStack.begin(); iStack != theStack.end(); iStack++)
		{
			pStackEl = *iStack;

			diff = testElement.m_Mag - pStackEl->m_Mag;
			if(diff > tolerance)
			{
				// We've passed out of range of this stack element.. add it to the world.
				pStackEl->AddToWorld(this);
				iStack = theStack.erase(iStack);
				delete pStackEl;

				// Restart the loop.
				--iStack;
			}
			else
			{
				// Ok, they're close enough on X, see if their real distance is close enough.
				dist = (*testElement.m_pPoint - *pStackEl->m_pPoint).Mag();
				if(dist < tolerance)
				{
					// They are close enough, so add the testElement's index to the stackEl.
					pStackEl->m_Attachments.push_back(testElement.m_pIndex);
					bAddToStack = false;
					break;
				}
			}
		}

		if(bAddToStack)
		{
			pNewStackEl = new PointStackEl;
			pNewStackEl->m_Mag = testElement.m_Mag;
			pNewStackEl->m_pPoint = testElement.m_pPoint;
			pNewStackEl->m_Attachments.push_back(testElement.m_pIndex);
			theStack.push_back(pNewStackEl);
		}

		theQueue.pop();
		++nPointsProcessed;
	}


	// Add the dangling stack elements to the world.
	for(iStack=theStack.begin(); iStack != theStack.end(); iStack++)
	{
		(*iStack)->AddToWorld(this);
		delete (*iStack);
	}
}

void CPreWorld::RemoveUnusedGeometry()
{
	RemoveUnusedSurfaces();
	RemoveUnusedPlanes();
	RemoveUnusedPoints();
}


void CPreWorld::RemoveUnusedSurfaces()
{
	GPOS pos;
	CPreSurface *pSurface;

	BuildSurfacePolyLists(m_Surfaces, m_Polies);

	for(pos=m_Surfaces; pos; )
	{
		pSurface = m_Surfaces.GetNext(pos);
		
		if(pSurface->m_PolyList.GetSize() == 0)
		{
			m_Surfaces.RemoveAt(pSurface);
			delete pSurface;
		}
	}

	TermSurfacePolyLists(m_Surfaces);
}


void CPreWorld::RemoveUnusedPlanes()
{
	GPOS pos, oldPos, surfacePos;
	CPrePlane *pPlane;


	// First clear the high bit on all the planes.
	for(pos=m_Planes; pos; )
		m_Planes.GetNext(pos)->ClearHighBit();


	// Now mark the used planes.
	for(surfacePos=m_Surfaces; surfacePos; )
	{
		m_Surfaces.GetNext(surfacePos)->m_pPlane->SetHighBit();
	}


	// Remove unmarked ones.
	for(pos=m_Planes; pos; )
	{
		oldPos = pos;
		pPlane = m_Planes.GetNext(pos);

		if(!pPlane->GetHighBit())
		{
			m_Planes.RemoveAt(oldPos);
			delete pPlane;
		}
	}
}


bool CPreWorld::RemoveUnusedPoints()
{
	GPOS pos;
	PVertexArray newPoints;
	CMoArray<uint32> indexRename;
	uint32 i, k, iVert;
	CPrePoly *pPoly;

	newPoints.SetCacheSize(1024);

	if(!indexRename.SetSize(m_Points.GetSize()))
		return false;

	for(i=0; i < indexRename; i++)
	{
		indexRename[i] = i;
	}

	// Mark which points are used.
	for(pos=m_Polies; pos; )
	{
		pPoly = m_Polies.GetNext(pos);

		for( k=0; k < pPoly->NumVerts(); k++ )
		{
			iVert = pPoly->Index(k);
			if(indexRename[iVert] == iVert)
			{
				// Ok, make a new output vert.
				newPoints.Append(m_Points[iVert]);
				indexRename[iVert] = newPoints.LastI();
			}
		}
	}

	// Replace the array and remap indices.
	m_Points.CopyArray(newPoints);
	
	for(pos=m_Polies; pos; )
	{
		pPoly = m_Polies.GetNext(pos);

		for( k=0; k < pPoly->NumVerts(); k++ )
		{
			pPoly->Index(k) = indexRename[pPoly->Index(k)];
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
// Tags the surfaces that are actually being used.
// Returns the number of surfaces being used.
// ----------------------------------------------------------------------- //

uint32 CPreWorld::TagUsedSurfaces()
{
	GPOS		pos;
	uint32		nUsed;
	CPreSurface	*pSurface;

	for(pos=m_Surfaces.GetHeadPosition(); pos; )
		m_Surfaces.GetNext(pos)->m_bUsed = FALSE;

	for(pos=m_Polies; pos; )
		m_Polies.GetNext(pos)->GetSurface()->m_bUsed = TRUE;

	nUsed = 0;
	for(pos=m_Surfaces.GetHeadPosition(); pos; )
	{
		pSurface = m_Surfaces.GetNext(pos);

		pSurface->m_UseIndex = nUsed;		
		nUsed += pSurface->m_bUsed;
	}

	return nUsed;
}



// ----------------------------------------------------------------------- //
//      ROUTINE:        CPreWorld::SetupIndexMap
//      PURPOSE:        Helps optimize the saving of BSP files by marking
//                      which points are used.
// ----------------------------------------------------------------------- //

void CPreWorld::SetupIndexMap( CMoByteArray &pointsUsed, CMoDWordArray &indexMap, uint32 &nPointsUsed )
{
	uint32 i, k, subtract;
	CPrePoly *pPoly;
	GPOS pos;

	
	// Init the structures.
	pointsUsed.SetSize( m_Points.GetSize() );
	indexMap.SetSize( m_Points.GetSize() );
	
	memset( pointsUsed.GetArray(), 0, m_Points.GetSize() );

	for( i=0; i < indexMap; i++ )
		indexMap[i] = i;


	// Mark which points are used.
	for(pos=m_Polies; pos; )
	{
		pPoly = m_Polies.GetNext(pos);

		for( k=0; k < pPoly->NumVerts(); k++ )
			pointsUsed[pPoly->Index(k)] = TRUE;
	}

	
	// Decrement index map stuff.
	subtract = 0;
	nPointsUsed = 0;

	for( i=0; i < pointsUsed; i++ )
	{
		if( pointsUsed[i] )
			++nPointsUsed;
		else
			subtract++;

		indexMap[i] -= subtract;
	}
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CPreWorld::FindOrAddPlane
//      PURPOSE:        Looks for the plane and if it doesn't exist, adds it.
// ----------------------------------------------------------------------- //

CPrePlane* CPreWorld::FindOrAddPlane(PVector normal, PReal dist)
{
	CPrePlane *pTestPlane;
	GPOS pos;


	for( pos=m_Planes.GetHeadPosition(); pos; )
	{
		pTestPlane = m_Planes.GetNext( pos );
		
		if( pTestPlane->m_Normal.x == normal.x &&
			pTestPlane->m_Normal.y == normal.y &&
			pTestPlane->m_Normal.z == normal.z &&
			pTestPlane->m_Dist == dist)
		{
			return pTestPlane;
		}
	}

	// Couldn't find it .. create a new one.
	return AddPlane(normal, dist);
}


CPrePlane* CPreWorld::AddPlane(PVector &normal, PReal dist)
{
	CPrePlane *pPlane;

	pPlane = new CPrePlane;
	pPlane->m_Normal = normal;
	pPlane->m_Dist = dist;
	m_Planes.AddTail( pPlane );	

	return pPlane;
}


void CPreWorld::SetPolyIndices()
{
	uint32 i;
	GPOS pos;

	i = 0;
	for(pos=m_Polies; pos; )
	{
		m_Polies.GetNext(pos)->m_Index = i;
		++i;
	}
}


void CPreWorld::SetPlaneIndices()
{
	uint32 i;
	GPOS pos;

	// Give each plane its index.
	i = 0;
	for(pos=m_Planes; pos; )
	{
		m_Planes.GetNext(pos)->SetIndex(i);
		i++;
	}				  	
}

void CPreWorld::GetTextureNames( CMoArray<const char*> &texNames )
{
	uint32			i;
	bool			bFound;
	GPOS			pos;
	CPreSurface		*pSurf;


	// Make the list of texture names.
	texNames.SetSize(0);
	for( pos=m_Surfaces.GetHeadPosition(); pos; )
	{
		pSurf = m_Surfaces.GetNext( pos );

		for(uint32 nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
		{
			bFound = false;
			for( i=0; i < texNames; i++ )
			{
				if( texNames[i] == pSurf->m_Texture[nCurrTex].m_pTextureName )
				{
					bFound = true;
					break;
				}
			}

			if( !bFound )
				texNames.Append( pSurf->m_Texture[nCurrTex].m_pTextureName );
		}
	}
}


WORD CPreWorld::FindTextureName( const char *pName, CMoArray<const char*> &texNames )
{
	if( !pName )
		return (WORD)-1;

	for( uint32 i=0; i < texNames; i++ )
		if( texNames[i] == pName )
			return (WORD)i;

	return (WORD)-1;
}


void SetupSurface(CPreSurface *pSurface)
{
	// Make sure TextureP and TextureQ are valid.
	// NOTE: this probably isn't necessary since geometry is never affected
	// by the texture plane.
	for(uint32 nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
	{
		CPreTexture& Tex = pSurface->m_Texture[nCurrTex];

		if(Tex.m_TextureP.Mag() < 0.0001f || Tex.m_TextureP.Mag() > 100000.0f || 
			Tex.m_TextureQ.Mag() < 0.0001f || Tex.m_TextureQ.Mag() > 100000.0f)
		{
			GetPerpendicularVector(&pSurface->m_pPlane->m_Normal, NULL, &Tex.m_TextureP);
			Tex.m_TextureP.Norm();
			Tex.m_TextureQ = Tex.m_TextureP.Cross(pSurface->m_pPlane->m_Normal);
		}
		
		// Setup the inverses..
		Tex.m_InverseTextureP = Tex.m_TextureP / Tex.m_TextureP.MagSqr();
		Tex.m_InverseTextureQ = Tex.m_TextureQ / Tex.m_TextureQ.MagSqr();
	}
}


// Adjusts pSurface->O so the texture coordinates on the specified poly have the
// smallest possible value.
void MinimizeTCoordValues(CPreSurface *pSurface, CPrePoly *pPoly)
{
	uint32 iUV, iPt;
	int nBlockPositions, iBlock, iBestBlock;
	PReal fMin, fMax, fCoord, fSum, fBestSum;
	PVector *pTexVec, *pInvTexVec, vSurfaceO;


	for(uint32 nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
	{
		// Minimize U and V separately.
		for(iUV=0; iUV < 2; iUV++)
		{
			if(iUV == 0)
			{
				pTexVec		= &pSurface->m_Texture[nCurrTex].m_TextureP;
				pInvTexVec	= &pSurface->m_Texture[nCurrTex].m_InverseTextureP;
			}
			else
			{
				pTexVec		= &pSurface->m_Texture[nCurrTex].m_TextureQ;
				pInvTexVec	= &pSurface->m_Texture[nCurrTex].m_InverseTextureQ;
			}

			// Calculate the min and max of the texture coordinate values.
			fMin = 100000.0f;
			fMax = -100000.0f;
			for(iPt=0; iPt < pPoly->NumVerts(); iPt++)
			{
				fCoord = pTexVec->Dot(pPoly->Pt(iPt) - pSurface->m_Texture[nCurrTex].m_TextureO);
				fMin = DMIN(fMin, fCoord);
				fMax = DMAX(fMax, fCoord);
			}

			// Determine how many block positions we should test (using MAX_DTX_SIZE).
			nBlockPositions = (int)((PReal)DMAX(fabs(fMax), fabs(fMin)) / MAX_DTX_SIZE + 0.5f);
			
			// Make sure it's not insane..
			nBlockPositions = DCLAMP(nBlockPositions, 0, 500);

			// Test all the blocks to see which is best.
			iBestBlock = 0;
			for(iBlock=-nBlockPositions; iBlock <= nBlockPositions; iBlock++)
			{
				vSurfaceO = pSurface->m_Texture[nCurrTex].m_TextureO + *pInvTexVec * (PReal)(iBlock * MAX_DTX_SIZE);

				// See how far away the numbers are.
				fSum = 0.0f;
				for(iPt=0; iPt < pPoly->NumVerts(); iPt++)
				{
					fSum += (PReal)fabs(pTexVec->Dot(pPoly->Pt(iPt) - vSurfaceO));
				}

				if(iBlock == -nBlockPositions || fSum < fBestSum)
				{
					iBestBlock = iBlock;
					fBestSum = fSum;
				}
			}

			// Adjust by the specified amount.
			if(iBestBlock != 0)
			{
				pSurface->m_Texture[nCurrTex].m_TextureO += *pInvTexVec * (PReal)(iBestBlock * MAX_DTX_SIZE);
			}
		}
	}
}


// This function takes the surfaces, which should have their TextureP and TextureQ set,
// and sets up the P and Q (lightmap) coordinate system.  It also sets up the inverse
// texture vectors for everything.
void CPreWorld::SetupSurfaceTextureVectors()
{
	GPOS pos;
	CPreSurface *pSurface;


	for(pos=m_Surfaces; pos; )
	{
		pSurface = m_Surfaces.GetNext(pos);
		
		SetupSurface(pSurface);
	}

	SetupSurfaceLMVectors();
}


void CPreWorld::SetupSurfaceLMVectors()
{
	GPOS pos;
	CPreSurface *pSurface;


	for(pos=m_Surfaces; pos; )
	{
		pSurface = m_Surfaces.GetNext(pos);
		
		pSurface->SetupLMVectors();
	}
}


void CPreWorld::MinimizeSurfaceTCoords()
{
	GPOS pos;
	CPreSurface *pSurface;


	BuildSurfacePolyLists(m_Surfaces, m_Polies);

	for(pos=m_Surfaces; pos; )
	{
		pSurface = m_Surfaces.GetNext(pos);
		
		// Certain hardware accelerators don't like large numbers so we seek to
		// minimize the texture coordinate values here.
		if(pSurface->m_PolyList.GetSize() > 0)
		{
			MinimizeTCoordValues(pSurface, pSurface->m_PolyList.GetHead());
		}
	}

	TermSurfacePolyLists(m_Surfaces);
}


void CPreWorld::GetBoundingBox(PVector *pMin, PVector *pMax)
{
	GPOS pos;
	CPrePoly *pPoly;
	PVector vec;
	uint32 i;

	pMin->Init((PReal)MAX_CREAL, (PReal)MAX_CREAL, (PReal)MAX_CREAL);
	*pMax = -(*pMin);
	
	for(pos=m_Polies; pos; )
	{
		pPoly = m_Polies.GetNext(pos);

		for(i=0; i < pPoly->NumVerts(); i++)
		{
			vec = pPoly->Pt(i);
			
			if(vec.x < pMin->x) pMin->x = vec.x;
			if(vec.y < pMin->y) pMin->y = vec.y;
			if(vec.z < pMin->z) pMin->z = vec.z;

			if(vec.x > pMax->x) pMax->x = vec.x;
			if(vec.y > pMax->y) pMax->y = vec.y;
			if(vec.z > pMax->z) pMax->z = vec.z;
		}
	}
}


void CPreWorld::UpdateBoundingBox()
{
	GetBoundingBox(&m_PosMin, &m_PosMax);
}


int CPreWorld::GetTreeDepth()
{
	g_pRecursionWorld = this;
	g_MaxRecursionDepth = 0;
	RecurseAndGetTreeDepth(m_RootNode, 1);
	return g_MaxRecursionDepth;
}


void CPreWorld::RemoveAllNodes()
{
	GDeleteAndRemoveElements(m_Nodes);
}

// ----------------------------------------------------------------------- //
//      ROUTINE:        CEditBrush::JoinPolies
//      PURPOSE:        Joins as many polies in the current plane as possible.
// ----------------------------------------------------------------------- //

static void AddToConnectivity(CPrePoly *pPoly, 
	CMoArray<PrePolyArray> &pointConnectivity)
{
	uint32 i;

	for(i=0; i < pPoly->NumVerts(); i++)
	{
		pointConnectivity[pPoly->Index(i)].Append(pPoly);
	}
}


CPrePoly* FindPolyWithEdge2(CMoArray<CPrePoly*> &polies, 
	CPrePoly *pReject,
	CPreSurface *pSurface,
	uint32 iPt1,
	uint32 iPt2,
	WORD &pt1Index, 
	WORD &pt2Index)
{
	uint32 i, iPoly, nextI;
	CPrePoly *pPoly;


	for(iPoly=0; iPoly < polies; iPoly++)
	{
		pPoly = polies[iPoly];

		if(pPoly->GetPostRemove() || pPoly == pReject || pPoly->m_pSurface != pSurface)
			continue;

		for(i=0; i < pPoly->NumVerts(); i++)
		{
			nextI = (i+1) % pPoly->NumVerts();

			if( (pPoly->Index(i)==iPt2) && (pPoly->Index(nextI)==iPt1) )
			{
				pt1Index = (WORD)nextI;
				pt2Index = (WORD)i;
				return pPoly;
			}
			else if( (pPoly->Index(nextI)==iPt2) && (pPoly->Index(i)==iPt1) )
			{
				pt1Index = (WORD)i;
				pt2Index = (WORD)nextI;
				return pPoly;
			}
		}
	}

	return NULL;
}


void CPreWorld::JoinPolyList(CGLinkedList<CPrePoly*> &polies, PReal convexThreshold)
{
	uint32 edge, nRemoved;
	CPrePoly *pPoly, *pOther;
	CPrePoly *pJoined;
	CMoArray<PrePolyArray> pointConnectivity;
	bool bAnyJoined;
	WORD otherIndex1, otherIndex2;
	GPOS pos;

	
	nRemoved = 0;
	pointConnectivity.SetSize(m_Points.GetSize());


	// Mark them all as non-removed.
	// Add them to the point connectivity thing.
	for(pos=polies; pos; )
	{
		pPoly = polies.GetNext(pos);

		pPoly->SetPostRemove(FALSE);
		AddToConnectivity(pPoly, pointConnectivity);
	}
	

	do
	{
		bAnyJoined = false;

		for(pos=polies; pos; )
		{
			pPoly = polies.GetNext(pos);
			
			if(pPoly->GetPostRemove())
				continue;

			// For each edge, try to find the same edge on another polygon.
			for(edge=0; edge < pPoly->NumVerts(); edge++)
			{
				if(
					(pOther = FindPolyWithEdge2(pointConnectivity[pPoly->Index(edge)], pPoly, pPoly->m_pSurface, 
						pPoly->Index(edge), pPoly->NextIndex(edge), otherIndex1, otherIndex2)) ||
					
					(pOther = FindPolyWithEdge2(pointConnectivity[pPoly->NextIndex(edge)], pPoly, pPoly->m_pSurface, 
						pPoly->Index(edge), pPoly->NextIndex(edge), otherIndex1, otherIndex2))
				)
				{
					if((pPoly->NumVerts() + pOther->NumVerts() - 2) < MAX_WORLDPOLY_VERTS)
					{
						// Ok, join them.
						pJoined = JoinPolygons(pPoly, pOther, edge, otherIndex1, 
							(edge+1)%pPoly->NumVerts(), otherIndex2);

						if(pJoined)
						{
							if(pJoined->PostJoinFixup(convexThreshold))
							{
								ASSERT(!pPoly->GetPostRemove() && !pOther->GetPostRemove());
								
								pPoly->SetPostRemove(TRUE);
								pOther->SetPostRemove(TRUE);
								++nRemoved;
								polies.Append(pJoined);
								AddToConnectivity(pJoined, pointConnectivity);

								// Start the loops over.
								bAnyJoined = true;
								break;
							}
							else
							{
								DeletePoly(pJoined);
							}
						}
					}
				}
			}
		}
	} while(bAnyJoined);

	// Remove all the polies that got marked.
	for(pos=polies; pos; )
	{
		pPoly = polies.GetNext(pos);

		if(pPoly->GetPostRemove())
		{
			polies.RemoveAt(pPoly);
			DeletePoly(pPoly);
		}
	}
}




// ----------------------------------------------------------------------- //
//      ROUTINE:        CPreWorld::JoinPolygons
//      PURPOSE:        Joins the two polygons, leaving the originals intact.
// ----------------------------------------------------------------------- //

CPrePoly* CPreWorld::JoinPolygons(CPrePoly *pPoly1, CPrePoly *pPoly2, 
	WORD poly1v1, WORD poly2v1, WORD poly1v2, WORD poly2v2)
{
	CPrePoly *pNew;
	uint32 i, i1, i2, nNewPolyVerts, end;


	i1 = poly2v1;
	i2 = poly2v2;

	// Make sure they came in the right order.
//	ASSERT(i2 != ((i1+1)%pPoly2->NumVerts()));
	if(i2 == ((i1+1)%pPoly2->NumVerts()))
	{
		return NULL;
	}

	
	// Join the two.
	nNewPolyVerts = (pPoly1->NumVerts() + pPoly2->NumVerts()) - 2;
	pNew = CreatePoly(CPrePoly, nNewPolyVerts, false);
	
	pNew->m_pSurface = pPoly1->m_pSurface;
	pNew->m_pOriginalBrushPoly = pPoly1->m_pOriginalBrushPoly;
	
	// Add poly1's vertices, except for the joined two.all of poly1's vertices.
	i = (poly1v2+1) % pPoly1->NumVerts();
	do
	{
		pNew->AddVert(pPoly1->Pt(i));
		pNew->Alpha(pNew->NumVerts()-1) = pPoly1->Alpha(i);
		pNew->Index(pNew->NumVerts()-1) = pPoly1->Index(i);
		i = (i+1) % pPoly1->NumVerts();
	} while(i != poly1v1);
	

	// Add all of poly2's vertices.
	i = end = poly2v1;
	do
	{
		pNew->AddVert(pPoly2->Pt(i));
		pNew->Alpha(pNew->NumVerts()-1) = pPoly2->Alpha(i);
		pNew->Index(pNew->NumVerts()-1) = pPoly2->Index(i);
		i = (i+1) % pPoly2->NumVerts();
	} while(i != end);

	ASSERT(pNew->NumVerts() == nNewPolyVerts);
	return pNew;
}




// ----------------------------------------------------------------------- //
//
//      ROUTINE:        CPreWorld counting functions
//
// ----------------------------------------------------------------------- //

uint32 CPreWorld::NumPolyVerts()
{
	uint32 num=0;
	GPOS pos;

	for(pos=m_Polies; pos; )
		num += m_Polies.GetNext(pos)->NumVerts();

	return num;
}

void CPreWorld::FindTextureOrigins()
{
	GPOS pos;
	CPrePoly *pPoly;

	// Make polygon origins.
	for(pos=m_Polies; pos; )
	{
		pPoly = m_Polies.GetNext(pos);
		pPoly->FindTextureOrigin(NULL, TRUE, (float)GetLMGridSize(pPoly));
	}
}

void CPreWorld::SetPolyWorldIndices()
{
	GPOS pos;
	uint32 myIndex;

	myIndex = m_pMainWorld->m_WorldModels.FindElement(this);
	if(myIndex >= m_pMainWorld->m_WorldModels.GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	m_PolyArray.SetSize(m_Polies.GetSize());

	uint32 nPolyIndex = 0;
	for(pos=m_Polies; pos; ++nPolyIndex)
	{
		CPrePoly *pPoly = m_Polies.GetNext(pos);
		pPoly->m_WorldIndex = myIndex;
		m_PolyArray[nPolyIndex] = pPoly;
	}
}

CPrePoly *CPreWorld::FindPolyByIndex(uint32 nIndex)
{
	// Quick check to make sure that our poly array has even been set up at all
	// Note that this still won't detect the situation where polys get added and removed
	// outside of the normal processing sequence
	if (m_PolyArray.GetSize() != m_Polies.GetSize())
	{
		SetPolyWorldIndices();
		ASSERT(m_PolyArray.GetSize() == m_Polies.GetSize());
	}
	
	return m_PolyArray[nIndex];
}
