// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeStalk.cpp
//
// PURPOSE : 
//
// CREATED : 5/17/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ltintersect.h"
#include <algorithm>
#include "AINodeStalk.h"
#include "PlayerObj.h"
#include "AIAssert.h"
#include "AINavMesh.h"
#include "AIQuadTree.h"
#include "AIWorkingMemory.h"
#include "AI.h"
#include "AIPathMgrNavMesh.h"
#include "AIBlackBoard.h"
#include "AIWorldState.h"
#include "AIWorkingMemoryCentral.h"

LINKFROM_MODULE( AINodeStalk );

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

// Set to 1 to continuously draw all volumes based on the players position.
#ifdef _FINAL
	#define AINODESTALK_CONTINUOUS_DRAW 0
#else
	#define AINODESTALK_CONTINUOUS_DRAW 1
#endif

#define AISTALK_BLINDOBJECTID	0x6aaf0885
#define AISTALK_VERSION_NUMBER	0

// Define a few stalking node specific data elements.

static bool  s_bInitializedConstants = false;
static float s_flAINodeStalkVisualizationRadius				= 50.f;
static float s_flAINodeStalkVisualizationVerticalThreshold	= 180.f;
static float s_flAINodeStalkRadiusProjectionScalar			= 1.0f;
static const int s_flAINodeStalkRadiusProjectionMaxProbesWidth	= 3;
static const int s_flAINodeStalkRadiusProjectionMaxProbesDepth	= 3;

static void InitConstants()
{
	if ( !s_bInitializedConstants )
	{
		s_flAINodeStalkVisualizationRadius				= g_pAIDB->GetMiscFloat( "AINodeStalkVisualizationRadius" );
		s_flAINodeStalkVisualizationVerticalThreshold	= g_pAIDB->GetMiscFloat( "AINodeStalkVisualizationVerticalThreshold" );
		s_flAINodeStalkRadiusProjectionScalar			= g_pAIDB->GetMiscFloat( "AINodeStalkRadiusProjectionScalar" );

		s_bInitializedConstants = true;
	}
}

class AINodeStalkPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeStalkPlugin()
	{
		AddValidNodeType( kNode_Stalk );
	}
};

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeStalk 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeStalk CF_HIDDEN

#endif

BEGIN_CLASS(AINodeStalk)
	ADD_REALPROP_FLAG(StalkDataIndex, -1.0f, PF_HIDDEN, "Internal index into the blind object data for the the Stalk") \
	ADD_STRINGPROP_FLAG(SmartObject,			"None", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC") \
	BOUNDARYRADIUS_PROPS( 1024.0f )
END_CLASS_FLAGS_PLUGIN(AINodeStalk, AINodeSmartObject, CF_HIDDEN_AINodeStalk, AINodeStalkPlugin, "AIs 'move to' stalking nodes when they want to move closer to a location while reducing their exposure")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeStalk)
CMDMGR_END_REGISTER_CLASS(AINodeStalk, AINode)

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::Constructor/Destructor()
//
//  PURPOSE:	Handle initializing the stalking node to an inert state.
//
// ----------------------------------------------------------------------- //

AINodeStalk::AINodeStalk() :
	m_nStalkDataIndex((uint32)-1)
	, m_vStalkCenter(0.f, 0.f, 0.f)
	, m_bDrawing(false)
{
	InitConstants();
}

AINodeStalk::~AINodeStalk()
{
}

void AINodeStalk::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_INT(m_nStalkDataIndex);

	m_BoundaryRadiusValidator.Load( pMsg );

	LoadBlindObjectData();
}

void AINodeStalk::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_INT(m_nStalkDataIndex);

	m_BoundaryRadiusValidator.Save( pMsg );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::ReadProp()
//
//  PURPOSE:	Read in the property values that were set by WorldEdit 
//				and by the processor (in the case of the StalkDataIndex);
//
// ----------------------------------------------------------------------- //

void AINodeStalk::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	float fStalkDataIndex		= pProps->GetReal( "StalkDataIndex", -1.0f );
	m_nStalkDataIndex	= (fStalkDataIndex >= 0.0f ? (uint32)fStalkDataIndex : 0xffffffff);

	m_BoundaryRadiusValidator.ReadProps( pProps );

	LoadBlindObjectData();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::LoadBlindObjectData()
//
//  PURPOSE:	Handle loading the blind object data into the Stalking 
//				node.  This only needs to be done on startup and on load.
//				Separate from ReadData to keep readData a pure reading 
//				function.
//
// ----------------------------------------------------------------------- //

void AINodeStalk::LoadBlindObjectData()
{
	// Get the blind object data pointer and size
	uint8* blindData = NULL;
	uint32 blindDataSize = 0;
	if( g_pLTServer->GetBlindObjectData( m_nStalkDataIndex, AISTALK_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
	{
		AIASSERT( 0, m_hObject, "AINodeStalk::ReadProp: No blind object data - missing a hull?" );
		return;
	}

	ReadData(blindData);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::ReadData()
//
//  PURPOSE:	Given a pointer to a data source, read in all of the data 
//				used to define this object.
//
// ----------------------------------------------------------------------- //

void AINodeStalk::ReadData(uint8* blindData)
{
	// Data Format:
	//
	// uint32	(Version)
	// float	(Height)
	// uint32	(Vertex Count)
	//		For each vertex in count
	//		float,float,float, (x, y, z)

	if (NULL == blindData)
	{
		return;
	}

	// Get the object data pointer and size.

	uint8* curBlindData = blindData;

	const uint32 nSizeFloat = sizeof(float);
	const uint32 nSizeInt = sizeof(int);

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	// Note: The blind data starts out with 3 32-bit quantities, so switch them in one shot
	LittleEndianToNative((uint32*)curBlindData, 3);
#endif // PLATFORM_XENON

	// Validate against a version number

	uint32 nAIStalkVersion = *((uint32*)curBlindData);
	curBlindData += nSizeInt;

	if( nAIStalkVersion != AISTALK_VERSION_NUMBER )
	{
		AIASSERT2( 0, m_hObject, "AINodeStalk::ReadProp: AINodeStalk version numbers do not match.  WorldPacker: %d Object: %d", nAIStalkVersion, AISTALK_VERSION_NUMBER );
		return;
	}

	// Read the height

	float m_flHeight = *((float*)curBlindData);	
	curBlindData += nSizeFloat;

	// Read the verts

	uint32 cVerts = *((uint32*)curBlindData);	
	curBlindData += nSizeInt;

	m_lstStalkVerts.reserve(cVerts);

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	// Note: Swap the verts all at the same time, each of which is just a vector
	LittleEndianToNative((LTVector*)curBlindData, cVerts);
#endif // PLATFORM_XENON

	for( uint32 iVert=0; iVert < cVerts; ++iVert )
	{
		LTVector vVert;
		vVert.x = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		vVert.y = *((float*)curBlindData);
		curBlindData += nSizeFloat;
		vVert.z = *((float*)curBlindData);
		curBlindData += nSizeFloat;

		m_lstStalkVerts.push_back(vVert);
	}

	// Set up the Stalk based on the data read

	uint32 nVerts = m_lstStalkVerts.size();

	for (uint32 i = 0; i < nVerts; ++i)
	{
		m_vStalkCenter += m_lstStalkVerts[i];
	}
	m_vStalkCenter /= (float)nVerts;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AINodeStalk::EngineMessageFn(uint32 messageID, void *pv, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// Call around to set the update (only temp for debugging)
			DWORD dwRet = super::EngineMessageFn(messageID, pv, fData);
			#if AINODESTALK_CONTINUOUS_DRAW
				SetNextUpdate( UPDATE_NEXT_FRAME );
			#endif
			return dwRet;
		}

		case MID_UPDATE:
		{
			// Call around to set the update (only temp for debugging)
			DWORD dwRet = super::EngineMessageFn(messageID, pv, fData);
			#if AINODESTALK_CONTINUOUS_DRAW
				Update();
				SetNextUpdate( UPDATE_NEXT_FRAME );
			#endif
			return dwRet;
		}
	}

	return super::EngineMessageFn(messageID, pv, fData);
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::DrawProbes()
//
//	PURPOSE:	Draw a list of points, color coded based on presence in 
//				the nav mesh.
//
// ----------------------------------------------------------------------- //

void AINodeStalk::DrawProbes(const LTOBB& SafeOBB, LTVector* paProbes, int nProbes, DebugLineSystem& system) const
{
	for (int i = 0; i < nProbes; ++i)
	{
		DebugLine::Color vColor(255, 0, 0);

		ENUM_NMPolyID ePolyID = g_pAIQuadTree->GetContainingNMPoly(paProbes[i], ALL_CHAR_TYPES, kNMPoly_Invalid);
		if (kNMPoly_Invalid != ePolyID)
		{
			CAINavMeshPoly* pNMPoly = g_pAINavMesh->GetNMPoly(ePolyID);
			if (pNMPoly)
			{
				LTVector vPolyCenter = pNMPoly->GetNMPolyCenter();

				// Verify that the poly found overlaps with the OBB.  The quadtree does not
				// have any depth, so the quadtree does a drill down/drill up pass to find 
				// the intersection.  If the overlap point exists only in the floor below 
				// the AI, this poly will be returned.  This watches for this case, and 
				// skips it if the point is outside.

				if ( (vPolyCenter.y <= (SafeOBB.Center().y + SafeOBB.HalfDims().y) ) 
					|| (vPolyCenter.y >= (SafeOBB.Center().y - SafeOBB.HalfDims().y )) )
				{
					vColor = DebugLine::Color(0, 255, 0);
				}
			}
		}

		system.AddLine(paProbes[i], paProbes[i] - LTVector(0.f, 64.f, 0.f), vColor);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::GetSafeOBBProbes()
//
//	PURPOSE:	Given an OBB, return a list of points to probe the nav 
//				mesh as potential locations to path to.
//
// ----------------------------------------------------------------------- //

void AINodeStalk::GetSafeOBBProbes(LTOBB& rOBB,
					  float flMinDistanceBetweenProbes,
					  int iSubDivisionWidthMax, int iSubDivisionDepthMax, 
					  int nMaxProbes, LTVector* avOutProbePoints, int& nOutProbeCount) const
{
	nOutProbeCount = 0;

	float flWidth = rOBB.Dims().x;
	float flDepth = rOBB.Dims().z;

	const int iSubDivisionWidth = LTCLAMP((int)(flWidth/flMinDistanceBetweenProbes), 1, iSubDivisionWidthMax );
	const int iSubDivisionDepth = LTCLAMP((int)(flWidth/flMinDistanceBetweenProbes), 1, iSubDivisionDepthMax );

	float flHalfBufferWidth = (flWidth - ((iSubDivisionWidth-1)*flMinDistanceBetweenProbes))/2.f;
	float flHalfBufferDepth = (flDepth - ((iSubDivisionDepth-1)*flMinDistanceBetweenProbes))/2.f;

	// With 1 width and 1 depth division:
	//
	// --------------------
	// |                  |
	// |                  |
	// |                  |
	// |                  |
	// |        x         |
	// |                  |
	// |                  |
	// |                  |
	// |                  |
	// --------------------
	//
	// With 1 width and 2 depth division:
	//
	// --------------------
	// |                  |
	// |                  |
	// |        x         |
	// |                  |
	// |                  |
	// |                  |
	// |        x         |
	// |                  |
	// |                  |
	// --------------------
	//
	// With 2 width and 1 depth division:
	//
	// --------------------
	// |                  |
	// |                  |
	// |                  |
	// |                  |
	// |    x       x     |
	// |                  |
	// |                  |
	// |                  |
	// |                  |
	// --------------------
	//

	LTVector vCenterBackLeft = rOBB.Center() 
		- rOBB.Right()	*(rOBB.HalfDims().x - flHalfBufferWidth) 
		- rOBB.Forward()*(rOBB.HalfDims().z - flHalfBufferDepth);

	for (int nDepth = 0; nDepth < iSubDivisionDepth; ++nDepth)
	{
		LTVector vDepthPos = vCenterBackLeft + (rOBB.Forward() * (nDepth*flMinDistanceBetweenProbes));
		for (int nWidth = 0; nWidth < iSubDivisionWidth; ++nWidth)
		{
			if (nOutProbeCount >= nMaxProbes)
			{
				AIASSERT(0, 0, "GetSafeOBBProbes : Overflowed the probe count.");
				return;
			}

			avOutProbePoints[nOutProbeCount] = vDepthPos + (rOBB.Right() * (nWidth*flMinDistanceBetweenProbes));
			++nOutProbeCount;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::Update()
//
//	PURPOSE:	Debugging update, used to draw the projected brush when 
//				AINODESTALK_CONTINUOUS_DRAW is defined to be 1.
//
// ----------------------------------------------------------------------- //

void AINodeStalk::Update()
{
	if (!m_bDrawing)
	{
		return;
	}

	// Build an general OBB for visualization with 'typical' AI stats.

	DebugLineSystem& system = LineSystem::GetSystem( this, "ShowAIStalk" );
	system.Clear();

	const CPlayerObj* pPlayer = *CPlayerObj::GetPlayerObjList().begin();

	LTOBB SafeOBB;
	if (!GetSafeStalkOBB(s_flAINodeStalkVisualizationRadius, s_flAINodeStalkVisualizationVerticalThreshold, pPlayer->GetLastNavMeshPos(), SafeOBB))
	{
		return;
	}

	system.AddOBB(SafeOBB);

	// Draw some example probes into the environment.

	const int kMaxProbes = s_flAINodeStalkRadiusProjectionMaxProbesWidth*s_flAINodeStalkRadiusProjectionMaxProbesDepth;

	LTVector aProbePoints[kMaxProbes];
	int nProbes = 0;
	GetSafeOBBProbes(SafeOBB, s_flAINodeStalkVisualizationRadius, s_flAINodeStalkRadiusProjectionMaxProbesWidth, s_flAINodeStalkRadiusProjectionMaxProbesDepth, kMaxProbes, aProbePoints, nProbes);
	DrawProbes(SafeOBB, aProbePoints, nProbes, system);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::GetDestinationPosition()
//
//	PURPOSE:	Returns true if a destination position at this node was 
//				found, false if one was not.  If true is returned, 
//				vOutPosition contains the location to move to.  If false 
//				is returned, this parameter is not changed.
//
//				If the AI already had selected a stalking position for this,
//				node, this position is returned without testing for validity.
//
// ----------------------------------------------------------------------- //

bool AINodeStalk::GetDestinationPosition(CAI* pAI, const LTVector& vThreatPosition, LTVector& vOutPosition) const
{
	// If the AI has chosen a location to move to, use it.  Otherwise, 
	// generate a position and return it.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Knowledge);
	factQuery.SetKnowledgeType(kKnowledge_StalkPosition);
	factQuery.SetTargetObject(m_hObject);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
	if (pFact)
	{
		vOutPosition = pFact->GetPos();
		return true;
	}

	// Build an general OBB for visualization with 'typical' AI stats.


	LTOBB SafeOBB;
	if (!GetSafeStalkOBB(pAI->GetRadius(), pAI->GetVerticalThreshold(), vThreatPosition, SafeOBB))
	{
		return false;
	}

	// Draw some example probes into the environment.

	const int kMaxProbes = s_flAINodeStalkRadiusProjectionMaxProbesWidth*s_flAINodeStalkRadiusProjectionMaxProbesDepth;

	LTVector aProbePoints[kMaxProbes];
	int nProbes = 0;
	GetSafeOBBProbes(SafeOBB, pAI->GetRadius(), 
		s_flAINodeStalkRadiusProjectionMaxProbesWidth, s_flAINodeStalkRadiusProjectionMaxProbesDepth, kMaxProbes, 
		aProbePoints, nProbes);

	if (m_bDrawing)
	{
		DebugLineSystem& system = LineSystem::GetSystem( this, "ShowAIStalk" );
		system.Clear();
		system.AddOBB(SafeOBB);
		DrawProbes(SafeOBB, aProbePoints, nProbes, system);
	}

	// No probes found.

	if (0 == nProbes)
	{
		return false;
	}

	// Randomize the points, then use the first valid position found as the destination.

	std::random_shuffle(&aProbePoints[0], &aProbePoints[0] + nProbes);

	ENUM_NMPolyID ePolyHint = kNMPoly_Invalid;
	for (int i = 0; i < nProbes; ++i)
	{
		ENUM_NMPolyID ePolyID = g_pAIQuadTree->GetContainingNMPoly(aProbePoints[i], ALL_CHAR_TYPES, ePolyHint, pAI); 

		// No poly found.

		if (kNMPoly_Invalid == ePolyID)
		{
			continue;
		}

		CAINavMeshPoly* pNMPoly = g_pAINavMesh->GetNMPoly(ePolyID);
		if (!pNMPoly)
		{
			continue;
		}

		LTVector vPolyCenter = pNMPoly->GetNMPolyCenter();

		// Verify that the poly found overlaps with the OBB.  The quadtree does not
		// have any depth, so the quadtree does a drill down/drill up pass to find 
		// the intersection.  If the overlap point exists only in the floor below 
		// the AI, this poly will be returned.  This watches for this case, and 
		// skips it if the point is outside.

		if ( (vPolyCenter.y > (SafeOBB.Center().y + SafeOBB.HalfDims().y) ) 
			|| (vPolyCenter.y < (SafeOBB.Center().y - SafeOBB.HalfDims().y )) )
		{
			continue;
		}

		if ( g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), ePolyID ))
		{
			vOutPosition = aProbePoints[i];
			return true;
		}

		// Update the hint to keep it close.
		ePolyHint = ePolyID;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::HandleAIArrival()
//
//	PURPOSE:	Handle the AI arriving at the node.  If face not is 
//				specified, the base class will attempt to orient the AI to 
//				face the node.  As the nodes position doesn't matter, this 
//				code orients the AI to face the target instead in this case.
//
// ----------------------------------------------------------------------- //

void AINodeStalk::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	// if facing the node is specified, face the enemy if there is one, 
	// otherwise face forward.  The position of the node itself is 
	// irrelevant for this node.
	if( m_bFaceNode )
	{
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		if (hTarget)
		{
			pAI->GetAIBlackBoard()->SetBBFaceObject( hTarget );
		}
		else
		{
			pAI->GetAIBlackBoard()->SetBBFaceDir( pAI->GetTorsoForward() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::GetSafeStalkOBB()
//
//	PURPOSE:	Returns true if a 'safe'  OBB was found, false if one was 
//				not.  If true is returned, rOutOBB contains an OBB which 
//				is out of sight based on the threats position.
//
//				This function internally handles trimming the OBB to take 
//				into account the passed in radius.
//
// ----------------------------------------------------------------------- //

bool AINodeStalk::GetSafeStalkOBB(float flAIRadius, float flHeight, const LTVector& vThreatPosition, LTOBB& rOutOBB) const
{
	// Get the projection based on the direction to the threat and the poly verts

	LTVector vUpNormal = LTVector(0, 1, 0);

	LTVector vThreatToCenterDir = m_vStalkCenter - vThreatPosition;
	vThreatToCenterDir.y = 0.f;

	LTVector vRightNorm = vThreatToCenterDir.Cross(vUpNormal);
	vRightNorm.Normalize();

	LTVector vForwardNorm = vThreatToCenterDir.GetUnit();

	// Find the inner and outer most points by projecting the Stalks verts
	// onto vRightNormal and vForwardNormal

	float flRightProjectionMin = 0.f;
	float flRightProjectionMax = 0.f;
	float flBackProjectionMax = 0.f;

	for (uint32 iVert = 0; iVert < m_lstStalkVerts.size(); ++iVert)
	{
		const LTVector& v0 = m_lstStalkVerts[iVert];

		// Handle both of the verts (this is redundant, but 
		// probably cheaper than the bookkeeping to keep track of 
		// which have and haven't been checked.

		float flDot = vRightNorm.Dot(v0 - m_vStalkCenter);
		if (flDot < flRightProjectionMin)
			flRightProjectionMin = flDot;
		if (flDot > flRightProjectionMax)
			flRightProjectionMax = flDot;
		if (flDot > flBackProjectionMax)
			flBackProjectionMax = flDot;
	}

	// Trim the projection based on the AIs radius.  Return false if the 
	// trimming shrinks the box to 0 or less

	flRightProjectionMax -= flAIRadius*s_flAINodeStalkRadiusProjectionScalar;
	flRightProjectionMin += flAIRadius*s_flAINodeStalkRadiusProjectionScalar;
	if (flRightProjectionMin >= flRightProjectionMax)
	{
		return false;
	}

	// Build the OBB

	float flHalfWidth = (flRightProjectionMax - flRightProjectionMin)/2.f;
	float flHalfDepth = 128.f;
	LTVector vCenter = m_vStalkCenter + vForwardNorm * (flBackProjectionMax + flHalfDepth);
	LTVector vHalfDims( flHalfWidth, flHeight, flHalfDepth);
	rOutOBB.Init(vCenter, vHalfDims, vRightNorm, vUpNormal, vForwardNorm);

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::IsNodeValid()
//
//	PURPOSE:	Returns true if this node is valid given the current state
//				of the node and AI.
//
// ----------------------------------------------------------------------- //

bool AINodeStalk::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	if (!super::IsNodeValid(pAI, vPosAI, hThreat, eThreatPos, dwStatusFlags))
	{
		return false;
	}

	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	if( dwFilteredStatusFlags & kNodeStatus_Damaged )
	{
		// AI was damaged recently. Damaged recently

		CAIWMFact factDamagedQuery;
		factDamagedQuery.SetFactType(kFact_Damage);
		CAIWMFact* pDamagedFact = pAI->GetAIWorkingMemory()->FindWMFact( factDamagedQuery );
		if( pDamagedFact 
			&& ( DidDamage( pAI, pDamagedFact ) )
			&& ( pDamagedFact->GetUpdateTime() < g_pLTServer->GetTime() - 4.f ) )
		{
			return false;
		}

		// An AI has been damaged at this node; it isn't safe to use.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Knowledge);
		factQuery.SetKnowledgeType(kKnowledge_DamagedAtNode);
		factQuery.SetTargetObject(m_hObject);

		CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
		if( pFact )
		{
			if (pFact->GetTime() < g_pLTServer->GetTime())
			{
				// Expired fact, remove it.

				g_pAIWorkingMemoryCentral->ClearWMFact(pFact);
			}
			else
			{
				// Fact applies, return.
				if ( DidDamage( pAI, pFact ) )
				{
					return false;
				}
			}
		}
	}

	// AI is no longer occluded, and may be seen by the threat.

	if (dwFilteredStatusFlags & kNodeStatus_ThreatOutsideFOV)
	{
		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		bool bAtNode = pProp && pProp->hWSValue == m_hObject;
		LTVector vTestPosition = vPosAI;
		if (!bAtNode)
		{
			CAIWMFact factQuery;
			factQuery.SetKnowledgeType(kKnowledge_StalkPosition);
			factQuery.SetTargetObject(m_hObject);
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
			if (pFact)
			{
				vTestPosition = pFact->GetPos();
			}
		}

		LTVector vThreatPos;
		ENUM_NMPolyID eThreatNMPoly;
		GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );

		LTOBB SafeOBB;
		if (!GetSafeStalkOBB(pAI->GetRadius(), pAI->GetVerticalThreshold(), vThreatPos, SafeOBB))
		{
			return false;
		}

		if (!LTIntersect::OBB_Point(SafeOBB, vTestPosition))
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::DrawSelf()
//
//	PURPOSE:	Turns on debug drawing.  In debug and release, turns on 
//				both consideration drawing and continuous drawing.  This 
//				this is undesirable, the type of drawing could be hooked 
//				up to a console var
//
//	TODO:		Depending on the number of stalking nodes LDs place, drawing
//				may need to occur over a few frames to avoid flooding the 
//				client and dropping lines.
//
// ----------------------------------------------------------------------- //

int AINodeStalk::DrawSelf()
{
	int nRet = super::DrawSelf();

	// Turn on drawing
	m_bDrawing = true;

	return nRet;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStalk::HideSelf()
//
//	PURPOSE:	Turns off debug drawing functionality.
//
// ----------------------------------------------------------------------- //

int AINodeStalk::HideSelf()
{
	int nRet = super::HideSelf();

	// Turn off drawing
	m_bDrawing = false;

	// Clear the line system.
	DebugLineSystem& system = LineSystem::GetSystem( this, "ShowAIStalk" );
	system.Clear();

	return nRet;
}
