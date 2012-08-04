/****************************************************************************
;
;	MODULE:			TronPlayerObj.cpp
;
;	PURPOSE:		Tron-specific player object
;
;	HISTORY:		1/28/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "TronPlayerObj.h"
#include "WeaponFireInfo.h"
#include "Attachments.h"
#include "ParsedMsg.h"
#include "Weapon.h"
#include "FXButeMgr.h"
#include "LightCycleMgr.h"

BEGIN_CLASS(CTronPlayerObj)
END_CLASS_DEFAULT_FLAGS(CTronPlayerObj, CPlayerObj, NULL, NULL, CF_HIDDEN)

#define TRIGGER_ACQUIRE_SUBROUTINE				"ACQUIRESUB"
#define TRIGGER_ACQUIRE_ADDITIVE				"ACQUIREADD"
#define TRIGGER_ACQUIRE_PROCEDURAL				"ACQUIREPROC"
#define TRIGGER_ACQUIRE_PRIMITIVE				"ACQUIREPRIM"
#define TRIGGER_ADD_BUILD_POINTS				"ADDBUILDPOINTS"
#define TRIGGER_START_LIGHTCYCLE				"STARTLIGHTCYCLE"
#define TRIGGER_END_LIGHTCYCLE					"ENDLIGHTCYCLE"

CMDMGR_BEGIN_REGISTER_CLASS( CTronPlayerObj )
	CMDMGR_ADD_MSG( ACQUIRESUB,		-1,		NULL,			"ACQUIRESUB <Subroutine Name> [state] [condition]")
	CMDMGR_ADD_MSG( ACQUIREADD,		-1,		NULL,			"ACQUIREADD <Additive Name>")
	CMDMGR_ADD_MSG( ACQUIREPROC,	-1,		NULL,			"ACQUIREPROC <Procedural Name>")
	CMDMGR_ADD_MSG( ACQUIREPRIM,	-1,		NULL,			"ACQUIREPRIM <Primitive Name>")
	CMDMGR_ADD_MSG( ACQUIREPRIM,	-1,		NULL,			"STARTLIGHTCYCLE")
	CMDMGR_ADD_MSG( ACQUIREPRIM,	-1,		NULL,			"ENDLIGHTCYCLE")
CMDMGR_END_REGISTER_CLASS( CTronPlayerObj, CPlayerObj )

// Subroutine states
char *g_SubStates[] =
{
	"ALPHA",
	"BETA",
	"GOLD"
};
int g_nNumSubStates = (sizeof(g_SubStates) / sizeof(char *));

// Subroutine conditions (subject to change)
char *g_SubConditions[] =
{
	"NORMAL",
	"INFECTED",
	"NONCOMPATIBLE"
};
int g_nNumSubConditions = (sizeof(g_SubConditions) / sizeof(char *));


namespace
{
	int8 const TRONPLAYEROBJ_NO_DEFEND = -1;
	float const TRONPLAYEROBJ_CLIENT_CAMERA_TIME_OLD_THRESHOLD = 1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CTronPlayerObj()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CTronPlayerObj::CTronPlayerObj()
	: CPlayerObj()
	, m_nDefendClientTimeStarted( 0 )
	, m_nDefendServerTimeStarted( 0 )
	, m_nDefendDuration( 0 )
	, m_cDefendType( TRONPLAYEROBJ_NO_DEFEND )
	, m_cDefendAmmoId( WMGR_INVALID_ID )
{
	// Default sheeyot
	memset(m_iPerformanceRatings,0,NUM_RATINGS*sizeof(uint8));
	m_byPSets = 0;
	m_byOldPSets = 0;
	m_nArmorPercentage = 0;
	m_nBuildPoints = 100; // Start the player off with 100 of these, so sayeth Andy.
	m_nOldBuildPoints = 100;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::~CTronPlayerObj()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CTronPlayerObj::~CTronPlayerObj()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::ObjectMessageFn()
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CTronPlayerObj::ObjectMessageFn( HOBJECT hSender,
                                        ILTMessage_Read *pMsg )
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch ( messageID )
	{
		case MID_PROJECTILE:
		{
			bool bMessageHandled;
			bMessageHandled =
					HandleMessageProjectile( hSender, pMsg );
			if ( bMessageHandled )
			{
				return 1;
			}
		}
		break;
	}

	return CPlayerObj::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleMessageProjectile()
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::HandleMessageProjectile( HOBJECT hSender,
                                              ILTMessage_Read *pMsg )
{
	// to check the results of engine functions
	LTRESULT ltResult;

	// get the projectile message's subtype
	uint8 nProjectileMessageSubType = pMsg->Readuint8();

	switch ( nProjectileMessageSubType )
	{
		//case MPROJ_RETURNED:
		// the place where the ammo gets incremented is in CWeapons


		case MPROJ_START_SWAT_BLOCK:
		case MPROJ_START_HOLD_BLOCK:
		case MPROJ_START_ARM_BLOCK:
		{
			// get the rest of the info

			// get the weapon id
			uint8 cAmmoId = pMsg->Readuint8();

			// get the timestamp
			uint32 nDefendTimestamp = pMsg->Readuint8();

			// get the animation length
			uint32 nDefendDuration = pMsg->Readuint8();

			// make sure the current weapon is the weapon
			// being blocked with
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
			if ( pWeapon->GetAmmoId() != cAmmoId )
			{
				// Tut tut, no cheating!
				//
				// The weapon the user is trying to block with is
				// different from the current weapon.
				ASSERT( 0 );

				return true;
			}

			if ( TRONPLAYEROBJ_NO_DEFEND != m_cDefendType )
			{
				// we are already handling a block, check
				// if the new one is valid
				if ( ( m_nDefendClientTimeStarted + m_nDefendDuration ) >
					 static_cast< int >( nDefendTimestamp ) )
				{
					// Tut, tut, no cheating!
					//
					// The timestamp passed by the client is faster
					// than is allowed.  If this assert is hit check,
					// to make sure a defensive animation isn't getting
					// nixed prematurely.
					ASSERT( 0 );

					return true;
				}
			}

			// we're ok with the new block, setup the info
			m_cDefendType = nProjectileMessageSubType;
			m_nDefendClientTimeStarted = nDefendTimestamp;
			m_nDefendServerTimeStarted =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);
			m_nDefendDuration = nDefendDuration;
			m_cDefendAmmoId = cAmmoId;

			return true;
		}
		break;

		case MPROJ_END_HOLD_BLOCK:
		{
			// get the rest of the info

			// get the weapon id
			uint8 cAmmoId = pMsg->Readuint8();

			// get the timestamp
			uint32 nDefendTimestamp = pMsg->Readuint8();

			// get the animation length
			uint32 nDefendDuration = pMsg->Readuint8();

			// make sure the current weapon is the weapon
			// being blocked with
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
			if ( pWeapon->GetAmmoId() != cAmmoId )
			{
				// Tut tut, no cheating!
				//
				// The weapon the user is trying to block with is
				// different from the current weapon.
				ASSERT( 0 );

				return true;
			}

			if ( MPROJ_START_HOLD_BLOCK != m_cDefendType )
			{
				// Woa!  We can't end the hold block until
				// after it starts!  Something is out of
				// order, or there's some cheating going on!
				ASSERT( 0 );

				return true;
			}

			if ( ( m_nDefendClientTimeStarted + m_nDefendDuration ) >
				 static_cast< int >( nDefendTimestamp ) )
			{
				// Tut, tut, no cheating!
				//
				// The timestamp passed by the client is faster
				// than is allowed.  If this assert is hit check,
				// to make sure a defensive animation isn't getting
				// nixed prematurely.
				ASSERT( 0 );

				return true;
			}

			// we're ok with the new block, setup the info
			m_cDefendType = nProjectileMessageSubType;
			m_nDefendClientTimeStarted = nDefendTimestamp;
			m_nDefendServerTimeStarted =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);
			m_nDefendDuration = nDefendDuration;
			m_cDefendAmmoId = cAmmoId;

			return true;
		}
		break;

		default:
		{
			// the message will be forwarded through the chain, eventually
			// to the CWeapon attachment where we want it to go

			// reset the message, we don't want to alter it
			pMsg->SeekTo(0);

			// return false, this did NOT handle the message
			return false;
		}
		break;
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Save
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if(!pMsg)
		return;

	for(int i=0;i<NUM_RATINGS;i++)
	{
		SAVE_BYTE(m_iPerformanceRatings[i]);
	}
	SAVE_BYTE(m_byPSets);
	SAVE_BYTE(m_byOldPSets);
	SAVE_BYTE(m_nArmorPercentage);
	SAVE_WORD(m_nBuildPoints);
	SAVE_WORD(m_nOldBuildPoints);

	CPlayerObj::Save(pMsg, dwSaveFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	for(int i=0;i<NUM_RATINGS;i++)
	{
		LOAD_BYTE(m_iPerformanceRatings[i]);
	}
	LOAD_BYTE(m_byPSets);
	LOAD_BYTE(m_byOldPSets);
	LOAD_BYTE(m_nArmorPercentage);
	LOAD_WORD(m_nBuildPoints);
	LOAD_WORD(m_nOldBuildPoints);

	CPlayerObj::Load(pMsg, dwLoadFlags);
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CTronPlayerObj::GetExtraFireMessageInfo()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::GetExtraFireMessageInfo( uint8 nWeaponType,
											  ILTMessage_Read *pMsg,
                                              WeaponFireInfo *pInfo )
{
	// 
	ASSERT( 0 != pInfo );

	LTRESULT ltResult;

	switch ( nWeaponType )
	{
		case MWEAPFIRE_VECTOR:
		case MWEAPFIRE_PROJECTILE:
		{
			// no extra information needed
		}
		break;

		case MWEAPFIRE_DISC:
		{
			// get the angle offset (from center) to fire disc
			pInfo->fDiscReleaseAngle = pMsg->Readfloat();

			// get the disc tracking type
			pInfo->nDiscTrackingType = pMsg->Readuint8();

			switch ( pInfo->nDiscTrackingType )
			{
				case MPROJ_DISC_TRACKING_HOMING:
				{
					// get the first object point
					pInfo->hObjectTarget = pMsg->ReadObject();
				}
				break;

				case MPROJ_DISC_TRACKING_STEADY:
				{
					// nothing more needed
				}
				break;

				case MPROJ_DISC_TRACKING_POINT_GUIDED:
				{
					// get the first target point
					pInfo->vPointTarget = pMsg->ReadLTVector();
				}
				break;

				case MPROJ_DISC_TRACKING_CONTROL_LINE:
				{
					// get the first control vector
					pInfo->vControlDirection = pMsg->ReadLTVector();

					// get the first target point
					pInfo->vControlPosition = pMsg->ReadLTVector();
				}
				break;

				default:
				{
					// An invalid DiscTracking type was encountered.
					// Check the construction of the message itself
					// (probably on the client).
					ASSERT( 0 );
				}
				break;
			};

			// is there extra cluster information?
			uint8 nIsCluster;
			ltResult = pMsg->ReadByteFL( nIsCluster );
			ASSERT( LT_OK == ltResult );

			if ( nIsCluster )
			{
				//
				// get the cluster information from the message
				//

				// get the horizontal spread
				uint16 nTmp;
				ltResult = pMsg->ReadWordFL( nTmp );
				ASSERT( LT_OK == ltResult );
				UncompressAngleFromShort( nTmp, &pInfo->fHorizontalSpread );

				// get the horizontal perturb
				uint8 nHorizontalPerturb;
				ltResult = pMsg->ReadByteFL( nHorizontalPerturb );
				ASSERT( LT_OK == ltResult );
				UncompressAngleFromByte( nHorizontalPerturb, &pInfo->fHorizontalPerturb );

				// get the vertical spread
				uint8 nVerticalSpread;
				ltResult = pMsg->ReadByteFL( nVerticalSpread );
				ASSERT( LT_OK == ltResult );
				UncompressAngleFromByte( nVerticalSpread, &pInfo->fVerticalSpread );

				// get the number of shards
				ltResult = pMsg->ReadByteFL( pInfo->nNumberOfShards );
				ASSERT( LT_OK == ltResult );
			}

		}
		break;

		default:
		{
			// An invalid weapon type was used, check
			// the creation of the WeaponFireInfo
			// struct, possibly the construction of the
			// message itself.
			ASSERT( 0 );
		}
		break;
	};
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::UpdateInterface( bool bForceUpdate )
{
	if ( !m_pPlayerAttachments ) return;
	if (!m_hClient || !g_pWeaponMgr) return;

	// See if the psets have changed...
	if(m_byPSets != m_byOldPSets)
	{
		m_byOldPSets = m_byPSets;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_PSETS_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_byPSets);
		g_pLTServer->SendToClient(cMsg.Read(), m_hClient, MESSAGE_GUARANTEED);
	}

	// See if the build points have changed
	if(m_nBuildPoints != m_nOldBuildPoints)
	{
		m_nOldBuildPoints = m_nBuildPoints;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_BUILD_POINTS_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat((LTFLOAT)m_nBuildPoints);
		g_pLTServer->SendToClient(cMsg.Read(), m_hClient, MESSAGE_GUARANTEED);
	}

	// Call the base class
	CPlayerObj::UpdateInterface(bForceUpdate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::Update
//
//	PURPOSE:	Main update
//
// ----------------------------------------------------------------------- //
LTBOOL CTronPlayerObj::Update()
{
	// Call down to the base class
	return(CPlayerObj::Update());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleCompile
//
//	PURPOSE:	What to do when we get a compile message from the client
//
// ----------------------------------------------------------------------- //
void CTronPlayerObj::HandleCompile(ILTMessage_Read *pMsg)
{
	// First read in the armor percentage
	m_nArmorPercentage = pMsg->Readuint8();

	// Read in the performance ratings
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		m_iPerformanceRatings[i] = pMsg->Readuint8();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::IsDefending()
//
//	PURPOSE:	Return if the player is attempting to defend.
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::IsDefending() const
{
	switch ( m_cDefendType )
	{
		case MPROJ_START_SWAT_BLOCK:
		{
			int32 nCurrentTime =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);

			if ( ( m_nDefendServerTimeStarted + m_nDefendDuration ) > nCurrentTime )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		break;

		case MPROJ_START_HOLD_BLOCK:
		{
			return true;
		}
		break;

		case MPROJ_END_HOLD_BLOCK:
		{
			int32 nCurrentTime =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);

			if ( ( m_nDefendServerTimeStarted + m_nDefendDuration ) > nCurrentTime )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		break;

		case MPROJ_START_ARM_BLOCK:
		{
			int32 nCurrentTime =
				static_cast< int >(
					g_pLTServer->GetTime() * 1000.0f
				);

			if ( ( m_nDefendServerTimeStarted + m_nDefendDuration ) > nCurrentTime )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		break;

		default:
		{
			// not blocking
			return false;
		}
		break;
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::GetDefensePercentage()
//
//	PURPOSE:	How much of the attack damage has our defenese prevented
//
//	NOTES:		There are actually 2 defense percentages.  One is based
//				on timing, i.e. player's reation speed, animation speed,
//				etc.  The other is based on the player's orientation to
//				the incoming projectile.  If the vector parameter is
//				specified, BOTH percentages will be computed and the
//				combined result will be returned.  If the vector is NOT
//				specified, only the timing will be computed.
//
// ----------------------------------------------------------------------- //
float CTronPlayerObj::GetDefensePercentage( LTVector const *pIncomingProjectilePosition /*=0*/) const
{
	if ( TRONPLAYEROBJ_NO_DEFEND == m_cDefendType )
	{
		// not blocking
		return 0.0f;
	}

	//
	// Do this in 2 passes.  The first pass willl determine
	// the contribution to the defense percentage due to the
	// timing of the player/animations.  The second pass
	// will add the contribution of the player's orientation.
	//
	float fDefenseTimingPercentage;
	float fDefenseOrientationPercentage;

	// get the weapon
	AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo( m_cDefendAmmoId );
	ASSERT( 0 != pAmmoData );
	ASSERT( 0 != pAmmoData->pProjectileFX );

	// get the ammo specific data
	DISCCLASSDATA *pDiscData =
		dynamic_cast< DISCCLASSDATA* >(
			pAmmoData->pProjectileFX->pClassData
		);
	ASSERT( 0 != pDiscData );


	//
	// Determine Timing Percentage
	//

	switch ( m_cDefendType )
	{
		case MPROJ_START_SWAT_BLOCK:
		case MPROJ_START_ARM_BLOCK:
		{
			// get the current server time
			float fCurrentServerTime = g_pLTServer->GetTime() * 1000.0f;

			// make sure we're within the range of the block
			if ( ( static_cast< int >( fCurrentServerTime ) -
			       m_nDefendServerTimeStarted ) > m_nDefendDuration )
			{
				// nope, the block is over
				return 0.0f;
			}

			// Swat and Arm defenses are similar, so fill out these
			// variables uniquely (depending on which case we are
			// handling), then use the common generic math to figure
			// out the answer.
			float fMidpointTime;
			float fMaxStartTime;
			float fMaxEndTime;
			float fStartDefendPercentage;
			float fMaxDefendPercentage;
			float fEndDefendPercentage;

			if ( MPROJ_START_SWAT_BLOCK == m_cDefendType )
			{
				// determine at exactly what time the midpoint takes place
				// NOTE: this is relative time, not absolute
				fMidpointTime = 
					( pDiscData->fSwatDefendMidpoint * m_nDefendDuration );

				// determine at exactly what time the max starts
				// NOTE: this is relative time, not absolute
				fMaxStartTime =
					fMidpointTime - 
					fMidpointTime * pDiscData->fSwatDefendStartMaxDefendPercentage;

				// determine at exactly what time the max ends
				// NOTE: this is relative time, not absolute
				fMaxEndTime =
					fMidpointTime +
					(
						( m_nDefendDuration - fMidpointTime ) *
						pDiscData->fSwatDefendEndMaxDefendPercentage
					);

				// determine the starting defend percentage
				fStartDefendPercentage = pDiscData->fSwatDefendStartDefendPercentage;

				// detecmine the max defend percentage					
				fMaxDefendPercentage = pDiscData->fSwatDefendMaxDefendPercentage;

				// determine the ending defend percentage
				fEndDefendPercentage = pDiscData->fSwatDefendEndDefendPercentage;
			}
			else if ( MPROJ_START_ARM_BLOCK == m_cDefendType )
			{
				// Not implemented yet.  The main question I haven't figured
				// out yet is where does the information come from?
				fMidpointTime = 0.0f;
				fMaxStartTime = 0.0f;
				fMaxEndTime = 0.0f;
				fStartDefendPercentage = 0.0f;
				fMaxDefendPercentage = 0.0f;
				fEndDefendPercentage = 0.0f;
			}

			// determine at exactly how much time we've been in the block
			// NOTE: this is relative time, not absolute
			float fBlockTime = 
				fCurrentServerTime - m_nDefendServerTimeStarted;

			if ( ( -MATH_EPSILON <= fBlockTime ) && 
			     ( fBlockTime <= fMaxStartTime ) )
			{
				// somewhere on the uprise
				fDefenseTimingPercentage =
					fStartDefendPercentage + 
						(
							(
								( 
									fMaxDefendPercentage -
									fStartDefendPercentage
								) /
								fMaxStartTime 
							) * 
							fBlockTime
						);
			}
			else if ( ( fMaxStartTime < fBlockTime ) &&
			          ( fBlockTime <= fMaxEndTime ) )
			{
				// within the max range
				fDefenseTimingPercentage = fMaxDefendPercentage;
			}
			else if ( ( fMaxEndTime < fBlockTime ) &&
			          ( fBlockTime <= m_nDefendDuration ) )
			{
				// somewhere on the downfall
				fDefenseTimingPercentage =
					fMaxDefendPercentage - 
						(
							(
								( 
									fMaxDefendPercentage -
									fEndDefendPercentage
								) /
								( m_nDefendDuration - fMaxEndTime )
							) * 
							( fBlockTime - fMaxEndTime )
						);
			}
			else
			{
				// math problem, we should be here if we are outside
				// the bounds of the defense
				ASSERT( 0 );
				fDefenseTimingPercentage = 0.0f;
			}
		}
		break;


		case MPROJ_START_HOLD_BLOCK:
		{
			// get the current server time
			float fCurrentServerTime = g_pLTServer->GetTime() * 1000.0f;

			// determine the starting defend percentage
			float fStartDefendPercentage = pDiscData->fHoldDefendStartDefendPercentage;

			// determine the max defend percentage					
			float fMaxDefendPercentage = pDiscData->fHoldDefendMaxDefendPercentage;

			// determine at exactly how much time we've been in the block
			// NOTE: this is relative time, not absolute
			float fBlockTime = 
				fCurrentServerTime - m_nDefendServerTimeStarted;

			if ( ( -MATH_EPSILON <= fBlockTime ) && 
			     ( fBlockTime <= m_nDefendDuration ) )
			{
				// somewhere on the uprise
				fDefenseTimingPercentage =
					fStartDefendPercentage + 
						(
							(
								( 
									fMaxDefendPercentage -
									fStartDefendPercentage
								) /
								static_cast< float >( m_nDefendDuration )
							) * 
							fBlockTime
						);
			}
			else if ( m_nDefendDuration < fBlockTime )
			{
				// within the max range
				fDefenseTimingPercentage = fMaxDefendPercentage;
			}
			else
			{
				// math problem, we should be here if we are outside
				// the bounds of the defense
				ASSERT( 0 );
				fDefenseTimingPercentage = 0.0f;
			}
		}
		break;

		case MPROJ_END_HOLD_BLOCK:
		{
			// get the current server time
			float fCurrentServerTime = g_pLTServer->GetTime() * 1000.0f;

			// make sure we're within the range of the block
			if ( ( static_cast< int >( fCurrentServerTime ) -
			       m_nDefendServerTimeStarted ) > m_nDefendDuration )
			{
				// nope, the block is over
				return 0.0f;
			}

			// detecmine the max defend percentage					
			float fMaxDefendPercentage = pDiscData->fHoldDefendMaxDefendPercentage;

			// determine the ending defend percentage
			float fEndDefendPercentage = pDiscData->fHoldDefendEndDefendPercentage;

			// determine at exactly how much time we've been in the block
			// NOTE: this is relative time, not absolute
			float fBlockTime = 
				fCurrentServerTime - m_nDefendServerTimeStarted;

			// somewhere on the downfall
			fDefenseTimingPercentage =
				fMaxDefendPercentage - 
					(
						(
							( 
								fMaxDefendPercentage -
								fEndDefendPercentage
							) /
							( m_nDefendDuration )
						) * 
						( fBlockTime )
					);
		}
		break;

		default:
		{
			// There is some type of block defined that we
			// are not handling, and we SHOULD be handling it.
			ASSERT( 0 );

			return 0.0f;
		}
		break;
	};


	//TODO: skip this section of the camera position is too old?

	// check if the oriention percentage should be computed
	if ( 0 == pIncomingProjectilePosition )
	{
		// No vector specifed, there is no way
		// to compute orientation defense.
		return fDefenseTimingPercentage;
	}

	//
	// Determine Orientation percentage
	//

	// The 3 cases are the same, but they could have different
	// control values.  Figure out what the specific variables
	// are (depending on the specific type of block), then apply
	// the generic equations.

	float fOrientationMinDefendPercentage;
	float fOrientationMaxDefendPercentage;
	float fOrientationDeadZone;
	float fOrientationMaxZone;

	switch ( m_cDefendType )
	{
		case MPROJ_START_SWAT_BLOCK:
		{
			fOrientationMinDefendPercentage  = pDiscData->fSwatDefendOrientationMinDefendPercentage;
			fOrientationMaxDefendPercentage  = pDiscData->fSwatDefendOrientationMaxDefendPercentage;
			fOrientationDeadZone             = MATH_PI - pDiscData->fSwatDefendOrientationDeadZone;
			fOrientationMaxZone              = pDiscData->fSwatDefendOrientationMaxZone;
		}
		break;

		case MPROJ_START_HOLD_BLOCK:
		case MPROJ_END_HOLD_BLOCK:
		{
			fOrientationMinDefendPercentage  = pDiscData->fHoldDefendOrientationMinDefendPercentage;
			fOrientationMaxDefendPercentage  = pDiscData->fHoldDefendOrientationMaxDefendPercentage;
			fOrientationDeadZone             = MATH_PI - pDiscData->fHoldDefendOrientationDeadZone;
			fOrientationMaxZone              = pDiscData->fHoldDefendOrientationMaxZone;
		}
		break;

		case MPROJ_START_ARM_BLOCK:
		{
			// Not implemented yet.  The main question I haven't figured
			// out yet is where does the information come from?
			fOrientationMinDefendPercentage  = 0.0f;
			fOrientationMaxDefendPercentage  = 0.0f;
			fOrientationDeadZone             = 0.0f;
			fOrientationMaxZone              = 0.0f;
		}
		break;

		default:
		{
			// There is some type of block defined that we
			// are not handling, and we SHOULD be handling it.
			ASSERT( 0 );

			return 0.0f;
		}
		break;
	};

	LTRESULT ltResult;
	LTVector vDefendPos;
	LTVector vDefendPosToProjectile;

	// get the player's position
	ltResult = g_pLTServer->GetObjectPos( m_hObject, &vDefendPos );
	ASSERT( LT_OK == ltResult );

	// REMOVE THIS CODE FOR FINAL RELEASE, BUT LEAVE FOR TESTING MULTIPLAYER
	// print a warning if the time is new enough
	if ( TRONPLAYEROBJ_CLIENT_CAMERA_TIME_OLD_THRESHOLD < ( g_pLTServer->GetTime() - m_nClientCameraOffsetTimeReceivedMS ) )
	{
		g_pLTServer->CPrint( "Client Camera Offset time is low, possible lag\n" );
		g_pLTServer->CPrint( "   condition that will affect defensive accuracy.\n" );
		g_pLTServer->CPrint( "   Currnt value is %5.3f units old.\n", ( g_pLTServer->GetTime() - m_nClientCameraOffsetTimeReceivedMS ) );

	}
	
	// add the camera offset vDefendPos += m_vClientCameraOffset;

	// find a unit vector from us to the projectile
	vDefendPosToProjectile  = *pIncomingProjectilePosition -
	                         ( m_vClientCameraOffset +
	                           vDefendPos );
	vDefendPosToProjectile.y = 0;
	vDefendPosToProjectile.Normalize();

	// determine a forward vector reprensenting the direction the
	// player is facing
	LTRotation vPlayerViewOrientation;
	ltResult = g_pLTServer->GetObjectRotation( m_hObject, &vPlayerViewOrientation );
	ASSERT( LT_OK == ltResult );

	LTVector vPlayerViewForward = vPlayerViewOrientation.Forward();
	vPlayerViewForward.y = 0;
	vPlayerViewForward.Normalize();

	float fDotProd = vPlayerViewForward.Dot( vDefendPosToProjectile );

	// find the angle between the two vectors
	float fDefenseAngle = ltacosf( vPlayerViewForward.Dot( vDefendPosToProjectile ) );

	if ( ( MATH_EPSILON <= fDefenseAngle) && ( fDefenseAngle <= fOrientationMaxZone ) )
	{
		// it's within the max zone
		fDefenseOrientationPercentage = fOrientationMaxDefendPercentage;
	}
	else if ( ( fOrientationMaxZone < fDefenseAngle) && ( fDefenseAngle <= fOrientationDeadZone ) )
	{
		// it's within the dropoff range
		fDefenseOrientationPercentage = 
			fOrientationMaxDefendPercentage +
			( fDefenseAngle - fOrientationMaxZone ) *
				( fOrientationMinDefendPercentage - fOrientationMaxDefendPercentage ) /
				( fOrientationDeadZone - fOrientationMaxZone );
	}
	else if ( fOrientationDeadZone <= fDefenseAngle)
	{
		// it's within the dead zone
		fDefenseOrientationPercentage = 0.0f;
	}

	//
	// Final Defense Result
	//

	float fFinalDefensePercentage = fDefenseTimingPercentage -
	                                fDefenseOrientationPercentage;
	return fFinalDefensePercentage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::FilterDamage()
//
//	PURPOSE:	Change the damage struct before damage is dealt to the player
//
// ----------------------------------------------------------------------- //

bool CTronPlayerObj::FilterDamage( DamageStruct *pDamageStruct )
{
	// DANO: for testing
	float fOrigDamage = pDamageStruct->fDamage;

	if ( IsDefending() )
	{
		// player is defending, attenuate the damage to take
		float fDefenseModifier = GetDefensePercentage( &pDamageStruct->vDir );

		pDamageStruct->fDamage *= ( 1.0f - fDefenseModifier );
	}

	// pass the filter up the hierarchy
	return CPlayerObj::FilterDamage( pDamageStruct );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::HandleDamage
//
//	PURPOSE:	Handles getting damaged
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::HandleDamage(const DamageStruct& damage)
{
	if ( WMGR_INVALID_ID != damage.nAmmoId )
	{
		// hit by a weapon of some sort
		if ( IsDefending() )
		{
			// send a message to the player detailing a successful block
			CAutoMessage cMsg;
			LTRESULT ltResult;

			cMsg.Writeuint8( MID_PROJECTILE );

			// write the projectile message subtype
			cMsg.Writeuint8( MPROJ_BLOCKED );

			// write the defense type
			cMsg.Writeuint8( m_cDefendType );

			// write the defense percentage
			cMsg.Writefloat( GetDefensePercentage( &damage.vDir ) );

			ltResult = g_pLTServer->SendToClient( cMsg.Read(), m_hClient, MESSAGE_GUARANTEED );
			ASSERT( LT_OK == ltResult );
		}
	}

	CPlayerObj::HandleDamage( damage );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::OnTrigger
//
//	PURPOSE:	Process a trigger message
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Acquire_Subroutine(TRIGGER_ACQUIRE_SUBROUTINE);
	static CParsedMsg::CToken s_cTok_Acquire_Additive(TRIGGER_ACQUIRE_ADDITIVE);
	static CParsedMsg::CToken s_cTok_Acquire_Procedural(TRIGGER_ACQUIRE_PROCEDURAL);
	static CParsedMsg::CToken s_cTok_Acquire_Primitive(TRIGGER_ACQUIRE_PRIMITIVE);
	static CParsedMsg::CToken s_cTok_Add_Build_Points(TRIGGER_ADD_BUILD_POINTS);
	static CParsedMsg::CToken s_cTok_Start_LightCycle(TRIGGER_START_LIGHTCYCLE);
	static CParsedMsg::CToken s_cTok_End_LightCycle(TRIGGER_END_LIGHTCYCLE);

	if(cMsg.GetArg(0) == s_cTok_Acquire_Subroutine)
	{
		char const* pSub = cMsg.GetArg(1);
		char const* pState = NULL;
		char const* pCondition = NULL;

		// Check for a 3rd and/or 4th parameter
		if(cMsg.GetArgCount() > 2)
		{
			// Check for a 3rd param
			if(CheckSubState(cMsg.GetArg(2)))
			{
				pState = cMsg.GetArg(2);
			}
			else if(CheckSubCondition(cMsg.GetArg(2)))
			{
				pCondition = cMsg.GetArg(2);
			}
			else
			{
				g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with bad 2nd param = (%s). Bad level designer. BAD!!!\n",cMsg.GetArg(2));
				ASSERT(FALSE);
				return false;
			}
			
			// Check for a 4th param
			if(cMsg.GetArgCount() > 3)
			{
				if(CheckSubState(cMsg.GetArg(3)))
				{
					pState = cMsg.GetArg(3);
				}
				else if(CheckSubCondition(cMsg.GetArg(3)))
				{
					pCondition = cMsg.GetArg(3);
				}
				else
				{
					g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with bad 3rd param = (%s). Bad level designer. BAD!!!\n",cMsg.GetArg(3));
					ASSERT(FALSE);
					return false;
				}
			}
		}

		return AcquireSubroutine(pSub,pState,pCondition);
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Additive)
	{
		return AcquireAdditive(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Procedural)
	{
		return AcquireProcedural(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Acquire_Primitive)
	{
		return AcquirePrimitive(cMsg.GetArg(1));
	}
	else if(cMsg.GetArg(0) == s_cTok_Add_Build_Points)
	{
		if(cMsg.GetArgCount() > 1)
		{
			int nBuildPoints = atoi(cMsg.GetArg(1));
			m_nBuildPoints += nBuildPoints;
#ifdef _DEBUG
			g_pLTServer->CPrint("%d Build Points added to player. Total is now %d\n",nBuildPoints,m_nBuildPoints);
#endif
		}

		return true;
	}
	else if(cMsg.GetArg(0) == s_cTok_Start_LightCycle)
	{
		StartLightCycle();
	}
	else if(cMsg.GetArg(0) == s_cTok_End_LightCycle)
	{
		EndLightCycle();
	}
	
	// Base-class it
	return(CPlayerObj::OnTrigger(hSender, cMsg));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireSubroutine
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquireSubroutine(char const* pName, char const* pState, char const* pCondition)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SUBROUTINE_OBTAINED);
	cMsg.WriteString(pName);
	
	if(pState)
	{
		// Sanity check
		if(CheckSubState(pState))
		{
			cMsg.WriteString(pState);
		}
		else
		{
			// We have a bad subroutine state!
			// Bad level deisgner... BAD!!!
			g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with alpha/beta/gold state = (%s). Bad level designer. BAD!!!\n",pState);
			ASSERT(FALSE);
			return false;
		}
	}
	else
	{
		cMsg.WriteString("ALPHA");
	}
	
	if(pCondition)
	{
		// Sanity check
		if(CheckSubCondition(pCondition))
		{
			cMsg.WriteString(pCondition);
		}
		else
		{
			// We have a bad subroutine condition!
			// Bad level deisgner... BAD!!!
			g_pLTServer->CPrint("ERROR - Could not give subroutine to the player with condition = (%s). Bad level designer. BAD!!!\n",pCondition);
			ASSERT(FALSE);
			return false;
		}
	}
	else
	{
		cMsg.WriteString("NORMAL");
	}
	
	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireAdditive
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquireAdditive(char const* pName)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_ADDITIVE_OBTAINED);
	cMsg.WriteString(pName);
	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquireProcedural
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquireProcedural(char const* pName)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PROCEDURAL_OBTAINED);
	cMsg.WriteString(pName);
	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::AcquirePrimitive
//
//	PURPOSE:	A message came telling us to acquire this
//
// --------------------------------------------------------------------------- //
bool CTronPlayerObj::AcquirePrimitive(char const* pName)
{
	// Send a message to the client
	CAutoMessage cMsg;
	cMsg.WriteString(pName);
	g_pLTServer->SendToClient(cMsg, MID_PRIMITIVE_OBTAINED, GetClient(), MESSAGE_GUARANTEED);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CheckSubState
//
//	PURPOSE:	Verifies whether or not a subroutine state is valid
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::CheckSubState(char const* pState)
{
	for(int i=0;i<g_nNumSubStates;i++)
	{
		if(stricmp(pState, g_SubStates[i]) == 0)
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::CheckSubCondition
//
//	PURPOSE:	Verifies whether or not a subroutine condition is valid
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::CheckSubCondition(char const* pCondition)
{
	for(int i=0;i<g_nNumSubConditions;i++)
	{
		if(stricmp(pCondition, g_SubConditions[i]) == 0)
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::ResetDefend
//
//	PURPOSE:	Reset the variables related to defense
//
// ----------------------------------------------------------------------- //

void CTronPlayerObj::ResetDefend()
{
	m_nDefendClientTimeStarted = 0;
	m_nDefendServerTimeStarted = 0;
	m_nDefendDuration = 0;
	m_cDefendType = TRONPLAYEROBJ_NO_DEFEND;
	m_cDefendAmmoId = 0;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::StartLightCycle
//
//	PURPOSE:	Player gets on a light cycle
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::StartLightCycle()
{
	// Temporarily disabled
	//SetPhysicsModel(PPM_LIGHTCYCLE);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerObj::EndLightCycle
//
//	PURPOSE:	Player gets off a light cycle
//
// ----------------------------------------------------------------------- //
bool CTronPlayerObj::EndLightCycle()
{
	// Go to normal physics model.
	SetPhysicsModel(PPM_NORMAL);
	return true;
}

void CTronPlayerObj::SetVehiclePhysicsModel(PlayerPhysicsModel eModel)
{
	if(eModel == PPM_LIGHTCYCLE)
	{
		// Add ourselves to the light cycle manager and start it up
		g_pLightCycleMgr->AddCyclist(m_hObject);
	}

	CPlayerObj::SetVehiclePhysicsModel(eModel);
}

void CTronPlayerObj::SetNormalPhysicsModel()
{
	if(m_ePPhysicsModel == PPM_LIGHTCYCLE)
	{
		// Remove ourselves from the light cycle manager
		g_pLightCycleMgr->RemoveCyclist(m_hObject);
	}

	CPlayerObj::SetNormalPhysicsModel();
}