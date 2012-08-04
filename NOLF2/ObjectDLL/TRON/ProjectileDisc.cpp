// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileDisc.cpp
//
// PURPOSE : Projectile Disc class - implementation
//
// CREATED : 12/12/2001
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileDisc.h"
#include "MsgIDs.h"
#include "WeaponFireInfo.h"
#include "TronPlayerObj.h"
#include "PlayerObj.h"
#include "AIStimulusMgr.h"
#include "CharacterMgr.h"
#include "ImpactType.h"
#include "FxDefs.h"
#include "FXButeMgr.h"

//
// LithTech Macros
//
BEGIN_CLASS(CDisc)
END_CLASS_DEFAULT_FLAGS(CDisc, CProjectile, NULL, NULL, CF_HIDDEN)

//
// Defines
//
#define CDISC_UPDATE_DELTA            0.001f


//
// Globals
//
extern CAIStimulusMgr* g_pAIStimulusMgr;

//
// Enumerations
//

enum DISC_STATE
{
	// this state is invalid while the projectile disc exists
	DISC_STATE_NONE,

	// the disc is flying toward a target
	DISC_STATE_FLYING,

	// the disc has been deflected, it isn't obeying properly
	DISC_STATE_DEFLECTING,

	// the disc is returning to the owner
	DISC_STATE_RETURNING,

	// always last
	DISC_STATE_COUNT
};

// Disc tracking modes refer to how it adjusts its flight path
// when it flys toward its target.  Once in the "return" state,
// it always operates the same.
enum DISC_TRACKING_MODE
{
	// invalid state
	DISC_TRACKING_MODE_NONE,

	// disc flys straight with no special tracking
	DISC_TRACKING_MODE_STEADY,

	// disc will change its flight path to hit a point
	DISC_TRACKING_MODE_POINT_GUIDED,

	// disc will use a control line emitted from the owner
	DISC_TRACKING_MODE_CONTROL_LINE,

	// disc will change its flight path to hit an object
	DISC_TRACKING_MODE_HOMING,

	// always last
	DISC_TRACKING_MODE_COUNT
};


// This is the method is uses while flying through the air,
// for example, the disc can travel straight at the target
// no matter what, or it can curve in interesting ways.
enum DISC_TRACKING_STYLE
{
	// invalid tracking style
	DISC_TRACKING_STYLE_NONE,

	// disc flys straight, nothing exerts control over it
	DISC_TRACKING_STYLE_STRAIGHT,

	// disc flys straight towards its target at maximum velocity
	DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET,

	// disc flys toward a control line provided by the owner
	DISC_TRACKING_STYLE_CONTROL_LINE,
};


namespace
{
	// MATH_EPSILON is too tight
	float const DISC_EPSILON = 0.000001f;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::RemoveObject()
//              
//	PURPOSE:	Temp demo hack!!  This is here because the discs RemoveObject
//				can be called from TestInsideObject, meaning that the thrower
//				never recieves the RETURNED message, and is then stuck
//				discless.  Remove this once a true solution has been
//				implemented
//              
//----------------------------------------------------------------------------
void CDisc::RemoveObject()
{
	if ( m_bSentReturnedMsg == LTFALSE )
	{
		// TEMP!!!  Force the message being sent if it hasn't been already.
		// This is a fix for the demo.
		HandleImpact(NULL);
	}

	CProjectile::RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::CDisc()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CDisc::CDisc() :
	  m_eDiscState( DISC_STATE_NONE )
	, m_eTrackingMode( DISC_TRACKING_MODE_NONE )
	, m_eTrackingStyle( DISC_TRACKING_STYLE_NONE )
	, m_bNewTarget( false )
	, m_vPointTarget()
	, m_hObjectTarget( LTNULL )
	, m_vControlDirection()
	, m_vControlPosition()
	, m_bReceivingMessages( false )
	, m_hReturnToSocket( INVALID_MODEL_SOCKET )
	, m_eDiscCatchableStimID( kStimID_Unset )
	, m_eDiscBlockableStimID( kStimID_Unset )
	, m_fLastUpdateTime( 0.0f )
	, m_fReturnVelocity( 0.0f )
	, m_fReturnHeightOffset( 0.0f )
	, m_fTurnRate( 0.0f )
	, m_fIncidentAngleToControlLine( 0.0f )
	, m_fIncidentAngleToControlLineDecay( 0.0f )
	, m_bSentReturnedMsg( LTFALSE )
//	, m_nDeflectServerTimeStart( 0 )
{
	// Construct
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::~CDisc()
//
//	PURPOSE:	Remove the stimulii from the stimulus mgr if the 
//				stimulusmgr still exists AND the stimulii are currently
//				set.
//
// ----------------------------------------------------------------------- //

CDisc::~CDisc()
{
	// Destruct
	
	UBER_ASSERT( m_bSentReturnedMsg == LTTRUE, "Failed to send return msg!!" );

	if ( g_pAIStimulusMgr )
	{
		if ( m_eDiscCatchableStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eDiscCatchableStimID );
			m_eDiscCatchableStimID = kStimID_Unset;
		}
		if ( m_eDiscBlockableStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eDiscBlockableStimID );
			m_eDiscBlockableStimID = kStimID_Unset;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::Setup()
//
//	PURPOSE:	Specialize the projectile disc setup (called externally)...
//
// ----------------------------------------------------------------------- //

LTBOOL CDisc::Setup( CWeapon const *pWeapon, WeaponFireInfo const &info )
{
	// setup the base class
	LTBOOL bResult = CProjectile::Setup( pWeapon, info );

	// get the ammo data
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
	ASSERT( 0 != pAmmo );

	// get the projectile
	PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
	ASSERT( 0 != pProjectileFX );

	// get the projectile class data
	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
	ASSERT( 0 != pDiscClassData );

	// bail if we have bad data
	if ( !pAmmo || !pProjectileFX || !pDiscClassData )
	{
		return LTFALSE;
	}

	// Save the socket the disc came from and will be returning to.
	m_hReturnToSocket = info.hSocket;

	// keep track of the disc's return velocity
	m_fReturnVelocity = pDiscClassData->fReturnVelocity;

	// keep track of the disc's return offset
	m_fReturnHeightOffset = pDiscClassData->fReturnHeightOffset;

	// if there is an angle offset specified, set the velocity
	// to start at that initial angle
	if ( DISC_EPSILON < info.fDiscReleaseAngle )
	{
		// get the fire direction
		LTRotation rFireRot( info.vPath, LTVector( 0.0f, 1.0f, 0.0f ) );
		rFireRot.Rotate( LTVector( 0.0f, 1.0f, 0.0f ), info.fDiscReleaseAngle );
		
		// scale to the velocity
		LTVector vOffsetVelocity;
		vOffsetVelocity = rFireRot.Forward();
		vOffsetVelocity *= static_cast< LTFLOAT >( pProjectileFX->nVelocity );

		// set the new offset velocity
		LTRESULT ltResult;
		ltResult = g_pPhysicsLT->SetVelocity( m_hObject, &vOffsetVelocity );
		ASSERT( LT_OK == ltResult );
	}

	switch ( info.nDiscTrackingType )
	{
		case MPROJ_DISC_TRACKING_STEADY:
		{
			//
			// disc flys straight with no special tracking
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_STEADY;

			// style doesn't apply
			m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT;
		}
		break;

		case MPROJ_DISC_TRACKING_POINT_GUIDED:
		{
			//
			// disc will change its flight path to hit a point
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_POINT_GUIDED;

			// get the initial target
			m_vPointTarget = info.vPointTarget;

			// remember to use it during our next update
			m_bNewTarget = true;

			// style is straight for point
			m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET;
		}
		break;

		case MPROJ_DISC_TRACKING_CONTROL_LINE:
		{
			//
			// disc will try to get to a control line specified by owner
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_CONTROL_LINE;

			// get the initial control direction
			m_vControlDirection = info.vControlDirection;

			// get the initial target
			m_vControlPosition = info.vControlPosition;

			// normalize the control direction
			m_vControlDirection.Normalize();

			// remember to use it during our next update
			m_bNewTarget = true;

			// get the player object
			CTronPlayerObj const *pPlayer =
				dynamic_cast< CTronPlayerObj* >( g_pCharacterMgr->FindPlayer() );

			// set the style of disc tracking
			m_eTrackingStyle = DISC_TRACKING_STYLE_CONTROL_LINE;

			// determine the turn rate based on perfomance ratings
			m_fTurnRate =
				InterpolatePerformanceRating(
					Rating_Accuracy,
					pDiscClassData->fTurnRateMin,
					pDiscClassData->fTurnRateMax,
					pDiscClassData->fTurnRateSurge
				);

			// determine incident angle based on perfomance ratings
			m_fIncidentAngleToControlLine =
				InterpolatePerformanceRating(
					Rating_Accuracy,
					pDiscClassData->fIncidentAngleToControlLineMin,
					pDiscClassData->fIncidentAngleToControlLineMax,
					pDiscClassData->fIncidentAngleToControlLineSurge
				);

			// determine incident angle decay based on perfomance ratings
			m_fIncidentAngleToControlLineDecay =
				InterpolatePerformanceRating(
					Rating_Accuracy,
					pDiscClassData->fIncidentAngleToControlLineDecayMin,
					pDiscClassData->fIncidentAngleToControlLineDecayMax,
					pDiscClassData->fIncidentAngleToControlLineDecaySurge
				);
		}
		break;

		case MPROJ_DISC_TRACKING_HOMING:
		{
			//
			// disc will change its flight path to hit an object
			//

			// start assuming the disc is flying towards a target
			m_eDiscState = DISC_STATE_FLYING;

			// set the tracking state
			m_eTrackingMode = DISC_TRACKING_MODE_HOMING;

			// get the initial target
			m_hObjectTarget = info.hObjectTarget;

			// remember to use it during our next update
			m_bNewTarget = true;

			// style is straight for the target
			m_eTrackingStyle = DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET;
		}
		break;

		default:
		{
			// The disc needs to be initialized in some type of mode,
			// check the WeaponFireInfo structure to be sure it has
			// been initialized properly.
			ASSERT( 0 );
		};

	};

	// we want messages, tell who shot us to send them along
	RequestStartSendingMessages();

	// Turn on the discs blockablity, but save the ID of the stimulus so that
	// when the disc starts to return this can be disabled

	ASSERT( m_eDiscBlockableStimID == kStimID_Unset );
	m_eDiscBlockableStimID = g_pAIStimulusMgr->RegisterStimulus(kStim_ProjectileVisible,
		m_hFiredFrom, m_hObject, CAIStimulusRecord::kDynamicPos_TrackTarget, LTVector(0,0,0) );

	return bResult;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::EngineMessageFn()
//
//	PURPOSE:	Override the default projecile engine message behaviour...
//
// ----------------------------------------------------------------------- //

uint32 CDisc::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch( messageID )
	{
		case MID_UPDATE:
		{
			// update the update counter manually since we don't call the base class
			++m_nUpdateNum;

			if ( true == UpdateDisc() )
			{
				// we handled the message, keep it away from the CProjectile base
				return LT_OK;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			// turn off ray hit so it doesn't intersect with the rays
			// the player casts to find a point for the disc to hit
			g_pCommonLT->SetObjectFlags( m_hObject,
			                             OFT_Flags,
			                             FLAG_FULLPOSITIONRES,
			                             ( FLAG_RAYHIT |
			                               FLAG_FULLPOSITIONRES ) );

			// record the start time so we know its life
			m_fStartTime = g_pLTServer->GetTime();

			// record the update time
			m_fLastUpdateTime = g_pLTServer->GetTime();

			// DANO: temp! hardcoded
			m_fLifeTime = 1000.0f;
		}
		break;

		default : break;
	}

	// pass the message to the base class
	return CProjectile::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::ObjectMessageFn()
//
//	PURPOSE:	Override the default projecile object message behaviour...
//
//	NOTE:		The messageID parameter is the projectile's message subtype.
//				This isn't an official ObjectMessageFn that the engine
//				calls, but instead a slave to the CWeapons class.
//				This has been done because it dosen't look like its 
//				possible to copy a message, or "peek" at its data, so it
//				is not possible to reroute using the "subtype" (1st byte)
//				of MID_PROJECTILE.
//
// ----------------------------------------------------------------------- //

uint32 CDisc::ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	ASSERT( 0 != pMsg );

	pMsg->SeekTo(0);
	uint32 mesasgeID = pMsg->Readuint32();

	LTRESULT ltResult;

	// get the projectile message subtype
	uint8 nProjectileMessageType = pMsg->Readuint8();

	switch ( nProjectileMessageType )
	{
		case MPROJ_UPDATE_POINT_TARGET:
		{
			// get the new target
			m_vPointTarget = pMsg->ReadLTVector();

			// remember to use it during our next update
			m_bNewTarget = true;
		}
		break;

		case MPROJ_UPDATE_OBJECT_TARGET:
		{
			// get the new target
			m_hObjectTarget = pMsg->ReadObject();

			// remember to use it during our next update
			m_bNewTarget = true;
		}
		break;

		case MPROJ_UPDATE_CONTROL_LINE:
		{
			// get the new control vector
			m_vControlDirection = pMsg->ReadLTVector();

			// get the new target
			m_vControlPosition = pMsg->ReadLTVector();

			// normalize the control direction
			m_vControlDirection.Normalize();

			// remember to use it during our next update
			m_bNewTarget = true;
		}
		break;

		case MPROJ_RETURNED:
		case MPROJ_RETURNING_DISTANCE_THRESHOLD:
		{
			// This should NOT be received by the disc.
			ASSERT( 0 );
		}
		break;

		case MPROJ_RETURN:
		{
			EnterReturnMode( false );
			g_pLTServer->CPrint( "----------------------------------------\n" );
		}
		break;

		default:
		{
			ASSERT( 0 );
		}
		break;
	};

	// pass the message to the base class
	return CProjectile::ObjectMessageFn( hSender, pMsg );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleTouch()
//
//	PURPOSE:	Override the default projecile touch notify behaviour...
//
// ----------------------------------------------------------------------- //

void CDisc::HandleTouch( HOBJECT hObj )
{
	switch ( m_eDiscState )
	{
		case DISC_STATE_FLYING:
		{
			CProjectile::HandleTouch( hObj );

			// base class will call our HandleImpact
			//HandleImpact( hObj );
		};
		break;

		case DISC_STATE_DEFLECTING:
		{
			// not implemented yet
		};
		break;

		case DISC_STATE_RETURNING:
		{
			LTRESULT ltResult;

			// check if we have returned to the owner
			if ( hObj == m_hFiredFrom )
			{
				// tell owner we don't need message updates anymore
				RequestStopSendingMessages();

				// send a message to the owner saying the disc returned
				CAutoMessage cMsg;

				cMsg.Writeuint32( MID_PROJECTILE );

				// write the projectile message subtype
				cMsg.Writeuint8( MPROJ_RETURNED );

				// write the weapon id
				cMsg.Writeuint8( m_nWeaponId );

				// write the ammo id
				cMsg.Writeuint8( m_nAmmoId );

				// write the ammo amount
				cMsg.Writeuint8( 1 );

				// send the return message to the owner
				ltResult = g_pLTServer->SendToObject( 
						cMsg.Read(),
						m_hObject,
						m_hFiredFrom,
						MESSAGE_GUARANTEED
					);
				ASSERT( LT_OK == ltResult );

				// Recognize that the message was sent
				m_bSentReturnedMsg = LTTRUE;

				// Send the appropriate message to the client...
				if ( IsPlayer( m_hFiredFrom ) )
				{
					// send the owner a "returned" message
					CAutoMessage cMsg;

					cMsg.Writeuint8( MID_PROJECTILE );

					// write the projectile message subtype
					cMsg.Writeuint8( MPROJ_RETURNED );

					// write the weapon id
					cMsg.Writeuint8( m_nWeaponId );

					// write the ammo id
					cMsg.Writeuint8( m_nAmmoId );

					// write the ammo amount
					cMsg.Writeuint8( 1 );

					CPlayerObj* pPlayer = static_cast< CPlayerObj* >( 
							g_pLTServer->HandleToObject( m_hFiredFrom ) );
					if ( pPlayer )
					{
						HCLIENT hClient = pPlayer->GetClient();
						if ( hClient )
						{
							ltResult = g_pLTServer->SendToClient( 
									cMsg.Read(),
									hClient,
									MESSAGE_GUARANTEED
								);
							ASSERT( LT_OK == ltResult );
						}
					}
				}

//				// play a sound saying that it was caught
//				// ??? - no spiffy sounds :(
	
				// we have returned to the owner
				RemoveObject();
			}
		};
		break;

		case DISC_STATE_NONE:
		default:
		{
			// invalid disc state
			ASSERT( 0 );
		}
		break;
	};
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::UpdateDisc()
//
//	PURPOSE:	Override the default projecile update behavior...
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateDisc()
{
	// Check to see if the owner is still in existance.  If the owner is
	// not still around, clean up the disc.  
	if ( m_hFiredFrom == NULL )
	{
		HandleOwnerDeath();
		return false;
	}

	// Process moving us through an invisible object if necessary.  This is
	// done here since the position/velocity can't be updated in the touch
	// notify (since the engine is in the process of updating it)...
	if (m_bProcessInvImpact)
	{
		m_bProcessInvImpact = LTFALSE;
		g_pPhysicsLT->SetVelocity(m_hObject, &m_vInvisVel);
		g_pLTServer->SetObjectPos(m_hObject, &m_vInvisNewPos);
	}

	// setup the next update
	g_pLTServer->SetNextUpdate( m_hObject, CDISC_UPDATE_DELTA );

	// the disc does very different things depending on its state
	switch ( m_eDiscState )
	{
		case DISC_STATE_FLYING:
		{
			return UpdateStateFlying();
		}
		break;

		case DISC_STATE_DEFLECTING:
		{
			return UpdateStateDeflecting();
		}
		break;

		case DISC_STATE_RETURNING:
		{
			return UpdateStateReturning();
		}
		break;

		case DISC_STATE_NONE:
		default:
		{
			// we shouldn't be in this state
			ASSERT( 0 );
		}
		break;
	};

	// this function did NOT handle the update
	return false;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::HandleOwnerDeath()
//              
//	PURPOSE:	Do something when the owner dies (probably delete the disc and 
//				launch off some sort of effect).  Currently simply removes the
//				the disk.
//              
//----------------------------------------------------------------------------

void CDisc::HandleOwnerDeath()
{
	// We have no owner, but do we need to notify that we don't need
	// accurate updates anymore??
	// RequestStopSendingMessages();

	// Delete the disk

	// No owner, so no message needs to be sent
	m_bSentReturnedMsg = LTTRUE;
	
	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::HandleImpact()
//
//	PURPOSE:	Override the default projecile impact behaviour...
//
// ----------------------------------------------------------------------- //

void CDisc::HandleImpact( HOBJECT hObj )
{
	// Check to see if our creator is still here before doing this -- we
	// don't want to run HandleImpact if we have no owner!
	if( m_hFiredFrom == NULL )
	{
		return;
	}

	// eventually we will determine the surface type
	SurfaceType eSurfaceType = ST_UNKNOWN;

	// get the object's position
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...
	LTVector vNormal(0.0f, 1.0f, 0.0f);

	// setup the default impact type
	IMPACT_TYPE eImpactType = IMPACT_TYPE_IMPACT;

	// if hObj in non-null, it is detonating on something
	if (hObj)
	{
		// projectile is impacting against something

		if ( IsCharacter( hObj ) )
		{
			// determine if the character is defending
			CCharacter *pCharacter = dynamic_cast< CCharacter * >(
					g_pLTServer->HandleToObject( hObj )
				);
			ASSERT( 0 != pCharacter );

			if ( pCharacter->IsDefending() )
			{
				eImpactType = IMPACT_TYPE_BLOCKED;

				// make the normal go contrary to the velocity
				g_pPhysicsLT->GetVelocity( m_hObject, &vNormal );
				vNormal.Normalize();
				vNormal = -vNormal;
			}
		}

		if (IsMainWorld(hObj) || GetObjectType(hObj) == OT_WORLDMODEL)
		{
			// get the collision info for this impact
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// check if we have a valid polygon
			if (info.m_hPoly != INVALID_HPOLY)
			{
				// get the surface type
				eSurfaceType = GetSurfaceType(info.m_hPoly);
			}

			// get the normal of the plane we impacted with
			LTPlane plane = info.m_Plane;
			vNormal = plane.m_Normal;

			//
			// Calculate where we really hit the plane
			// and make sure we don't tunnel through an object
			//

			LTVector vVel, vP0, vP1, vDir;
			// get the velocity of the projectile
			g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

			// get the direction of the projectile
			vDir = vVel;
			vDir.Normalize();

			// determine how much we've travelled this frame
			vVel *= g_pLTServer->GetFrameTime();

			// get a point just a little in front and behind the impact point
			vP0 = vPos - vVel;  // a little "behind" of the impact point
			vP1 = vPos + vVel;  // a littel "forward" of the impact point

			// throw an intersect segment to determine where we really hit
			IntersectInfo iInfo;
			IntersectQuery qInfo;

			// fill out the info for this test
			qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_From	  = vP0;
			qInfo.m_To		  = vPos;
			qInfo.m_FilterFn  = SpecificObjectFilterFn;
			qInfo.m_pUserData = m_hObject;

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
			{
				// we did hit the plane
				
				// get the intersect information
				vPos    = iInfo.m_Point - vDir;
				eSurfaceType   = GetSurfaceType(iInfo);
				vNormal = iInfo.m_Plane.m_Normal;
			}
			else
			{
				// plane was NOT hit

				// fake the impact position
				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - info.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - info.m_Plane.m_Dist;

				if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
				     ( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}
			}

			// reset the projectile's rotation
			LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);
		}
	}
	else
	{
		// Since hObj was null, this means the projectile's lifetime was up,
		// so we just blew-up in the air.
		eSurfaceType = ST_AIR;
	}

	// if the surface type has not been determined, try one more time
	if (eSurfaceType == ST_UNKNOWN)
	{
		// get the object's surface type
		eSurfaceType = GetSurfaceType(hObj);
	}

	// do all the impact work
	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eSurfaceType, eImpactType);

	// Handle impact damage...
	if (hObj)
	{
		HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;
		ImpactDamageObject(hDamager, hObj);
	}

	// Do the state change to the Disc's returning state
	EnterReturnMode( true );
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CDisc::EnterReturnMode()
//
//	PURPOSE:	State change keyed off of the Return state transition
//
//----------------------------------------------------------------------------

void CDisc::EnterReturnMode( bool bImpact )
{
	UBER_ASSERT( m_eDiscState!=DISC_STATE_RETURNING, "Disc already entered return mode." );

	// Add the catchable stimulus so that if thrown by an AI, it will attempt 
	// to catch it.
	if ( m_eDiscCatchableStimID == kStimID_Unset )
	{
		m_eDiscCatchableStimID = g_pAIStimulusMgr->RegisterStimulus(kStim_CatchableProjectileVisible,
			m_hFiredFrom, m_hObject, CAIStimulusRecord::kDynamicPos_TrackTarget, LTVector(0,0,0) );
	}

	// Remove the discs blockability (it flies through walls and all.. 
	// blocking wouldn't be effective!) 
	if ( m_eDiscBlockableStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eDiscBlockableStimID );
		m_eDiscBlockableStimID = kStimID_Unset;
	}

	// if we didn't impact anything, play the return effect
	if ( !bImpact )
	{
		// get the projectile
		PROJECTILEFX *pProjectileFX = m_pAmmoData->pProjectileFX;
		ASSERT( 0 != pProjectileFX );

		// get the disc data
		DISCCLASSDATA *pDiscData = 
			dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
		ASSERT( 0 != pDiscData );

		// pass the position to the client
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );

		// if there is an effect to spawn...
		if ( '\0' != pDiscData->szReturnFXName[ 0 ] )
		{
			// spawn the return effect
			PlayClientFX( pDiscData->szReturnFXName, 0, &vPos );
		}
	}

	// change the physics, let it pass through all geometry.
	g_pCommonLT->SetObjectFlags( m_hObject,
	                             OFT_Flags,
	                             ( FLAG_GOTHRUWORLD |
	                               FLAG_FULLPOSITIONRES ),
	                             ( FLAG_GRAVITY |
	                               FLAG_SOLID | 
	                               FLAG_REMOVEIFOUTSIDE |
	                               FLAG_STAIRSTEP |
	                               FLAG_GOTHRUWORLD |
	                               FLAG_FULLPOSITIONRES ) );

	// let owner know that we don't want any messages anymore
	RequestStopSendingMessages();

	// Mark the disc as returning
	m_eDiscState = DISC_STATE_RETURNING;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDisc::Detonate()
//
//	PURPOSE:	Override the default projecile detonate behaviour...
//
// ----------------------------------------------------------------------- //

void CDisc::Detonate( HOBJECT hObj )
{
//	HandleImpact( hObj );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::UpdateStateFlying()
//
//	PURPOSE:	Update the disc's movement when the disc is flying towards
//				a target
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateStateFlying()
{
	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );

	// this will become more refined as time progresses
	// for now, only compute this stuff if the point is new
	if ( !m_bNewTarget || ( DISC_TRACKING_MODE_STEADY == m_eTrackingMode ) )
	{
		// just update "normally"
		return true;
	}

	switch ( m_eTrackingStyle )
	{
		case DISC_TRACKING_STYLE_STRAIGHT_FOR_TARGET:
		{
			//
			// Simple basic easy disc tracking method
			//

			// get the current velocity, it will probably change
			LTVector vVel;
			g_pPhysicsLT->GetVelocity( m_hObject, &vVel );

			// Figure out a new velocity
			LTVector vTarget;
			LTVector vSelfPos;
			LTVector vSelfToTarget;

			// trying to hit a target, figure out where the target is
			vTarget = m_vPointTarget;

			// get the disc's location
			g_pLTServer->GetObjectPos( m_hObject, &vSelfPos );

			// find a vector from the disc and its target
			vSelfToTarget = vTarget - vSelfPos;

			// get the ammo data
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
			ASSERT( 0 != pAmmo );

			// get the projectile
			PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
			ASSERT( 0 != pProjectileFX );

			// get the disc class data
			DISCCLASSDATA *pDiscClassData = 
				dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
			ASSERT( 0 != pDiscClassData );

			// bail if we have bad data
			if ( !pAmmo || !pProjectileFX || !pDiscClassData )
			{
				return false;
			}

			// get the direction
			vSelfToTarget.Normalize();

			// scale the direction to the desired velocity
			vVel = vSelfToTarget;
			vVel *= m_fVelocity;

			// set the velocity
			g_pPhysicsLT->SetVelocity( m_hObject, &vVel );

			// orient the disc along the path of travel
			LTRESULT ltResult;
			LTRotation rotDiscRotation;
			ltResult = g_pLTServer->GetObjectRotation( m_hObject, &rotDiscRotation );
			LTVector vUp = rotDiscRotation.Up();
			LTRotation rotNewDiscRotation( vSelfToTarget, vUp );
			g_pLTServer->SetObjectRotation( m_hObject, &rotNewDiscRotation );

			// keep track that we updated the target
			m_bNewTarget = false;
		}
		break;

		case DISC_TRACKING_STYLE_CONTROL_LINE:
		{
			//
			// Disc tracking, pass2
			//

			LTRESULT ltResult;

			//
			// find a normalized vector pointing from the disc to the
			// closest point on a line extending from the control point
			// in the direction of the control direction
			//

			// get the disc position
			LTVector vDiscPos;
			ltResult = g_pLTServer->GetObjectPos( m_hObject, &vDiscPos );
			ASSERT( LT_OK == ltResult );

			// get the disc's current direction
			LTVector vDiscDirection;
			ltResult = g_pPhysicsLT->GetVelocity( m_hObject, &vDiscDirection );
			ASSERT( LT_OK == ltResult );
			vDiscDirection.Normalize();

			// using the direction, get the point on that line that is
			// closest to the current location of the disc
			LTVector vVectorToControlLine;
			LTVector vVectorToControlLineNormalized;
			GetVectorToLine( m_vControlPosition,
							 m_vControlDirection,
							 vDiscPos,
							 &vVectorToControlLine );
			vVectorToControlLineNormalized = vVectorToControlLine;
			vVectorToControlLineNormalized.Normalize();

			//
			// determine the incident angle
			//

			// get the elapsed time
			LTFLOAT fLifeElapsed = g_pLTServer->GetTime() - m_fStartTime;

			// find the incident angle, which is the initial angle
			// minus the decay times the time elapsed
			LTFLOAT fIncidentAngle;
			fIncidentAngle = m_fIncidentAngleToControlLine -
				m_fIncidentAngleToControlLineDecay * fLifeElapsed;

			// make sure the angle is not less than 0
			if ( DISC_EPSILON > fIncidentAngle )
			{
				fIncidentAngle = 0.0f;
			}

			//
			// determine the desired direction of the disc
			//

			// convert the angle to a percentage (of 90 degrees)
			// and linearly interpolate it
			LTVector vDesiredDirection;
			VEC_LERP(
					vDesiredDirection,
					m_vControlDirection,
					vVectorToControlLineNormalized,
					( ( fIncidentAngle ) / MATH_HALFPI )
				);

			// renormalize the result to eliminate the skew
			vDesiredDirection.Normalize();

			//
			// determine the angle between the disc's current direction
			// and the desired direction
			//

			// change the dot product equation around to get the angle
			// NOTE: these should both be unit vectors
			LTFLOAT vDirectionAngle;
			ASSERT( DISC_EPSILON > ( 1.0f - vDesiredDirection.Mag() ) );
			ASSERT( DISC_EPSILON > ( 1.0f - vDiscDirection.Mag() ) );
			vDirectionAngle = ltacosf( vDesiredDirection.Dot( vDiscDirection ) );

			//
			// determine how much time has passed since the last update
			//
			LTFLOAT fTimeSinceLastUpdate;
			fTimeSinceLastUpdate = g_pLTServer->GetTime() - m_fLastUpdateTime;

			//
			// interpolate the vector's current direction toward the
			// desired direction
			//

			// determine how much to interpolate
			LTFLOAT fInterpolation = m_fTurnRate * fTimeSinceLastUpdate;

			// don't interpolate more that we're supposed to
			if ( 1.0f < fInterpolation )
			{
				fInterpolation = 1.0f;
			}

			// interpolate the current and desired directions to
			// determine the new direction to go
			LTVector vNewDirection;
			VEC_LERP(
					vNewDirection,
					vDiscDirection,
					vDesiredDirection,
					( m_fTurnRate * fTimeSinceLastUpdate )
				);

			// scale the new direction to the disc's travelling velocity
			vNewDirection.Normalize();
			LTVector vNewVelocity = vNewDirection;
			vNewVelocity *= m_fVelocity;

			// set the new velocity
			g_pPhysicsLT->SetVelocity( m_hObject, &vNewVelocity );

			//
			// orient the disc 
			//
			// not currently implemented

			// keep track of the update time
			m_fLastUpdateTime = g_pLTServer->GetTime();
		}
		break;

		default:
		{
			ASSERT( !"Invalid tracking style" );
		}
		break;
	};

	/*
	//DANO: hardcoded lifetime
	if ( g_pLTServer->GetTime() >= ( m_fStartTime + 10.0f ) )
	{
		Detonate(LTNULL);
	}
	*/

	// we handled it
	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::UpdateStateDeflecting()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateStateDeflecting()
{
	// [kml] 4/2/02 Commenting this out as per DanO's request
	// get the current time
	/*int nCurrentServerTime =
		static_cast< int >( g_pLTServer->GetTime() * 1000.0f );

	// get the length of the deflection
	int nDeflectDuration = 1000;

	if ( nCurrentServerTime > ( m_nDeflectServerTimeStart + nDeflectDuration ) )
	{
		EnterReturnMode();
		return true;
	}*/

	return true;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::GetReturnToPosition()
//              
//	PURPOSE:	Returns the location the Disc should try to return to. 
//				Attempts to use the specified node if it exists and is
//				accessable.
//              
//----------------------------------------------------------------------------
LTVector CDisc::GetReturnToPosition(const LTVector& vOffset)
{
	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );
	ASSERT( 0 != g_pModelLT );

	// Get the position of the owner
	LTRESULT ltResult;
	LTVector vOwnerOrigin;
	ltResult = g_pLTServer->GetObjectPos( m_hFiredFrom, &vOwnerOrigin );
	ASSERT( LT_OK == ltResult );

	// Get the default position to return to
	LTVector vDefaultPosition = vOwnerOrigin + vOffset;

	// If we have a node to return to, then try going there
	if ( m_hReturnToSocket != INVALID_MODEL_SOCKET)
	{
		// Get the position of the Socket
		LTransform transform;
		LTRESULT SocketTransform = g_pModelLT->GetSocketTransform(m_hFiredFrom, m_hReturnToSocket, transform, LTTRUE);
		ASSERT( SocketTransform == LT_OK );
		LTVector vSocketPos = transform.m_Pos;

		// Get the name of the currently playing animation
		ANIMTRACKERID nTracker;
		g_pModelLT->GetMainTracker(m_hFiredFrom, nTracker);
		HMODELANIM hAnim;
		g_pModelLT->GetCurAnim(m_hFiredFrom, nTracker, hAnim );

		LTVector vDims;
		g_pCommonLT->GetModelAnimUserDims(m_hFiredFrom, &vDims, hAnim );
		ASSERT( ltResult == LT_OK );

		// See if the socket position is inside the touch box.
		// If it is not, then we have a problem as the return may fail, so if
		// the position is outside the box, then attempt to return to the
		// default position instead
		if (vSocketPos.x > vOwnerOrigin.x+vDims.x || 
			vSocketPos.x < vOwnerOrigin.x-vDims.x ||
			vSocketPos.y > vOwnerOrigin.y+vDims.y ||
			vSocketPos.y < vOwnerOrigin.y-vDims.y ||
			vSocketPos.z > vOwnerOrigin.z+vDims.z ||
			vSocketPos.z < vOwnerOrigin.z-vDims.z )
		{
			// Get the name of the model
			const int MODEL_LEN = 128;
			char szFileName[MODEL_LEN];
			char szSkinName[MODEL_LEN];
			g_pModelLT->GetFilenames(m_hFiredFrom, szFileName, MODEL_LEN, szSkinName, MODEL_LEN);

			// Get the name of the currently playing animation
			ANIMTRACKERID nTracker;
			g_pModelLT->GetMainTracker(m_hFiredFrom, nTracker);
			HMODELANIM hAnim;
			g_pModelLT->GetCurAnim(m_hFiredFrom, nTracker, hAnim );

			// Print out an error so that it can be corrected.
			char szError[1024];
			sprintf( szError, "Return to node outside dims: Model: %s Anim: %d", szFileName, hAnim );
			g_pLTServer->CPrint( szError );
		
			// Set the position we are going to be returning to back to a 
			// known safe location -- the owners position if this happens
			vSocketPos = vDefaultPosition;
		}
	
		// Okay, lets go with the socket position...
		return vSocketPos;
	}

	// If There is no node to return to, then to the deault
	return vDefaultPosition;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::UpdateStateReturning()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CDisc::UpdateStateReturning()
{
	// pointer check
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pPhysicsLT );

	// state check
	ASSERT( DISC_STATE_RETURNING == m_eDiscState );

	// for checking return values
	LTRESULT ltResult;

	// get the current velocity, it will probably change
	LTVector vVel;
	ltResult = g_pPhysicsLT->GetVelocity( m_hObject, &vVel );
	ASSERT( LT_OK == ltResult );

	// get the ammo data
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
	ASSERT( 0 != pAmmo );

	// get the projectile
	PROJECTILEFX *pProjectileFX = pAmmo->pProjectileFX;
	ASSERT( 0 != pProjectileFX );

	// get the projectile class data
	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
	ASSERT( 0 != pDiscClassData );

	// bail if we have bad data
	if ( !pAmmo || !pProjectileFX || !pDiscClassData )
	{
		return false;
	}

	// Figure out a new velocity, make the disc
	// return straight to the player
	LTVector vReturnToPos;
	LTVector vSelfPos;
	LTVector vSelfToTarget;

	// return to the owner, figure out where the owner is
	LTVector vOffset = LTVector(0,pDiscClassData->fReturnHeightOffset,0);
	vReturnToPos = GetReturnToPosition( vOffset );
	
	// get the disc's location
	ltResult = g_pLTServer->GetObjectPos( m_hObject, &vSelfPos );
	ASSERT( LT_OK == ltResult );

	// find a vector from the disc and its target
	vSelfToTarget = vReturnToPos - vSelfPos;

	// get the direction
	vSelfToTarget.Normalize();

	// scale the direction to the desired velocity
	vVel = vSelfToTarget;
	vVel *= pDiscClassData->fReturnVelocity;

	// set the velocity
	ltResult = g_pPhysicsLT->SetVelocity( m_hObject, &vVel );
	ASSERT( LT_OK == ltResult );

	// orient the disc along the path of travel
	LTRotation rotDiscRotation;
	ltResult = g_pLTServer->GetObjectRotation( m_hObject, &rotDiscRotation );
	LTVector vUp = rotDiscRotation.Up();
	LTRotation rotNewDiscRotation( vSelfToTarget, vUp );
	g_pLTServer->SetObjectRotation( m_hObject, &rotNewDiscRotation );

/*
	//DANO: hardcoded lifetime
	if ( g_pLTServer->GetTime() >= ( m_fStartTime + 10.0f ) )
	{
		Detonate( LTNULL );
	}
*/

	// we handled it
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CDisc::IsDiscReturning()
//              
//	PURPOSE:	Returns true if the disc is returning, false if it is not.
//				This allows us to keep the enumeration in the disc itself
//				instead of spreading it out over all derived classes as well.
//              
//----------------------------------------------------------------------------
bool CDisc::IsDiscReturning()
{
	return ( m_eDiscState == DISC_STATE_RETURNING );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::RequestStartSendingMessages()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CDisc::RequestStartSendingMessages()
{
	if ( false == m_bReceivingMessages )
	{
		//
		// tell owner we want messages
		//

		ASSERT( 0 != m_hObject );
		ASSERT( m_hFiredFrom );

		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint32( MID_PROJECTILE );

		// write the mesasge subtype
		cMsg.Writeuint8( MPROJ_START_SENDING_MESSAGES );

		// write the number of objects that want messages
		cMsg.Writeuint8( 1 );

		// write the hobject that wants updates
		cMsg.WriteObject( m_hObject );

		// send to the owner
		ltResult = g_pLTServer->SendToObject(
				cMsg.Read(),
				m_hObject,
				m_hFiredFrom,
				MESSAGE_GUARANTEED
			);
		ASSERT( LT_OK == ltResult );

		// keep track that we want messages
		m_bReceivingMessages = true;
	}
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CDisc::RequestStopSendingMessages()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CDisc::RequestStopSendingMessages()
{
	if ( true == m_bReceivingMessages )
	{
		//
		// tell owner we don't want messages
		//

		ASSERT( 0 != m_hObject );
		ASSERT( m_hFiredFrom );

		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint32( MID_PROJECTILE );

		// write the mesasge subtype
		cMsg.Writeuint8( MPROJ_STOP_SENDING_MESSAGES );

		// write the number of objects that don't need messages anymore
		cMsg.Writeuint8( 1 );

		// write the hobject to remove from getting updates
		cMsg.WriteObject( m_hObject );

		// send to the owner
		ltResult = g_pLTServer->SendToObject(
				cMsg.Read(),
				m_hObject,
				m_hFiredFrom,
				MESSAGE_GUARANTEED
			);
		ASSERT( LT_OK == ltResult );

		// keep track that we don't want messages
		m_bReceivingMessages = false;
	}
}
