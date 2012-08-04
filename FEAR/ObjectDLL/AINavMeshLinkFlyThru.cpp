// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkFlyThru.cpp
//
// PURPOSE : AI NavMesh Link FlyThru class implementation.
//
// CREATED : 09/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkFlyThru.h"
#include "AINavMesh.h"
#include "FxDefs.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkFlyThru );

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINAVMESHLINKFLYTHRU CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINAVMESHLINKFLYTHRU 0

#endif


BEGIN_CLASS( AINavMeshLinkFlyThru )
	ADD_STRINGPROP_FLAG(SmartObject,	"FlyThru",				0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkFlyThru, AINavMeshLinkAbstract, CF_HIDDEN_AINAVMESHLINKFLYTHRU, AINavMeshLinkFlyThruPlugin, "This link is used to specify that the brush contains a solid space the AI flys through, leaving an effect on the wall as he enters or exits" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkFlyThru )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkFlyThru, AINavMeshLinkAbstract )


#define FLYTHRU_ENTER_FX	"FlyThruEnterFX"
#define FLYTHRU_EXIT_FX		"FlyThruExitFX"


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlyThru::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkFlyThru::AINavMeshLinkFlyThru()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlyThru::IsLinkRelevant
//              
//	PURPOSE:	Return true if link is relevant to a goal.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkFlyThru::IsLinkRelevant( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// We have already traversed this link, and have not yet exited it.

	if( pAI->GetAIBlackBoard()->GetBBTraversingNMLink() == m_eNMLinkID )
	{
		return false;
	}

	// Bail if poly is invalid.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return false;
	}

	// Traverse think link when within the entry offset.

	LTVector vLookAhead = pAI->GetPosition() + ( pAI->GetForwardVector() * GetNMLinkOffsetEntryDistA() );
	if( !pPoly->RayIntersectPoly2D( pAI->GetPosition(), vLookAhead, NULL ) )
	{
		return false;
	}

	// AI needs to traverse.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlyThru::ModifyMovement
//              
//	PURPOSE:	Modify the new position as an AI moves.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkFlyThru::ModifyMovement( CAI* pAI, CAIMovement::State eStatePrev, LTVector* pvNewPos, CAIMovement::State* peStateCur )
{
	// Sanity check.

	if( !( pAI && pvNewPos ) )
	{
		return;
	}

	// Only modify movement while following a path.

	if( eStatePrev != CAIMovement::eStateSet )
	{
		return;
	}

	// Bail if poly is invalid.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return;
	}

	// New position does not get close enough to the link.

	LTVector vLookAhead = *pvNewPos + ( pAI->GetForwardVector() * GetNMLinkOffsetEntryDistA() );
	if( !pPoly->RayIntersectPoly2D( *pvNewPos, vLookAhead, NULL ) )
	{
		return;
	}

	// Choose a new point on the vector of travel.

	LTVector vTravelDir = *pvNewPos - pAI->GetPosition();
	if( vTravelDir != LTVector::GetIdentity() )
	{
		vTravelDir.Normalize();
	}

	// Stop the AI at the specified distance away from the link.

	LTVector vIntersect;
	if( pPoly->RayIntersectPoly2D( pAI->GetPosition(), pAI->GetPosition() + ( vTravelDir * 1000.f), &vIntersect ) )
	{
		float fNewPosY = pvNewPos->y;
		*pvNewPos = vIntersect - ( vTravelDir * GetNMLinkOffsetEntryDistA() );
		pvNewPos->y = fNewPosY;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlyThru::HandleNavMeshLinkEnter
//              
//	PURPOSE:	Handle entering the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkFlyThru::HandleNavMeshLinkEnter( CAI* pAI )
{
	const char* pszFX = g_pAIDB->GetMiscString( FLYTHRU_ENTER_FX );
	CreateFlyThruFX( pAI, -1.f, pszFX );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlyThru::HandleNavMeshLinkExit
//              
//	PURPOSE:	Handle exiting the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkFlyThru::HandleNavMeshLinkExit( CAI* pAI )
{
	const char* pszFX = g_pAIDB->GetMiscString( FLYTHRU_EXIT_FX );
	CreateFlyThruFX( pAI, 1.f, pszFX );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlyThru::CreateFlyThruFX
//              
//	PURPOSE:	Create FX for flying thru something.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkFlyThru::CreateFlyThruFX( CAI* pAI, float fDirMult, const char* pszFX )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Intersect the wall we are flying thru.

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = pAI->GetPosition() + fDirMult * ( pAI->GetForwardVector() * pAI->GetRadius() );
	IQuery.m_To = pAI->GetPosition() - fDirMult * ( pAI->GetForwardVector() * pAI->GetRadius() );

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = GroundFilterFn;

	// No intersection.

	if( !g_pLTServer->IntersectSegment(IQuery, &IInfo) )
	{
		return;
	}

	// Not intersecting a wall.

	if( !( IsMainWorld(IInfo.m_hObject) || 
		 ( OT_WORLDMODEL == GetObjectType(IInfo.m_hObject) ) ) )
	{
		return;
	}

	// Found an intersection.

	LTVector vPos = IInfo.m_Point + ( IInfo.m_Plane.m_Normal * 1.f );
	LTRotation rRot( IInfo.m_Plane.m_Normal, LTVector( 0.f, 1.f, 0.f ) );

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_CLIENTFXGROUPINSTANT );
	cMsg.WriteString( pszFX );
	cMsg.Writebool( false );
	cMsg.Writebool( false );

	// No special parent.
	cMsg.Writebool( false );
	cMsg.WriteLTVector( vPos );
	cMsg.WriteCompLTRotation( rRot );

	// No target.
	cMsg.Writebool( false );

	g_pLTServer->SendSFXMessage( cMsg.Read(), 0 );
}











