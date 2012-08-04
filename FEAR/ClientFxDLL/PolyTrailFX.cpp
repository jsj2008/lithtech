#include "stdafx.h"
#include "PolyTrailFX.h"
#include "ClientFX.h"
#include "ClientFXVertexDeclMgr.h"
#include "MemoryPageMgr.h"
#include "iperformancemonitor.h"
#include "GameRenderLayers.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXPolyTrail("ClientFX_PolyTrail", "ClientFX");

//------------------------------------------------------------------
// Trail Memory Management
//------------------------------------------------------------------

//the size of the memory pages that will be allocated. Particles must not be larger than this
//size
#define POLYTRAIL_PAGE_SIZE		(8 * 1024)		//8k

//global particle page manager. This handles all of the memory allocation for the particles
static CMemoryPageMgr	g_PolyTrailPageMgr(POLYTRAIL_PAGE_SIZE);


//------------------------------------------------------------------
// CPolyTrailFX
//------------------------------------------------------------------

CPolyTrailFX::CPolyTrailFX() :	
	CBaseFX(CBaseFX::ePolyTrailFX),
	m_nNumSamples(0),
	m_fSampleElapsed(0.0f),
	m_fTotalDistance(0.0f),
	m_pHeadSample(NULL),
	m_pFreeListAlloc(NULL),
	m_pFreeListFree(NULL),
	m_pPageHead(NULL),
	m_nNumTrackedNodes(0),
	m_nTrailSamplePitch(0),
	m_fAccumulatedDist(0.0f),
	m_fTotalElapsed(0.0f),
	m_hObject(NULL)
{
}

CPolyTrailFX::~CPolyTrailFX()
{
	Term();
}

bool CPolyTrailFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation
	if (!CBaseFX::Init(pBaseData, pProps)) 
		return false;

	LTVector vCreatePos;
	LTRotation rCreateRot;
	GetCurrentTransform(0.0f, vCreatePos, rCreateRot);

	ObjectCreateStruct ocs;
	ocs.m_ObjectType	= OT_CUSTOMRENDER;
	ocs.m_Flags			|= FLAG_VISIBLE;

	if(!GetProps()->m_bRenderSolid)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;
	if(!GetProps()->m_bTranslucentLight)
		ocs.m_Flags |= FLAG_NOLIGHT;

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(GetProps()->m_eInSky);

	ocs.m_Pos		= vCreatePos;
	ocs.m_Rotation	= rCreateRot;

	m_hObject = g_pLTClient->CreateObject(&ocs);
	if( !m_hObject )
		return false;

	//setup our rendering layer
	if(GetProps()->m_bPlayerView)
		g_pLTClient->GetRenderer()->SetObjectDepthBiasTableIndex(m_hObject, eRenderLayer_Player);

	//setup our custom render object
	//setup the callback on the object so that it will render us
	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_World);
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_hObject, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_hObject, this);

	//load up the material for this particle system, and assign it to the object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszMaterial);
	g_pLTClient->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//reset our elapsed emission
	m_fSampleElapsed = 0.0f;

	//setup our tracked nodes
	InitTrackedNodes();

	//success
	return true;
}

void CPolyTrailFX::Term()
{
	//free our memory pages
	CMemoryPage* pCurrPage = m_pPageHead;
	while(pCurrPage)
	{
		CMemoryPage* pNextPage = pCurrPage->m_pNextPage;
		g_PolyTrailPageMgr.FreePage(pCurrPage);
		pCurrPage = pNextPage;
	}

	//clear out our pointers into those pages
	m_pPageHead			= NULL;
	m_pHeadSample		= NULL;
	m_pFreeListAlloc	= NULL;
	m_pFreeListFree		= NULL;
	m_pPageHead			= NULL;
	m_nNumSamples		= 0;

	if( m_hObject )
	{
		g_pLTClient->RemoveObject(m_hObject);
		m_hObject = NULL;
	}
}

bool CPolyTrailFX::IsVisibleWhileSuspended()	
{ 
	if(m_pHeadSample)
		return (m_pHeadSample->m_fLifetime < GetProps()->m_fSampleLifetimeS);

	return false;
}

bool CPolyTrailFX::IsFinishedShuttingDown()	
{ 
	return (m_nNumSamples < 2) || (m_pHeadSample->m_fLifetime >= GetProps()->m_fSampleLifetimeS); 
}


bool CPolyTrailFX::Update(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXPolyTrail);

	//allow the base class to update first
	BaseUpdate(tmFrameTime);

	//our base object position doesn't really matter since all of our samples are in world space
	//so don't waste performance by updating it

	//determine our current lifetime
	float fUnitLifetime = GetUnitLifetime();

	//accumulate this time into our elapsed time for samples
	m_fTotalElapsed  += tmFrameTime;
	m_fSampleElapsed += tmFrameTime;

	//now determine the positioning of our object
	LTRigidTransform tObjectTrans;
	GetCurrentTransform(fUnitLifetime, tObjectTrans.m_vPos, tObjectTrans.m_rRot);

	//we can skip out on the creation and updating of samples if we are shutting down
	if(!IsShuttingDown())
	{
		//if this is our initial frame or if we have elapsed our time period, we need to 
		//create a new sample to collect into
		if(IsInitialFrame() || (m_fSampleElapsed >= GetProps()->m_fSampleFrequencyS))
		{
			//add our current head distance onto our accumulated distance before we move
			//to a new head
			if(ShouldCreateNewSample())
			{
				m_fTotalDistance += CalcHeadSegmentDistance();

				//we need to create a new sample
				if(!AllocateHeadSample())
					return false;
			}

			//and wrap our time around
			if(GetProps()->m_fSampleFrequencyS == 0.0f)
				m_fSampleElapsed = 0.0f;
			else
				m_fSampleElapsed = fmodf(m_fSampleElapsed, GetProps()->m_fSampleFrequencyS);
		}

		//update our tail sample so that way we have up to date transforms
		UpdateCurrentSamplePositions(tObjectTrans.m_vPos);
	}

	//get the current positioning of this object
	LTVector vObjectPos;
	g_pLTClient->GetObjectPos(m_hObject, &vObjectPos);

	//now we need to handle updating our current samples so they expire appropriately
	UpdateSamples(tmFrameTime, vObjectPos);

	return true;
}

bool CPolyTrailFX::SuspendedUpdate(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXPolyTrail);

	if(!CBaseFX::SuspendedUpdate(tmFrameTime))
		return false;

	//determine our current lifetime
	float fUnitLifetime = GetUnitLifetime();

	//get the current positioning of this object
	LTVector vObjectPos;
	g_pLTClient->GetObjectPos(m_hObject, &vObjectPos);

	//now we need to handle updating our current samples so they expire appropriately
	UpdateSamples(tmFrameTime, vObjectPos);

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CPolyTrailFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hObject)
	{
		pfnObjectCB(this, m_hObject, pUserData);
	}
}

//called to examine the properties and setup our tracked node information for this effect
void CPolyTrailFX::InitTrackedNodes()
{
	//start out with one tracked node (our base object position)
	m_nNumTrackedNodes = 1;

	//we can't get nodes unless we are tracking an object
	if(GetParentObject())
	{
		//run through each node, and add it to our list if valid
		for(uint32 nCurrNode = 0; nCurrNode < CPolyTrailProps::knMaxTrackedNodes - 1; nCurrNode++)
		{
			//get the name of the node that we are trying to find
			const char* pszNodeName = GetProps()->m_TrackedNodes[nCurrNode].m_pszNodeName;

			//see if this is a valid node to track
			if(LTStrEmpty(pszNodeName))
				continue;

			//the structure that we will be filling in information into
			STrackedNode& TrackedNode = m_TrackedNodes[m_nNumTrackedNodes - 1];

			//this is not empty, so try and find it
			if(FindModelNodeRecurse(GetParentObject(), pszNodeName, TrackedNode.m_hObject, TrackedNode.m_hNode))
			{
				//we found a node!
				TrackedNode.m_bIsNode = true;
				m_nNumTrackedNodes++;
			}
			else
			{
				//we didn't find a model node, so see if it is a socket
				if(FindModelSocketRecurse(GetParentObject(), pszNodeName, TrackedNode.m_hObject, TrackedNode.m_hSocket))
				{
					//we found a socket!
					TrackedNode.m_bIsNode = false;
					m_nNumTrackedNodes++;
				}
			}
		}
	}

	//and now time to determine the pitch in bytes of our node structure
	m_nTrailSamplePitch = sizeof(CTrailSample) - sizeof(CTrailSample::SNodeSample) * (CPolyTrailProps::knMaxTrackedNodes - m_nNumTrackedNodes);
}

//called to update the current positions of the nodes and store them within the head of the
//sample list
void CPolyTrailFX::UpdateCurrentSamplePositions(const LTVector& vObjectPos)
{
	//we must have a tail to write into
	if(!m_pHeadSample)
		return;

	//write our base object position out as the first tracked node
	m_pHeadSample->m_Nodes[0].m_vPos = vObjectPos;

	//now fill in the rest of the samples
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumTrackedNodes - 1; nCurrNode++)
	{
		//get the transform
		LTTransform tTrans;
		if(m_TrackedNodes[nCurrNode].m_bIsNode)
		{
			g_pLTClient->GetModelLT()->GetNodeTransform(m_TrackedNodes[nCurrNode].m_hObject, m_TrackedNodes[nCurrNode].m_hNode, tTrans, true);
		}
		else
		{
			g_pLTClient->GetModelLT()->GetSocketTransform(m_TrackedNodes[nCurrNode].m_hObject, m_TrackedNodes[nCurrNode].m_hSocket, tTrans, true);
		}

		//make sure to apply our offset into the space
		m_pHeadSample->m_Nodes[nCurrNode + 1].m_vPos = tTrans * GetProps()->m_TrackedNodes[nCurrNode].m_vTrackedOffset;
	}

	//determine the total distance along the poly trail for this point
	float fSegmentDistance = m_fTotalDistance + CalcHeadSegmentDistance();

	//now map that into the V range
	m_pHeadSample->m_fVTexCoord = fSegmentDistance / GetProps()->m_fTextureLength;

	//and finally calculate the normals for this and the adjacent sample
	UpdateHeadSampleNormals();
}

//called to update the current samples that we have and also the visibility for this object
void CPolyTrailFX::UpdateSamples(float fElapsed, const LTVector& vObjectPos)
{
	LTVector vMin = vObjectPos;
	LTVector vMax = vObjectPos;
	bool bFirstPt = true;

	//start at the head and work our way to the tail
	CTrailSample* pCurrSample = m_pHeadSample;
	while(pCurrSample)
	{
		//update the extents of our bounding box
		for(uint32 nCurrNode = 0; nCurrNode < m_nNumTrackedNodes; nCurrNode++)
		{
			const LTVector& vPt = pCurrSample->m_Nodes[nCurrNode].m_vPos;

			if(bFirstPt)
			{
				vMin = vPt;
				vMax = vPt;
				bFirstPt = false;
			}
			else
			{
				vMin.Min(vPt);
				vMax.Max(vPt);
			}
		}

		//update our lifetime
		pCurrSample->m_fLifetime += fElapsed;

		//see if the lifetime of this sample has expired
		if((pCurrSample->m_fLifetime >= GetProps()->m_fSampleLifetimeS) && (pCurrSample != m_pHeadSample))
		{
			//free all of the nodes that follow this node
			FreeSamplesAfter(*pCurrSample);
			break;
		}

		//and finally move onto the next node
		pCurrSample = pCurrSample->m_pNextSample;
	}

	//expand the visibility box if we are a single node by the single node width
	if(m_nNumTrackedNodes == 1)
	{
		float fHalfWidth = GetProps()->m_fSingleNodeWidth * 0.5f;
		LTVector vExpand(fHalfWidth, fHalfWidth, fHalfWidth);
		vMin -= vExpand;
		vMax += vExpand;
	}

	//and now shift our box by our object position and update our visible rectangle
	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_hObject, vMin - vObjectPos, vMax - vObjectPos);
}

//utility function that calculates the distance between the head sample and the next sample
//for the node that is selected to be the node used for distance tracking
float CPolyTrailFX::CalcHeadSegmentDistance() const
{
	//we need at least two active samples to determine this 
	if(!m_pHeadSample || !m_pHeadSample->m_pNextSample)
		return 0.0f;

	//ok, we have two valid samples, now determine the distance
	uint32 nNodeIndex = GetProps()->m_nDistanceNode;

	//if we are out of range, just use the object position
	if(nNodeIndex >= m_nNumTrackedNodes)
		nNodeIndex = 0;

	//the points we will be performing the distance between
	const LTVector& vPos1 = m_pHeadSample->m_Nodes[nNodeIndex].m_vPos;
	const LTVector& vPos2 = m_pHeadSample->m_pNextSample->m_Nodes[nNodeIndex].m_vPos;

	return vPos1.Dist(vPos2);
}

//utility function that will update the normals and tangents of the head and adjacent sample
void CPolyTrailFX::UpdateHeadSampleNormals()
{
	//we need a head and next, otherwise there is no point in trying
	if(!m_pHeadSample || !m_pHeadSample->m_pNextSample)
		return;

	//compute our three sections that will be used for generation of normals (third should never be modified)
	CTrailSample* pFirstSample			= m_pHeadSample;
	CTrailSample* pSecondSample			= pFirstSample->m_pNextSample;
	const CTrailSample* pThirdSample	= (pSecondSample->m_pNextSample) ? pSecondSample->m_pNextSample : pSecondSample;

	//now we have two different methods for updating the normals. When we have two or more samples
	//we actually have a normal and should generate it based upon the geometry, otherwise we precalculate
	//other vectors used for rendering and store those in the normal.
	if(m_nNumTrackedNodes == 1)
	{
		//we only have a single node, so don't compute normals, but instead we want neighbor vectors
		//for the first and second samples

		//generate the vector from the first node to the second, which gives the avg direction of the first node
		pFirstSample->m_Nodes[0].m_vNormal = pSecondSample->m_Nodes[0].m_vPos - pFirstSample->m_Nodes[0].m_vPos;
		pFirstSample->m_Nodes[0].m_vNormal.Normalize();

		//generate the vector from the first node to the third, which gives the avg direction of the second node
		pSecondSample->m_Nodes[0].m_vNormal = pThirdSample->m_Nodes[0].m_vPos - pFirstSample->m_Nodes[0].m_vPos;
		pSecondSample->m_Nodes[0].m_vNormal.Normalize();
	}
	else
	{
		//handle the top and bottom nodes explicitly so we don't need to perform lots of logic in a loop

		//also, very frequently we can get two samples on the same position, in such a case what we
		//need to do is copy over the tangent space from one of the vertices that is valid (which can be
		//guaranteed)
		bool bValidSample[2][CPolyTrailProps::knMaxTrackedNodes];

		//top tracked node
		bValidSample[0][0] = GenerateTangentSpace(pFirstSample, 0, pFirstSample, pSecondSample, 0, 1);
		bValidSample[1][0] = GenerateTangentSpace(pSecondSample, 0, pFirstSample, pThirdSample, 0, 1);

		//bottom tracked node
		uint32 nLastNode = m_nNumTrackedNodes - 1;
		bValidSample[0][nLastNode] = GenerateTangentSpace(pFirstSample, nLastNode, pFirstSample, pSecondSample, nLastNode - 1, nLastNode);
		bValidSample[1][nLastNode] = GenerateTangentSpace(pSecondSample, nLastNode, pFirstSample, pThirdSample, nLastNode - 1, nLastNode);

		//everything in between
		for(uint32 nCurrNode = 1; nCurrNode < nLastNode; nCurrNode++)
		{
			bValidSample[0][nCurrNode] = GenerateTangentSpace(pFirstSample, nCurrNode, pFirstSample, pSecondSample, nCurrNode - 1, nCurrNode + 1);
			bValidSample[1][nCurrNode] = GenerateTangentSpace(pSecondSample, nCurrNode, pFirstSample, pThirdSample, nCurrNode - 1, nCurrNode + 1);
		}

		//scan through, find a valid sample, and fill that into invalid samples
		CTrailSample* pSamples[2] = { pFirstSample, pSecondSample };
		for(uint32 nCurrSample = 0; nCurrSample < 2; nCurrSample++)
		{
			//run through and find a valid sample
			for(uint32 nValidNode = 0; nValidNode < m_nNumTrackedNodes; nValidNode++)
			{
				if(bValidSample[nCurrSample][nValidNode])
					break;
			}

			LTASSERT(nValidNode < m_nNumTrackedNodes, "Error: Unable to find valid node to copy tangent space from");

			//now run through and apply the valid sample to the invalid samples
			for(uint32 nCopyNode = 0; nCopyNode < m_nNumTrackedNodes; nCopyNode++)
			{
				if(!bValidSample[nCurrSample][nValidNode])
				{
					pSamples[nCurrSample]->m_Nodes[nCopyNode].m_vNormal  = pSamples[nCurrSample]->m_Nodes[nValidNode].m_vNormal;
					pSamples[nCurrSample]->m_Nodes[nCopyNode].m_vTangent = pSamples[nCurrSample]->m_Nodes[nValidNode].m_vTangent;
				}
			}
		}
	}
}

//function called to calculate the tangent space for the specified node, and store it within
//that node. This takes in the sample and node within the sample to calculate, and also the 
//samples that should be used for the horizontal support and indices to use for the vertical support
bool CPolyTrailFX::GenerateTangentSpace(CTrailSample* pCalcSample, uint32 nCalcNode, 
										const CTrailSample* pPrevSample, const CTrailSample* pNextSample,
										uint32 nPrevNode, uint32 nNextNode)
{
	//compute the horizontal support
	LTVector vHorizontal = pNextSample->m_Nodes[nCalcNode].m_vPos - pPrevSample->m_Nodes[nCalcNode].m_vPos;

	//compute the vertical support
	LTVector vVertical = pCalcSample->m_Nodes[nNextNode].m_vPos - pCalcSample->m_Nodes[nPrevNode].m_vPos;

	//form the normal from these
	LTVector vNormal = vHorizontal.Cross(vVertical);
	
	//see if we have a valid normal (possible if the nodes are overlapping on current or on neighboring
	//sample)
	float fNormalMagSqr = vNormal.MagSqr();
	if(fNormalMagSqr == 0.0f)
	{
		return false;
	}

	//now normalize our normal now that we know it is safe
	vNormal /= LTSqrt(fNormalMagSqr);

	//now we need to form the tangent, which just happens to be our vertical since our texture is aligned
	//with our edge
	LTVector vBinormal = vNormal.Cross(vVertical);
	LTVector vTangent  = vNormal.Cross(vBinormal).GetUnit();

	//now store these out
	pCalcSample->m_Nodes[nCalcNode].m_vNormal  = vNormal;
	pCalcSample->m_Nodes[nCalcNode].m_vTangent = vTangent;

	return true;
}

//called to determine whether or not a new head sample should be created
bool CPolyTrailFX::ShouldCreateNewSample() const
{
	//always create a sample if we don't already have at least two
	if(!m_pHeadSample || !m_pHeadSample->m_pNextSample)
		return true;

	const CTrailSample* pNextSample = m_pHeadSample->m_pNextSample;

	//now make sure that at least one of the nodes has moved
    static const float kfMinMovementSqr = 1.0f * 1.0f;
	for(uint32 nCurrNode = 0; nCurrNode < m_nNumTrackedNodes; nCurrNode++)
	{
		if(m_pHeadSample->m_Nodes[nCurrNode].m_vPos.DistSqr(pNextSample->m_Nodes[nCurrNode].m_vPos) >= kfMinMovementSqr)
			return true;
	}

	//no nodes moved, so don't create a sample
	return false;
}

//function to allocate a new sample. This will remove it from the free list, and set this up so
//that the list refers to this as the new head
bool CPolyTrailFX::AllocateHeadSample()
{
	//see if we have an element on the free list, if not, we need to allocate a new page
	if(!m_pFreeListAlloc)
	{
		if(!AddNewPageToFreeList())
			return false;
	}

	//take the node off of the allocation list
	CTrailSample* pNewNode = m_pFreeListAlloc;

	if(m_pFreeListAlloc == m_pFreeListFree)
	{
		//end of the list, clear out both
		m_pFreeListAlloc = NULL;
		m_pFreeListFree  = NULL;
	}
	else
	{
		//not the end of the list, just advance the allocation pointer
		m_pFreeListAlloc = m_pFreeListAlloc->m_pNextSample;
	}

	//add this onto our head
	pNewNode->m_pNextSample = m_pHeadSample;
	m_pHeadSample = pNewNode;

	//and initialize data in this node
	m_pHeadSample->m_fLifetime = 0.0f;

	//update our count
	m_nNumSamples++;

	//success
	return true;
}

//called to free the nodes that are in the list that fall after the one provided
void CPolyTrailFX::FreeSamplesAfter(CTrailSample& Node)
{
	CTrailSample* pFreeNode = Node.m_pNextSample;
	while(pFreeNode)
	{
		if(m_pFreeListAlloc)
		{
			//just take the head sample and add it onto the free list's free end
			m_pFreeListFree->m_pNextSample = pFreeNode;
			m_pFreeListFree = pFreeNode;
		}
		else
		{
			//this is the start of the list
			m_pFreeListAlloc = pFreeNode;
			m_pFreeListFree  = pFreeNode;
		}

		//and advance node that we are freeing
		pFreeNode = pFreeNode->m_pNextSample;

		//and clear out our link on the head sample to ensure we don't corrupt our lists
		m_pFreeListFree->m_pNextSample = NULL;

		//update our count
		m_nNumSamples--;
	}

	//and tie off the nodes after the one we just removed
	Node.m_pNextSample = NULL;
}

//called to allocate a new memory page for this trail and add it to the free list
bool CPolyTrailFX::AddNewPageToFreeList()
{
	//we have no more room to allocate from, so allocate a new page
	CMemoryPage* pNewPage = g_PolyTrailPageMgr.AllocatePage();
	if(!pNewPage)
		return false;

	//sanity check
	LTASSERT(pNewPage->GetMemoryBlockSize() >= m_nTrailSamplePitch, "Error: Encountered memory page not large enough to hold a single trail sample");

	//we now need to setup this page with trail samples
	uint8* pCurrOffset = pNewPage->GetMemoryBlock();
	const uint8* pEndOffset = pCurrOffset + pNewPage->GetMemoryBlockSize() - m_nTrailSamplePitch;

	//run through and setup our samples, adding them to the list
	while(pCurrOffset <= pEndOffset)
	{
		//add this sample into our list
		CTrailSample* pSample = (CTrailSample*)pCurrOffset;
		pSample->m_pNextSample = NULL;

		//if we already have a list, add it on, otherwise start our new list
		if(m_pFreeListFree)
		{
			m_pFreeListFree->m_pNextSample = pSample;
		}
		else
		{
			m_pFreeListAlloc = pSample;
		}

		//and this becomes our new tail
		m_pFreeListFree = pSample;

		pCurrOffset += m_nTrailSamplePitch;
	}

	//and now add this page onto our page list
	pNewPage->m_pPrevPage = NULL;
	pNewPage->m_pNextPage = m_pPageHead;

	if(m_pPageHead)
		m_pPageHead->m_pPrevPage = pNewPage;

	m_pPageHead = pNewPage;

	//success
	return true;
}

//-----------------------------------------------------------------------------------------
// PolyTrail Rendering

//hook for the custom render object, this will just call into the render function
void CPolyTrailFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	LTASSERT(pUser, "Error: Invalid user data provided to the poly trail custom render callback");
	CPolyTrailFX* pPolyTrail = (CPolyTrailFX*)pUser;
	pPolyTrail->RenderPolyTrail(pInterface, tCamera);
}

//this function will allocate the specified batch, and will return the number of vertices within the
//batch (or zero if failure occurred). Note that this assumes that the number of vertices is a multiple of
//2
static uint32 AllocateVertexBuffer(	uint32 nNumVerts, ILTCustomRenderCallback* pInterface,
									STexTangentSpaceVert*& pOutVert, STexTangentSpaceVert*& pEndVert,
									SDynamicVertexBufferLockRequest& LockRequest)
{
	LTASSERT(nNumVerts % 2 == 0, "Error: Expected a multiple of two for the number of vertices requested");

	//determine the maximum number of vertices that we can render in a single batch (and we only
	//render in pairs so make sure it is a multiple of 2)
	const uint32 nMaxVertsPerBatch = (DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(STexTangentSpaceVert) / 2) * 2;
	LTASSERT(nMaxVertsPerBatch > 4, "Error: Not enough vertices per batch to render poly trail");

	//determine how large this batch will actually be and also subtract out the number of remaining
	//vertices
	uint32 nBatchSize = LTMIN(nNumVerts, nMaxVertsPerBatch);

	//now lock that down
	if(pInterface->LockDynamicVertexBuffer(nBatchSize, LockRequest) != LT_OK)
		return 0;

	pOutVert = (STexTangentSpaceVert*)LockRequest.m_pData;
	pEndVert = pOutVert + nBatchSize;

	return nBatchSize;
}


//calling this function with two vertices that will be written, it will write it out to the current 
//vertex buffer, and 
static bool WriteVerts(const STexTangentSpaceVert& Vert1, const STexTangentSpaceVert& Vert2,
					   ILTCustomRenderCallback* pInterface,
					   STexTangentSpaceVert*& pOutVert, STexTangentSpaceVert*& pEndVert,
					   SDynamicVertexBufferLockRequest& LockRequest, uint32& nRemainingVerts)
{
	//we should ALWAYS have vertices remaining
	LTASSERT(pOutVert != pEndVert, "Error: Attempted to write vertices without a buffer to add them to");

	//write these vertices out
	pOutVert[0] = Vert1;
	pOutVert[1] = Vert2;

	//and move on to the next item
	pOutVert += 2;

	//if we have filled up the buffer, go ahead and render it
	if(pOutVert == pEndVert)
	{
		//we are at the end, flush out the existing buffer
		pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pInterface->Render(	eCustomRenderPrimType_TriangleStrip, LockRequest.m_nStartIndex, LockRequest.m_nNumVertices);
		pOutVert = NULL;
		pEndVert = NULL;

		//and if we have more vertices left, we need to create a new buffer
		if(nRemainingVerts > 0)
		{
			//determine how many vertices we need left (including our two repeated vertices)
			uint32 nBatchSize = AllocateVertexBuffer(nRemainingVerts + 2, pInterface, pOutVert, pEndVert, LockRequest);
			if(nBatchSize == 0)
				return false;

			//update our remaining vertex count			
			nRemainingVerts -= nBatchSize - 2;

			//and write out our initial vertices
			pOutVert[0] = Vert1;
			pOutVert[1] = Vert2;

			//and move on to the next item
			pOutVert += 2;
		}
	}

	return true;
}

//function that handles the custom rendering
void CPolyTrailFX::RenderPolyTrail(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXPolyTrail);

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//bail if we don't have any samples
	if(m_nNumSamples < 2)
		return;

	//we can now compute the total number of vertices that we will need to render
	uint32 nRemainingVerts;
	if(m_nNumTrackedNodes <= 2)
		nRemainingVerts = m_nNumSamples * 2;
	else
		nRemainingVerts = m_nNumSamples * (m_nNumTrackedNodes - 1) * 2 + (m_nNumTrackedNodes - 2) * 2;

	//pointers into our vertex buffer
	STexTangentSpaceVert* pOutVert = NULL;
	STexTangentSpaceVert* pEndVert = NULL;

	SDynamicVertexBufferLockRequest LockRequest;

	//allocate our initial batch of vertices
	uint32 nBatchSize = AllocateVertexBuffer(nRemainingVerts, pInterface, pOutVert, pEndVert, LockRequest);
	if(nBatchSize == 0)
		return;

	nRemainingVerts -= nBatchSize;

	//precalculate the inverse of the maximum lifetime of a segment for quicker looking up of colors
	float fInvSampleLifetime = 1.0f / LTMIN(m_fTotalElapsed, GetProps()->m_fSampleLifetimeS);

	//we need to handle different algorithms for the different number of nodes that we are tracking.
	//for 1 node we need to generate the vertices and strip it into a single strip, for 2 we can use
	//existing vertices but need to strip it into a single strip, for 3+ we need to strip vertically
	if(m_nNumTrackedNodes == 1)
	{
		float fHalfWidth = GetProps()->m_fSingleNodeWidth * 0.5f;

		//we need to run through, generate, and strip
		for(const CTrailSample* pCurrSample = m_pHeadSample; pCurrSample; pCurrSample = pCurrSample->m_pNextSample)
		{
			//determine the color for this segment
			float fSampleLife =  LTMIN(pCurrSample->m_fLifetime * fInvSampleLifetime, 1.0f);
			uint32 nSampleColor = CFxProp_Color4f::ToColor(GetProps()->m_cfcSampleColor.GetValue(fSampleLife));

			//we need to generate the vectors for this vertex (the normal stores the average direction
			//of this node)
			LTVector vToCamera	= pCurrSample->m_Nodes[0].m_vPos - tCamera.m_vPos;
			LTVector vTangent	= vToCamera.Cross(pCurrSample->m_Nodes[0].m_vNormal).GetUnit();
			LTVector vNormal	= vTangent.Cross(pCurrSample->m_Nodes[0].m_vNormal);
			LTVector vBinormal	= vTangent.Cross(vNormal);

			LTVector vScaledTangent = vTangent * fHalfWidth;

			//now fill in our renderable vertices
			STexTangentSpaceVert v0, v1;
			v0.m_vPos = pCurrSample->m_Nodes[0].m_vPos + vScaledTangent;
			v1.m_vPos = pCurrSample->m_Nodes[0].m_vPos - vScaledTangent;

			v0.m_vUV.x = 1.0f;
			v1.m_vUV.x = 0.0f;

			v0.m_nPackedColor	= v1.m_nPackedColor = nSampleColor;
			v0.m_vUV.y			= v1.m_vUV.y		= pCurrSample->m_fVTexCoord;
			v0.m_vNormal		= v1.m_vNormal		= vNormal;
			v0.m_vTangent		= v1.m_vTangent		= vTangent;
			v0.m_vBinormal		= v1.m_vBinormal	= vBinormal;

			//and write them out to our buffer
			if(!WriteVerts(v0, v1, pInterface, pOutVert, pEndVert, LockRequest, nRemainingVerts))
				return;
		}
	}
	else
	{
		//fill in our texture coordinates for faster access
		float fTexCoords[CPolyTrailProps::knMaxTrackedNodes];
		fTexCoords[0] = 0.0f;
		for(uint32 nCurrCoord = 1; nCurrCoord < m_nNumTrackedNodes; nCurrCoord++)
			fTexCoords[nCurrCoord] = GetProps()->m_TrackedNodes[nCurrCoord - 1].m_fUCoord;

		//now run through and render our strips
		STexTangentSpaceVert LastVert;

		for(uint32 nCurrStrip = 0; nCurrStrip < m_nNumTrackedNodes - 1; nCurrStrip++)
		{
			//determine the indices to use for our nodes
			uint32 nNode0 = nCurrStrip;
			uint32 nNode1 = nCurrStrip + 1;

			//and the texture coordinates to use as well
			float fU0 = fTexCoords[nNode0];
			float fU1 = fTexCoords[nNode1];

			//determine if we need to generate degenerate quads to start this strip
			bool bCreateDegenerateTris = (nCurrStrip > 0);
		
			//now run through and render our strip

			//this could be a for loop, but we break it apart to avoid an if since we need to know when we
			//hit the last segment inside of the loop
			const CTrailSample* pCurrSample = m_pHeadSample;
			while(1)
			{
				//determine the color for this segment
				float fSampleLife = LTMIN(pCurrSample->m_fLifetime * fInvSampleLifetime, 1.0f);
				uint32 nSampleColor = CFxProp_Color4f::ToColor(GetProps()->m_cfcSampleColor.GetValue(fSampleLife));

				//now fill in our renderable vertices
				STexTangentSpaceVert v0, v1;
				v0.m_vPos = pCurrSample->m_Nodes[nNode0].m_vPos;
				v0.m_nPackedColor = nSampleColor;
				v0.m_vUV.Init(fU0, pCurrSample->m_fVTexCoord);
				v0.m_vNormal	= pCurrSample->m_Nodes[nNode0].m_vNormal;
				v0.m_vTangent	= pCurrSample->m_Nodes[nNode0].m_vTangent;
				v0.m_vBinormal	= pCurrSample->m_Nodes[nNode0].m_vTangent.Cross(pCurrSample->m_Nodes[nNode0].m_vNormal);

				v1.m_vPos = pCurrSample->m_Nodes[nNode1].m_vPos;
				v1.m_nPackedColor = nSampleColor;
				v1.m_vUV.Init(fU1, pCurrSample->m_fVTexCoord);
				v1.m_vNormal	= pCurrSample->m_Nodes[nNode1].m_vNormal;
				v1.m_vTangent	= pCurrSample->m_Nodes[nNode1].m_vTangent;
				v1.m_vBinormal	= pCurrSample->m_Nodes[nNode1].m_vTangent.Cross(pCurrSample->m_Nodes[nNode1].m_vNormal);

				//if we are at the beginning, and we need to create degenerate strip ends, go ahead and
				//generate those
				if(bCreateDegenerateTris)
				{
					//duplicate the last vertex we rendered and the first vertex we are going to render
					if(!WriteVerts(LastVert, v0, pInterface, pOutVert, pEndVert, LockRequest, nRemainingVerts))
						return;

					//and we don't need to do this again until the next strip
					bCreateDegenerateTris = false;
				}

				//and write them out to our buffer
				if(!WriteVerts(v0, v1, pInterface, pOutVert, pEndVert, LockRequest, nRemainingVerts))
					return;

				pCurrSample = pCurrSample->m_pNextSample;
				if(!pCurrSample)
				{
					//save this last vertex so we can use it again
					LastVert = v1;

					//and stop this looping
					break;
				}
			}
		}
	}
}


