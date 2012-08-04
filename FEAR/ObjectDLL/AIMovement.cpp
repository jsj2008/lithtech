// ----------------------------------------------------------------------- //
//
// MODULE  : AIMovement.cpp
//
// PURPOSE : 
//
// CREATED : 
//
// (c) 1997-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIMovement.h"
#include "AI.h"
#include "AIDB.h"
#include "AnimationContext.h"
#include "AIUtils.h"
#include "AIBrain.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"
#include "CharacterMgr.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIMovementUtils.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::CAIMovement / ~CAIMovement
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAIMovement::CAIMovement()
{
	m_pAI = NULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
	m_bUnderwater = false;
	m_bClimbing = false;
	m_bFaceDest = true;
	m_eCurrentMovementType = kAD_None;
	m_eLastMovementType = kAD_None;
	m_fLastMovementUpdate = 0.f;
	m_fAnimRate = 1.f;
	m_bNoDynamicPathfinding = false;
	m_bAllowTargetPenetration = true;
	m_bMoved = false;

	m_bDoParabola = false;
	m_fParabolaPeakDist = 0.f;
	m_fParabolaPeakHeight = 0.f;
	m_fParabola_a = 0.f;
	m_bParabolaPeaked = false;

	m_bSetupScale = false;
	m_vScale.Init( 1.f, 1.f, 1.f );

	m_bSetupHeightInterpolation = false;
	m_fHeightInterpolationTotalDist = 0.f;
	m_fHeightInterpolationTotalHeight = 0.f;
	m_fHeightInterpolationInitialHeight = 0.f;

	m_fSpeed = 250.0f;

	m_fRotationTime = -1.0f;
	m_fRotationTimer = 0.0f;
	m_bLimitRotation = true;

	m_vObjectRight.Init();
	m_vObjectUp.Init();
	m_vObjectForward.Init();

    m_rTargetRot.Init();

	m_flLastSlide = 0.0f;
}

CAIMovement::~CAIMovement()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::InitAIMovement
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

bool CAIMovement::InitAIMovement(CAI *pAI)
{
	m_pAI = pAI;
		
	// Save our current position and our target rot
	
	// TODO: to maintain original behavior, this should only be done the 
	// first initial update.  Either these should be done by the AI and fed
	// into the NavigationMgr, or they should be done conditionally here.

	g_pLTServer->GetObjectRotation(m_pAI->m_hObject, &m_rTargetRot);
	m_vTargetForward = m_rTargetRot.Forward();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::IsAtDest
//
//	PURPOSE:	Checks if AI is already at the dest.
//
// ----------------------------------------------------------------------- //

bool CAIMovement::IsAtDest(const LTVector& vDest)
{
	LTVector vMove = vDest - m_pAI->GetPosition();

	// See if we're already at the dest exactly.

	float fRemainingDist = vMove.Mag();
	if(fRemainingDist == 0.f)
	{
		SetMovementDone();
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Update
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIMovement::Update()
{
	UpdateRotation();
	UpdateMovement();

	m_fLastMovementUpdate = g_pLTServer->GetTime();

	// Clear the movement encoding every frame to prevent accidental
	// accumulation.

	g_pModelLT->ResetMovementEncodingTransform( m_pAI->m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::AvoidDynamicObstacles
//
//	PURPOSE:	Repel AI off other characters.
//
// ----------------------------------------------------------------------- //

void CAIMovement::AvoidDynamicObstacles(LTVector* pvNewPos, State eState, EnumAnimDesc eMovementType)
{
	// Constrain the periods the AI may apply dynamic obstacle avoidance.

	if( ( eState != eStateSet ) 
		|| ( m_bNoDynamicPathfinding ) 
		|| ( eMovementType == kAD_MOV_Encode_V ) 
		|| ( g_pLTServer->GetTime() - m_flLastSlide < 1.0 ) )
	{
		return;
	}

	float fRadius = 128.f;
	float fRadiusSqr = fRadius * fRadius;

	LTVector vMyPos = m_pAI->GetPosition();

	// Calculate the horizontal velocity.

	LTVector vVel = *pvNewPos - vMyPos;
	vVel.y = 0.f;

	// Bail if no velocity.

	if( ( vVel.x == 0.f ) && ( vVel.z == 0.f ) )
	{
		return;
	}

	float fMag = vVel.Mag();

	LTVector vTotalForce(0.f, 0.f, 0.f);
	LTVector vObstaclePos;
	float fDistSqr;
	float fForce;
	LTVector vForce;

	CTList<CCharacter*>* lstChars	= NULL;
	CCharacter** pCur				= NULL;

	// Iterate over all characters in the world.

	int cCharLists = g_pCharacterMgr->GetNumCharacterLists();
	for ( int iList = 0 ; iList < cCharLists ; ++iList )
	{
		lstChars = g_pCharacterMgr->GetCharacterList(iList);

		pCur = lstChars->GetItem(TLIT_FIRST);
		while( pCur )
		{
			CCharacter* pChar = (CCharacter*)*pCur;
			pCur = lstChars->GetItem(TLIT_NEXT);

			// Ignore myself.

			if( pChar == m_pAI )
			{
				continue;
			}

			// Ignore characters that are too close to our dest.
			// The pathfinding system requires AIs to reach waypoints.

			g_pLTServer->GetObjectPos( pChar->m_hObject, &vObstaclePos );
			if( vObstaclePos.DistSqr( m_vDest ) <= fRadiusSqr )
			{
				continue;
			}

			// Only characters within radius have forces that affect me.

			fDistSqr = vObstaclePos.DistSqr( vMyPos );
			if( fDistSqr >= fRadiusSqr )
			{
				continue;
			}

			// Calculate the force vector from the obstacle to myself.

			fForce = fRadius - (float)sqrt( fDistSqr );
			fForce /= fRadius;
			fForce *= fForce;
			fForce *= ( 2.f * fMag );

			if( vMyPos != vObstaclePos )
			{
				vForce = vMyPos - vObstaclePos;
				vForce.y = 0.f;
				if( ( vForce.x != 0.f ) ||
					( vForce.z != 0.f ) )
				{
					vForce.Normalize();
					vForce *= fForce;
				}

				// Accumulate the total force from all obstacles.

				vTotalForce += vForce;
			}
		}
	}

	// Bail if no forces are affecting me.

	if( ( vTotalForce.x == 0.f ) && ( vTotalForce.z == 0.f ) )
	{
		return;
	}

	// Calculate a new velocity vector.

	LTVector vNewVel = vVel + vTotalForce;

	// Constrain velocity so that is never deviates more than 
	// 90 degrees in either direction.  This prevents AIs from ever
	// reversing their direction when the forces are stronger than
	// the initial velocity.

	if( vNewVel.Dot( vVel ) < 0.f )
	{
		vVel.Normalize();
		LTVector vUp( 0.f, 1.f, 0.f );
		LTVector vRight = vUp.Cross( vVel );

		if( vRight.Dot( vNewVel ) < 0.f )
		{
			vNewVel = -vRight;
		}
		else {
			vNewVel = vRight;
		}
	}

	// Keep magnitude of velocity constant.

	vNewVel.Normalize();
	LTVector vOffset = vNewVel * fMag;

	// Calculate new position.

	LTVector vNewPos = vMyPos + vOffset;

	// Ensure AI stays in NavMesh.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vNewPos, m_pAI->GetLastNavMeshPoly(), m_pAI->GetRadius() ) )
	{
		return;
	}

	// Move toward new position.

	*pvNewPos = vNewPos;

	// Face the appropriate direction.

	FacePath( eMovementType, vNewPos, vNewVel );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateConstantVelocity
//
//	PURPOSE:	Updates movement for constant velocity animations.
//
// ----------------------------------------------------------------------- //

bool CAIMovement::UpdateConstantVelocity( EnumAnimDesc eMovementType, LTVector* pvNewPos )
{
	// Find our unit movement vector

	LTVector vMove = m_vDest - m_pAI->GetPosition();

	// Set our speed based on our movement type

	switch( eMovementType )
	{
		case kAD_MOV_Set:
			vMove.y = 0.0f;
			m_fSpeed = m_fSetSpeed;
			break;

		case kAD_MOV_Walk:
			vMove.y = 0.0f;
			m_fSpeed = m_pAI->GetWalkSpeed();
			break;

		case kAD_MOV_Run:
			vMove.y = 0.0f;
			m_fSpeed = m_pAI->GetRunSpeed();
			break;

		case kAD_MOV_Swim:
			vMove.y = 0.0f;
			m_fSpeed = m_pAI->GetSwimSpeed();
			break;

		case kAD_MOV_Climb:
			vMove.x = 0.0f;
			vMove.z = 0.0f;
			m_fSpeed = m_pAI->GetWalkSpeed();

			// Turn off gravity
			m_pAI->SetCheapMovement(false);
			m_bFaceDest = false;
			break;

		case kAD_MOV_JumpOver:
			vMove.y = 0.0f;
			m_fSpeed = m_pAI->GetJumpOverSpeed();

			// Turn off gravity
			m_pAI->SetCheapMovement(false);
			break;

		case kAD_MOV_JumpUp:
			m_fSpeed = m_pAI->GetJumpSpeed();
			m_bFaceDest = false;

			// Turn off gravity
			m_pAI->SetCheapMovement(false);

			if( m_eLastMovementType != kAD_MOV_JumpUp )
			{
				vMove = m_vDest - m_pAI->GetPosition();
			}
			break;

		case kAD_MOV_Fall:
			m_fSpeed = m_pAI->GetFallSpeed();
			m_bFaceDest = false;

			// Turn off gravity
			m_pAI->SetCheapMovement(false);

			if( m_eLastMovementType != kAD_MOV_Fall )
			{
				vMove = m_vDest - m_pAI->GetPosition();
			}
			break;

		default:
			AIASSERT( 0, m_pAI->GetHOBJECT(), "Unknown Movement type!" );
			break;
	}

	// See if we'll overshoot our dest.

	float fRemainingDist = vMove.Mag();
	if(fRemainingDist == 0.f)
	{
		SetMovementDone();
		return false;
	}

	float fMoveDist;
	float fTimeDelta = (float)(g_pLTServer->GetTime() - m_fLastMovementUpdate);

	fMoveDist = GetSpeed()*fTimeDelta;

	// If we'd overshoot our destination, just move us there

	if ( fRemainingDist < fMoveDist )
	{
		*pvNewPos = m_vDest;

		// If the movement does not include any vertical, then
		// do not affect the elevation of the AI. Let CheapMovement
		// take care of putting the AI on the ground.

		if( vMove.y == 0.f )
		{
			pvNewPos->y = m_pAI->GetPosition().y;
		}

		SetMovementDone();
		return true;
	}

	// Scale based on our movement distance

	vMove.Normalize();
	vMove *= fMoveDist;

	// Calculate our new position

	*pvNewPos = m_pAI->GetPosition() + vMove;

	// Face us in the right direction

	if ( m_bFaceDest )
	{
		m_pAI->GetAIBlackBoard()->SetBBFacePos( *pvNewPos );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateParabola
//
//	PURPOSE:	Updates height in parabola based on distance covered.
//
// ----------------------------------------------------------------------- //

float CAIMovement::UpdateParabola()
{
	LTVector vOrigin2D = m_vParabolaOrigin;
	vOrigin2D.y = 0.f;

	LTVector vPos = m_pAI->GetPosition();
	vPos.y = 0.f;

	float fDist = vOrigin2D.Dist( vPos );

	// Reset the parabola variables after it has hit its peak
	// height, to account for a destination at a different
	// elevation than the origin.

	if(!m_bParabolaPeaked && (fDist > m_fParabolaPeakDist))
	{
		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From = m_vDest;
		IQuery.m_To = m_vDest;
		IQuery.m_From.y += m_pAI->GetDims().y;
		IQuery.m_To.y -= m_pAI->GetDims().y * 10.f;

		IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		IQuery.m_FilterFn = GroundFilterFn;

		float fHeight = m_vDest.y;
		if( g_pLTServer->IntersectSegment(IQuery, &IInfo) && ( IsMainWorld(IInfo.m_hObject) || ( OT_WORLDMODEL == GetObjectType(IInfo.m_hObject) ) ) )
		{
			fHeight = IInfo.m_Point.y;
		}

		m_fParabolaPeakHeight += m_vParabolaOrigin.y - ( fHeight + m_pAI->GetDims().y );
		m_vParabolaOrigin.y = fHeight + m_pAI->GetDims().y;

		m_fParabola_a = m_fParabolaPeakHeight / ( m_fParabolaPeakDist * m_fParabolaPeakDist );
		m_bParabolaPeaked = true;
	}

	return (-m_fParabola_a * ((fDist - m_fParabolaPeakDist) * (fDist - m_fParabolaPeakDist)) ) + m_fParabolaPeakHeight;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateMovementEncoding
//
//	PURPOSE:	Updates movement for movement encoded animations.
//
// ----------------------------------------------------------------------- //

bool CAIMovement::UpdateMovementEncoding( EnumAnimDesc eMovementType, LTVector* pvNewPos )
{
	// Encode_NG means encoding with NoGravity.
	bool bUseGravity = true;
	if( ( eMovementType == kAD_MOV_Encode_NG ) || 
		( eMovementType == kAD_MOV_Encode_NGB ) ||
		( eMovementType == kAD_MOV_Encode_NGL ) ||
		( eMovementType == kAD_MOV_Encode_NGR ) ||
		( eMovementType == kAD_MOV_Encode_V ) )
	{
		// Turn off gravity
		m_pAI->SetCheapMovement(false);
		bUseGravity = false;
	}

	//get the current movement encoding transformation
	LTRigidTransform tMovementEncoding;
	g_pModelLT->GetMovementEncodingTransform( m_pAI->m_hObject, tMovementEncoding);

	// Scale the movement encoding based on the current animation blending on 
	// the main tracker.

	float flBlendPercent = m_pAI->GetAnimationContext()->GetBlendPercent();
	if ( (  1.0f != flBlendPercent )
		&& ( m_pAI->GetAnimationContext()->GetBlendFlags() & kBlendFlag_BlendTranslation ) )
	{
		// This is interpolating from 0,0,0 to tMovementEncoding.m_vPos by flBlendPercent:
		// tMovementEncoding.m_vPos = LTVector::GetIdentity() + (tMovementEncoding.m_vPos - LTVector::GetIdentity() ) * flBlendPercent;
		// As it is always starting at zero, none of this math really needs to get done.
		tMovementEncoding.m_vPos = tMovementEncoding.m_vPos * flBlendPercent;
	}

	// If the AI has some target rotation set, apply movement encoding
	// as if the AI has already reached the target rotation.  This prevents
	// the need to immediatly pop the AI's rotation, or wait until the AI has
	// reached a rotation before animating.

	LTRigidTransform tObject;
	g_pLTServer->GetObjectPos(m_pAI->m_hObject, &tObject.m_vPos);
	if( m_fRotationTimer < m_fRotationTime )
	{
		tObject.m_rRot = m_rTargetRot;
	}
	else {
		g_pLTServer->GetObjectRotation(m_pAI->m_hObject, &tObject.m_rRot);
	}

	LTVector vOldPos = tObject.m_vPos;

	//the world space movement encoding transform
	LTRigidTransform tMovementWS = tObject * tMovementEncoding;

	// HACK: This is a hack for FEAR to fix AI sliding sidewalks while
	// standing up from a crawl.

	if( m_pAI->GetMoveToFloor() &&
		m_pAI->GetAnimationContext()->IsTransitioning() &&
		( ( eMovementType == kAD_MOV_Encode_NG ) ||
		( eMovementType == kAD_MOV_Encode_NGB ) ||
		( eMovementType == kAD_MOV_Encode_NGL ) ||
		( eMovementType == kAD_MOV_Encode_NGR ) ) )
	{
		m_eState = eStateUnset;
	}

	// END HACK

	// No destination is set, so just add the hint vector.
	if( m_eState != eStateSet )
	{
		LTRotation rRot = tObject.m_rRot;
		ApplyMovementEncodingToRot( &rRot, tMovementEncoding );

		// If the AI has some target rotation set, apply rotation to the
		// target rotation rather than apply it to the AI.  This prevents
		// the need to immediatly pop the AI's rotation, or wait until the 
		// AI has reached a rotation before animating.

		if( m_fRotationTimer < m_fRotationTime )
		{
			m_rTargetRot = rRot;
			ApplyMovementEncodingToRot( &m_rStartRot, tMovementEncoding );
		}
		else {
			g_pLTServer->SetObjectRotation( m_pAI->m_hObject, rRot );
		}

		// Ensure animation does not move AI out of NavMesh.
		// AI may exit NavMesh while playing animations without gravity.

		if( bUseGravity )
		{
			ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( tMovementWS.m_vPos, m_pAI->GetCharTypeMask(), m_pAI->GetLastNavMeshPoly(), m_pAI );
			if( ePoly == kNMPoly_Invalid )
			{
				return false;
			}
		}

		*pvNewPos = tMovementWS.m_vPos;
	}

	// A destination has been set, so just use the magnitude of 
	// the xz compnents of the hint vector, to guarantee we're 
	// headed to our dest.

	else
	{
		LTVector vOldDirToDest;
		LTVector vNewDirToDest;

		if( eMovementType == kAD_MOV_Encode_V )
		{
			// The movement encoding hint for a vertical animation 
			// should only include changes in y.

			*pvNewPos = vOldPos;
			pvNewPos->y = tMovementWS.m_vPos.y;//vOldPos + tMovementEncoding.m_vPos;
			
			// AI has reached the top.

			if( ( vOldPos.y < m_vDest.y ) &&
				( pvNewPos->y >= m_vDest.y ) )
			{
				SetMovementDone();

				*pvNewPos = m_vDest;
				return true;
			}
			
			// AI has reached the bottom.

			if( ( vOldPos.y - m_pAI->GetDims().y > m_vDest.y ) &&
				( pvNewPos->y - m_pAI->GetDims().y <= m_vDest.y ) )
			{
				SetMovementDone();

				*pvNewPos = m_vDest;
				pvNewPos->y += m_pAI->GetDims().y;
				return true;
			}

			return true;
		}

		else {
			if( ( vOldPos.x == m_vDest.x ) &&
				( vOldPos.z == m_vDest.z ) )
			{
				SetMovementDone();
				return false;
			}

			// Get the direction from the old pos to the dest.

			vOldDirToDest = m_vDest - vOldPos;
			vOldDirToDest.y = 0.f;
			vOldDirToDest.Normalize();

			// Get the x and z components of the hint vector.

			LTVector vHintXZ = (tMovementWS.m_vPos - vOldPos);
			vHintXZ.y = 0.f;
		
			// Interpolate AI's height to reach the destination.

			if( m_bSetupHeightInterpolation && !m_bLimitRotation )
			{
				SetupHeightInterpolation();
			}

			// Scale the animation to reach the destination.

			else if( m_bSetupScale && !m_bLimitRotation )
			{
				SetupMovementScaling();
			}

			// Set the new pos to the sum of the old pos, and the 
			// magnitude of the xz components of the hint, towards 
			// the dest.  Then add the y component.

			*pvNewPos = vOldPos + ( vOldDirToDest * vHintXZ.Mag() * m_vScale.z );
			pvNewPos->y += tMovementEncoding.m_vPos.y * m_vScale.y;

			if( m_fHeightInterpolationTotalDist > 0.f )
			{
				pvNewPos->y = InterpolateMovementHeight();
			}

			// See if we'll overshoot our dest.

			vNewDirToDest = m_vDest - *pvNewPos;
			vNewDirToDest.y = 0.f;
			if( vNewDirToDest != LTVector::GetIdentity() )
			{
				vNewDirToDest.Normalize();
			}
		}

		if( vOldDirToDest.Dot(vNewDirToDest) < 0.f )
		{
			SetMovementDone();

			*pvNewPos = m_vDest;

			// If the movement is not vertical, then do not affect the 
			// elevation of the AI. Let CheapMovement take care of 
			// putting the AI on the ground.

			if( eMovementType != kAD_MOV_Encode_V )
			{
				pvNewPos->y = m_pAI->GetPosition().y;
			}
		}
		else if( m_bFaceDest ) {
			FacePath( eMovementType, m_vDest, vNewDirToDest );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::SetupMovementScaling
//
//	PURPOSE:	Calculate the amount to scale each axis.
//
// ----------------------------------------------------------------------- //

void CAIMovement::SetupMovementScaling()
{
	if( m_pAI->GetAnimationContext()->IsTransitioning() )
	{
		return;
	}

	// Calculate scale based on the currently playing animation.

	HMODELANIM hAnim;
	g_pModelLT->GetCurAnim( m_pAI->m_hObject, MAIN_TRACKER, hAnim );

	// Number of keyframes.

	uint32 nNumKeyFrames;
	g_pModelLT->GetAnimKeyFrames( m_pAI->m_hObject, hAnim, nNumKeyFrames );

	// Root node.

	HMODELNODE hNode;
	g_pModelLT->GetRootNode( m_pAI->m_hObject, hNode );

	// Get the transform for the end of the animation.

	LTRigidTransform tTransform;
	g_pModelLT->GetAnimNodeTransform( m_pAI->m_hObject, hAnim, hNode, nNumKeyFrames - 1, tTransform );

	// Get the transform now.

	LTTransform tTransformNow;
	g_pModelLT->GetNodeTransform( m_pAI->m_hObject, hNode, tTransformNow, false );

	// Calculate horizontal scaling.

	LTVector vToDest = m_vDest - m_pAI->GetPosition();
	vToDest.y = 0.f;
	float fZDist = vToDest.Mag();

	LTVector vAnim = tTransform.m_vPos - tTransformNow.m_vPos;
	vAnim.y = 0.f;
	float fZAnim = vAnim.Mag();

	m_vScale.z = fZDist / fZAnim;

	// Scale the animation rate to match the distance travelled.

	m_fAnimRate = 1.f / m_vScale.z;
	m_pAI->GetAnimationContext()->SetOverrideAnimRate( m_fAnimRate );

	// Calculate vertical scaling.

	float fYDist = m_vDest.y - m_pAI->GetPosition().y;
	float fYAnim = tTransform.m_vPos.y - tTransformNow.m_vPos.y;

	m_vScale.y = fYDist / fYAnim;

	m_bSetupScale = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::SetupHeightInterpolation
//
//	PURPOSE:	Calculate the total distance to interpolate.
//
// ----------------------------------------------------------------------- //

void CAIMovement::SetupHeightInterpolation()
{
	if( m_pAI->GetAnimationContext()->IsTransitioning() )
	{
		return;
	}

	// Calculate the total horizontal distance to cover.

	LTVector vToDest = m_vDest - m_pAI->GetPosition();
	vToDest.y = 0.f;
	m_fHeightInterpolationTotalDist = vToDest.Mag();

	// Calculate the total vertical distance to cover.

	m_fHeightInterpolationTotalHeight = m_vDest.y - m_pAI->GetPosition().y;
	m_fHeightInterpolationInitialHeight = m_pAI->GetPosition().y;

	m_bSetupHeightInterpolation = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::InterpolateMovementHeight
//
//	PURPOSE:	Return the interpolated height at this point in the movement.
//
// ----------------------------------------------------------------------- //

float CAIMovement::InterpolateMovementHeight()
{
	// Calculate horizontal distance to dest.

	LTVector vToDest = m_vDest - m_pAI->GetPosition();
	vToDest.y = 0.f;
	float fDistToDest = vToDest.Mag();

	// Calculate what percentage of the way we are to our dest.

	float percent = 1.f - ( fDistToDest / m_fHeightInterpolationTotalDist );

	// Calculate the interpolated height.

	return m_fHeightInterpolationInitialHeight + ( m_fHeightInterpolationTotalHeight * percent );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::FacePath
//
//	PURPOSE:	Face some direction relative to the AI's path.
//
// ----------------------------------------------------------------------- //

void CAIMovement::FacePath( EnumAnimDesc eMovementType, const LTVector& vDest, const LTVector& vDestDir )
{
	// Do not turn the AI while playing a transition
	// with no gravity, because transition may movement 
	// encode the AI out of something (e.g. a bed, or off 
	// a wall, etc).

	if( m_pAI->GetMoveToFloor() &&
		m_pAI->GetAnimationContext()->IsTransitioning() &&
		( ( eMovementType == kAD_MOV_Encode_NG ) ||
		  ( eMovementType == kAD_MOV_Encode_NGB ) ||
		  ( eMovementType == kAD_MOV_Encode_NGL ) ||
		  ( eMovementType == kAD_MOV_Encode_NGR ) ) )
	{
		m_bLimitRotation = false;
		return;
	}

	switch( eMovementType )
	{
		case kAD_MOV_Encode_GB:
		case kAD_MOV_Encode_NGB:
			m_pAI->GetAIBlackBoard()->SetBBFacePos( m_pAI->GetPosition() );
			m_bLimitRotation = false;
			break;

		case kAD_MOV_Encode_GL:
		case kAD_MOV_Encode_NGL:
			{
				LTVector vUp( 0.f, 1.f, 0.f );
				LTVector vRight = vDestDir.Cross( vUp );
				m_pAI->GetAIBlackBoard()->SetBBFaceDir( vRight );
				m_bLimitRotation = false;
			}
			break;

		case kAD_MOV_Encode_GR:
		case kAD_MOV_Encode_NGR:
			{
				LTVector vUp( 0.f, 1.f, 0.f );
				LTVector vRight = vDestDir.Cross( vUp );
				m_pAI->GetAIBlackBoard()->SetBBFaceDir( -vRight );
				m_bLimitRotation = false;
			}
			break;

		default:
			m_pAI->GetAIBlackBoard()->SetBBFacePos( vDest );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::ApplyMovementEncodingToRot
//
//	PURPOSE:	Apply the movement encoding transform to a rotation.
//
// ----------------------------------------------------------------------- //

void CAIMovement::ApplyMovementEncodingToRot( LTRotation* pRot, const LTRigidTransform& tMovementEncoding )
{	
	// Sanity check.

	if( !pRot )
	{
		return;
	}

	LTRigidTransform tObject;
	tObject.m_rRot = *pRot;

	//the world space movement encoding transform
	LTRigidTransform tMovementWS = tObject * tMovementEncoding;

	LTRotation rRot = tMovementWS.m_rRot;
	float fInvMag = 1.0f/sqrtf(rRot.m_Quat[0]*rRot.m_Quat[0] + rRot.m_Quat[1]*rRot.m_Quat[1] +
								rRot.m_Quat[2]*rRot.m_Quat[2] + rRot.m_Quat[3]*rRot.m_Quat[3]);

	rRot.m_Quat[0] *= fInvMag;
	rRot.m_Quat[1] *= fInvMag;
	rRot.m_Quat[2] *= fInvMag;
	rRot.m_Quat[3] *= fInvMag;

	*pRot = rRot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::SetMovementDone
//
//	PURPOSE:	Helper function to handle setting the movement to a 'done'
//				state when movement is complete.
//
// ----------------------------------------------------------------------- //

void CAIMovement::SetMovementDone()
{
	ClearMovement();
	m_eState = eStateDone;
	m_pAI->GetAIBlackBoard()->SetBBAdvancePath(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovement::Load(ILTMessage_Read *pMsg)
{	
	LOAD_INT_CAST(m_eState, State);
    LOAD_VECTOR(m_vDest);
	LOAD_bool(m_bUnderwater);
    LOAD_bool(m_bClimbing);
	LOAD_bool(m_bFaceDest);
	LOAD_FLOAT(m_fSetSpeed);
	LOAD_INT_CAST(m_eCurrentMovementType, EnumAnimDesc);
	LOAD_INT_CAST(m_eLastMovementType, EnumAnimDesc);
	LOAD_TIME(m_fLastMovementUpdate);
	LOAD_FLOAT(m_fAnimRate);
	LOAD_bool(m_bNoDynamicPathfinding);
	LOAD_bool(m_bAllowTargetPenetration);
	LOAD_bool(m_bMoved);

	LOAD_bool(m_bDoParabola);
	LOAD_VECTOR(m_vParabolaOrigin);
	LOAD_FLOAT(m_fParabolaPeakDist);
	LOAD_FLOAT(m_fParabolaPeakHeight);
	LOAD_FLOAT(m_fParabola_a);
	LOAD_bool(m_bParabolaPeaked);

	LOAD_bool(m_bSetupScale);
	LOAD_VECTOR(m_vScale);

	LOAD_bool(m_bSetupHeightInterpolation);
	LOAD_FLOAT(m_fHeightInterpolationTotalDist);
	LOAD_FLOAT(m_fHeightInterpolationTotalHeight);
	LOAD_FLOAT(m_fHeightInterpolationInitialHeight);

	LOAD_FLOAT(m_fSpeed);

	LOAD_ROTATION(m_rTargetRot);
    LOAD_ROTATION(m_rStartRot);
    LOAD_VECTOR(m_vTargetForward);
    LOAD_DOUBLE(m_fRotationTime);
    LOAD_DOUBLE(m_fRotationTimer);
	LOAD_bool(m_bLimitRotation);

	LOAD_VECTOR(m_vObjectRight);
	LOAD_VECTOR(m_vObjectUp);
	LOAD_VECTOR(m_vObjectForward);

	LOAD_TIME(m_flLastSlide);

	// If we're loading from a transition, then our positional information is invalid.
	if( g_pGameServerShell->GetLGFlags( ) == LOAD_TRANSITION )
	{
		m_bMoved = false;

		ClearMovement();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void CAIMovement::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_eState);
    SAVE_VECTOR(m_vDest);
	SAVE_bool(m_bUnderwater);
    SAVE_bool(m_bClimbing);
	SAVE_bool(m_bFaceDest);
	SAVE_FLOAT(m_fSetSpeed);
	SAVE_INT(m_eCurrentMovementType);
	SAVE_INT(m_eLastMovementType);
	SAVE_TIME(m_fLastMovementUpdate);
	SAVE_FLOAT(m_fAnimRate);
	SAVE_bool(m_bNoDynamicPathfinding);
	SAVE_bool(m_bAllowTargetPenetration);
	SAVE_bool(m_bMoved);

	SAVE_bool(m_bDoParabola);
	SAVE_VECTOR(m_vParabolaOrigin);
	SAVE_FLOAT(m_fParabolaPeakDist);
	SAVE_FLOAT(m_fParabolaPeakHeight);
	SAVE_FLOAT(m_fParabola_a);
	SAVE_bool(m_bParabolaPeaked);

	SAVE_bool(m_bSetupScale);
	SAVE_VECTOR(m_vScale);

	SAVE_bool(m_bSetupHeightInterpolation);
	SAVE_FLOAT(m_fHeightInterpolationTotalDist);
	SAVE_FLOAT(m_fHeightInterpolationTotalHeight);
	SAVE_FLOAT(m_fHeightInterpolationInitialHeight);

	SAVE_FLOAT(m_fSpeed);

	SAVE_ROTATION(m_rTargetRot);
    SAVE_ROTATION(m_rStartRot);
    SAVE_VECTOR(m_vTargetForward);
    SAVE_DOUBLE(m_fRotationTime);
    SAVE_DOUBLE(m_fRotationTimer);
	SAVE_bool(m_bLimitRotation);

	SAVE_VECTOR(m_vObjectRight);
	SAVE_VECTOR(m_vObjectUp);
	SAVE_VECTOR(m_vObjectForward);

	SAVE_TIME(m_flLastSlide);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::SetMovementDest
//
//	PURPOSE:	Sets the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovement::SetMovementDest(const LTVector& vDest)
{
	m_vDest = vDest;
	m_eState = eStateSet;
	m_fSetSpeed = 0.f;
	m_bFaceDest = true;
	m_bDoParabola = false;
	m_bLimitRotation = true;

	m_bSetupScale = m_pAI->GetAIBlackBoard()->GetBBScaleMovement();
	m_vScale.Init( 1.f, 1.f, 1.f );

	m_bSetupHeightInterpolation = m_pAI->GetAIBlackBoard()->GetBBInterpolateMovementHeight();
	m_fHeightInterpolationTotalDist = 0.f;
	m_fHeightInterpolationTotalHeight = 0.f;
	m_fHeightInterpolationInitialHeight = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::ClearMovement
//
//	PURPOSE:	Clears the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovement::ClearMovement()
{ 
	m_eState = eStateUnset; 
	m_fSetSpeed = 0.f;
	m_bFaceDest = true;
	m_bAllowTargetPenetration = true;

	if( m_bDoParabola )
	{
		m_bDoParabola = false;
		m_pAI->SetCheapMovement( true );
	}

	if( m_fAnimRate != 1.f )
	{
		m_pAI->GetAnimationContext()->ClearOverrideAnimRate();
		m_fAnimRate = 1.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::SetParabola
//
//	PURPOSE:	Calculates a parabola
//
// ----------------------------------------------------------------------- //

void CAIMovement::SetParabola( float fHeight )
{
	//
	// Parabola:  y = ax^2
	//   Flip it: y = -ax^2
	//   Get to specified height:  a = h / (d^2), where d = 1/2 * dist
	//   Get above origin: y = -ax^2 + h
	//   Get right of origin: y = -a(x - d)^2 + h
	//

	if( fHeight < 0.f )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIMovement::SetParabola: Parabola height must be positive." );
		fHeight = 0.f;
	}

	m_bDoParabola = true;

	m_vParabolaOrigin = m_pAI->GetPosition();

	m_bParabolaPeaked = false;

	LTVector vOrigin2D = m_vParabolaOrigin;
	vOrigin2D.y = 0.f;

	LTVector vDest2D = m_vDest;
	vDest2D.y = 0.f;

	m_fParabolaPeakDist = vDest2D.Dist( vOrigin2D ) * 0.5f;
	m_fParabolaPeakHeight = fHeight;
	
	m_fParabola_a = m_fParabolaPeakHeight / ( m_fParabolaPeakDist * m_fParabolaPeakDist );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::GetSpeed
//
//	PURPOSE:	Gets our current movement speed
//
// ----------------------------------------------------------------------- //

float CAIMovement::GetSpeed()
{
	// We basically want to slow down if we're not pointing the way we're moving

	float fModifier = (1.0f + m_vTargetForward.Dot(m_vObjectForward))/2.0f;
	if( fModifier <= 0.f )
	{
		fModifier = 1.f;
	}

	return fModifier*m_fSpeed;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateMovement
//
//	PURPOSE:	Update the objects position.
//
// ----------------------------------------------------------------------- //

bool CAIMovement::UpdateMovement()
{
	// Clear any past movement.
	m_fSpeed = 0.0f;
	m_pAI->SetCheapMovement(true);

	LTVector vNewPos;
	bool bMove = true;

	State eStatePrev = m_eState;
	EnumAnimDesc eMovementType = m_pAI->GetAnimationContext()->GetAnimMovementType();

	switch( eMovementType )
	{
	case kAD_None: 

		// Ensure AI stays in volumes.

		bMove = false;
		m_bMoved = false;
		break;

	case kAD_MOV_Set:
	case kAD_MOV_Walk:
	case kAD_MOV_Run:
	case kAD_MOV_JumpUp:
	case kAD_MOV_JumpOver:
	case kAD_MOV_Fall:
	case kAD_MOV_Climb:
	case kAD_MOV_Swim:
		if( (m_eState != eStateSet) || 
			(!UpdateConstantVelocity( eMovementType, &vNewPos ) ) )
		{
			bMove = false;
		}
		break;

	case kAD_MOV_Encode_NG:
	case kAD_MOV_Encode_NGB:
	case kAD_MOV_Encode_NGL:
	case kAD_MOV_Encode_NGR:
	case kAD_MOV_Encode_G:
	case kAD_MOV_Encode_GB:
	case kAD_MOV_Encode_GL:
	case kAD_MOV_Encode_GR:
	case kAD_MOV_Encode_V:
		if( (m_eState == eStateDone) ||
			( !UpdateMovementEncoding( eMovementType, &vNewPos ) ) )
		{
			bMove = false;
		}
		break;

	default:
		AIASSERT( 0, m_pAI->GetHOBJECT(), "Unknown Movement type!" );
		break;
	}

	m_eLastMovementType = m_eCurrentMovementType;
	m_eCurrentMovementType = eMovementType;

	if( bMove && ( m_pAI->GetPosition() != vNewPos ) )
	{
		// Do not allow movement to penetrate a target.
		// By default, movement may penetrate the target.
		// Actions (e.g. AttackMelee) may choose not to.

		m_pAI->GetAIBlackBoard()->SetBBCollidedWithTarget( false );
		if( !m_bAllowTargetPenetration )
		{
			if( !PreventTargetPenetration( vNewPos ) )
			{
				m_pAI->GetAIBlackBoard()->SetBBCollidedWithTarget( true );
				return false;
			}
		}

		// Adjust the height for a parabola.

		if( m_bDoParabola )
		{
			m_pAI->SetCheapMovement( false );
			vNewPos.y = m_vParabolaOrigin.y + UpdateParabola();
		}

		// Ensure AI does not movement encode out of the NavMesh.

		else if( eStatePrev != eStateSet ) 
		{
			// Allow a NavMeshLink to modify the new position.

			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
			if( pLink )
			{
				pLink->ModifyMovement( m_pAI, eStatePrev, &vNewPos, &m_eState );
			}

			// Don't movement encode out of the NavMesh completely,
			// when playing a no-gravity animation.

			if( ( eMovementType == kAD_MOV_Encode_NG ) ||
				( eMovementType == kAD_MOV_Encode_NGB ) ||
				( eMovementType == kAD_MOV_Encode_NGL ) ||
				( eMovementType == kAD_MOV_Encode_NGR ) )
			{
				ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vNewPos, m_pAI->GetCharTypeMask(), m_pAI->GetLastNavMeshPoly(), m_pAI );
				if( ePoly == kNMPoly_Invalid )
				{
					return false;
				}
			}
			else 
			{
				// Depending on blackboard settings, either perform a straight
				// path test the AIs radius or as a point. Tests as a point may
				// make sense for 'intentional' movement encoding.
				
				float flTestRadius = m_pAI->GetAIBlackBoard()->GetBBMovementEncodeUseRadius() ? m_pAI->GetRadius() : 0.0f;
				if( !g_pAIPathMgrNavMesh->StraightPathExists( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vNewPos, m_pAI->GetLastNavMeshPoly(), flTestRadius ) )
				{
					return false;
				}
			}
		}

		// Ensure AI does not move into a disabled Link.

		else if( eStatePrev == eStateSet )
		{
			ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vNewPos, m_pAI->GetCharTypeMask(), m_pAI->GetLastNavMeshPoly(), m_pAI );
			if( ePoly == kNMPoly_Invalid )
			{
				// Something is very wrong if the AI is trying to pathfind 
				// out of the NavMesh while walking on the ground.

				if( eMovementType == kAD_MOV_Encode_G )
				{
					m_pAI->GetAIBlackBoard()->SetBBInvalidatePath( true );
				}
				return false;
			}
			if( ePoly != m_pAI->GetLastNavMeshPoly() )
			{
				CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
				if( pPoly && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
				{
					AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
					if( pLink && ( !pLink->IsNMLinkEnabledToAI( m_pAI, !LINK_CHECK_TIMEOUT ) ) )
					{
						m_pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Failed );
						return false;
					}
				}
			}

			// Allow a NavMeshLink to modify the new position.

			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
			if( pLink )
			{
				pLink->ModifyMovement( m_pAI, eStatePrev, &vNewPos, &m_eState );
			}
		}

		// Limit rotation to data-defined rotation speed.

		LimitRotation( &vNewPos, eStatePrev );

		// Do not apply any obstacle avoidance unless the ObstacleAvoidance flag is set.

		if( m_pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() & kAIMovementFlag_ObstacleAvoidance )
		{
			// Avoid characters if AI has a destination, and movement 
			// is not locked, and AI is in volumes.

			AvoidDynamicObstacles( &vNewPos, eStatePrev, eMovementType );

			// Attempt to improve the the path by sliding along any obstructing characters.

			SlideAlongObstacles( &vNewPos, eStatePrev, eMovementType );

			// Prevent AIs from moving into other AIs

			PreventMovementIntoObstacles( &vNewPos, eStatePrev, eMovementType );
		}

		// Move us - tells the AI where to move to

		m_pAI->Move(vNewPos);
		m_bMoved = true;

		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::PreventMovementIntoObstacles
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIMovement::PreventMovementIntoObstacles( LTVector* pvNewPos, State eState, EnumAnimDesc eMovementType )
{
	// Sanity checks

	if( !pvNewPos )
	{
		return;
	}

	// Constrain the times the AI can be prevented from moving. 

	if( eMovementType == kAD_MOV_Encode_V )
	{
		return;
	}

	// Do not slide if we are in a link, and that link does not allow dynamic movement.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if ( pLink 
		&& ( pLink->IsInLinkOrOffsetEntry( m_pAI ) 
		&& !pLink->AllowDynamicMovement( m_pAI ) ) )
	{
		return;
	}

	// Generate some values that will be used repeatedly.

	LTVector vMovement = *pvNewPos - m_pAI->GetPosition();
	LTVector vMovement2D( vMovement.x, 0.0f, vMovement.z );
	float flMovementDistance = vMovement2D.Mag();
	float flMovementInY = vMovement.y;

	LTVector vTesterPos, vTesterDims;
	g_pLTServer->GetObjectPos( m_pAI->GetHOBJECT(), &vTesterPos );
	g_pPhysicsLT->GetObjectDims( m_pAI->GetHOBJECT(), &vTesterDims );
	float flTesterRadiusSqr = m_pAI->GetRadius();
	flTesterRadiusSqr *= flTesterRadiusSqr;

	// Determine if the position increases the collision with a character.  If it 
	// does, determine the sliding direction.
	
	int cCharLists = g_pCharacterMgr->GetNumCharacterLists();
	for ( int iList = 0 ; iList < cCharLists ; ++iList )
	{
		CTList<CCharacter*>* lstChars = g_pCharacterMgr->GetCharacterList(iList);

		CCharacter** pCur = lstChars->GetItem(TLIT_FIRST);
		while( pCur && *pCur)
		{
			CCharacter* pTestCharacter = *pCur;
			pCur = lstChars->GetItem(TLIT_NEXT);

			//
			// Ignore yourself.
			//

			if ( pTestCharacter == m_pAI )
			{
				continue;
			}

			//
			// Ignore dead characters
			//

			if ( pTestCharacter->IsDead() )
			{
				continue;
			}

			//
			// Ignore characters which are flagged as non solid to pathing.
			//

			if ( !pTestCharacter->IsSolidToAI() )
			{
				continue;
			}

			//
			// Ignore characters which do not collide with the new position
			//

			LTVector vCharacterPos, vCharacterDims;
			g_pLTServer->GetObjectPos( pTestCharacter->GetHOBJECT(), &vCharacterPos );
			g_pPhysicsLT->GetObjectDims( pTestCharacter->GetHOBJECT(), &vCharacterDims );
			float flCharacterRadiusSqr = pTestCharacter->GetRadius();
			flCharacterRadiusSqr *= flCharacterRadiusSqr;

			if ( !AIMovementUtils::Collides( 
				vCharacterPos, vCharacterDims, flCharacterRadiusSqr,
				vTesterPos, vTesterDims, flTesterRadiusSqr,
				AIMovementUtils::GetRadiusBuffer( pTestCharacter->GetHOBJECT() ) ) )
			{
				continue;
			}

			//
			// Ignore collisions if they reduce the collision (for instance,
			// an AI inside of another is moving away from it, which may 
			// occur if an AI was recently allowed to be solid.
			//

			LTVector vDistance2D = vCharacterPos - vTesterPos;
			vDistance2D.y = 0.0f;
			float flDot = vDistance2D.Dot( vMovement2D );
			if ( flDot <= 0 )
			{
				continue;
			}

			//
			// Handle a collision by preventing movement and flagging a 
			// collision if this is intentional movement by update working
			// memory and the black board.
			//

			if ( eStateSet == eState )
			{
				// Update the collision flags.

				uint32 iCollisionFlags = m_pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags();

				ENUM_AIWMKNOWLEDGE_TYPE eKnowledgeType = kKnowledge_InvalidType;				
				if ( AIMovementUtils::BlocksPosition2D( vCharacterPos, flCharacterRadiusSqr, flTesterRadiusSqr, m_vDest ) )
				{
					iCollisionFlags |= kAIMovementFlag_BlockedDestination; 
					eKnowledgeType = kKnowledge_BlockedDestination;
				}
				else
				{
					iCollisionFlags |= kAIMovementFlag_BlockedPath; 
					eKnowledgeType = kKnowledge_BlockedPath;
				}

				m_pAI->GetAIBlackBoard()->SetBBMovementCollisionFlags( iCollisionFlags );
			
				// Update or create a fact with details about the obstruction.

				if ( kKnowledge_InvalidType != eKnowledgeType )
				{
					CAIWMFact queryFact;
					queryFact.SetFactType( kFact_Knowledge );
					queryFact.SetKnowledgeType( eKnowledgeType );
					CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
					if ( !pFact )
					{
						pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
						if ( pFact )
						{
							pFact->SetKnowledgeType( eKnowledgeType );
						}
					}

					if ( pFact )
					{
						pFact->SetPos( m_vDest );
						pFact->SetTargetObject( pTestCharacter->GetHOBJECT() );
					}
				}
			}

			// Optionally prevent the movement. This behavior may or may not
			// be desired by different AIs. Only perform this if both
			// characters are flagged to prevent movement into obstacles. If
			// we don't do a bi-directional test, a soldier would be blocked
			// by a cockroach, or a cockroach by a soldier depending on who is
			// moving.

			if (m_pAI->GetAIAttributes()->bUsePreventMovementIntoObstacles
				&& pTestCharacter->GetAIAttributes()->bUsePreventMovementIntoObstacles)
			{
				*pvNewPos = m_pAI->GetPosition();
				return;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::SlideAlongObstacles
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIMovement::SlideAlongObstacles( LTVector* pvNewPos, State eState, EnumAnimDesc eMovementType )
{
	// Sanity checks

	if( !pvNewPos )
	{
		return;
	}

	// Do not slide if the movement is vertical.

	if( eMovementType == kAD_MOV_Encode_V )
	{
		return;
	}

	// Do not slide if we are in a link, and that link does not allow dynamic movement.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if ( pLink 
		&& ( pLink->IsInLinkOrOffsetEntry( m_pAI ) 
		&& !pLink->AllowDynamicMovement( m_pAI ) ) )
	{
		return;
	}

	// Generate some values that will be used repeatedly.

	LTVector vMovement = *pvNewPos - m_pAI->GetPosition();
	LTVector vMovement2D( vMovement.x, 0.0f, vMovement.z );
	LTVector vGoalMovement2D = vMovement2D;

	// Return if there is no movement along an axis we can slide on.

	if ( vMovement2D == LTVector::GetIdentity() )
	{
		return;
	}

	float flMovementDistance = vMovement2D.Mag();
	float flMovementInY = vMovement.y;

	LTVector vTesterPos, vTesterDims;
	g_pLTServer->GetObjectPos( m_pAI->GetHOBJECT(), &vTesterPos );
	g_pPhysicsLT->GetObjectDims( m_pAI->GetHOBJECT(), &vTesterDims );
	float flTesterRadiusSqr = m_pAI->GetRadius();
	flTesterRadiusSqr *= flTesterRadiusSqr;

	// Determine if the position increases the collision with a character.  If it 
	// does, determine the sliding direction.
	
	int cCharLists = g_pCharacterMgr->GetNumCharacterLists();
	for ( int iList = 0 ; iList < cCharLists ; ++iList )
	{
		CTList<CCharacter*>* lstChars = g_pCharacterMgr->GetCharacterList(iList);

		CCharacter** pCur = lstChars->GetItem(TLIT_FIRST);
		while( pCur && *pCur)
		{
			CCharacter* pTestCharacter = *pCur;
			pCur = lstChars->GetItem(TLIT_NEXT);

			//
			// Ignore yourself
			//

			if ( pTestCharacter == m_pAI )
			{
				continue;
			}

			//
			// Ignore dead characters
			//

			if ( pTestCharacter->IsDead() )
			{
				continue;
			}

			//
			// Ignore characters which do not collide with the new position
			//

			LTVector vCharacterPos, vCharacterDims;
			g_pLTServer->GetObjectPos( pTestCharacter->GetHOBJECT(), &vCharacterPos );
			g_pPhysicsLT->GetObjectDims( pTestCharacter->GetHOBJECT(), &vCharacterDims );
			float flCharacterRadiusSqr = pTestCharacter->GetRadius();
			flCharacterRadiusSqr *= flCharacterRadiusSqr;

			if ( !AIMovementUtils::Collides( 
				vCharacterPos, vCharacterDims, flCharacterRadiusSqr,
				vTesterPos, vTesterDims, flTesterRadiusSqr,
				AIMovementUtils::GetRadiusBuffer( pTestCharacter->GetHOBJECT() ) ) )
			{
				continue;
			}

			//
			// Ignore collisions if they reduce the collision (for instance,
			// an AI inside of another is moving away from it, which may 
			// occur if an AI was recently allowed to be solid.
			//

			LTVector vDistance2D = vCharacterPos - vTesterPos;
			vDistance2D.y = 0.0f;
			float flDot = vDistance2D.Dot( vMovement2D );
			if ( flDot <= 0 )
			{
				continue;
			}

			//
			// Ignore collisions if the the object is blocking the destination.  If an 
			// AIs destination is blocked, sliding cannot fix this.
			//

			if ( AIMovementUtils::BlocksPosition2D( vCharacterPos, flCharacterRadiusSqr, flTesterRadiusSqr, m_vDest ) )
			{
				continue;
			}

			//
			// Slide against the object, correcting the movement vector.
			//

			// Adjust the movement to avoid collision and rescale it to 
			// maintain the magnitude of the movement.  Add a small amount 
			// to the dest to handle floating point inaccuracy when later
			// determining if a collision exists.

			LTVector vClipNorm = -vDistance2D.GetUnit();
			float flDist = vMovement2D.Dot( vClipNorm );
			vMovement2D += vClipNorm * (-flDist * 1.001f);

			// Collision resulted in no movement possible.  

			if ( vMovement2D == LTVector::GetIdentity() )
			{
				return;
			}

			// Collision results in no movement in the desired direction

			if ( vMovement2D.Dot(vGoalMovement2D) < 0 )
			{
				return;
			}

			// Rescale the movement in X/Z to the original magnitude.

			vMovement2D.SetMagnitude( flMovementDistance );
		}
	}

	LTVector vNewPos = m_pAI->GetPosition() + LTVector( vMovement2D.x, flMovementInY, vMovement2D.z );

	// Return if sliding did not modify the position.

	if ( vNewPos == *pvNewPos )
	{
		return;
	}

	// 
	// Verify the new position is valid.
	//

	// Ensure AI stays in NavMesh.  If the adjusted position doesn't keep him in 
	// the mesh, either block the movement or apply the original movement.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vNewPos, m_pAI->GetLastNavMeshPoly(), m_pAI->GetRadius() ) )
	{
		return;
	}

	// Fail to slide if it moves us into a link, and that link:
	// 1) Does not allow dynamic movement.
	// 2) Is not active to the AI.

	ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vNewPos, m_pAI->GetCharTypeMask(), m_pAI->GetLastNavMeshPoly(), m_pAI );
	if( ePoly == kNMPoly_Invalid )
	{
		return;
	}

	if( ePoly != m_pAI->GetLastNavMeshPoly() )
	{
		CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
		if( pPoly && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
		{
			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
			if( pLink )
			{
				if ( !pLink->IsNMLinkEnabledToAI( m_pAI, !LINK_CHECK_TIMEOUT ) 
					|| !pLink->AllowDynamicMovement( m_pAI ) )
				{
					m_pAI->GetAIBlackBoard()->SetBBDestStatus( kNav_Failed );
					return;
				}
			}
		}
	}

	//
	// Sliding was successful.  Apply the modifications.
	//

	// Record the last slide time.  This allows different movement 
	// constraints which may interfere with sliding to fail for some 
	// period of time.

	m_flLastSlide = g_pLTServer->GetTime();

	*pvNewPos = vNewPos;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateRotation
//
//	PURPOSE:	Update the objects rotation.
//
// ----------------------------------------------------------------------- //

void CAIMovement::UpdateRotation()
{
	// Determine if we have a new rotation destination to rotation to.
	// If so, apply it.  At some point, we will probably want to be more 
	// descerning about when we apply rotations.

	switch (m_pAI->GetAIBlackBoard()->GetBBFaceType())
	{
	case kFaceType_Object:
		FaceObject(m_pAI->GetAIBlackBoard()->GetBBFaceObject());
		break;
	
	case kFaceType_Dir:
		FaceDir(m_pAI->GetAIBlackBoard()->GetBBFaceDir());
		break;

	case kFaceType_Pos:
		FacePos(m_pAI->GetAIBlackBoard()->GetBBFacePos());
		break;
	
	case kFaceType_None:
	default:
		break;
	}

	// Clear any previously set facing.

	m_pAI->GetAIBlackBoard()->SetBBFaceType( kFaceType_None );

	// Ignore this rotation if the animation itself handles rotation.  Do 
	// this after the facetype has been processed to insure any side effects
	// occur.

	if ( kAD_COND_Rotating == m_pAI->GetAnimationContext()->GetDescriptor( kADG_Condition ) )
	{
		// Insure clear FaceTargetRotImmediately is cleared, just in case it 
		// was set, to prevent the request from leaking.

		m_pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(false);
		m_fRotationTime = 0.0f;
		return;
	}

	// If we should immediately face our direction, just do it.

	if (m_pAI->GetAIBlackBoard()->GetBBFaceTargetRotImmediately())
	{
		FaceTargetRotImmediately();
		m_pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(false);
	}

	if ( m_fRotationTimer < m_fRotationTime )
	{
		m_fRotationTimer += g_pLTServer->GetTime() - m_fLastMovementUpdate;
        m_fRotationTimer = LTMIN(m_fRotationTime, m_fRotationTimer);

		// Interpolate the rotation based on the time

        float fRotationInterpolation = (float)(m_fRotationTimer/m_fRotationTime);
 
		// Rotate us if our timer is going

        LTRotation rNewRot;
		rNewRot.Slerp(m_rStartRot, m_rTargetRot, fRotationInterpolation);

		// Set our rotation

		g_pLTServer->SetObjectRotation(m_pAI->m_hObject, rNewRot);

		// Update our rotation vectors

		m_vObjectRight = rNewRot.Right();
		m_vObjectUp = rNewRot.Up();
		m_vObjectForward = rNewRot.Forward();

		// We're done.

		if( m_vTargetForward == m_vObjectForward )
		{
			m_fRotationTimer = m_fRotationTime;
		}

		if( m_fRotationTimer == m_fRotationTime )
		{
			m_vTargetForward = m_vObjectForward;
		}
	}
	else
	{
		// Retrieve AI vectors for current frame..

		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_pAI->m_hObject, &rRot);
		m_vObjectRight = rRot.Right();
		m_vObjectUp = rRot.Up();
		m_vObjectForward = rRot.Forward();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::FaceObject()
//
//	PURPOSE:	Turn to face a specific object
//
// ----------------------------------------------------------------------- //

bool CAIMovement::FaceObject(HOBJECT hObj)
{
    LTVector vTargetPos;
	g_pLTServer->GetObjectPos(hObj, &vTargetPos);

	return FacePos(vTargetPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::FaceDir()
//
//	PURPOSE:	Turn to face a specific direciton
//
// ----------------------------------------------------------------------- //

bool CAIMovement::FaceDir(const LTVector& vDir)
{
	return FacePos(m_pAI->GetPosition() + ( vDir * 10.f ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::FacePos()
//
//	PURPOSE:	Turn to face a specific pos
//
// ----------------------------------------------------------------------- //

bool CAIMovement::FacePos(const LTVector& vTargetPos)
{
    LTVector vDir = (vTargetPos - m_pAI->GetPosition());
   	vDir.y = 0.0f; // Don't look up/down
   
 	if ( vDir.MagSqr() < 1.f )
   	{
 		// Ignore requests to to face a pos very close to where we already are.
 		// They are likely to make the AI pop the wrong direction.
   
        return true;
   	}


	// Rotate fast when standing still.
	// The static speed should be much faster than the moving speed.

	float fRotationSpeed = m_pAI->GetAIAttributes()->fRotationTimeStatic;
	EnumAnimDesc eMovementType = m_pAI->GetAnimationContext()->GetAnimMovementType();
	if( eMovementType == kAD_None )
	{
		fRotationSpeed = m_pAI->GetAIAttributes()->fRotationTimeStatic;
	}

	// Limited rotation is enabled.

	else if( m_bLimitRotation && m_pAI->GetAIAttributes()->fRotationTimeLimited > 0.f )
	{
		fRotationSpeed = m_pAI->GetAIAttributes()->fRotationTimeLimited;
	}

	// Base rotation speed on awareness while moving.

	else {
		switch(m_pAI->GetAIBlackBoard()->GetBBAwareness())
		{
		case kAware_Relaxed:
			fRotationSpeed = m_pAI->GetAIAttributes()->fRotationTimeMoving;
			break;
		case kAware_Suspicious:
			fRotationSpeed = m_pAI->GetAIAttributes()->fRotationTimeMoving;
			break;
		case kAware_Alert:
			fRotationSpeed = m_pAI->GetAIAttributes()->fRotationTimeStatic;
			break;
		}
	}


  	vDir.Normalize();
   
    float fDpForward = vDir.Dot(m_vObjectForward);

	// Ignore requests to rotate very close to our current rotation.

	if( fDpForward >= c_fFacingThreshhold )
   	{
		return true;
	}
   
	if( fDpForward <= -1.f )
  	{
		fDpForward = -1.f;
   
		// If we're goign to turn 180 degrees, take into account
		// the position of our target, and nudge the desired direction
		// toward our target. This prevents us prom doing piroettes by
		// choosing the wring direction for the 180 degree turn.

		if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
		{
			LTVector vToTarget = m_pAI->GetAIBlackBoard()->GetBBTargetPosition() - m_pAI->GetPosition();
			vToTarget.Normalize();
			vDir += vToTarget * 0.1f;
		}
	}

  	g_pLTServer->GetObjectRotation(m_pAI->m_hObject, &m_rStartRot);
	m_rTargetRot = LTRotation(vDir, LTVector(0.f, 1.f, 0.f));
  	m_vTargetForward = m_rTargetRot.Forward();

	float fDegrees = MATH_RADIANS_TO_DEGREES( (float)acos( fDpForward ) );
	m_fRotationTime = fDegrees * ( fRotationSpeed / 360.f );
	m_fRotationTimer = 0.f;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::FaceTargetRotImmediately()
//
//	PURPOSE:	Turn to the TargetRot NOW!!
//
// ----------------------------------------------------------------------- //

void CAIMovement::FaceTargetRotImmediately()
{
	if( m_fRotationTimer < m_fRotationTime )
	{
		// Set our rotation
		g_pLTServer->SetObjectRotation(m_pAI->m_hObject, m_rTargetRot);

		m_vObjectRight = m_rTargetRot.Right();
		m_vObjectUp = m_rTargetRot.Up();
		m_vObjectForward = m_rTargetRot.Forward();

		m_fRotationTime = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::LimitRotation()
//
//	PURPOSE:	Return true of the position was adjusted to fall 
//				within the rotational limits.
//
// ----------------------------------------------------------------------- //

void CAIMovement::LimitRotation( LTVector* pvNewPos, State eState )
{
	// Sanity check.

	if( !pvNewPos )
	{
		return;
	}

	// Constrain the times an AI may apply limited rotation.
	// Bail if rotation limitation has been shut off.
	// It gets turned back on each time a dest is set.

	if( ( !m_bLimitRotation )
		|| ( eState != eStateSet )
		|| ( g_pLTServer->GetTime() - m_flLastSlide < 1.0 ) )
	{
		return;
	}

	// Bail if limited rotation is disabled for this AI.

	if( m_pAI->GetAIAttributes()->fRotationTimeLimited <= 0.f )
	{
		m_bLimitRotation = false;
		return;
	}

	// Calculate the horizontal direction to the dest.

	LTVector vCurPos = m_pAI->GetPosition();
	LTVector vDirToDest = m_vDest - vCurPos;
	vDirToDest.y = 0.f;
	if( vDirToDest != LTVector::GetIdentity() )
	{
		vDirToDest.Normalize();
	}

	// Bail if AI is already heading the correct direction.

	float fDot = vDirToDest.Dot( m_pAI->GetForwardVector() );
	if( ( fDot >= 1.f ) || ( fDot <= -1.f ) )
	{
		m_bLimitRotation = false;
		return;
	}

	// Calculate the angle between our current direction, and 
	// the direction to the dest.

	float fAngle = LTArcCos( fDot );

	// Calculate the elapsed time.

	float fDeltaTime = (float)(g_pLTServer->GetTime() - m_fLastMovementUpdate);

	// Bail if AI's desired rotation is already within the limits of his rotation speed.

	float fMaxAngle = fDeltaTime * ( MATH_CIRCLE / m_pAI->GetAIAttributes()->fRotationTimeLimited );
	if( fMaxAngle >= fAngle )
	{
		m_bLimitRotation = false;
		return;
	}

	// Bail if destination is unreachable due to the turning radius.
	// If we do not bail here, the AI will circle the destination infinitely, 
	// never able to reach the destination.
	// Calculate the diameter of the turning radius based on the circumference
	// of the circle:  C = PI * D
	// Solving for the diamater D = C / PI.
	// Where the circumference C = ( 360 degrees / MaxAngle ) * DistToMove

	float fDistToMove = pvNewPos->Dist( vCurPos );
	float fTurningDiameter = ( fDistToMove * ( MATH_CIRCLE / fMaxAngle ) ) / MATH_PI;
	LTVector vDest = m_vDest;
	vDest.y = vCurPos.y;
	if( vDest.Dist( vCurPos ) <= fTurningDiameter )
	{
		m_bLimitRotation = false;
		return;
	}

	// Interpolate based on the ratio of the partial angle to the full angle to the dest.

	LTRotation rNewRot;
	float fInterp = fMaxAngle / fAngle;
	static LTVector vUp( 0.f, 1.f, 0.f );
	LTRotation rCur( m_pAI->GetForwardVector(), vUp );
	LTRotation rDest( vDirToDest, vUp );
	rNewRot.Slerp( rCur, rDest, fInterp );
	LTVector vNewForward = rNewRot.Forward();

	// Move the same distance, but along the circumfrence of the turning radius.

	LTVector vPos = vCurPos + ( vNewForward * fDistToMove );

	// Ensure AI does not rotate out of the NavMesh.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( m_pAI, m_pAI->GetCharTypeMask(), m_pAI->GetPosition(), vPos, m_pAI->GetLastNavMeshPoly(), m_pAI->GetRadius() ) )
	{
		m_bLimitRotation = false;
		return;
	}

	// Ensure AI does move to a position that makes his dest unreachable.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( m_pAI, m_pAI->GetCharTypeMask(), vPos, m_vDest, m_pAI->GetLastNavMeshPoly(), 0.f ) )
	{
		m_bLimitRotation = false;
		return;
	}

	// Position was adjusted.

	float fHeight = pvNewPos->y;
	*pvNewPos = vPos;
	pvNewPos->y = fHeight;
	m_bLimitRotation = true;
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::PreventTargetPenetration()
//
//	PURPOSE:	Return true of the position does not penetrate the target.
//
// ----------------------------------------------------------------------- //

bool CAIMovement::PreventTargetPenetration( const LTVector& vNewPos )
{
	// No target to penetrate.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !hTarget )
	{
		return true;
	}
	LTVector vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();

	// Create a bounding box based on the target's dims.

	SAABB aabb;
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( hTarget, &vDims );
	aabb.vMin = vTargetPos - vDims;
	aabb.vMax = vTargetPos + vDims;

	// Ignore any height variation when calculating the direction vector.

	LTVector vNewDest = vNewPos;
	vNewDest.y = vTargetPos.y;

	LTVector vDir = vTargetPos - vNewDest;
	vDir.Normalize();

	// Test both the new position, and the radius offset.
	// This is just to have a better chance of catching fast-moving objects.

	LTVector vRadiusOffset = vNewDest + ( vDir * m_pAI->GetRadius() * AIMovementUtils::GetRadiusBuffer( hTarget ) );
	if( aabb.IntersectPoint( vNewDest ) || aabb.IntersectPoint( vRadiusOffset ) )
	{
		return false;
	}

	// New position does not penetrate the target.

	return true;
}


