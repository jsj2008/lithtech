// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIMovement.h"
#include "AIHuman.h"
#include "AIVolumeMgr.h"
#include "AnimationMovement.h"
#include "AIVolume.h"
#include "AnimationMgr.h"
#include "AIUtils.h"
#include "AIBrain.h"
#include "CharacterMgr.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::CAIMovement / ~CAIMovement
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAIMovement::CAIMovement()
{
	m_pAI = LTNULL;
	m_eState = eStateUnset;
	m_vDest = LTVector(0,0,0);
	m_pDestVolume = LTNULL;
	m_bUnderwater = LTFALSE;
	m_bClimbing = LTFALSE;
	m_bFaceDest = LTTRUE;
	m_bIgnoreVolumes = LTFALSE;
	m_eLastMovementType = kAM_None;
	m_fAnimRate = 1.f;
	m_bMovementLocked = LTFALSE;
	m_bRotationLocked = LTFALSE;
	m_bNoDynamicPathfinding = LTFALSE;
	m_vLastValidVolumePos.Init( 0.0f, 0.0f, 0.0f );
	m_bMoved = LTFALSE;

	m_bNewPathSet = LTFALSE;
	m_cBoundPts = 0;
	m_iBoundPt = 0;

	m_bDoParabola = LTFALSE;
	m_fParabolaPeakDist = 0.f;
	m_fParabolaPeakHeight = 0.f;
	m_fParabola_a = 0.f;
	m_bParabolaPeaked = LTFALSE;
}

CAIMovement::~CAIMovement()
{
	m_stackAnimations.Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Init
//
//	PURPOSE:	Initializes the Movement
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovement::Init(CAI *pAI)
{
	m_pAI = pAI;
	
	if( m_pAI->GetBrain() &&
		m_pAI->GetBrain()->GetAIDataExist( kAIData_NoDynamicPathfinding ) &&
		( m_pAI->GetBrain()->GetAIData( kAIData_NoDynamicPathfinding ) > 0.f ) )
	{
		m_bNoDynamicPathfinding = LTTRUE;
	}
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::IsAtDest
//
//	PURPOSE:	Checks if AI is already at the dest.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovement::IsAtDest(const LTVector& vDest)
{
	LTVector vMove = vDest - m_pAI->GetPosition();

	if ( !m_pAI->GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbUp) && !m_pAI->GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbDown) &&
		 !m_pAI->GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbUp) && !m_pAI->GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_ClimbDown) )
	{
		vMove.y = 0.0f;
	}

	// See if we're already at the dest exactly.

	LTFLOAT fRemainingDist = vMove.Mag();
	if(fRemainingDist == 0.f)
	{
		Clear();
		m_eState = eStateDone;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Update
//
//	PURPOSE:	If 
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovement::Update()
{
	// Clear any past movement.
	m_pAI->Stop();
	m_pAI->SetCheapMovement(LTTRUE);

	LTVector vNewPos;
	LTBOOL bMove = LTTRUE;

	// Bound a new path to the volume.

	if( ( m_eState == eStateSet ) && m_bNewPathSet )
	{
		BoundPathToVolume( m_pDestVolume );
		m_bNewPathSet = LTFALSE;
		m_iBoundPt = 0;
	}

	EnumAnimMovement eMovementType = m_pAI->GetAnimationContext()->GetAnimMovementType();

	switch( eMovementType )
	{
		case kAM_None: 

			// Safety mechanism to pop AI up if they fall thru the level.

			if( m_bMoved && 
				( m_pAI->GetLastVolume() ) && 
				( m_pAI->GetPosition().y + Max( 16.f, m_pAI->GetDims().y ) < m_pAI->GetLastVolume()->GetBackBottomLeft().y ) )
			{
				LTVector vPos = m_pAI->GetPosition();
				vPos = ConvertToDEditPos( vPos );
				AIError( "AI '%s' fell thru the level at pos(%.2f %.2f %.2f )!! Popping back up.", 
					m_pAI->GetName(), vPos.x, vPos.y, vPos.z );
				LTVector vDir = m_pAI->GetPosition() - m_vLastValidVolumePos;
				vDir.y = 0.f;
				if( vDir.MagSqr() == 0.f )
				{
					vDir = LTVector( 1.f, 0.f, 0.f );
				}
				vDir.Normalize();

				vNewPos = m_pAI->GetPosition() + vDir;
				vNewPos.y = m_pAI->GetLastVolume()->GetBackBottomLeft().y + m_pAI->GetDims().y;
				m_pAI->Move( vNewPos );

				m_bMoved = LTFALSE;
				return LTTRUE;
			}

			// Ensure AI stays in volumes.

			bMove = LTFALSE;
			m_bMoved = LTFALSE;
			if( m_pAI->GetCurrentVolume() )
			{
				m_vLastValidVolumePos = m_pAI->GetPosition();
			}
			else if( ( !m_bIgnoreVolumes ) && m_pAI->GetLastVolume() )
			{
				vNewPos = m_vLastValidVolumePos;
				bMove = LTTRUE;
			}
			break;

		case kAM_Set:
		case kAM_Walk:
		case kAM_Run:
		case kAM_JumpUp:
		case kAM_JumpOver:
		case kAM_Fall:
		case kAM_Climb:
		case kAM_Swim:

		case kAM_Hover:
			if( (m_eState != eStateSet) || 
				(!UpdateConstantVelocity( eMovementType, &vNewPos ) ) )
			{
				bMove = LTFALSE;
			}
			break;

		case kAM_Encode_NG:
		case kAM_Encode_G:
		case kAM_Encode_GB:
		case kAM_Encode_V:
			if( LTFALSE == UpdateMovementEncoding( eMovementType, &vNewPos ) )
			{
				bMove = LTFALSE;
			}
			break;

		default:
			AIASSERT( 0, m_pAI->GetHOBJECT(), "Unknown Movement type!" );
			break;
	}

	m_pAI->ClearLastHintTransform();

	m_eLastMovementType = eMovementType;

	if( bMove && ( m_pAI->GetPosition() != vNewPos ) )
	{
		// Make sure new position is inside an AI volume.
		// This NEEDS to check eAxisAll with the vertical threshold.

		AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, vNewPos, eAxisAll, m_pAI->GetVerticalThreshold(), m_pAI->GetLastVolume() );		
		if( ( pVolume && pVolume->IsVolumeEnabled() ) || m_bIgnoreVolumes )
		{
			if( m_eState != eStateSet )
			{
				// If an AI is playing a movement encoded animation with
				// gravity, and does not have a destination, do not allow
				// him to move over a large verticle drop.
				// (For example, do not let ninjas fall off roofs while
				// drawing a sword and taking a step).
	
				if( ( ( eMovementType == kAM_Encode_G ) || ( eMovementType == kAM_Encode_GB ) ) && 
					pVolume &&
					( m_vLastValidVolumePos.y > pVolume->GetBackTopLeft().y + m_pAI->GetVerticalThreshold() ) )
				{
					return LTFALSE;
				}

				// If an AI does not have a destination do not allow
				// him to move into a JumpOver volume.

				if( pVolume && ( pVolume->GetVolumeType() == AIVolume::kVolumeType_JumpOver ) )
				{
					return LTFALSE;
				}
			}

			// Adjust the height for a parabola.

			if( m_bDoParabola )
			{
				m_pAI->SetCheapMovement( LTFALSE );
				vNewPos.y = m_vParabolaOrigin.y + UpdateParabola();
			}

			// Do not allow any elevation changes in door volumes.
			// Keeps AI from popping onto doors.

			if( pVolume && pVolume->HasDoors() )
			{
				m_pAI->SetCheapMovement(LTFALSE);
			}

			// Avoid characters if AI has a destination, and movement 
			// is not locked, and AI is in volumes.

			if( ( m_eState == eStateSet ) && 
				( m_pDestVolume ) &&
				( !m_bNoDynamicPathfinding ) &&
				( !m_bIgnoreVolumes ) &&
				( !m_bMovementLocked ) &&
				( !m_bRotationLocked ) &&
				( eMovementType != kAM_Encode_V ) )
			{
				AvoidDynamicObstacles( &vNewPos, eMovementType );
			}

			// Move us - tells the AI where to move to

			m_pAI->Move(vNewPos);
			m_bMoved = LTTRUE;

			// Record last valid volume position.

			if( pVolume )
			{
				m_vLastValidVolumePos = vNewPos;
			}

			// If we reached an intermediate bound point, move on
			// to the next one.

			if( ( m_eState == eStateDone ) && ( m_iBoundPt + 1 < m_cBoundPts ) )
			{
				++m_iBoundPt;
				m_vDest = m_vBoundPts[m_iBoundPt];
				m_eState = eStateSet;
			}

			return LTTRUE;
		}

		// Destination is unreachable by volumes.
		// Put us somewhere valid.

		else if( m_eState == eStateSet )
		{
			m_pAI->Move(m_vLastValidVolumePos);
			m_eState = eStateDone;
			m_bMoved = LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::AvoidDynamicObstacles
//
//	PURPOSE:	Repel AI off other characters.
//
// ----------------------------------------------------------------------- //

void CAIMovement::AvoidDynamicObstacles(LTVector* pvNewPos, EnumAnimMovement eMovementType)
{
	LTFLOAT fRadius = 128.f;
	LTFLOAT fRadiusSqr = fRadius * fRadius;

	LTVector vMyPos = m_pAI->GetPosition();

	// Calculate the horizontal velocity.

	LTVector vVel = *pvNewPos - vMyPos;
	vVel.y = 0.f;

	// Bail if no velocity.

	if( ( vVel.x == 0.f ) && ( vVel.z == 0.f ) )
	{
		return;
	}

	LTFLOAT fMag = vVel.Mag();

	LTVector vTotalForce(0.f, 0.f, 0.f);
	LTVector vObstaclePos;
	LTFLOAT fDistSqr;
	LTFLOAT fForce;
	LTVector vForce;

	CTList<CCharacter*>* lstChars	= LTNULL;
	CCharacter** pCur				= LTNULL;

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

			fForce = fRadius - (LTFLOAT)sqrt( fDistSqr );
			fForce /= fRadius;
			fForce *= fForce;
			fForce *= ( 2.f * fMag );

			vForce = vMyPos - vObstaclePos;
			vForce.y = 0.f;
			vForce.Normalize();
			vForce *= fForce;

			// Accumulate the total force from all obstacles.

			vTotalForce += vForce;			
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
	vNewVel *= fMag;

	// Calculate new position.
	// Bail if new position is out of volumes.
	// Bail if new position is in wrong volume.

	LTVector vNewPos = vMyPos + vNewVel;
	if( !m_pDestVolume->Inside2d( vNewPos, m_pAI->GetRadius() ) )
	{
		return;
	}


	// Move toward new position.

	*pvNewPos = vNewPos;

	if( eMovementType == kAM_Encode_GB )
	{
		m_pAI->FacePosMoving( m_pAI->GetPosition() );
	}
	else {
		m_pAI->FacePosMoving( vNewPos );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::BoundPathToVolume
//
//	PURPOSE:	Ensure that AI's body does not extend outside of volume,
//              where there is no neighbor volume.
//              (e.g. clipping into walls, desks, trashcans, etc).
//
// ----------------------------------------------------------------------- //

void CAIMovement::BoundPathToVolume(AIVolume* pDestVolume)
{
	m_cBoundPts = 0;

	// Do not bound if no dest volume is specified.

	if( !pDestVolume )
	{
		return;
	}

	// Do not bound if doing special movement.

	if( m_bNoDynamicPathfinding || m_bIgnoreVolumes || m_bMovementLocked )
	{
		return;
	}

	// Only bound sugnificantly long paths.

	if( m_pAI->GetPosition().DistSqr( m_vDest ) < 255.f * 255.f )
	{
		return;
	}

	// Do not bound path in special volumes.

	if( ( pDestVolume->GetVolumeType() != AIVolume::kVolumeType_BaseVolume ) &&
		( pDestVolume->GetVolumeType() != AIVolume::kVolumeType_Junction ) )
	{
		return;
	}

	// Do not bound if AI is going thru a door.

	if( pDestVolume->HasDoors() || 
		( m_pAI->GetLastVolume() && m_pAI->GetLastVolume()->HasDoors() ) )
	{
		return;
	}

	// Do not bound if origin and dest are not inside of dest volume.

	if( !( pDestVolume->Inside2d( m_pAI->GetPosition(), 0.f ) && 
		   pDestVolume->Inside2d( m_vDest, 0.f ) ) )
	{
		return;
	}

	LTFLOAT fAIRadius = m_pAI->GetRadius();
	LTVector vAIPos = m_pAI->GetPosition();

	// Check if origin or dest leaks over volume boundaries, when AIs radius is considered.

	LTBOOL bOriginInside = pDestVolume->Inside2d( vAIPos, fAIRadius );
	LTBOOL bDestInside = pDestVolume->Inside2d( m_vDest, fAIRadius );

	// If no leaks, no bounding is necessary, so bail.

	if( bOriginInside && bDestInside )
	{
		return;
	}

	// If the entire path is inside of volumes, when AI's radius is considered, no
	// bounding is necessary.

	if( g_pAIVolumeMgr->StraightRadiusPathExists( m_pAI, vAIPos, m_vDest, fAIRadius, FLT_MAX, 0, m_pAI->GetLastVolume() ) )
	{
		return;
	}

	LTVector vBoundPt;
	LTVector vVolumeCenter = pDestVolume->GetCenter();
	LTVector vVolumeDims = pDestVolume->GetDims();

	// Add a bound point to get away from the orgin.

	if( !bOriginInside )
	{
		vBoundPt = vAIPos;
		if( vBoundPt.z + fAIRadius > vVolumeCenter.z + vVolumeDims.z )
		{
			vBoundPt.z = ( vVolumeCenter.z + vVolumeDims.z ) - fAIRadius;
		}
		else if( vBoundPt.z - fAIRadius < vVolumeCenter.z - vVolumeDims.z )
		{
			vBoundPt.z = ( vVolumeCenter.z - vVolumeDims.z ) + fAIRadius;
		}

		if( vBoundPt.x + fAIRadius > vVolumeCenter.x + vVolumeDims.x )
		{
			vBoundPt.x = ( vVolumeCenter.x + vVolumeDims.x ) - fAIRadius;
		}
		else if( vBoundPt.x - fAIRadius < vVolumeCenter.x - vVolumeDims.x )
		{
			vBoundPt.x = ( vVolumeCenter.x - vVolumeDims.x ) + fAIRadius;
		}

		m_vBoundPts[m_cBoundPts] = vBoundPt; 
		++m_cBoundPts;
	}
	
	// Add a bound point to get away from the dest.

	if( !bDestInside )
	{
		vBoundPt = m_vDest;
		if( vBoundPt.z + fAIRadius > vVolumeCenter.z + vVolumeDims.z )
		{
			vBoundPt.z = ( vVolumeCenter.z + vVolumeDims.z ) - fAIRadius;
		}
		else if( vBoundPt.z - fAIRadius < vVolumeCenter.z - vVolumeDims.z )
		{
			vBoundPt.z = ( vVolumeCenter.z - vVolumeDims.z ) + fAIRadius;
		}

		if( vBoundPt.x + fAIRadius > vVolumeCenter.x + vVolumeDims.x )
		{
			vBoundPt.x = ( vVolumeCenter.x + vVolumeDims.x ) - fAIRadius;
		}
		else if( vBoundPt.x - fAIRadius < vVolumeCenter.x - vVolumeDims.x )
		{
			vBoundPt.x = ( vVolumeCenter.x - vVolumeDims.x ) + fAIRadius;
		}

		m_vBoundPts[m_cBoundPts] = vBoundPt; 
		++m_cBoundPts;
	}
	
	// The last bound point is the true dest.

	m_vBoundPts[m_cBoundPts] = m_vDest;
	++m_cBoundPts;

	// The current dest is the first bound point.

	m_vDest = m_vBoundPts[0];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateConstantVelocity
//
//	PURPOSE:	Updates movement for constant velocity animations.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIMovement::UpdateConstantVelocity( EnumAnimMovement eMovementType, LTVector* pvNewPos )
{
	// Find our unit movement vector

	LTVector vMove = m_vDest - m_pAI->GetPosition();

	// Set our speed based on our movement type

	switch( eMovementType )
	{
		case kAM_Set:
			vMove.y = 0.0f;
			m_pAI->SetSpeed( m_fSetSpeed );
			break;

		case kAM_Walk:
			vMove.y = 0.0f;
			m_pAI->Walk();
			break;

		case kAM_Run:
			vMove.y = 0.0f;
			m_pAI->Run();
			break;

		case kAM_Hover:
			m_bFaceDest = LTTRUE;
			vMove.y = 0.0f;
			m_pAI->Hover();
			break;

		case kAM_Swim:
			vMove.y = 0.0f;
			m_pAI->Swim();
			break;

		case kAM_Climb:
			vMove.x = 0.0f;
			vMove.z = 0.0f;
			m_pAI->Walk();

			// Turn off gravity
			m_pAI->SetCheapMovement(LTFALSE);
			m_bFaceDest = LTFALSE;
			break;

		case kAM_JumpOver:
			vMove.y = 0.0f;
			m_pAI->JumpOver();

			// Turn off gravity
			m_pAI->SetCheapMovement(LTFALSE);

			if( m_eLastMovementType != kAM_JumpOver )
			{
				SetupJump( eMovementType );
			}
			break;

		case kAM_JumpUp:
			m_pAI->Jump();
			m_bFaceDest = LTFALSE;

			// Turn off gravity
			m_pAI->SetCheapMovement(LTFALSE);

			if( m_eLastMovementType != kAM_JumpUp )
			{
				SetupJump( eMovementType );
				vMove = m_vDest - m_pAI->GetPosition();
			}
			break;

		case kAM_Fall:
			m_pAI->Fall();
			m_bFaceDest = LTFALSE;

			// Turn off gravity
			m_pAI->SetCheapMovement(LTFALSE);

			if( m_eLastMovementType != kAM_Fall )
			{
				SetupJump( eMovementType );
				vMove = m_vDest - m_pAI->GetPosition();
			}
			break;

		default:
			AIASSERT( 0, m_pAI->GetHOBJECT(), "Unknown Movement type!" );
			break;
	}

	// See if we'll overshoot our dest.

	LTFLOAT fRemainingDist = vMove.Mag();
	if(fRemainingDist == 0.f)
	{
		Clear();
		m_eState = eStateDone;
		return LTFALSE;
	}

	LTFLOAT fMoveDist;
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	fMoveDist = m_pAI->GetSpeed()*fTimeDelta;

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

		Clear();
		m_eState = eStateDone;
		return LTTRUE;
	}

	// Scale based on our movement distance

	vMove.Normalize();
	vMove *= fMoveDist;

	// Calculate our new position

	*pvNewPos = m_pAI->GetPosition() + vMove;

	// Face us in the right direction

	if ( m_bFaceDest )
	{
		m_pAI->FacePosMoving( *pvNewPos );
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateParabola
//
//	PURPOSE:	Updates height in parabola based on distance covered.
//
// ----------------------------------------------------------------------- //

LTFLOAT CAIMovement::UpdateParabola()
{
	LTVector vOrigin2D = m_vParabolaOrigin;
	vOrigin2D.y = 0.f;

	LTVector vPos = m_pAI->GetPosition();
	vPos.y = 0.f;

	LTFLOAT fDist = vOrigin2D.Dist( vPos );

	// Reset the parabola variables after it has hit its peak
	// height, to account for a destination at a different
	// elevation than the origin.

	if(!m_bParabolaPeaked && (fDist > m_fParabolaPeakDist))
	{
		m_fParabolaPeakHeight += m_vParabolaOrigin.y - m_vDest.y;
		m_vParabolaOrigin.y = m_vDest.y;

		m_fParabola_a = m_fParabolaPeakHeight / ( m_fParabolaPeakDist * m_fParabolaPeakDist );
		m_bParabolaPeaked = LTTRUE;
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

LTBOOL CAIMovement::UpdateMovementEncoding( EnumAnimMovement eMovementType, LTVector* pvNewPos )
{
	// Encode_NG means encoding with NoGravity.
	if( ( eMovementType == kAM_Encode_NG ) || ( eMovementType == kAM_Encode_V ) )
	{
		// Turn off gravity
		m_pAI->SetCheapMovement(LTFALSE);

		m_bIgnoreVolumes = LTTRUE;
	}

	LTVector vHintPosDelta;
	LTRotation rHintRotDelta;
	g_pTransLT->Get( m_pAI->GetLastHintTransform(), vHintPosDelta, rHintRotDelta );

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_pAI->m_hObject, &rRot );

	rRot = rRot * rHintRotDelta;
	LTFLOAT fInvMag = 1.0f/(LTFLOAT)sqrt(rRot.m_Quat[0]*rRot.m_Quat[0] + rRot.m_Quat[1]*rRot.m_Quat[1] +
										 rRot.m_Quat[2]*rRot.m_Quat[2] + rRot.m_Quat[3]*rRot.m_Quat[3]);

	rRot.m_Quat[0] *= fInvMag;
	rRot.m_Quat[1] *= fInvMag;
	rRot.m_Quat[2] *= fInvMag;
	rRot.m_Quat[3] *= fInvMag;

	g_pLTServer->SetObjectRotation( m_pAI->m_hObject, &rRot );

	LTVector vOldPos;
	g_pLTServer->GetObjectPos( m_pAI->m_hObject, &vOldPos );

	// No destination is set, so just add the hint vector.

	if( m_eState != eStateSet )
	{
		*pvNewPos = vOldPos + vHintPosDelta;
	}

	// A destination has been set, so just use the magnitude of 
	// the xz compnents of the hint vector, to guarantee we're 
	// headed to our dest.

	else
	{
		if( vOldPos == m_vDest )
		{
			Clear();
			m_eState = eStateDone;
			return LTFALSE;
		}

		LTVector vOldDirToDest;
		LTVector vNewDirToDest;

		if( eMovementType == kAM_Encode_V )
		{
			// Get the direction from the old pos to the dest.

			vOldDirToDest = m_vDest - vOldPos;
			vOldDirToDest.Normalize();
		
			// Set the new pos to the sum of the old pos, and the 
			// magnitude of the xz components of the hint, towards 
			// the dest.  Then add the y component.

			*pvNewPos = vOldPos + ( vOldDirToDest * vHintPosDelta.Length() );

			// See if we'll overshoot our dest.

			vNewDirToDest = m_vDest - *pvNewPos;
			vNewDirToDest.Normalize();
		}
		else {
			// Get the direction from the old pos to the dest.

			vOldDirToDest = m_vDest - vOldPos;
			vOldDirToDest.y = 0.f;
			vOldDirToDest.Normalize();

			// Get the x and z components of the hint vector.

			LTVector vHintXZ = vHintPosDelta;
			vHintXZ.y = 0.f;
		
			// Set the new pos to the sum of the old pos, and the 
			// magnitude of the xz components of the hint, towards 
			// the dest.  Then add the y component.

			*pvNewPos = vOldPos + ( vOldDirToDest * vHintXZ.Length() );
			pvNewPos->y += vHintPosDelta.y;

			// See if we'll overshoot our dest.

			vNewDirToDest = m_vDest - *pvNewPos;
			vNewDirToDest.y = 0.f;
			vNewDirToDest.Normalize();
		}

		if( vOldDirToDest.Dot(vNewDirToDest) < 0.f )
		{
			Clear();
			m_eState = eStateDone;
			*pvNewPos = m_vDest;

			// If the movement is not vertical, then do not affect the 
			// elevation of the AI. Let CheapMovement take care of 
			// putting the AI on the ground.

			if( eMovementType != kAM_Encode_V )
			{
				pvNewPos->y = m_pAI->GetPosition().y;
			}
		}
		else if( m_bFaceDest ) {
			
			// Face backwards.

			if( eMovementType == kAM_Encode_GB )
			{
				LTVector vFacePos = vOldPos - vOldDirToDest;
				m_pAI->FacePosMoving( vFacePos );
			}

			// Face forwards.

			else {
				m_pAI->FacePosMoving( m_vDest );
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::UpdateAnimator
//
//	PURPOSE:	Updates our animator
//
// ----------------------------------------------------------------------- //

void CAIMovement::UpdateAnimation()
{
	// TODO: is this okay?
	//_ASSERT(m_eState == eStateSet);

	m_pAI->GetAnimationContext()->SetProp(kAPG_Movement, m_stackAnimations.Top());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void FnLoadAnimationProp(ILTMessage_Read *pMsg, EnumAnimProp& eProp)
{
	int nTemp;
	LOAD_INT(nTemp);
	eProp = (EnumAnimProp)nTemp;
}

void CAIMovement::Load(ILTMessage_Read *pMsg)
{	
	m_stackAnimations.Load(pMsg, FnLoadAnimationProp);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bUnderwater);
	LOAD_VECTOR(m_vDest);
	LOAD_COBJECT(m_pDestVolume, AIVolume);

	LOAD_VECTOR(m_vLastValidVolumePos);
	LOAD_BOOL(m_bClimbing);
	LOAD_BOOL(m_bFaceDest);
	LOAD_FLOAT(m_fSetSpeed);
	LOAD_BOOL(m_bIgnoreVolumes);
	LOAD_DWORD_CAST(m_eLastMovementType, EnumAnimMovement);
	LOAD_FLOAT(m_fAnimRate);
	LOAD_BOOL(m_bMovementLocked);
	LOAD_BOOL(m_bRotationLocked);
	LOAD_BOOL(m_bNoDynamicPathfinding);
	LOAD_BOOL(m_bMoved);

	LOAD_BOOL(m_bNewPathSet);
	LOAD_VECTOR(m_vBoundPts[0]);
	LOAD_VECTOR(m_vBoundPts[1]);
	LOAD_VECTOR(m_vBoundPts[2]);
	LOAD_DWORD(m_cBoundPts);
	LOAD_DWORD(m_iBoundPt);

	LOAD_BOOL(m_bDoParabola);
	LOAD_VECTOR(m_vParabolaOrigin);
	LOAD_FLOAT(m_fParabolaPeakDist);
	LOAD_FLOAT(m_fParabolaPeakHeight);
	LOAD_FLOAT(m_fParabola_a);
	LOAD_BOOL(m_bParabolaPeaked);

	// If we're loading from a transition, then our positional information is invalid.
	if( g_pGameServerShell->GetLGFlags( ) == LOAD_TRANSITION )
	{
		m_bMovementLocked = LTFALSE;
		m_bRotationLocked = LTFALSE;
		m_bMoved = LTFALSE;

		Clear( );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //

void FnSaveAnimationProp(ILTMessage_Write *pMsg, EnumAnimProp& eProp)
{
	SAVE_INT(eProp);
}

void CAIMovement::Save(ILTMessage_Write *pMsg)
{
	m_stackAnimations.Save(pMsg, FnSaveAnimationProp);

	SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bUnderwater);
	SAVE_VECTOR(m_vDest);
	SAVE_COBJECT(m_pDestVolume);

	SAVE_VECTOR(m_vLastValidVolumePos);
	SAVE_BOOL(m_bClimbing);
	SAVE_BOOL(m_bFaceDest);
	SAVE_FLOAT(m_fSetSpeed);
	SAVE_BOOL(m_bIgnoreVolumes);
	SAVE_DWORD(m_eLastMovementType);
	SAVE_FLOAT(m_fAnimRate);
	SAVE_BOOL(m_bMovementLocked);
	SAVE_BOOL(m_bRotationLocked);
	SAVE_BOOL(m_bNoDynamicPathfinding);
	SAVE_BOOL(m_bMoved);

	SAVE_BOOL(m_bNewPathSet);
	SAVE_VECTOR(m_vBoundPts[0]);
	SAVE_VECTOR(m_vBoundPts[1]);
	SAVE_VECTOR(m_vBoundPts[2]);
	SAVE_DWORD(m_cBoundPts);
	SAVE_DWORD(m_iBoundPt);

	SAVE_BOOL(m_bDoParabola);
	SAVE_VECTOR(m_vParabolaOrigin);
	SAVE_FLOAT(m_fParabolaPeakDist);
	SAVE_FLOAT(m_fParabolaPeakHeight);
	SAVE_FLOAT(m_fParabola_a);
	SAVE_BOOL(m_bParabolaPeaked);
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
	m_bFaceDest = LTTRUE;
	m_bIgnoreVolumes = LTFALSE;
	m_bDoParabola = LTFALSE;

	m_bNewPathSet = LTTRUE;

	m_pDestVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, m_vDest, eAxisAll, m_pAI->GetVerticalThreshold(), m_pAI->GetLastVolume() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::Clear
//
//	PURPOSE:	Clears the destination point
//
// ----------------------------------------------------------------------- //

void CAIMovement::Clear()
{ 
	m_eState = eStateUnset; 
	m_fSetSpeed = 0.f;
	m_bFaceDest = LTTRUE;
	m_bIgnoreVolumes = LTFALSE;
	m_pDestVolume = LTNULL;

	if( m_bDoParabola )
	{
		m_bDoParabola = LTFALSE;
		m_pAI->SetCheapMovement( LTTRUE );
	}

	if( m_fAnimRate != 1.f )
	{
		m_pAI->GetAnimationContext()->SetAnimRate( 1.f );
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

void CAIMovement::SetParabola( LTFLOAT fHeight )
{
	//
	// Parabola:  y = ax^2
	//   Flip it: y = -ax^2
	//   Get to specified height:  a = h / (d^2), where d = 1/2 * dist
	//   Get above origin: y = -ax^2 + h
	//   Get right of origin: y = -a(x - d)^2 + h
	//

	m_bDoParabola = LTTRUE;

	m_vParabolaOrigin = m_pAI->GetPosition();

	m_bParabolaPeaked = LTFALSE;

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
//	ROUTINE:	CAIMovement::SetupJump
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAIMovement::SetupJump( EnumAnimMovement eMovementType )
{
	LTVector vOrigin = m_pAI->GetPosition();

	// Calc distance for jumping up or down.
	// Figure out how long it takes to fly to the dest.
	
	LTFLOAT fDist;
	LTFLOAT fJumpTime;

	switch( eMovementType )
	{
		case kAM_JumpOver:
			{
				fDist = vOrigin.Dist( m_vDest );
				fJumpTime = fDist / m_pAI->GetJumpOverSpeed();
			}
			break;

		case kAM_JumpUp:
			{
				// Move dest to be directly above origin.

				m_vDest.x = vOrigin.x;
				m_vDest.z = vOrigin.z;

				fDist = m_vDest.y - vOrigin.y;
				fJumpTime = fDist / m_pAI->GetJumpSpeed();
			}
			break;

		case kAM_Fall:
			{
				// Move dest to be directly above origin.

				m_vDest.x = vOrigin.x;
				m_vDest.z = vOrigin.z;

				fDist = vOrigin.y - m_vDest.y;
				fJumpTime = fDist / m_pAI->GetFallSpeed();
			}
			break;

		default:
			{
				AIASSERT( 0, m_pAI->GetHOBJECT(), "Unknown Movement type!" );
			}
			break;
	}
	
	// Find the length of the fly animation.

	LTFLOAT fAnimLength = m_pAI->GetAnimationContext()->GetCurAnimationLength();

	// Calculate how fast to play the fly animation.

	m_fAnimRate = fAnimLength / fJumpTime;
	m_pAI->GetAnimationContext()->SetAnimRate( m_fAnimRate );

	m_bIgnoreVolumes = LTTRUE;
}
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMovement::LockMovement
//
//	PURPOSE:	Do not allow anyone to change our movement or rotation.
//
// ----------------------------------------------------------------------- //

void CAIMovement::LockMovement()
{
	m_bMovementLocked = LTTRUE; 
	m_bRotationLocked = LTTRUE;
}

void CAIMovement::LockMovement(LTBOOL bLockRotation) 
{
	m_bMovementLocked = LTTRUE; 
	m_bRotationLocked = bLockRotation; 
}

void CAIMovement::UnlockMovement() 
{
	m_bMovementLocked = LTFALSE; 
	m_bRotationLocked = LTFALSE; 
}
