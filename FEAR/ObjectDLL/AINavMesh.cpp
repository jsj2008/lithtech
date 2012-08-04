// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMesh.cpp
//
// PURPOSE : AI NavMesh class implementation.
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMesh.h"
#include "AIClassFactory.h"
#include "AIUtils.h"
#include "AINavMeshGenTypes.h"
#include "AINavMeshLinkAbstract.h"
#include "AIRegion.h"
#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"
#include "DebugLineSystem.h"
#include <algorithm>
#include "AINode.h"

#include "CharacterDB.h"

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

#define AINAVMESH_BLINDOBJECTID			0x83f47c31		// ID for AINavMesh blind object data
#define INVALID_NAV_MESH_DATA_INDEX		0xffffffff

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAINavMeshPoly
//

CAINavMeshPoly::CAINavMeshPoly()
{
	m_eNMPolyID = kNMPoly_Invalid;

	m_eNMNormalID = kNMNormal_Invalid;

	m_eNMLinkID = kNMLink_Invalid;

	m_eNMComponentID = kNMComponent_Invalid;

	m_dwNMCharTypeMask = 0;

	m_cNMPolyEdges = 0;
	m_pNMPolyEdgeList = NULL;

	m_cAIRegions = 0;
	m_pAIRegionList = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::FixUpNavMeshPointers()
//              
//	PURPOSE:	Fix-up pointers from raw NavMesh data.
//              
//----------------------------------------------------------------------------

void CAINavMeshPoly::FixUpNavMeshPointers( ENUM_NMEdgeID* pAINavMeshEdgeLists, ENUM_AIRegionID* pAIRegionLists )
{
	// Sanity check.

	if( !( pAINavMeshEdgeLists && pAIRegionLists ) )
	{
		return;
	}

	// Convert the parent NavMesh ID into the character type mask.

	uint32 iNavMesh = m_dwNMCharTypeMask;
	m_dwNMCharTypeMask = g_pAINavMesh->GetNMCharTypeMask( iNavMesh );

	// Convert index to pointer into edge lists.

	uint32 iNMPolyEdgeList = (uint32)m_pNMPolyEdgeList;
	m_pNMPolyEdgeList = pAINavMeshEdgeLists + iNMPolyEdgeList;

	// Convert index to pointer into region lists.

	uint32 iAIRegionList = (uint32)m_pAIRegionList;
	m_pAIRegionList = pAIRegionLists + iAIRegionList;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::GetNMPolyEdge()
//              
//	PURPOSE:	Return pointer to poly edge at specified index.
//              
//----------------------------------------------------------------------------

CAINavMeshEdge*	CAINavMeshPoly::GetNMPolyEdge( int iEdge )
{
	// Bail if index is invalid.

	if( ( iEdge >= 0 ) && ( iEdge < m_cNMPolyEdges ) )
	{
		return g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::GetNMPolyNeighborAtEdge()
//              
//	PURPOSE:	Return pointer to neighbor poly of specified index.
//              
//----------------------------------------------------------------------------

CAINavMeshPoly*	CAINavMeshPoly::GetNMPolyNeighborAtEdge( int iNeighbor )
{
	CAINavMeshEdge* pEdge;
	CAINavMeshPoly*	pNeighborPoly = NULL;

	// Bail if index is invalid.

	if( ( iNeighbor >= 0 ) && ( iNeighbor < m_cNMPolyEdges ) )
	{
		// Find edge with matching neighbor index.

		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iNeighbor] );
		if( pEdge )
		{
			// Determine if edge has a neighbor.

			if( pEdge->GetNMPolyIDA() != m_eNMPolyID )
			{
				pNeighborPoly = g_pAINavMesh->GetNMPoly( pEdge->GetNMPolyIDA() );
			}
			else if( pEdge->GetNMPolyIDB() != m_eNMPolyID )
			{
				pNeighborPoly = g_pAINavMesh->GetNMPoly( pEdge->GetNMPolyIDB() );
			}
		}
	}

	return pNeighborPoly;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::GetNMPolyNeighborEdge()
//              
//	PURPOSE:	Return pointer to the edge between this poly and a specified neighbor.
//              
//----------------------------------------------------------------------------

CAINavMeshEdge*	CAINavMeshPoly::GetNMPolyNeighborEdge( ENUM_NMPolyID eNMPolyNeighbor )
{
	CAINavMeshEdge* pEdge;

	for( int iEdge=0; iEdge < m_cNMPolyEdges; ++iEdge )
	{
		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );
		if( pEdge )
		{
			// Determine if edge has specified neighbor.

			if( ( pEdge->GetNMPolyIDA() == eNMPolyNeighbor ) ||
				( pEdge->GetNMPolyIDB() == eNMPolyNeighbor ) )
			{
				return pEdge;
			}
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::GetAIRegion()
//              
//	PURPOSE:	Returns ID of AIRegion at some index.
//              
//----------------------------------------------------------------------------

ENUM_AIRegionID	CAINavMeshPoly::GetAIRegion( int iAIRegion )
{
	// AIRegion index is out of range.

	if( ( iAIRegion < 0 ) || ( iAIRegion >= m_cAIRegions ) )
	{
		return kAIRegion_Invalid;
	}

	return m_pAIRegionList[iAIRegion];
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::IsContainedByAIRegion()
//              
//	PURPOSE:	Returns true if poly is contained by specified AIRegion.
//              
//----------------------------------------------------------------------------

bool CAINavMeshPoly::IsContainedByAIRegion( ENUM_AIRegionID eAIRegion )
{
	// Sanity check.

	if( ( m_cAIRegions == 0 ) || 
		( eAIRegion == kAIRegion_Invalid ) )
	{
		return false;
	}

	// Found a match.

	for( int iAIRegion=0; iAIRegion < m_cAIRegions; ++iAIRegion )
	{
		if( eAIRegion == m_pAIRegionList[iAIRegion] )
		{
			return true;
		}
	}

	// No match found.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::ContainsPoint2D()
//              
//	PURPOSE:	Returns true if poly contains point.
//              
//----------------------------------------------------------------------------

bool CAINavMeshPoly::ContainsPoint2D( const LTVector& vPos )
{
	// First test the pos against the poly's bounding box.

	if( ( m_aabbNMPolyBounds.vMin.x > vPos.x ) ||
		( m_aabbNMPolyBounds.vMax.x < vPos.x ) ||
		( m_aabbNMPolyBounds.vMin.z > vPos.z ) ||
		( m_aabbNMPolyBounds.vMax.z < vPos.z ) )
	{
		return false;
	}

	// Determine if point is on the outside of any
	// of the poly's edges.
 
	CAINavMeshEdge* pEdge;
	LTVector vTest, vEdgeN;

	for( int iEdge=0; iEdge < m_cNMPolyEdges; ++iEdge )
	{
		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );

		vTest = pEdge->GetNMEdgeMidPt() - vPos;
		vTest.y = 0.f;
		
		if( !pEdge->GetNMEdgeN( m_eNMPolyID, &vEdgeN ) )
		{
			return false;
		}

		// Point is on the outside of the edge.

		static float fEpsilon = 0.01f;
		if( vEdgeN.Dot( vTest ) > fEpsilon )
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::RayIntersectPoly2D()
//              
//	PURPOSE:	Return true if an intersection point was found.
//              
//----------------------------------------------------------------------------

bool CAINavMeshPoly::RayIntersectPoly2D( const LTVector& vRay0, const LTVector& vRay1, LTVector* pvIntersect )
{
	LTVector vR0, vR1, vL0, vL1;

	vR0 = vRay0;
	vR0.y = 0.f;

	vR1 = vRay1;
	vR1.y = 0.f;

	// Iterate over edges searching for an intersection.

	CAINavMeshEdge* pEdge;
	LTVector vLineSeg0, vLineSeg1;
	for( int iEdge=0; iEdge < m_cNMPolyEdges; ++iEdge )
	{
		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );
		if( pEdge )
		{
			vL0 = pEdge->GetNMEdge0();
			vL0.y = 0.f;

			vL1 = pEdge->GetNMEdge1();
			vL1.y = 0.f;

			// Found an intersection.

			if( kRayIntersect_Failure != RayIntersectLineSegment( vL0, vL1, vR0, vR1, false, pvIntersect ) )
			{
				return true;
			}
		}
	}

	// No intersection.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::PrintVerts()
//              
//	PURPOSE:	Print vertices of NavMeshPoly.
//              
//----------------------------------------------------------------------------

void CAINavMeshPoly::PrintVerts()
{
	TRACE( "NMPoly %d:\n", m_eNMPolyID );

	CAINavMeshEdge* pEdge;
	CAINavMeshEdge* pEdgeLast;

	pEdgeLast = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[m_cNMPolyEdges - 1] );

	for( int iEdge=0; iEdge < m_cNMPolyEdges; ++iEdge )
	{
		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );

		if( ( pEdge->GetNMEdge0() != pEdgeLast->GetNMEdge0() ) &&
			( pEdge->GetNMEdge0() != pEdgeLast->GetNMEdge1() ) )
		{
			TRACE( "  %d: %.2f %.2f %.2f\n", iEdge, pEdge->GetNMEdge0().x, pEdge->GetNMEdge0().y, pEdge->GetNMEdge0().z );
		}
		else {
			TRACE( "  %d: %.2f %.2f %.2f\n", iEdge, pEdge->GetNMEdge1().x, pEdge->GetNMEdge1().y, pEdge->GetNMEdge1().z );
		}

		pEdgeLast = pEdge;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::DrawSelf()
//              
//	PURPOSE:	Draw a NavMesh poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshPoly::DrawSelf()
{
	// Clear the current line system.

	DebugLineSystem& system = LineSystem::GetSystem( this, "ShowNavMesh" );
	system.Clear();

	// Determine if this poly should be drawn or not.  This enables filtering 
	// the layer which should draw.

#ifndef __FINAL
	static VarTrack s_PolyFilterVarTrack;
	s_PolyFilterVarTrack.Init( g_pLTServer, "ShowNavMeshFilter", "none", 0.0f );
	const char* const pszFilterString = s_PolyFilterVarTrack.GetStr(); 
	if ( pszFilterString && !LTStrIEquals( pszFilterString, "none" ) )
	{
		ENUM_AIAttributesID eID = g_pAIDB->GetAIAttributesRecordID( pszFilterString );
		if ( kAIAttributesID_Invalid != eID )
		{
			if ( 0 == ( m_dwNMCharTypeMask & ( 1 << eID ) ) )
			{
				return;
			}
		}
	}
#endif // __FINAL

	// Generate the name of the poly.

	char szLabel[512];
	LTSNPrintF( szLabel, LTARRAYSIZE(szLabel), "%d", m_eNMPolyID );

	// Select colors.

	DebugLine::Color colorBorder = Color::Red;
	DebugLine::Color colorLink = Color::Purple;
	DebugLine::Color colorConnection = Color::Yellow;
	if( m_eNMLinkID != kNMLink_Invalid )
	{
		colorBorder = Color::Blue;
		colorConnection = Color::Cyan;
	
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_eNMLinkID );
		if (pLink)
		{
			// Use red/dark red, as this is a disabled link.  If this leads to
			// border ambiguity issues, we may need to change these colors.

			if (!pLink->GetNMLinkEnabled())
			{
				colorBorder = Color::Red;
				colorConnection = Color::DkRed;
			}

			// This is a link.  Include the name and the enabled state.

			LTSNPrintF( szLabel, LTARRAYSIZE(szLabel), "%d\n(%s %s)", 
				m_eNMPolyID, 
				(pLink->GetName() ? pLink->GetName() : "<null>"),
				(pLink->GetNMLinkEnabled() ? "Enabled" : "Disabled") );
		}
	}

	// Draw edges.

	CAINavMeshEdge* pEdge;
	for( int iEdge=0; iEdge < m_cNMPolyEdges; ++iEdge )
	{
		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );
		if( !pEdge )
		{
			continue;
		}

		if( pEdge->GetNMEdgeType() == kNMEdgeType_Border )
		{
			system.AddLine( pEdge->GetNMEdge0(), pEdge->GetNMEdge1(), colorBorder, 255 );
		}
		else {
			ENUM_NMPolyID eNeighbor = ( pEdge->GetNMPolyIDA() != m_eNMPolyID ) ? pEdge->GetNMPolyIDA() : pEdge->GetNMPolyIDB();
			CAINavMeshPoly* pNeighbor = g_pAINavMesh->GetNMPoly( eNeighbor );
			if( pNeighbor && ( pNeighbor->GetNMLinkID() != kNMLink_Invalid ) )
			{
				AINavMeshLinkAbstract* pNeighborLink = g_pAINavMesh->GetNMLink( pNeighbor->GetNMLinkID() );
				if( pNeighborLink && pNeighborLink->GetNMLinkEnabled() )
				{
					system.AddLine( pEdge->GetNMEdge0(), pEdge->GetNMEdge1(), Color::Cyan, 255 );
				}
				else {
					system.AddLine( pEdge->GetNMEdge0(), pEdge->GetNMEdge1(), Color::DkRed, 255 );
				}
			}
			else {
				system.AddLine( pEdge->GetNMEdge0(), pEdge->GetNMEdge1(), colorConnection, 255 );
			}
		}
	}

	// Draw center.

	LTVector vVertStart = m_vNMPolyCenter;
	LTVector vVertEnd = m_vNMPolyCenter;
	vVertStart.x -= 5.f;
	vVertEnd.x += 5.f;
	system.AddLine( vVertStart, vVertEnd, colorBorder, 255 );

	vVertStart = m_vNMPolyCenter;
	vVertEnd = m_vNMPolyCenter;
	vVertStart.z -= 5.f;
	vVertEnd.z += 5.f;
	system.AddLine( vVertStart, vVertEnd, colorBorder, 255 );

	// Draw label.

	system.SetDebugString( szLabel );
	system.SetDebugStringPos( m_vNMPolyCenter + LTVector(0.0f, 16.0f, 0.0f) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::HideSelf()
//              
//	PURPOSE:	Hide a NavMesh poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshPoly::HideSelf()
{
  	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowNavMesh");
  	system.SetDebugString("");
  	system.Clear();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::DrawSelfInAIRegion()
//              
//	PURPOSE:	Draw NavMesh poly edges that bound some AIRegion.
//              
//----------------------------------------------------------------------------

void CAINavMeshPoly::DrawSelfInAIRegion( AIRegion* pAIRegion, bool bDrawName )
{
	// Sanity check.

	if( !pAIRegion )
	{
		return;
	}

	DebugLineSystem& system = LineSystem::GetSystem( this, "ShowAIRegions" );
	system.Clear();

	// Draw edges that form border of AIRegion.

	LTVector v0, v1;
	CAINavMeshEdge* pEdge;
	for( int iEdge=0; iEdge < m_cNMPolyEdges; ++iEdge )
	{
		pEdge = g_pAINavMesh->GetNMEdge( m_pNMPolyEdgeList[iEdge] );
		if( !pEdge )
		{
			continue;
		}

		// Border edges must form border of AIRegion.

		bool bDrawEdge = false;
		if( pEdge->GetNMEdgeType() == kNMEdgeType_Border )
		{
			bDrawEdge = true;
		}

		// Edges form border if the neighboring poly is not in the AIRegion.

		else {
			ENUM_NMPolyID eNeighbor = ( pEdge->GetNMPolyIDA() != m_eNMPolyID ) ? pEdge->GetNMPolyIDA() : pEdge->GetNMPolyIDB();
			if( !pAIRegion->ContainsNMPoly( eNeighbor ) )
			{
				bDrawEdge = true;
			}
		}

		// Draw the edge.

		if( bDrawEdge )
		{
			v0 = pEdge->GetNMEdge0();
			v1 = pEdge->GetNMEdge1();

			v0.y += 10.f;
			v1.y += 10.f;

			system.AddLine( v0, v1, Color::Green, 255 );
		}
	}

	// Draw label.
	
	if( bDrawName )
	{
		LTVector vPos = m_vNMPolyCenter;

		system.SetDebugString( pAIRegion->GetName() );
		system.SetDebugStringPos( vPos + LTVector(0.0f, 16.0f, 0.0f) );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::HideSelfInAIRegion()
//              
//	PURPOSE:	Hide NavMesh poly edges that bound an AIRegion.
//              
//----------------------------------------------------------------------------

void CAINavMeshPoly::HideSelfInAIRegion()
{
	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowAIRegions");
	system.SetDebugString("");
	system.Clear();
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAINavMeshEdge
//

CAINavMeshEdge::CAINavMeshEdge()
{
	m_eNMEdgeID = kNMEdge_Invalid;

	m_eNMEdgeType = kNMEdgeType_Invalid;

	m_eNMPolyIDA = kNMPoly_Invalid;
	m_eNMPolyIDB = kNMPoly_Invalid;

	m_bIsBorderVert0 = false;
	m_bIsBorderVert1 = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshEdge::GetNMEdgeN()
//              
//	PURPOSE:	Get the edge normal corresponding to the specified
//              neighbor poly.
//              
//----------------------------------------------------------------------------

bool CAINavMeshEdge::GetNMEdgeN( ENUM_NMPolyID eNMPolyID, LTVector* pvEdgeN )
{
	if( !pvEdgeN )
	{
		return false;
	}

	// Return the edge normal corresponding to polyA.

	if( eNMPolyID == m_eNMPolyIDA )
	{
		*pvEdgeN = m_vNMEdgeN_A;
		return true;
	}

	// Return the edge normal corresponding to polyB,
	// which is the negation of the normal corresponding 
	// to polyA.

	if( eNMPolyID == m_eNMPolyIDB )
	{
		*pvEdgeN = -m_vNMEdgeN_A;
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAINavMeshComponent
//

CAINavMeshComponent::CAINavMeshComponent()
{
	m_eNMComponentID = kNMComponent_Invalid;
	m_eNMSensoryComponentID = kNMSensoryComponent_Invalid;

	m_cNMComponentNeighbors = 0;
	m_pNMComponentNeighborList = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshComponent::FixUpNavMeshPointers()
//              
//	PURPOSE:	Fix-up pointers from raw NavMesh data.
//              
//----------------------------------------------------------------------------

void CAINavMeshComponent::FixUpNavMeshPointers( ENUM_NMComponentID* pAINavMeshComponentNeighborLists )
{
	// Sanity check.

	if( !pAINavMeshComponentNeighborLists )
	{
		return;
	}

	// Convert index to pointer into component neighbor lists.

	uint32 iNMComponentNeighborList = (uint32)m_pNMComponentNeighborList;
	m_pNMComponentNeighborList = pAINavMeshComponentNeighborLists + iNMComponentNeighborList;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshComponent::GetNMComponentNeighbor()
//              
//	PURPOSE:	Return pointer to neighbor component of specified index.
//              
//----------------------------------------------------------------------------

CAINavMeshComponent* CAINavMeshComponent::GetNMComponentNeighbor( int iNeighbor )
{
	CAINavMeshComponent* pNeighbor = NULL;

	// Bail if index is invalid.

	if( ( iNeighbor >= 0 ) && ( iNeighbor < m_cNMComponentNeighbors ) )
	{
		// Find component with matching neighbor index.

		ENUM_NMComponentID eNeighbor = m_pNMComponentNeighborList[iNeighbor];
		pNeighbor = g_pAINavMesh->GetNMComponent( eNeighbor );
	}

	return pNeighbor;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// AINavMesh:
// This class is different from CAINavMesh! The only purpose for this 
// object is to edit an AINavMesh in WorldEdit, and retreive the BlindObjectData.
//

// WorldEdit

LINKFROM_MODULE( AINavMesh );

BEGIN_CLASS( AINavMesh )
	ADD_REALPROP_FLAG(BlindDataIndex, -1.0f, CF_HIDDEN, "Internal index to the blind object data")
	ADD_LONGINTPROP_FLAG(AINavMeshIndex, -1, CF_HIDDEN, "Internal NavMesh index")

	ADD_STRINGPROP_FLAG(NavMeshType, "AI", PF_STATICLIST, "Mesh type - currently only AI")
	ADD_AI_CHAR_TYPE_RESTRICTIONS_AGGREGATE( PF_GROUP(1) )

END_CLASS_FLAGS_PLUGIN( AINavMesh, GameBase, 0, AINavMeshPlugin, "A collection of brushes that make up navigation network" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMesh )
CMDMGR_END_REGISTER_CLASS( AINavMesh, GameBase )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMesh::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMesh::AINavMesh()
{
	m_nBlindDataIndex = INVALID_NAV_MESH_DATA_INDEX;
	m_eNavMeshID = kNMNavMesh_Invalid;
	m_pdwAINavMeshPolyFlags = NULL;
	m_nAINavMeshPolyFlagsCount = 0;
	m_iNavMeshCharacterTypeMask = 0;

	AddAggregate( &m_CharTypeRestrictions );
}

AINavMesh::~AINavMesh()
{
	if( m_pdwAINavMeshPolyFlags )
	{
		debug_deletea( m_pdwAINavMeshPolyFlags );
		m_pdwAINavMeshPolyFlags = NULL;
	}
	m_nAINavMeshPolyFlagsCount = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMesh::EngineMessageFn
//              
//	PURPOSE:	Handle engine messages.
//              
//----------------------------------------------------------------------------

uint32 AINavMesh::EngineMessageFn(uint32 messageID, void *pv, float fData)
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
			SetNextUpdate( UPDATE_NEVER );
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pv, fData);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMesh::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMesh::ReadProp(const GenericPropList *pProps)
{
	// Read the BlindDataIndex assigned by the preprocessor
	// to the AINavMesh data.

	float tmp = pProps->GetReal( "BlindDataIndex", -2.0f );
	if( tmp > 0.0f )
	{
		m_nBlindDataIndex = (uint32)tmp;

		// Set the AINavMesh data for the global AINavMesh singleton.

		if( g_pAINavMesh )
		{
			g_pAINavMesh->SetAINavMeshObject( this );
		}
	}
	else {
		m_nBlindDataIndex = INVALID_NAV_MESH_DATA_INDEX;
	}

	// Read the NavMesh index.

	int32 nNavMeshID = pProps->GetLongInt( "AINavMeshIndex", m_eNavMeshID );
	m_eNavMeshID = ( ENUM_NMNavMeshID )nNavMeshID;

	// Read the nav mesh type

	const char* const pszType = pProps->GetString( "NavMeshType", "AI" );
	uint32 iNavMeshTypeRecords = g_pAIDB->GetNumAINavMeshTypes();
	const AIDB_AINavMeshTypeRecord* pNavMeshRecord = NULL;
	for ( uint32 i = 0; i < iNavMeshTypeRecords; ++i )
	{
		const AIDB_AINavMeshTypeRecord* pCurrentRecord = g_pAIDB->GetAINavMeshTypeRecord( i );
		if ( pCurrentRecord && LTStrEquals( pCurrentRecord->m_szName, pszType ) )
		{
			pNavMeshRecord = pCurrentRecord;
			break;
		}
	}

	AIASSERT( pNavMeshRecord, NULL, "AINavMesh::ReadProp: No AINavMeshTypeRecord, this nav mesh will not be used!" )
	if ( pNavMeshRecord )
	{
		m_iNavMeshCharacterTypeMask = pNavMeshRecord->m_dwCharacterTypeMask;
	}

	// Read the character type restrictions.

	m_CharTypeRestrictions.ReadProp( pProps );

	// Add a mask to the NavMesh.

	if( m_eNavMeshID != kNMNavMesh_Invalid )
	{
		g_pAINavMesh->AddNMCharTypeMask( m_eNavMeshID, 
			m_iNavMeshCharacterTypeMask & m_CharTypeRestrictions.GetCharTypeMask() );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMesh::Save/Load
//              
//	PURPOSE:	Save/Load the object.
//              
//----------------------------------------------------------------------------

void AINavMesh::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_nBlindDataIndex);
	SAVE_DWORD(m_eNavMeshID);
	SAVE_INT(m_iNavMeshCharacterTypeMask);
	SAVE_INT(m_nAINavMeshPolyFlagsCount);
	pMsg->WriteData(m_pdwAINavMeshPolyFlags, sizeof(uint32)*m_nAINavMeshPolyFlagsCount);

	m_CharTypeRestrictions.Save( pMsg );

}

void AINavMesh::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT(m_nBlindDataIndex);
	LOAD_DWORD_CAST(m_eNavMeshID, ENUM_NMNavMeshID);
	LOAD_INT(m_iNavMeshCharacterTypeMask);
	LOAD_INT(m_nAINavMeshPolyFlagsCount);

	AIASSERT(m_pdwAINavMeshPolyFlags == NULL, NULL, "m_pdwAINavMeshPolyFlags is not NULL on Load()");
	m_pdwAINavMeshPolyFlags = debug_newa( uint32, m_nAINavMeshPolyFlagsCount );
	pMsg->ReadData(m_pdwAINavMeshPolyFlags, sizeof(uint32)*m_nAINavMeshPolyFlagsCount);

	m_CharTypeRestrictions.Load( pMsg );

	// Set the AINavMesh data for the global AINavMesh singleton.

	if( g_pAINavMesh && ( m_nBlindDataIndex != INVALID_NAV_MESH_DATA_INDEX ) )
	{
		g_pAINavMesh->SetAINavMeshObject( this );
	}

	// Add a mask to the NavMesh.

	g_pAINavMesh->AddNMCharTypeMask( m_eNavMeshID, 
		m_iNavMeshCharacterTypeMask & m_CharTypeRestrictions.GetCharTypeMask() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMesh::OnNavMeshCreated
//              
//	PURPOSE:	Clear NavMesh flags.
//              
//----------------------------------------------------------------------------

void AINavMesh::OnNavMeshCreated(int nPolyCount)
{
	// Pointer will be NULL if this level was loaded for the first time.  
	// Pointer will be valid the level was loaded.  This difference is
	// caused by the fact NavMeshGen happens after the load, meaning that
	// the polyflags state would get stomped if this check was not performed.
	// When the NavMeshGen is moved into the preprocess stage, several 
	// changes to the construct of a navmesh are likely to occur.  When this
	// happens, a better solution to flag save/load should be evaluated.

	if( m_pdwAINavMeshPolyFlags )
	{
		debug_deletea( m_pdwAINavMeshPolyFlags );
	}

	m_nAINavMeshPolyFlagsCount = nPolyCount;
	m_pdwAINavMeshPolyFlags = debug_newa( uint32, m_nAINavMeshPolyFlagsCount );
	memset(m_pdwAINavMeshPolyFlags, kNMPolyFlag_None, sizeof(uint32)*m_nAINavMeshPolyFlagsCount);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshPlugin::PreHook_EditStringList
//              
//	PURPOSE:	Initialize WolrdEdit drop-downs.
//              
//----------------------------------------------------------------------------

LTRESULT AINavMeshPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Let the AICharTypeRestrictions plugin have a go at it...

	if( m_AICharTypeRestrictionsPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	// Add the nav mesh 'types'.  These coorespond to default 
	// inclusion/exclusion filter masks

	if ( LTStrEquals( szPropName, "NavMeshType" ) )
	{
		for ( uint32 i = 0; i < g_pAIDB->GetNumAINavMeshTypes(); ++i )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], g_pAIDB->GetAINavMeshTypeRecord( i )->m_szName, cMaxStringLength );
		}
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAINavMesh
//

// Globals

CAINavMesh* g_pAINavMesh = NULL;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAINavMesh::CAINavMesh()
{
	g_pAINavMesh = this;

	m_pAINavMeshObject = NULL;
	m_pPackedNavMeshData = NULL;
	m_bDeletePackedNavMeshData = false;

	m_bNMInitialized = false;
	m_bDrawingNavMesh = false;
	m_bDrawingAIRegions = false;

	m_cAINavMeshPolys = 0;
	m_pAINavMeshPolys = NULL;

	m_cAINavMeshPolyNormals = 0;
	m_pAINavMeshPolyNormals = NULL;

	m_cAINavMeshEdges = 0;
	m_pAINavMeshEdges = NULL;

	m_cAINavMeshEdgeLists = 0;
	m_pAINavMeshEdgeLists = NULL;

	m_cAIRegionLists = 0;
	m_pAIRegionLists = NULL;

	m_cAINavMeshComponents = 0;
	m_pAINavMeshComponents = NULL;

	m_cAINavMeshComponentNeighborLists = 0;
	m_pAINavMeshComponentNeighborLists = NULL;

	m_cNMPolyLists = 0;
	m_pNMPolyLists = NULL;

	m_cAINavMeshLinks = 0;
	m_pAINavMeshLinkData = NULL;

	m_cAINavMeshLinkBoundaryVerts = 0;
	m_pAINavMeshLinkBoundaryVerts = NULL;

	m_cAIRegions = 0;
	m_pAIRegionData = NULL;

	m_cClusteredAINodes = 0;
	m_pClusteredAINodes = NULL;

	m_pszClusteredAINodeNameList = NULL;

	m_cAIQuadTreeNodes = 0;
	m_pAIQuadTreeNodes = NULL;

	m_cAIQuadTreeNMPolyLists = 0;
	m_pAIQuadTreeNMPolyLists = NULL;
}

CAINavMesh::~CAINavMesh()
{
	TermNavMesh();

	g_pAINavMesh = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::InitNavMesh()
//              
//	PURPOSE:	Initialize NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMesh::InitNavMesh()
{
	m_bNMInitialized = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::TermNavMesh()
//              
//	PURPOSE:	Terminate NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMesh::TermNavMesh()
{
	m_bNMInitialized = false;

	if( m_bDeletePackedNavMeshData )
	{
		debug_deletea( m_pPackedNavMeshData );
		m_pPackedNavMeshData = NULL;
		m_bDeletePackedNavMeshData = false;
	}

	m_pAINavMeshPolys = NULL;
	m_cAINavMeshPolys = 0;

	m_pAINavMeshPolyNormals = NULL;
	m_cAINavMeshPolyNormals = 0;

	m_pAINavMeshEdges = NULL;
	m_cAINavMeshEdges = 0;

	m_pAINavMeshEdgeLists = NULL;
	m_pAINavMeshEdgeLists = 0;

	m_cAIRegionLists = 0;
	m_pAIRegionLists = NULL;

	m_cAINavMeshComponents = 0;
	m_pAINavMeshComponents = NULL;

	m_cAINavMeshComponentNeighborLists = 0;
	m_pAINavMeshComponentNeighborLists = NULL;

	m_cNMPolyLists = 0;
	m_pNMPolyLists = NULL;

	m_lstAINavMeshLinks.resize( 0 );
	m_lstAIRegions.resize( 0 );
	m_lstAINavMeshCharTypeMasks.resize( 0 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::RuntimeSetup()
//              
//	PURPOSE:	Setup the NavMesh for run-time.
//              
//----------------------------------------------------------------------------

void CAINavMesh::RuntimeSetup( uint8* pData, bool bDelete )
{
	// Sanity check.

	if( !pData )
	{
		return;
	}

	// Already initialized.

	if( m_bNMInitialized )
	{
		return;
	}

	// The CAINavMesh gets its data from the AINavMesh.
	// The AINavMesh is the object in WorldEdit.  There may be multiple
	// AINavMeshes, but the preprocessor combines them into one
	// AINavMesh per level.

	if( !m_pAINavMeshObject )
	{
		return;
	}

	m_pPackedNavMeshData = pData;
	m_bDeletePackedNavMeshData = bDelete;

	uint32 nSizeInt = sizeof(uint32);
	uint32 nSizeVector = sizeof(LTVector);


	//
	// Read header.
	//

	uint8* pCur = pData;
	uint32 nVersion = *(uint32*)pCur;
	pCur += nSizeInt;

	// Bail if incorrect version.

	if( nVersion != AINAVMESH_VERSION_NUMBER )
	{
		AIASSERT2( 0, NULL, "CAINavMesh::InitNavMesh: Packed NavMesh is incorrect version: %d != %d", nVersion, AINAVMESH_VERSION_NUMBER );
		return;
	}

	// Read fixed-up flag.  This flag allows us to keep BlindObjectData in memory,
	// even after massaging it.

	m_pbAINavMeshFixedUp = (bool*)pCur;
	pCur += nSizeInt;

	//
	// Read edges.
	//

	m_cAINavMeshEdges = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshEdges = (CAINavMeshEdge*)pCur;
	pCur += m_cAINavMeshEdges * sizeof( CAINavMeshEdge );


	//
	// Read polys.
	//

	// Edge lists.

	m_cAINavMeshEdgeLists = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshEdgeLists = (ENUM_NMEdgeID*)pCur;
	pCur += m_cAINavMeshEdgeLists * nSizeInt;

	// Region lists.

	m_cAIRegionLists = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAIRegionLists = (ENUM_AIRegionID*)pCur;
	pCur += m_cAIRegionLists * nSizeInt;

	// Poly normals.

	m_cAINavMeshPolyNormals = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshPolyNormals = (LTVector*)pCur;
	pCur += m_cAINavMeshPolyNormals * nSizeVector;

	// Polys.

	m_cAINavMeshPolys = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshPolys = (CAINavMeshPoly*)pCur;
	pCur += m_cAINavMeshPolys * sizeof( CAINavMeshPoly );


	//
	// Read regions.
	//

	// Region poly lists.

	m_cNMPolyLists = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pNMPolyLists = (ENUM_NMPolyID*)pCur;
	pCur += m_cNMPolyLists * nSizeInt;

	// Regions.

	m_cAIRegions = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAIRegionData = (AIREGION_DATA*)pCur;
	pCur += m_cAIRegions * sizeof( AIREGION_DATA );


	//
	// Read components.
	//

	// Components.

	m_cAINavMeshComponents = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshComponents = (CAINavMeshComponent*)pCur;
	pCur += m_cAINavMeshComponents * sizeof( CAINavMeshComponent );

	// Component neighbor lists.

	m_cAINavMeshComponentNeighborLists = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshComponentNeighborLists = (ENUM_NMComponentID*)pCur;
	pCur += m_cAINavMeshComponentNeighborLists * nSizeInt;


	//
	// Read NavMeshLinks.
	//

	// NavMeshLinks.

	m_cAINavMeshLinks = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshLinkData = (NAVMESH_LINK_DATA*)pCur;
	pCur += m_cAINavMeshLinks * sizeof( NAVMESH_LINK_DATA );

	// Boundary verts.

	m_cAINavMeshLinkBoundaryVerts = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAINavMeshLinkBoundaryVerts = (LTVector*)pCur;
	pCur += m_cAINavMeshLinkBoundaryVerts * nSizeVector;


	//
	// Read node clusters.
	//

	// Nodes.

	m_cClusteredAINodes = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pClusteredAINodes = (AINODE_CLUSTER_DATA*)pCur;
	pCur += m_cClusteredAINodes * sizeof( AINODE_CLUSTER_DATA );

	// Name list.

	uint32 nNameListSize = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pszClusteredAINodeNameList = (char*)pCur;
	pCur += nNameListSize;


	//
	// Read quad tree.
	//

	// Quad tree nodes.

	m_cAIQuadTreeNodes = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAIQuadTreeNodes = (CAIQuadTreeNode*)pCur;
	pCur += m_cAIQuadTreeNodes * sizeof( CAIQuadTreeNode );

	// Quad tree poly references.

	m_cAIQuadTreeNMPolyLists = *(uint32*)pCur;
	pCur += nSizeInt;

	m_pAIQuadTreeNMPolyLists = (ENUM_NMPolyID*)pCur;
	pCur += m_cAIQuadTreeNMPolyLists * nSizeInt;


	// Fix-up pass on raw data.
	// Only necessary if BlindObjectData was not already in memory.

	if( !( *m_pbAINavMeshFixedUp ) )
	{
		FixUpNavMeshPointers();
	}

	// Setup NavMeshLink Bounds.

	AINavMeshLinkAbstract* pNMLink;
	NAVMESH_LINK_DATA* pNMLinkData;
	for( uint32 iNMLink=0; iNMLink < m_cAINavMeshLinks; ++iNMLink )
	{
		pNMLinkData = &( m_pAINavMeshLinkData[iNMLink] );

		// Setup the actual NavMeshLink object.

		pNMLink = GetNMLink( pNMLinkData->eNMlinkID );
		if( pNMLink )
		{
			pNMLink->SetNMLinkBounds( pNMLinkData->cBoundaryVerts, pNMLinkData->pBoundaryVerts );
		}
	}

	//
	// Setup AIRegions.
	//

	AIREGION_DATA* pAIRegionData;
	for( uint32 iRegion=0; iRegion < m_cAIRegions; ++iRegion )
	{
		// Skip invalid regions.

		pAIRegionData = &( m_pAIRegionData[iRegion] );
		if( (uint32)pAIRegionData->eAIRegionID < m_lstAIRegions.size() )
		{
			AIRegion* pAIRegion = m_lstAIRegions[pAIRegionData->eAIRegionID];
			pAIRegion->SetupAIRegion( pAIRegionData->vBoundingSphereCenter,
				pAIRegionData->fBoundingSphereRadius,
				pAIRegionData->cNMPolys,
				pAIRegionData->pNMPolyIDs );
		}
	}

	// Runtime, notify the navmeshdata object that the navmesh has been
	// constructed, so that it may reflect the change and provide a flag set.
	m_pAINavMeshObject->OnNavMeshCreated(m_cAINavMeshPolys);

	// Connect the navmesh links
	ConnectNMLinks();

	// Initialization complete.

	m_bNMInitialized = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::FixUpNavMeshPointers()
//              
//	PURPOSE:	Fix-up pass on raw NavMesh data.
//              
//----------------------------------------------------------------------------

void CAINavMesh::FixUpNavMeshPointers()
{
	//
	// Indicate that this BlindObjectData has already been fixed-up
	// and does not need to be fixed up again.
	//

	*m_pbAINavMeshFixedUp = true;

	//
	// Fix up polys.
	//

	CAINavMeshPoly* pNMPoly;
	for( int iPoly=0; iPoly < m_cAINavMeshPolys; ++iPoly )
	{
		pNMPoly = &( m_pAINavMeshPolys[iPoly] );
		pNMPoly->FixUpNavMeshPointers( m_pAINavMeshEdgeLists, m_pAIRegionLists );
	}


	//
	// Fix up regions.
	//

	AIREGION_DATA* pAIRegionData;
	for( uint32 iRegion=0; iRegion < m_cAIRegions; ++iRegion )
	{
		// Skip invalid regions.

		pAIRegionData = &( m_pAIRegionData[iRegion] );
		if( (uint32)pAIRegionData->eAIRegionID < m_lstAIRegions.size() )
		{
			// Convert poly list index into pointer.

			uint32 iPolyList = (uint32)pAIRegionData->pNMPolyIDs;
			pAIRegionData->pNMPolyIDs = m_pNMPolyLists + iPolyList;
		}
	}


	//
	// Fix up components.
	//

	CAINavMeshComponent* pNMComponent;
	for( int iComponent=0; iComponent < m_cAINavMeshComponents; ++iComponent )
	{
		pNMComponent = &( m_pAINavMeshComponents[iComponent] );
		pNMComponent->FixUpNavMeshPointers( m_pAINavMeshComponentNeighborLists );
	}


	//
	// Fix up NavMeshLinks.
	//

	NAVMESH_LINK_DATA* pNMLinkData;
	for( uint32 iNMLink=0; iNMLink < m_cAINavMeshLinks; ++iNMLink )
	{
		pNMLinkData = &( m_pAINavMeshLinkData[iNMLink] );

		// Convert bounds list index into pointer.

		uint32 iBoundsList = (uint32)pNMLinkData->pBoundaryVerts;
		pNMLinkData->pBoundaryVerts = m_pAINavMeshLinkBoundaryVerts + iBoundsList;
	}


	//
	// Fix up node clusters.
	//

	AINode* pNode;
	HOBJECT hNode;
	AINODE_CLUSTER_DATA* pNodeClusterData;
	for( uint32 iNode=0; iNode < m_cClusteredAINodes; ++iNode )
	{
		// Convert name list index into string pointer.

		pNodeClusterData = &( m_pClusteredAINodes[iNode] );
		uint32 iNameList = (uint32)pNodeClusterData->pszName;
		pNodeClusterData->pszName = m_pszClusteredAINodeNameList + iNameList;

		// Find node by name, and set cluster ID.

		if( LT_OK == FindNamedObject( pNodeClusterData->pszName, hNode ) )
		{
			pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
			pNode->SetAINodeClusterID( pNodeClusterData->eAINodeClusterID );
		}
	}


	//
	// Fix up quad tree.
	//

	// Set the tree's root.

	g_pAIQuadTree->SetQuadTreeRoot( m_pAIQuadTreeNodes );

	// Fix up pointers.

	CAIQuadTreeNode* pQTNode;
	for( uint32 iQTNode=0; iQTNode < m_cAIQuadTreeNodes; ++iQTNode )
	{
		pQTNode = &( m_pAIQuadTreeNodes[iQTNode] );
		pQTNode->FixUpNavMeshPointers( m_pAIQuadTreeNodes, m_pAIQuadTreeNMPolyLists );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::AddNMCharTypeMask()
//              
//	PURPOSE:	Add a character type mask at a specified index.
//              
//----------------------------------------------------------------------------

void CAINavMesh::AddNMCharTypeMask( uint32 iMaskIndex, uint32 dwMask )
{
	// Expand to fit new index.

	if( iMaskIndex >= m_lstAINavMeshCharTypeMasks.size() )
	{
		m_lstAINavMeshCharTypeMasks.resize( iMaskIndex + 1 );
	}

	// Assign mask at index.

	m_lstAINavMeshCharTypeMasks[iMaskIndex] = dwMask;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::SortAINavMeshLinks()
//              
//	PURPOSE:	Sort NavMeshLinks before setting up packed data.
//              
//----------------------------------------------------------------------------

static bool SortNMLinks(const AINavMeshLinkAbstract* pLeft, const AINavMeshLinkAbstract* pRight)
{
	return ( pLeft->GetNMLinkID() < pRight->GetNMLinkID() );
}

/*
static bool SortAIRegions(const AIRegion*& pLeft, const AIRegion*& pRight)
{
	return ( pLeft->GetAIRegionID() < pRight->GetAIRegionID() );
}
*/

void CAINavMesh::SortAINavMeshLinks()
{
	// Sort links by NMLinkID, so that they can be indexed by ID.

	std::sort( m_lstAINavMeshLinks.begin(), m_lstAINavMeshLinks.end(), SortNMLinks );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::AddAINavMeshLink()
//              
//	PURPOSE:	Add a NavMeshLink to the NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMesh::AddAINavMeshLink( AINavMeshLinkAbstract* pLink )
{
	// Sanity check.

	if( !pLink )
	{
		return;
	}

	ENUM_NMLinkID eLink = pLink->GetNMLinkID();
	if( eLink != kNMLink_Invalid )
	{
		m_lstAINavMeshLinks.push_back( pLink );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::ConnectNMLinks()
//              
//	PURPOSE:	Connect links back to polys that they are linked to.
//              
//----------------------------------------------------------------------------

void CAINavMesh::ConnectNMLinks()
{
	// Prior to calling this function, the NavMeshPolys 
	// know their links, but the links don't know their polys.
	// This function establishes the backwards connection,
	// which is sometimes necessary when processing links.

	// This information cannot be obtained earlier, because
	// the NavMesh generation process re-indexes the polys.

	CAINavMeshPoly* pPoly;
	AINavMeshLinkAbstract* pLink;
	for( int iPoly=0; iPoly < m_cAINavMeshPolys; ++iPoly )
	{
		pPoly = &( m_pAINavMeshPolys[iPoly] );
		if( pPoly->GetNMLinkID() != kNMLink_Invalid )
		{
			pLink = GetNMLink( pPoly->GetNMLinkID() );
			if( pLink )
			{
				pLink->SetNMPolyID( pPoly->GetNMPolyID() );
				pLink->SetupNMLinkEdges();
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::AddAIRegion()
//              
//	PURPOSE:	Add an AIRegion to the NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMesh::AddAIRegion( AIRegion* pAIRegion )
{
	// Sanity check.

	if( !pAIRegion )
	{
		return;
	}

	ENUM_AIRegionID eRegionID = pAIRegion->GetAIRegionID();
	if( eRegionID != kAIRegion_Invalid )
	{
		m_lstAIRegions.push_back( pAIRegion );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::SetNMPolyFlags()
//              
//	PURPOSE:	Set flags associated with a NMPoly.
//              
//----------------------------------------------------------------------------

void CAINavMesh::SetNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags )
{
	if (!m_pAINavMeshObject)
	{
		return;
	}

	if( ( ePoly > kNMPoly_Invalid ) && 
		( ePoly < m_cAINavMeshPolys ) )
	{
		m_pAINavMeshObject->SetNMPolyFlags(ePoly, dwFlags);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::TestNMPolyFlags()
//              
//	PURPOSE:	Return true if all of the specified flags are 
//              set on the NMPoly.
//              
//----------------------------------------------------------------------------

bool CAINavMesh::TestNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags )
{
	if (!m_pAINavMeshObject)
	{
		return false;
	}

	if( ( ePoly > kNMPoly_Invalid ) && 
		( ePoly < m_cAINavMeshPolys ) )
	{
		return m_pAINavMeshObject->TestNMPolyFlags(ePoly, dwFlags);
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::ClearNMPolyFlags()
//              
//	PURPOSE:	Clear flags associated with a NMPoly.
//              
//----------------------------------------------------------------------------

void CAINavMesh::ClearNMPolyFlags( ENUM_NMPolyID ePoly, uint32 dwFlags )
{
	if (!m_pAINavMeshObject)
	{
		return;
	}

	if( ( ePoly > kNMPoly_Invalid ) && 
		( ePoly < m_cAINavMeshPolys ) )
	{
		return m_pAINavMeshObject->ClearNMPolyFlags(ePoly, dwFlags);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNMCharTypeMask()
//              
//	PURPOSE:	Return the character type mask at the specified index.
//              
//----------------------------------------------------------------------------

uint32 CAINavMesh::GetNMCharTypeMask( uint32 iMaskIndex )
{
	if( iMaskIndex < m_lstAINavMeshCharTypeMasks.size() )
	{
		return m_lstAINavMeshCharTypeMasks[iMaskIndex];
	}

	// If the specified mask cannot be found, 
	// allow all characters to use the NavMesh.

	return ALL_CHAR_TYPES;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNMPoly()
//              
//	PURPOSE:	Get a pointer to a NavMesh poly.
//              
//----------------------------------------------------------------------------

CAINavMeshPoly* CAINavMesh::GetNMPoly( ENUM_NMPolyID ePoly )
{
	if( ( ePoly > kNMPoly_Invalid ) && 
		( ePoly < m_cAINavMeshPolys ) )
	{
		return &( m_pAINavMeshPolys[ePoly] );
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNMPolyNormal()
//              
//	PURPOSE:	Return the normal of a NavMeshPoly.
//              
//----------------------------------------------------------------------------

bool CAINavMesh::GetNMPolyNormal( ENUM_NMNormalID eNormal, LTVector* pvNormal )
{
	if( !pvNormal )
	{
		return false;
	}

	if( ( eNormal > kNMNormal_Invalid ) && 
		( eNormal < m_cAINavMeshPolyNormals ) )
	{
		*pvNormal = m_pAINavMeshPolyNormals[eNormal];
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNMLink()
//              
//	PURPOSE:	Get a pointer to a NavMesh link.
//              
//----------------------------------------------------------------------------

AINavMeshLinkAbstract* CAINavMesh::GetNMLink( ENUM_NMLinkID eLink )
{
	// Links must be Invalid, or in range.
	AIASSERT(eLink == kNMLink_Invalid ||
		(eLink >= 0 && eLink < (int)m_lstAINavMeshLinks.size()), 
		NULL,  "Out of bounds eLink, returning NULL.");

	if( ( eLink > kNMLink_Invalid ) && 
		( eLink < (int)m_lstAINavMeshLinks.size() ) )
	{
		return m_lstAINavMeshLinks[eLink];
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNMComponent()
//              
//	PURPOSE:	Get a pointer to a NavMesh component.
//              
//----------------------------------------------------------------------------

CAINavMeshComponent* CAINavMesh::GetNMComponent( ENUM_NMComponentID eComponent )
{
	if( ( eComponent > kNMComponent_Invalid ) && 
		( eComponent < m_cAINavMeshComponents ) )
	{
		return &( m_pAINavMeshComponents[eComponent] );
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNMEdge()
//              
//	PURPOSE:	Get a pointer to a NavMesh edge.
//              
//----------------------------------------------------------------------------

CAINavMeshEdge* CAINavMesh::GetNMEdge( ENUM_NMEdgeID eEdge )
{
	if( ( eEdge > kNMEdge_Invalid ) && 
		( eEdge < m_cAINavMeshEdges ) )
	{
		return &( m_pAINavMeshEdges[eEdge] );
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetAIRegion()
//              
//	PURPOSE:	Get a pointer to an AIRegion.
//              
//----------------------------------------------------------------------------

AIRegion* CAINavMesh::GetAIRegion( ENUM_AIRegionID eRegion )
{
	if( ( eRegion > kAIRegion_Invalid ) && 
		( eRegion < (int)m_lstAIRegions.size() ) )
	{
		return m_lstAIRegions[eRegion];
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::UpdateDebugRendering()
//              
//	PURPOSE:	Draw or hide NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMesh::UpdateDebugRendering( float fNavMeshVarTrack, float fAIRegionsVarTrack )
{
	// Handle NavMesh debug rendering.

	if( !m_bDrawingNavMesh && fNavMeshVarTrack )
	{
		DrawNavMesh();
	}
	else if( m_bDrawingNavMesh && !fNavMeshVarTrack )
	{
		HideNavMesh();
	}

	// Handle AIRegions debug rendering.

	if( !m_bDrawingAIRegions && fAIRegionsVarTrack )
	{
		DrawAIRegions();
	}
	else if( m_bDrawingAIRegions && !fAIRegionsVarTrack )
	{
		HideAIRegions();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::DrawNavMesh()
//              
//	PURPOSE:	Sets NavMesh to draw and remembers that the NavMesh is 
//				being drawn.
//              
//----------------------------------------------------------------------------

void CAINavMesh::DrawNavMesh()
{
	m_bDrawingNavMesh = true;

	// Each poly has its own LineSystem.

	for( int iPoly=0; iPoly < m_cAINavMeshPolys; ++iPoly )
	{
		m_pAINavMeshPolys[iPoly].DrawSelf();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::HideNavMesh()
//              
//	PURPOSE:	Sets NavMesh to Hide
//              
//----------------------------------------------------------------------------

void CAINavMesh::HideNavMesh()
{
	m_bDrawingNavMesh = false;

	// Each poly has its own LineSystem.

	for( int iPoly=0; iPoly < m_cAINavMeshPolys; ++iPoly )
	{
		m_pAINavMeshPolys[iPoly].HideSelf();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::DrawAIRegions()
//              
//	PURPOSE:	Sets AIRegions to draw and remembers that AIRegions are 
//				being drawn.
//              
//----------------------------------------------------------------------------

void CAINavMesh::DrawAIRegions()
{
	m_bDrawingAIRegions = true;

	// Each AIRegion draws itself.

	AIRegion* pAIRegion;
	AIREGION_LIST::iterator itRegion;
	for( itRegion = m_lstAIRegions.begin(); itRegion != m_lstAIRegions.end(); ++itRegion )
	{
		pAIRegion = *itRegion;
		pAIRegion->DrawSelf();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::HideAIRegions()
//              
//	PURPOSE:	Sets AIRegions to Hide
//              
//----------------------------------------------------------------------------

void CAINavMesh::HideAIRegions()
{
	m_bDrawingAIRegions = false;

	// Each AIRegion hides itself.

	AIRegion* pAIRegion;
	AIREGION_LIST::iterator itRegion;
	for( itRegion = m_lstAIRegions.begin(); itRegion != m_lstAIRegions.end(); ++itRegion )
	{
		pAIRegion = *itRegion;
		pAIRegion->HideSelf();
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::IsDrawing()
//              
//	PURPOSE:	Returns true if the nav mesh is being drawn, false if it is 
//				not.
//              
//----------------------------------------------------------------------------

bool CAINavMesh::IsDrawing() const
{
	return m_bDrawingNavMesh; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::RedrawPoly()
//              
//	PURPOSE:	Forces the redrawing of the passed in poly.  This can be used
//				to reflect link status changes such as enabled/disable state.
//              
//----------------------------------------------------------------------------

void CAINavMesh::RedrawPoly(ENUM_NMPolyID ePoly)
{
	// Poly is out of range.

	if (ePoly < 0 || ePoly > m_cAINavMeshPolys)
	{
		return;
	}

	m_pAINavMeshPolys[ePoly].DrawSelf();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::GetNavMeshBlindObjectData
//              
//	PURPOSE:	Returns the blind object data containing the navmesh
//              
//----------------------------------------------------------------------------

bool CAINavMesh::GetNavMeshBlindObjectData( uint8*& blindData, uint32& blindDataSize )
{
	if( !( g_pAINavMesh && g_pAINavMesh->m_pAINavMeshObject ) )
	{
		return false;
	}
	AINavMesh* pAINavMeshData = g_pAINavMesh->m_pAINavMeshObject;

	// Bail if there is no BlindObjectData.

	if( pAINavMeshData->m_nBlindDataIndex == 0xffffffff )
	{
		return false;
	}

	// grab the blind data
	blindData = NULL;
	blindDataSize = 0;
	if( g_pLTServer->GetBlindObjectData( pAINavMeshData->m_nBlindDataIndex, AINAVMESH_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMesh::IsNavMeshBlindDataProcessed
//              
//	PURPOSE:	Determines if the blind data is processed or raw
//              
//----------------------------------------------------------------------------

bool CAINavMesh::IsNavMeshBlindDataProcessed( uint8* blindData, uint32 nSize )
{
	if( blindData == NULL )
		return false;

	if( nSize < sizeof(uint32) )
		return false;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative((uint32*)blindData, 1);
#endif // PLATFORM_XENON

	return (((uint32*)blindData)[0] != 0);
}
