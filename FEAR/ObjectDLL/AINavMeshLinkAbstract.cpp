// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkAbstract.cpp
//
// PURPOSE : AI NavMesh Link abstract class implementation.
//
// CREATED : 07/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkAbstract.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AIAStarNavMesh.h"
#include "AIPathMgrNavMesh.h"
#include "AIQuadTree.h"
#include "AIAssert.h"
#include "AIUtils.h"
#include <algorithm>


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkAbstract );

BEGIN_CLASS( AINavMeshLinkAbstract )
	ADD_REALPROP_FLAG(AINavMeshLinkID,			(float)kNMLink_Invalid,		CF_HIDDEN,			"Internal ID for the AINavMesh link")
	ADD_STRINGPROP_FLAG(SmartObject,			"None",						0|PF_STATICLIST,	"SmartObject used to specify animations for traversing the link")
	ADD_STRINGPROP_FLAG(RequiredAwarenessMod,	"None",						0|PF_STATICLIST,	"Awareness modifier required to treat this as an enabled link.")
	ADD_STRINGPROP_FLAG(MinEnabledAwareness,	"Relaxed",					0|PF_STATICLIST,	"Minimum awareness required to treat this as an enabled link.")
	ADD_STRINGPROP_FLAG(MaxEnabledAwareness,	"Alert",					0|PF_STATICLIST,	"Maximum awareness required to treat this as an enabled link.")
	ADD_STRINGPROP_FLAG(MinActiveAwareness,		"Relaxed",					0|PF_STATICLIST,	"Minimum awareness required to treat this as an active link.")
	ADD_STRINGPROP_FLAG(MaxActiveAwareness,		"Alert",					0|PF_STATICLIST,	"Maximum awareness required to treat this as an active link.")
	ADD_BOOLPROP_FLAG(StartDisabled,			false,						0, "If true the AINavMeshLink will begin disabled.")
	ADD_BOOLPROP_FLAG(Active,					true,						0, "If true the AINavMeshLink will be Active.")
END_CLASS_FLAGS_PLUGIN(AINavMeshLinkAbstract, GameBase, CF_HIDDEN, AINavMeshLinkAbstractPlugin, "This is a base class for all AI NavMesh links")


static bool ValidateRemoveMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateRemoveMsg()" );
		pInterface->CPrint( "    MSG - REMOVE - Unable to remove AINavMeshLink.  Use DISABLE instead!" );

		return false;
	}

	return true;
}

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkAbstract )

	ADD_MESSAGE( ACTIVE,				2,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleActiveMsg ),				"ACTIVE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( DISABLE,				1,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleDisableMsg ),				"DISABLE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( ENABLE,				1,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleEnableMsg ),				"ENABLE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( MINACTIVEAWARENESS,	2,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleMinActiveAwarenessMsg ),	"MINACTIVEAWARENESS", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( MAXACTIVEAWARENESS,	2,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleMaxActiveAwarenessMsg ),	"MAXACTIVEAWARENESS", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( MINENABLEDAWARENESS,	2,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleMinEnabledAwarenessMsg ),	"MINENABLEDAWARENESS", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( MAXENABLEDAWARENESS,	2,	NULL,	MSG_HANDLER( AINavMeshLinkAbstract, HandleMaxEnabledAwarenessMsg ),	"MAXENABLEDAWARENESS", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( REMOVE,				1,	ValidateRemoveMsg,	MSG_HANDLER( AINavMeshLinkAbstract, HandleRemoveMsg ),	"REMOVE", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS_HANDLER( AINavMeshLinkAbstract, GameBase, 0, MSG_HANDLER( AINavMeshLinkAbstract, HandleAllMsgs ) )


#define NMLINK_NEIGHBOR_A	0
#define NMLINK_NEIGHBOR_B	1

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkAbstract::AINavMeshLinkAbstract()
{
	m_eNMLinkID = kNMLink_Invalid;
	m_eNMPolyID = kNMPoly_Invalid;
	m_nSmartObjectID = (uint8)-1;
	m_bLinkActive = true;
	m_bLinkEnabled = true;
	m_bTraversalTimedOut = false;
	m_fNextTraversalTime = 0.f;
	m_fNextPreferredTime = 0.f;
	m_fPreferredDelay = 0.f;
	m_eEnabledAwarenessMod = kAwarenessMod_Invalid;
	m_eMinEnabledAwareness = kAware_Relaxed;
	m_eMaxEnabledAwareness = kAware_Alert;
	m_eMinActiveAwareness = kAware_Relaxed;
	m_eMaxActiveAwareness = kAware_Alert;
	m_cLinkBounds = 0;
	m_pvLinkBounds = NULL;
	m_fLinkDistXZ = 0.f;
	m_fEntryOffsetDistA = 0.f;
	m_fEntryOffsetDistB = 0.f;
	m_fExitOffsetDistA = 0.f;
	m_fExitOffsetDistB = 0.f;
	m_fFloorTop = 0.f;
	m_fFloorBottom = 0.f;
	m_hReservingAI = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::EngineMessageFn
//              
//	PURPOSE:	Handle engine messages.
//              
//----------------------------------------------------------------------------

uint32 AINavMeshLinkAbstract::EngineMessageFn(uint32 messageID, void *pv, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pv, fData);

			if ( (int)fData == PRECREATE_WORLDFILE || (int)fData == PRECREATE_STRINGPROP )
			{
				ObjectCreateStruct* pocs = (ObjectCreateStruct*)pv;
				ReadProp(&pocs->m_cProperties);

				// Ensure the object will never be sent to the client.
				pocs->m_Flags = FLAG_NOTINWORLDTREE;
			}

			return dwRet;
		}
		break;

	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pv);
		}
		break;

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pv);
		}
		break;

	case MID_INITIALUPDATE:
		{
			if( g_pAINavMesh )
			{
				g_pAINavMesh->AddAINavMeshLink( this );
			}

			SetNextUpdate( UPDATE_NEVER );
		}
		break;

	case MID_ALLOBJECTSCREATED:
		{
			InitialUpdate();
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pv, fData);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstract::ReadProp(const GenericPropList *pProps)
{
	const char* pszPropString = pProps->GetString( "Name", "" );
	if( pszPropString[0] )
	{
		m_strName = pszPropString;
	}

	// Read the NavMeshLinkID assigned to this link.

	float f = pProps->GetReal( "AINavMeshLinkID", (float)m_eNMLinkID );
	m_eNMLinkID = ( ENUM_NMLinkID )( int )f;

	// Read SmartObject type.

	AIDB_SmartObjectRecord* pSmartObject = NULL;
	pszPropString = pProps->GetString( "SmartObject", "" );
	if( pszPropString[0] )
	{
		m_nSmartObjectID = g_pAIDB->GetAISmartObjectRecordID( pszPropString );
		AIASSERT( m_nSmartObjectID != kAISmartObjectID_Invalid, m_hObject, "AINavMeshLinkAbstract::ReadProp: SmartObject is NULL.");
	}

	// Read EnabledAwarenessMod

	pszPropString = pProps->GetString( "RequiredAwarenessMod", "" );
	if( pszPropString[0] )
	{
		m_eEnabledAwarenessMod = StringToAwarenessMod( pszPropString );
	}

	// Read MinEnabledAwareness.

	pszPropString = pProps->GetString( "MinEnabledAwareness", "Relaxed" );
	if( pszPropString[0] )
	{
		m_eMinEnabledAwareness = StringToAwareness( pszPropString );
	}

	// Read MaxEnabledAwareness.

	pszPropString = pProps->GetString( "MaxEnabledAwareness", "Alert" );
	if( pszPropString[0] )
	{
		m_eMaxEnabledAwareness = StringToAwareness( pszPropString );
	}

	// Read MinActiveAwareness.

	pszPropString = pProps->GetString( "MinActiveAwareness", "Relaxed" );
	if( pszPropString[0] )
	{
		m_eMinActiveAwareness = StringToAwareness( pszPropString );
	}

	// Read MaxActiveAwareness.

	pszPropString = pProps->GetString( "MaxActiveAwareness", "Alert" );
	if( pszPropString[0] )
	{
		m_eMaxActiveAwareness = StringToAwareness( pszPropString );
	}
	
	// Read Active flag.

	m_bLinkActive = pProps->GetBool( "Active", m_bLinkActive );

	// Read StartDisabled flag.

	m_bLinkEnabled = !pProps->GetBool( "StartDisabled", !m_bLinkEnabled );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the NavMeshLinkAbstract 
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstract::Save(ILTMessage_Write *pMsg)
{
	SAVE_STDSTRING(m_strName);

	SAVE_INT(m_eNMLinkID);
	SAVE_INT(m_eNMPolyID);

	std::string strSmartObject;
	strSmartObject = g_pAIDB->GetAISmartObjectRecordName( (ENUM_AISmartObjectID)m_nSmartObjectID );
	SAVE_STDSTRING( strSmartObject );

	SAVE_bool(m_bLinkActive);
	SAVE_bool(m_bLinkEnabled);
	SAVE_bool(m_bTraversalTimedOut);
	SAVE_TIME(m_fNextTraversalTime);
	SAVE_TIME(m_fNextPreferredTime);
	SAVE_FLOAT(m_fPreferredDelay);
	SAVE_DWORD(m_eEnabledAwarenessMod);
	SAVE_DWORD(m_eMinEnabledAwareness);
	SAVE_DWORD(m_eMaxEnabledAwareness);
	SAVE_DWORD(m_eMinActiveAwareness);
	SAVE_DWORD(m_eMaxActiveAwareness);

	// Don't save:
	// m_cLinkBounds
	// m_pvLinkBounds;

	SAVE_VECTOR(m_vLinkEdgeA0);
	SAVE_VECTOR(m_vLinkEdgeA1);
	SAVE_VECTOR(m_vMidPtLinkEdgeA);

	SAVE_VECTOR(m_vLinkEdgeB0);
	SAVE_VECTOR(m_vLinkEdgeB1);
	SAVE_VECTOR(m_vMidPtLinkEdgeB);

	SAVE_VECTOR(m_vLinkDirXZ);
	SAVE_FLOAT(m_fLinkDistXZ);

	SAVE_FLOAT(m_fEntryOffsetDistA);
	SAVE_FLOAT(m_fEntryOffsetDistB);	

	SAVE_FLOAT(m_fExitOffsetDistA);
	SAVE_FLOAT(m_fExitOffsetDistB);	

	SAVE_FLOAT(m_fFloorTop);
	SAVE_FLOAT(m_fFloorBottom);

	SAVE_HOBJECT(m_hReservingAI);
}

void AINavMeshLinkAbstract::Load(ILTMessage_Read *pMsg)
{
	LOAD_STDSTRING(m_strName);

	LOAD_INT_CAST(m_eNMLinkID, ENUM_NMLinkID);
	LOAD_INT_CAST(m_eNMPolyID, ENUM_NMPolyID);

	std::string strSmartObject;
	LOAD_STDSTRING( strSmartObject );
	m_nSmartObjectID = g_pAIDB->GetAISmartObjectRecordID( strSmartObject.c_str() );

	LOAD_bool(m_bLinkActive);
	LOAD_bool(m_bLinkEnabled);
	LOAD_bool(m_bTraversalTimedOut);
	LOAD_TIME(m_fNextTraversalTime);
	LOAD_TIME(m_fNextPreferredTime);
	LOAD_FLOAT(m_fPreferredDelay);
	LOAD_DWORD_CAST(m_eEnabledAwarenessMod, EnumAIAwarenessMod);
	LOAD_DWORD_CAST(m_eMinEnabledAwareness, EnumAIAwareness);
	LOAD_DWORD_CAST(m_eMaxEnabledAwareness, EnumAIAwareness);
	LOAD_DWORD_CAST(m_eMinActiveAwareness, EnumAIAwareness);
	LOAD_DWORD_CAST(m_eMaxActiveAwareness, EnumAIAwareness);

	// Don't load:
	// m_cLinkBounds
	// m_pvLinkBounds;

	LOAD_VECTOR(m_vLinkEdgeA0);
	LOAD_VECTOR(m_vLinkEdgeA1);
	LOAD_VECTOR(m_vMidPtLinkEdgeA);

	LOAD_VECTOR(m_vLinkEdgeB0);
	LOAD_VECTOR(m_vLinkEdgeB1);
	LOAD_VECTOR(m_vMidPtLinkEdgeB);

	LOAD_VECTOR(m_vLinkDirXZ);
	LOAD_FLOAT(m_fLinkDistXZ);

	LOAD_FLOAT(m_fEntryOffsetDistA);
	LOAD_FLOAT(m_fEntryOffsetDistB);

	LOAD_FLOAT(m_fExitOffsetDistA);
	LOAD_FLOAT(m_fExitOffsetDistB);

	LOAD_FLOAT(m_fFloorTop);
	LOAD_FLOAT(m_fFloorBottom);

	LOAD_HOBJECT(m_hReservingAI);

	if( g_pAINavMesh )
	{
		g_pAINavMesh->AddAINavMeshLink( this );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::InitialUpdate
//              
//	PURPOSE:	Add the NavMeshLink to the NavMesh.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstract::InitialUpdate()
{
	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( pSmartObject )
	{
		// Set entry offset distances from the SmartObject.

		m_fEntryOffsetDistA = pSmartObject->fEntryOffsetDistA;
		m_fEntryOffsetDistB = pSmartObject->fEntryOffsetDistB;

		m_fExitOffsetDistA = pSmartObject->fExitOffsetDistA;
		m_fExitOffsetDistB = pSmartObject->fExitOffsetDistB;

		// Find the true top and bottom of the link.

		if( pSmartObject->fFindFloorOffset != 0.f )
		{
			m_fFloorTop = FindFloor( m_vMidPtLinkEdgeA, -m_vLinkDirXZ, pSmartObject->fFindFloorOffset );
			m_fFloorBottom = FindFloor( m_vMidPtLinkEdgeB, m_vLinkDirXZ, pSmartObject->fFindFloorOffset );
		}
		else {
			m_fFloorTop = m_vMidPtLinkEdgeA.y;
			m_fFloorBottom = m_vMidPtLinkEdgeB.y;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::FindFloor
//              
//	PURPOSE:	Return the height of the floor under some position.
//              
//----------------------------------------------------------------------------

float AINavMeshLinkAbstract::FindFloor( const LTVector& vPos, const LTVector& vDir, float fOffset )
{
	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos + ( vDir * fOffset );
	IQuery.m_To = IQuery.m_From;
	IQuery.m_To.y -= 10000.f;

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = GroundFilterFn;

	if( g_pLTServer->IntersectSegment(IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject) ) ) )
	{
		return IInfo.m_Point.y;
	}

	return vPos.y;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::SetNMLinkBounds
//              
//	PURPOSE:	Use the poly from WorldEdit to set the bounds of the Link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstract::SetNMLinkBounds( uint32 cBounds, LTVector* pvBounds )
{
	// Sanity check.

	if( !pvBounds )
	{
		return;
	}

	m_cLinkBounds = cBounds;
	m_pvLinkBounds = pvBounds;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::SetupNMLinkEdges
//              
//	PURPOSE:	Setup the LinkEdges based on the original bounds.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstract::SetupNMLinkEdges()
{
	// Find the two edges of the original link boundaries that
	// connect to other polys.

	bool bFoundLinkEdge;
	int cLinkEdges = 0;
	LTVector v0, v1, vMidPt;
	CAINavMeshEdge* pEdge;
	for( int iVert=0; iVert < m_cLinkBounds; ++iVert )
	{
		v0 = m_pvLinkBounds[iVert];
		v1 = m_pvLinkBounds[( iVert + 1 ) % m_cLinkBounds];

		// Find an edge of the NMPoly that has these 
		// verts as endpoints.

		pEdge = FindMatchingPolyEdge( v0, v1 );
		bFoundLinkEdge = false;

		// No match was found, so the original edge was split.
		// This means that the edge does have a neighbor,
		// so consider it a LinkEdge.

		if( !pEdge )
		{
			bFoundLinkEdge = true;
			++cLinkEdges;
		}

		// A match was found, so the original edge was not split.
		// Consider it a LinkEdge if it has two neighbors.

		else if( ( pEdge->GetNMPolyIDA() != kNMPoly_Invalid ) &&
				 ( pEdge->GetNMPolyIDB() != kNMPoly_Invalid ) )
		{
			bFoundLinkEdge = true;
			++cLinkEdges;
		}

		// Found a link edge.

		if(	bFoundLinkEdge )
		{
			// Found LinkEdgeA.

			if( cLinkEdges == 1 )
			{
				m_vLinkEdgeA0 = v0;
				m_vLinkEdgeA1 = v1;
			}

			// Found LinkEdgeB. We're done.

			else if( cLinkEdges == 2 )
			{
				m_vLinkEdgeB0 = v0;
				m_vLinkEdgeB1 = v1;
				break;
			}
		}
	}

	// Bail if link is only attached to one poly!

	if( cLinkEdges < 2 )
	{
		return;
	}

	// Find the mid-points of the LinkEdges.

	m_vMidPtLinkEdgeA = ( m_vLinkEdgeA0 + m_vLinkEdgeA1 ) * 0.5f;
	m_vMidPtLinkEdgeB = ( m_vLinkEdgeB0 + m_vLinkEdgeB1 ) * 0.5f;

	// Always force LinkEdgeA to be the highest LinkEdge.

	if( m_vMidPtLinkEdgeA.y < m_vMidPtLinkEdgeB.y )
	{
		// Swap LinkEdgeA and LinkEdgeB.

		v0 = m_vLinkEdgeA0;
		v1 = m_vLinkEdgeA1;
		vMidPt = m_vMidPtLinkEdgeA;

		m_vLinkEdgeA0 = m_vLinkEdgeB0;
		m_vLinkEdgeA1 = m_vLinkEdgeB1;
		m_vMidPtLinkEdgeA = m_vMidPtLinkEdgeB;

		m_vLinkEdgeB0 = v0;
		m_vLinkEdgeB1 = v1;
		m_vMidPtLinkEdgeB = vMidPt;
	}

	// Find the distance and direction from LinkEdge A to B.

	m_vLinkDirXZ = m_vMidPtLinkEdgeB - m_vMidPtLinkEdgeA;
	m_vLinkDirXZ.y = 0.f;
	m_fLinkDistXZ = m_vLinkDirXZ.Mag();
	m_vLinkDirXZ.Normalize();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::FindMatchingPolyEdge
//              
//	PURPOSE:	Return NMPolyEdge that has specified verts as endpoints.
//              
//----------------------------------------------------------------------------

CAINavMeshEdge* AINavMeshLinkAbstract::FindMatchingPolyEdge( const LTVector& v0, const LTVector& v1 )
{
	// Bail if can't find the NavMesh poly associated with this Link.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return NULL;
	}

	int iEdge;
	int cEdges = pPoly->GetNumNMPolyEdges();

	// Iterate over edges searching for match.

	CAINavMeshEdge* pEdge;
	for( iEdge=0; iEdge < cEdges; ++iEdge )
	{
		pEdge = pPoly->GetNMPolyEdge( iEdge );
		if( !pEdge )
		{
			continue;
		}

		// Found a match.

		// Added a small delta to the comparision, as we were getting
		// false results using floating point comparisions when the 
		// values were large.
		if( ( pEdge->GetNMEdge0().NearlyEquals(v0, 0.001f) ) &&
			( pEdge->GetNMEdge1().NearlyEquals(v1, 0.001f) ) )
		{
			return pEdge;
		}

		// Found a match (vert order reversed).

		if( ( pEdge->GetNMEdge0().NearlyEquals(v1, 0.001f) ) &&
			( pEdge->GetNMEdge1().NearlyEquals(v0, 0.001f) ) )
		{
			return pEdge;
		}
	}

	// No match found.

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::GetNumNMLinkNeighbors
//              
//	PURPOSE:	Return true if we set the number of pathfinding 
//              node neighbors of this link.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::GetNumNMLinkNeighbors( int* pcNeighbors )
{
	// No entry offsets.

	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	// There are two neighbors: the poly containing
	// the point that is x units in front of LinkEdgeA,
	// and the poly that is x units in front of LinkEdgeB.

	*pcNeighbors = 2;
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::GetNMLinkNeighborAtEdge
//              
//	PURPOSE:	Return true if we found a pointer to the NavMesh poly 
//              neighbor at a specified index.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::GetNMLinkNeighborAtEdge( CAIAStarMapNavMesh* pMap, int iNeighbor, CAINavMeshPoly* pNMPolyParent, CAINavMeshPoly*& pNMPolyNeighbor )
{
	// No entry offsets.

	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	// Links with EntryOffsets have 2 neighbors.

	if( iNeighbor > NMLINK_NEIGHBOR_B )
	{
		return false;
	}

	// Find the last path position on the stairs.

	LTVector vLastPos;
	if( pNMPolyParent )
	{
		CAINavMeshEdge*	pEdge = pNMPolyParent->GetNMPolyNeighborEdge( m_eNMPolyID );
		if( !pEdge )
		{
			vLastPos = pNMPolyParent->GetNMPolyCenter();
		}
		else
		{
			vLastPos = pEdge->GetNMEdgeMidPt();
		}
	}
	else {
		vLastPos = pMap->GetPathSource();
	}

	// Find the offset entry position to enter the stairs from.

	LTVector vOffsetPos;
	if( !GetNeighborOffsetEntryPos( iNeighbor, vLastPos, &vOffsetPos ) )
	{
		return false;
	}

	// Find the NavMesh poly that contains this point.

	ENUM_NMPolyID eNeighborPoly = g_pAIQuadTree->GetContainingNMPoly( vOffsetPos, pMap->GetCharTypeMask(), kNMPoly_Invalid );

	// Return the neighbor.

	pNMPolyNeighbor = g_pAINavMesh->GetNMPoly( eNeighborPoly );
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::GetNMLinkOffsetEntryPos
//              
//	PURPOSE:	Return true if an entry position can be found.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::GetNMLinkOffsetEntryPos( ENUM_NMPolyID ePolyDest, const LTVector& vSourcePos, LTVector* pvOffset )
{
	// No entry offsets.

	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	// Bail if dest poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePolyDest );
	if( !pPoly )
	{
		return false;
	}

	// Determine which LinkEdge is closer to the dest poly.

	int iNeighbor = NMLINK_NEIGHBOR_A;
	if( pPoly->GetNMPolyCenter().DistSqr( m_vMidPtLinkEdgeA ) >
		pPoly->GetNMPolyCenter().DistSqr( m_vMidPtLinkEdgeB ) )
	{
		iNeighbor = NMLINK_NEIGHBOR_B;
	}

	// Bail if no entry pos found.

	if( !GetNeighborOffsetEntryPos( iNeighbor, vSourcePos, pvOffset ) )
	{
		return false;
	}

	// Found the entry pos.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::GetNeighborOffsetEntryPos
//              
//	PURPOSE:	Return true if an offset entry position is found.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::GetNeighborOffsetEntryPos( int iNeighbor, const LTVector& vSourcePos, LTVector* pvOffset )
{
	// No entry offsets.

	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	// Sanity check.

	if( !pvOffset )
	{
		return false;
	}

	// Find the intersection point with the top or bottom of the stairs.

	LTVector vIntersect;
	LTVector vLineSeg0, vLineSeg1;
	LTVector vRay0, vRay1;

	if( iNeighbor == NMLINK_NEIGHBOR_A )
	{
		vLineSeg0 = m_vLinkEdgeA0;
		vLineSeg1 = m_vLinkEdgeA1;

		vRay0 = vSourcePos;
		vRay0.y = m_vMidPtLinkEdgeA.y;
		vRay1 = vRay0 - ( 5000.f * m_vLinkDirXZ );
	}
	else {
		vLineSeg0 = m_vLinkEdgeB0;
		vLineSeg1 = m_vLinkEdgeB1;

		vRay0 = vSourcePos;
		vRay0.y = m_vMidPtLinkEdgeB.y;
		vRay1 = vRay0 + ( 5000.f * m_vLinkDirXZ );
	}

	if( kRayIntersect_Failure == RayIntersectLineSegment( vLineSeg0, vLineSeg1, vRay0, vRay1, false, &vIntersect ) )
	{
		return false;
	}

	// Find the offset position to enter the stairs from.

	if( iNeighbor == NMLINK_NEIGHBOR_A )
	{
		*pvOffset = vIntersect - ( GetNMLinkOffsetEntryDistA() * m_vLinkDirXZ );
	}
	else {
		*pvOffset = vIntersect + ( GetNMLinkOffsetEntryDistB() * m_vLinkDirXZ );
	}
	pvOffset->y += 10.f;

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::IsInLinkOrOffsetEntry
//              
//	PURPOSE:	Return true if AI is inside the Link of the Link's offset entry.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::IsInLinkOrOffsetEntry( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// In the link.

	if( ( pAI->GetAIBlackBoard()->GetBBEnteringNMLink() == m_eNMLinkID ) ||
		( pAI->GetCurrentNavMeshLink() == m_eNMLinkID ) )
	{
		return true;
	}

	// Not in the link.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkStairs::AllowDynamicMovement
//              
//	PURPOSE:	Return true if the AI may use dynamic movement while in this 
//				link.  Otherwise returns false.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::AllowDynamicMovement( CAI* pAI )
{
	// Allow dynamic movement if the link is not active to the AI.  This 
	// allows AIs to move normally when the link is inactive.

	if ( !IsNMLinkActiveToAI(pAI) )
	{
		return true;
	}

	// By default, do not allow dynamic movement such as sliding.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkStairs::SetNMLinkOffsetEntryPathLink
//              
//	PURPOSE:	Return true if Link sets up the PathLink.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::SetNMLinkOffsetEntryPathLink( SPATH_NODE* pPathNode, CAI* pAI )
{
	// No entry offsets.

	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	// Sanity check.

	if( !pPathNode )
	{
		return false;
	}

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pPathNode->ePoly );
	if( !pPoly )
	{
		return false;
	}

	// Determine which LinkEdge is closer to the dest poly.

	// Closer to the bottom.

	LTVector v0, v1;
	if( pPathNode->vWayPt.DistSqr( m_vMidPtLinkEdgeA ) >
		pPathNode->vWayPt.DistSqr( m_vMidPtLinkEdgeB ) )
	{
		v0 = m_vLinkEdgeB0 + ( m_vLinkDirXZ * GetNMLinkOffsetEntryDistB() );
		v1 = m_vLinkEdgeB1 + ( m_vLinkDirXZ * GetNMLinkOffsetEntryDistB() );
	}

	// Closer to the top.

	else {
		v0 = m_vLinkEdgeA0 - ( m_vLinkDirXZ * GetNMLinkOffsetEntryDistA() );
		v1 = m_vLinkEdgeA1 - ( m_vLinkDirXZ * GetNMLinkOffsetEntryDistA() );
	}

	// Trim PathLink endpoints to containing poly.

	LTVector vIntersect;
	if( pPoly->RayIntersectPoly2D( pPathNode->vWayPt, v0, &vIntersect ) )
	{
		v0 = vIntersect;
		v0.y = pPathNode->vWayPt.y;
	}
	if( pPoly->RayIntersectPoly2D( pPathNode->vWayPt, v1, &vIntersect ) )
	{
		v1 = vIntersect;
		v1.y = pPathNode->vWayPt.y;
	}

	// Set trimmed endpoints.

	pPathNode->vLink0 = v0;
	pPathNode->vLink1 = v1;

	// Reset the way point to the center of the new 
	// offset entry link.

	pPathNode->vWayPt = ( v0 + v1 ) * 0.5f;
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::IsPullStringsModifying
//              
//	PURPOSE:	Function returns true when the pull string may modify the 
//				path, false when it promises not to.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::IsPullStringsModifying( CAI* pAI )
{
	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkStairs::PullStrings
//              
//	PURPOSE:	Return true if the Link adjusts the path waypoint.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::PullStrings( const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos )
{
	// No entry offsets.

	if( ( GetNMLinkOffsetEntryDistA() == 0.f ) &&
		( GetNMLinkOffsetEntryDistB() == 0.f ) )
	{
		return false;
	}

	// Sanity check.

	if( !pvNewPos )
	{
		return false;
	}

	LTVector vRay;
	LTVector vL0, vL1;
	LTVector vIntersect;

	// Determine which LinkEdge is closer to the dest.

	// Closer to the bottom.

	if( pvNewPos->DistSqr( m_vMidPtLinkEdgeA ) >
		pvNewPos->DistSqr( m_vMidPtLinkEdgeB ) )
	{
		vL0 = m_vLinkEdgeB0;
		vL1 = m_vLinkEdgeB1;
		vRay = vPtPrev + ( m_vLinkDirXZ * 5000.f );
	}

	// Closer to the top.

	else {
		vL0 = m_vLinkEdgeA0;
		vL1 = m_vLinkEdgeA1;
		vRay = vPtPrev - ( m_vLinkDirXZ * 5000.f );
	}

	// Found a new point on the LinkEdge.

	if( kRayIntersect_Failure != RayIntersectLineSegment( vL0, vL1, vPtPrev, vRay, false, &vIntersect ) )
	{
		*pvNewPos = vIntersect;
		return true;
	}

	// No intersection found.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::IsNMLinkActiveToAI
//              
//	PURPOSE:	Return true if the link is active for an AI.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::IsNMLinkActiveToAI( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Link is active to AI.

	if( m_bLinkActive && 
		( pAI->GetAIBlackBoard()->GetBBAwareness() >= m_eMinActiveAwareness ) &&
		( pAI->GetAIBlackBoard()->GetBBAwareness() <= m_eMaxActiveAwareness ) )
	{
		return true;
	}

	// Link is not active to AI.

	return false; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::IsNMLinkEnabledToAI
//              
//	PURPOSE:	Return true if the link is enabled for an AI.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::IsNMLinkEnabledToAI( CAI* pAI, bool bCheckTimeout )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Link is timed-out.

	if( bCheckTimeout && m_bTraversalTimedOut )
	{
		// Link is still timed out.

		if( m_bLinkActive && ( g_pLTServer->GetTime() < m_fNextTraversalTime ) )
		{
			return false;
		}

		// Timeout has expired.

		m_bTraversalTimedOut = false;

		// All AIs need to clear existing knowledge of paths,
		// because enabling and disabling NavMeshLinks changes the connectivity.

		g_pAIPathMgrNavMesh->InvalidatePathKnowledge( pAI->m_hObject );
	}

	// Link is not enabled.

	if( !m_bLinkEnabled  )
	{
		return false;
	}

	// Awareness is too low or too high.

	if( ( pAI->GetAIBlackBoard()->GetBBAwareness() < m_eMinEnabledAwareness ) ||
		( pAI->GetAIBlackBoard()->GetBBAwareness() > m_eMaxEnabledAwareness ) )
	{
		return false;
	}

	// Awareness mod is required and incorrect.

	if ( m_eEnabledAwarenessMod != kAwarenessMod_Invalid 
		&& m_eEnabledAwarenessMod != pAI->GetAIBlackBoard()->GetBBAwarenessMod() )
	{
		return false;
	}

	// Link is enabled to AI.

	return true; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::GetSmartObject
//              
//	PURPOSE:	Return a pointer to the link's SmartObject.
//              
//----------------------------------------------------------------------------

AIDB_SmartObjectRecord* AINavMeshLinkAbstract::GetSmartObject()
{
	return g_pAIDB->GetAISmartObjectRecord( m_nSmartObjectID );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::ActivateTraversal
//              
//	PURPOSE:	Setup AI to traverse the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkAbstract::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( !pSmartObject )
	{
		return;
	}
	
	// Timeout.

	if( pSmartObject->fTimeout > 0.f )
	{
		// No one may traverse this link for some timeout.

		m_bTraversalTimedOut = true;
		m_fNextTraversalTime = g_pLTServer->GetTime() + pSmartObject->fTimeout;

		// All AIs need to clear existing knowledge of paths,
		// because enabling and disabling NavMeshLinks changes the connectivity.

		g_pAIPathMgrNavMesh->InvalidatePathKnowledge( pAI->m_hObject );

		// Invalidate path knowledge again when the timeout expires.

		g_pAIPathMgrNavMesh->PostPathKnowledgeInvalidationRequest( pAI->m_hObject, m_fNextTraversalTime );
	}

	// Unpreferred.

	if( pSmartObject->fUnpreferredTime > 0.f )
	{
		m_fNextPreferredTime = g_pLTServer->GetTime() + pSmartObject->fUnpreferredTime;
		m_fPreferredDelay = pSmartObject->fUnpreferredTime;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::IsLinkValid
//              
//	PURPOSE:	Return true if link is currently valid for this AI.
//
//----------------------------------------------------------------------------

bool AINavMeshLinkAbstract::IsLinkValid( CAI* pAI, EnumAIActionType eActionType, bool bTraversalInProgress )
{
	// Link is timed-out.

	if( m_bTraversalTimedOut && !bTraversalInProgress )
	{
		return false;
	}

	// Link is valid.

	return true; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINavMeshLinkAbstract::IsLinkPassable
//
//	PURPOSE:	Return the true if link is passable to AI.
//
// ----------------------------------------------------------------------- //

bool AINavMeshLinkAbstract::IsLinkPassable( CAI* pAI, ENUM_NMPolyID ePolyTo )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// By default, links are not passable to limping AI.

	if( pAI->GetAIBlackBoard()->GetBBAwarenessMod() == kAwarenessMod_Injured )
	{
		return false;
	}

	// If a required ActionAbility is specified, verify that the AI has an 
	// action with this ability.  If he does not, this link cannot be used.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( pSmartObject )
	{
		if ( kActionAbility_InvalidType != pSmartObject->eActionAbilityRequired )
		{
			if ( g_pAIActionMgr->ActionSetSupportsAbility(
				pAI->GetAIBlackBoard()->GetBBAIActionSet(),
				pSmartObject->eActionAbilityRequired ) )
			{
				return false;
			}
		}
	}

	// Link is passable.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINavMeshLinkAbstract::GetNMLinkPathingWeight
//
//	PURPOSE:	Return the multiplier applied to pathfinding with this link.
//
// ----------------------------------------------------------------------- //

float AINavMeshLinkAbstract::GetNMLinkPathingWeight(CAI* pAI)
{
	// Sanity check.

	if( !pAI )
	{
		return 1.f;
	}

	// Do not add extra weight to the poly we are standing in!

	if( pAI->GetCurrentNavMeshPoly() == m_eNMPolyID )
	{
		return 1.f;
	}

	// No SmartObject exists to define the weight.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( !pSmartObject )
	{
		return 1.f;
	}

	// Weight unpreferred links much more heavily.

	double fCurTime = g_pLTServer->GetTime();
	if( fCurTime < m_fNextPreferredTime )
	{
		// Weight decreases linearly over time.

		float fInterpFactor = ((float)( m_fNextPreferredTime - fCurTime )) / m_fPreferredDelay;
		return LTMAX( 1.f, 10000.f * fInterpFactor );
	}

	// Weight reserved links much more heavily,
	// if the link is reserved by someone else.

	if( m_hReservingAI && 
		( m_hReservingAI != pAI->m_hObject ) &&
		( !IsDeadAI( m_hReservingAI ) ) )
	{
		return 1000.f;
	}

	// Default weight.

	return 1.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINavMeshLinkAbstract::ReserveNMLink
//
//	PURPOSE:	Reserve a NavMesh link, or clear reservation.
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::ReserveNMLink( CAI* pAI, bool bReserve )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Reserve the link.
	// Only reserve a link if the blackboard directs AI to reserve links 
	// (set by the Goal), or the SmartObject has an UnpreferredTime.

	if( bReserve )
	{
		// Never reserve a link that has no SmartObject.
		// Base links and LoseTarget links should not affect the chosen route.

		AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
		if( !pSmartObject )
		{
			return;
		}

		if( ( pAI->GetAIBlackBoard()->GetBBReserveNMLinks() ) ||
		    ( pSmartObject->fUnpreferredTime > 0.f ) )
		{
			m_hReservingAI = pAI->m_hObject;
		}
	}
	
	// Clear the reservation.

	else if( pAI->m_hObject == m_hReservingAI )
	{
		m_hReservingAI = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINavMeshLinkAbstract::ApplyNMLinkDeathDelay
//
//	PURPOSE:	Apply delay if AI just died while using this link for pathfinding.
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::ApplyNMLinkDeathDelay( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Never apply delay to a link that has no SmartObject.
	// Base links and LoseTarget links should not affect the chosen route.

	AIDB_SmartObjectRecord* pSmartObject = GetSmartObject();
	if( !pSmartObject )
	{
		return;
	}

	// AI should prefer not to use links along path where someone 
	// was recently killed.

	float fDeathDelay = g_pAIDB->GetAIConstantsRecord()->fNavMeshLinkDeathDelay;
	if( fDeathDelay > 0.f )
	{
		double fNext = g_pLTServer->GetTime() + fDeathDelay;
		if( fNext > m_fNextPreferredTime )
		{
			m_fNextPreferredTime = fNext;
			m_fPreferredDelay = fDeathDelay;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINavMeshLinkAbstract::HandleAllMsgs
//
//	PURPOSE:	Handle all commands.
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleAllMsgs( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	g_vtAIConsoleVar.Init( g_pLTServer, "AIShowMessages", NULL, 0.0f );
	if( g_vtAIConsoleVar.GetFloat() <= 0.0f )
	{
		return;
	}

	switch( crParsedMsg.GetArgCount() )
	{
		case 1: ObjectCPrint( GetName(), "Received Msg: %s", crParsedMsg.GetArg( 0 ).c_str() );
			break;
		case 2: ObjectCPrint( GetName(), "Received Msg: %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str() );
			break;
		case 3: ObjectCPrint( GetName(), "Received Msg: %s %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str() );
			break;
		case 4: ObjectCPrint( GetName(), "Received Msg: %s %s %s %s", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str(), crParsedMsg.GetArg( 3 ).c_str() );
			break;
		default: ObjectCPrint( GetName(), "Received Msg: %s %s %s %s ...", crParsedMsg.GetArg( 0 ).c_str(), crParsedMsg.GetArg( 1 ).c_str(), crParsedMsg.GetArg( 2 ).c_str(), crParsedMsg.GetArg( 3 ).c_str() );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleActiveMsg
//
//  PURPOSE:	Handle an ACTIVE message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleActiveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Activate or deactivate the link.

		const char* pszArg = crParsedMsg.GetArg(1);	
		SetNMLinkActive( IsTrueChar( pszArg[0] ) );

		// Clear timeout if link is deactivated.

		if( m_bTraversalTimedOut && !m_bLinkActive )
		{
			// Clear timeout.

			m_bTraversalTimedOut = false;

			// All AIs need to clear existing knowledge of paths,
			// because enabling and disabling NavMeshLinks changes the connectivity.

			g_pAIPathMgrNavMesh->InvalidatePathKnowledge( hSender );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleDisableMsg
//
//  PURPOSE:	Handle an DISABLE message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleDisableMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Disable the link.

	SetNMLinkEnabled( false );

	// All AIs need to clear existing knowledge of paths,
	// because enabling and disabling NavMeshLinks changes the connectivity.

	g_pAIPathMgrNavMesh->InvalidatePathKnowledge( hSender );

	// If the nav mesh is currently drawing, this poly needs to be redrawn 
	// as its enabled status changed.
	if (g_pAINavMesh->IsDrawing())
	{
		g_pAINavMesh->RedrawPoly(m_eNMPolyID);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleEnableMsg
//
//  PURPOSE:	Handle an ENABLE message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleEnableMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
  	// Enable the link.

	SetNMLinkEnabled( true );

	// All AIs need to clear existing knowledge of paths,
	// because enabling and disabling NavMeshLinks changes the connectivity.

	g_pAIPathMgrNavMesh->InvalidatePathKnowledge( hSender );

	// If the nav mesh is currently drawing, this poly needs to be redrawn 
	// as its enabled status changed.
	if (g_pAINavMesh->IsDrawing())
	{
		g_pAINavMesh->RedrawPoly(m_eNMPolyID);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleMinActiveAwarenessMsg
//
//  PURPOSE:	Handle an MINACTIVEAWARENESS message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleMinActiveAwarenessMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_eMinActiveAwareness = StringToAwareness( crParsedMsg.GetArg(1) );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleMaxActiveAwarenessMsg
//
//  PURPOSE:	Handle an MAXACTIVEAWARENESS message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleMaxActiveAwarenessMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_eMaxActiveAwareness = StringToAwareness( crParsedMsg.GetArg(1) );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleMinEnabledAwarenessMsg
//
//  PURPOSE:	Handle an MINENABLEDAWARENESS message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleMinEnabledAwarenessMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_eMinEnabledAwareness = StringToAwareness( crParsedMsg.GetArg(1) );

		// All AIs need to clear existing knowledge of paths,
		// because enabling and disabling NavMeshLinks changes the connectivity.

		g_pAIPathMgrNavMesh->InvalidatePathKnowledge( hSender );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleMaxEnabledAwarenessMsg
//
//  PURPOSE:	Handle an MAXENABLEDAWARENESS message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleMaxEnabledAwarenessMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		m_eMaxEnabledAwareness = StringToAwareness( crParsedMsg.GetArg(1) );

		// All AIs need to clear existing knowledge of paths,
		// because enabling and disabling NavMeshLinks changes the connectivity.

		g_pAIPathMgrNavMesh->InvalidatePathKnowledge( hSender );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINavMeshLinkAbstract::HandleRemoveMsg
//
//  PURPOSE:	Handle a REMOVE message...
//
// ----------------------------------------------------------------------- //

void AINavMeshLinkAbstract::HandleRemoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	AIError( "Attempting to remove AINavMeshLink \"%s\"! Disabling instead.", GetName() );
	HandleDisableMsg( hSender, crParsedMsg );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// AINavMeshLinkAbstractPlugin
//

LTRESULT AINavMeshLinkAbstractPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !LTStrICmp("SmartObject", szPropName) )
	{
		uint32 cSmartObjects = g_pAIDB->GetNumAISmartObjectRecords();
		for(uint32 iSmartObject=0; iSmartObject < cSmartObjects; ++iSmartObject)
		{
			// Out of space to add more strings.

			if ( (*pcStrings) + 1 == cMaxStrings )
			{
				break;
			}

			// If the currently indexed smartobject is a link, add it

			const AIDB_SmartObjectRecord* pSmartObject = g_pAIDB->GetAISmartObjectRecord(iSmartObject);
			if ( !pSmartObject )
			{
				continue;
			}

			if( ( pSmartObject->eNodeType == GetSmartObjectFilterType() ) ||
				( pSmartObject->eNodeType == kNode_NavMeshLink ) ||
				( pSmartObject->eNodeType == kNode_Base ) )
			{
				LTStrCpy(aszStrings[(*pcStrings)++], pSmartObject->strName.c_str(), cMaxStringLength);
			}
		}

		// Alphabetize the strings, skipping the 'None' entry which is always first.

		if (*pcStrings > 1)
		{
			qsort( aszStrings+1, (*pcStrings)-1, sizeof( char * ), CaseInsensitiveCompare );		
		}
		
		return LT_OK;
	}

	if( ( !LTStrICmp("MinEnabledAwareness", szPropName) ) ||
		( !LTStrICmp("MaxEnabledAwareness", szPropName) ) ||
		( !LTStrICmp("MinActiveAwareness", szPropName) ) ||
		( !LTStrICmp("MaxActiveAwareness", szPropName) ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "Relaxed" );
		strcpy( aszStrings[(*pcStrings)++], "Suspicious" );
		strcpy( aszStrings[(*pcStrings)++], "Alert" );

		return LT_OK;
	}

	if( !LTStrICmp("RequiredAwarenessMod", szPropName) )
	{
		strcpy( aszStrings[(*pcStrings)++], "None" );

		for(uint32 iAwarenessMod=0; iAwarenessMod < kAwarenessMod_Count; ++iAwarenessMod)
		{
			strcpy( aszStrings[(*pcStrings)++], s_aszAIAwarenessMods[iAwarenessMod] );
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
