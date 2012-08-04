// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIBrain.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "ProjectileTypes.h"
#include "AIVolume.h"
#include "AINodeMgr.h"
#include "AIPathMgr.h"
#include "AICentralKnowledgeMgr.h"
#include "AIVolumeMgr.h"
#include "CharacterMgr.h"
#include "AnimatorPlayer.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "AnimationMgr.h"

DEFINE_AI_FACTORY_CLASS(CAIBrain);


CAIBrain::CAIBrain()
{
	m_szName[0] = 0;
	m_pBrain = LTNULL;
	m_pAIHuman = LTNULL;

	m_fRangeStatusCheckTime = 0.0f;
	m_eRangeStatusLast = eRangeStatusOk;

	m_fRandomRangedVariation = 0.f;

	m_eDodgeStatusLast = eDodgeStatusOk;
	m_hstrDodgeProjectileName = LTNULL;
}

CAIBrain::~CAIBrain()
{
	FREE_HSTRING(m_hstrDodgeProjectileName);
}

void CAIBrain::Init(CAIHuman* pAIHuman, const char* szName)
{
	m_pAIHuman = pAIHuman;

	strcpy(m_szName, szName);
	int32 iBrain = g_pAIButeMgr->GetBrainIDByName(szName);
	if ( -1 != iBrain )
	{
		m_pBrain = g_pAIButeMgr->GetBrain(iBrain);
	}
	else
	{
		g_pLTServer->CPrint("DANGER!!!! AI had no brain, but settled for default!!!!");
		m_pBrain = g_pAIButeMgr->GetBrain(0);
	}

	if ( !m_pBrain )
	{
		g_pLTServer->CPrint("DANGER!!!! AI has no brain!!!!");
		return;
	}

	m_eDodgeStatusLast = eDodgeStatusOk;
	m_fDodgeStatusCheckTimeVector = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeVectorCheckTime);
	m_fDodgeStatusCheckTimeProjectile = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeProjectileCheckTime);

	m_fGrenadeStatusCheckTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fAttackGrenadeThrowTime);

	m_fRandomRangedVariation = GetRandom( 0.f, 255.f );
}

void CAIBrain::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eRangeStatusLast, RangeStatus);
	LOAD_TIME(m_fRangeStatusCheckTime);

	LOAD_DWORD_CAST(m_eDodgeStatusLast, DodgeStatus);
	LOAD_TIME(m_fDodgeStatusCheckTimeVector);
	LOAD_TIME(m_fDodgeStatusCheckTimeProjectile);
	LOAD_HSTRING(m_hstrDodgeProjectileName);
	LOAD_VECTOR(m_vDodgeProjectilePosition);
	LOAD_TIME(m_fGrenadeStatusCheckTime);
	LOAD_FLOAT(m_fRandomRangedVariation);
}

void CAIBrain::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_eRangeStatusLast);
	SAVE_TIME(m_fRangeStatusCheckTime);

	SAVE_DWORD(m_eDodgeStatusLast);
	SAVE_TIME(m_fDodgeStatusCheckTimeVector);
	SAVE_TIME(m_fDodgeStatusCheckTimeProjectile);
	SAVE_HSTRING(m_hstrDodgeProjectileName);
	SAVE_VECTOR(m_vDodgeProjectilePosition);
	SAVE_TIME(m_fGrenadeStatusCheckTime);
	SAVE_FLOAT(m_fRandomRangedVariation);
}

void CAIBrain::DodgeDelay()
{
	m_fDodgeStatusCheckTimeVector = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeVectorDelayTime);
	m_fDodgeStatusCheckTimeProjectile = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeProjectileDelayTime);
}

void CAIBrain::GetDodgeStatus(DodgeStatus* peDodgeStatus, Direction* peDirection, DodgeAction* peDodgeAction, HOBJECT* phNode, LTBOOL bForceDodge)
{
	if ( !GetAI()->HasTarget() || !GetAI()->HasLastVolume() || GetAI()->GetAnimationContext()->IsLocked() ) 
	{
		*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
		return;
	}

	//
	// Dodge vector weapons.
	//
	
	// Is it time to dodge vector weapons?

	if ( bForceDodge || ( g_pLTServer->GetTime() >= m_fDodgeStatusCheckTimeVector ) )
	{
		// Randomly decide to dodge.

		m_fDodgeStatusCheckTimeVector = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeVectorCheckTime);

		if ( bForceDodge || ( GetRandom(0.0f, 1.0f) <= RAISE_BY_DIFFICULTY(m_pBrain->fDodgeVectorCheckChance) ) )
		{
			// Dodge if we are being aimed at.

			if ( bForceDodge || TargetIsAimingAtMe() )
			{
				// Randomly choose dodge action (roll, shuffle, block).

				LTFLOAT fCheckDistance;
				LTFLOAT fRandom = GetRandom(0.0f, 1.0f);

				if ( fRandom > m_pBrain->fDodgeVectorRollChance + m_pBrain->fDodgeVectorBlockChance )
				{
					*peDodgeAction = eDodgeActionShuffle;
					fCheckDistance = m_pBrain->fDodgeVectorShuffleDist;
				}
				else if( fRandom > m_pBrain->fDodgeVectorBlockChance )
				{
					*peDodgeAction = eDodgeActionRoll;
					fCheckDistance = m_pBrain->fDodgeVectorRollDist;
				}

				// Forced dodges are intended to move an AI out of the way,
				// so do not use a block as a forced dodge.

				else if( !bForceDodge )
				{
					// Blocks do not move the AI, so return without further checks.

					*peDodgeAction = eDodgeActionBlock;
					*peDirection = eDirectionRight;
					*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusVector;
					return;
				}

				// Do not roll if there are too many crouchers, and roll will
				// result in a crouch.  Only crouch if allowed to by brain and weapon type.

				LTBOOL bRollOK = LTTRUE;
				if( ( GetAttackPoseCrouchChance() > 0.f ) && 
					GetAI()->HasTarget() &&
					g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_AttackCroucher, GetAI(), g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject()) ) )
				{
					bRollOK = LTFALSE;
				}

				// MAKE SURE WE WON'T DODGE OUT OF THE VOLUME
				
				// Randomize left or right check first.
				fRandom = GetRandom(0.0f, 1.0f);
				*peDirection = ( fRandom < 0.5f ) ? eDirectionRight : eDirectionLeft;
				*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusVector;

				LTVector vPos = GetAI()->GetPosition();
				LTFLOAT fRadius = GetAI()->GetRadius();

				uint32 dwExcludeVolumes =	AIVolume::kVolumeType_Ladder | 
											AIVolume::kVolumeType_Stairs |
											AIVolume::kVolumeType_JumpOver | 
											AIVolume::kVolumeType_JumpUp | 
											AIVolume::kVolumeType_AmbientLife |
											AIVolume::kVolumeType_Teleport;

				for( uint8 iActions = 0; iActions < 2; ++iActions )
				{
					if( bRollOK || ( *peDodgeAction != eDodgeActionRoll ) )
					{
						LTVector vOffset = GetAI()->GetTorsoRight() * ( fCheckDistance + fRadius );

						// Check if dodge stays in volume in the desired direction.

						for( uint8 iDirections = 0; iDirections < 2; ++iDirections )
						{
							switch( *peDirection )
							{
								case eDirectionRight:
									if( g_pAIVolumeMgr->StraightPathExists( GetAI(), vPos, vPos + vOffset, GetAI()->GetVerticalThreshold(), dwExcludeVolumes, GetAI()->GetLastVolume() ) )
									{
										if( ( bForceDodge ) || ( !g_pCharacterMgr->RayIntersectAI( vPos, vPos + vOffset, GetAI(), LTNULL, LTNULL ) ) )
										{
											return;
										}
									}
									break;

								case eDirectionLeft:
									if( g_pAIVolumeMgr->StraightPathExists( GetAI(), vPos, vPos - vOffset, GetAI()->GetVerticalThreshold(), dwExcludeVolumes, GetAI()->GetLastVolume() ) )
									{
										if( ( bForceDodge ) || ( !g_pCharacterMgr->RayIntersectAI( vPos, vPos - vOffset, GetAI(), LTNULL, LTNULL ) ) )
										{
											return;
										}
									}
									break;
							}

							// Swap direction and try again.

							*peDirection = ( *peDirection == eDirectionRight ) ? eDirectionLeft : eDirectionRight;
						}
					}

					// If Shuffles fail, try rolls and vice-versa.

					if( ( *peDodgeAction == eDodgeActionShuffle ) && ( m_pBrain->fDodgeVectorRollChance > 0.f ) )
					{
						*peDodgeAction = eDodgeActionRoll;
						fCheckDistance = m_pBrain->fDodgeVectorRollDist;
					}
					else if ( m_pBrain->fDodgeVectorShuffleChance > 0.f ) 
					{
						*peDodgeAction = eDodgeActionShuffle;
						fCheckDistance = m_pBrain->fDodgeVectorShuffleDist;
					}

					// No more options.

					else {
						*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
						return;
					}
				}

				// Out of volume for rolls and shuffles for both directions.

				*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
				return;
			}
		}
	}

	//
	// Dodge projectiles.
	//

	if ( g_pLTServer->GetTime() >= m_fDodgeStatusCheckTimeProjectile )
	{
		m_fDodgeStatusCheckTimeProjectile = g_pLTServer->GetTime() + RAISE_BY_DIFFICULTY(m_pBrain->fDodgeProjectileCheckTime);

		if ( GetRandom(0.0f, 1.0f) <= RAISE_BY_DIFFICULTY(m_pBrain->fDodgeProjectileCheckChance) )
		{
			CGrenade* pGrenade;
			if ( FindGrenadeDangerPosition(GetAI()->GetPosition(), 40000.0f, &m_vDodgeProjectilePosition, &pGrenade) )
			{
				FREE_HSTRING(m_hstrDodgeProjectileName);
				// $STRING
				char aName[256];
				g_pLTServer->GetObjectName(pGrenade->m_hObject, aName, sizeof(aName));
				m_hstrDodgeProjectileName = g_pLTServer->CreateString(aName);

				*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusProjectile;
				*peDodgeAction = eDodgeActionFlee;

				return;
			}
		}
	}

	*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
	return;
}

LTBOOL CAIBrain::TargetIsAimingAtMe()
{
	if ( GetAI()->HasTarget() && GetAI()->GetTarget()->IsVisiblePartially() )
	{
		CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
		return CharacterIsAimingAtMe( pCharacter );
	}

	return LTFALSE;
}

LTBOOL CAIBrain::CharacterIsAimingAtMe(CCharacter* pCharacter)
{
	if ( pCharacter->HasDangerousWeapon() )
	{
		LTRotation rRot;
		LTVector vNull, vForward;
		g_pLTServer->GetObjectRotation( pCharacter->m_hObject, &rRot);
		vForward = rRot.Forward();

		LTVector vPos;
		g_pLTServer->GetObjectPos( pCharacter->m_hObject, &vPos);

		LTVector vDir;
		vDir = GetAI()->GetPosition() - vPos;
		vDir.y = 0;
		vDir.Normalize();

		// TODO: bute this

		const static LTFLOAT fThreshhold = 0.95f;

		if ( (vDir.Dot(vForward) > fThreshhold) && (GetAI()->GetTorsoForward().Dot(vForward) < -fThreshhold) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

GrenadeStatus CAIBrain::GetGrenadeStatus()
{
	if ( g_pLTServer->GetTime() >= m_fGrenadeStatusCheckTime )
	{
		m_fGrenadeStatusCheckTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fAttackGrenadeThrowTime);

		if ( GetRandom(0.0f, 1.0f) <= RAISE_BY_DIFFICULTY(m_pBrain->fAttackGrenadeThrowChance) )
		{
			LTFLOAT fRangeMinSqr = m_pBrain->fAttackThrownRangeMin*m_pBrain->fAttackThrownRangeMin;
			LTFLOAT fRangeMaxSqr = m_pBrain->fAttackThrownRangeMax*m_pBrain->fAttackThrownRangeMax;
			LTFLOAT fDistanceSqr = GetAI()->GetPosition().DistSqr(GetAI()->GetTarget()->GetVisiblePosition());
			if ( (fDistanceSqr > fRangeMinSqr) && (fDistanceSqr < fRangeMaxSqr) )
			{
				return eGrenadeStatusThrow;
			}
		}
	}
	
	return eGrenadeStatusDontThrow;
}

RangeStatus CAIBrain::GetRangeStatus()
{
	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( GetAI()->HasTarget() && pWeapon)
	{
		WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon( pWeapon->GetId() );
		if (pWeaponData)
		{
			return GetRangeStatus( (EnumAIWeaponType)pWeaponData->nAIWeaponType );	
		}
	}

	return eRangeStatusOk;
}

RangeStatus CAIBrain::GetRangeStatus(EnumAIWeaponType eWeaponType)
{
	return GetRangeStatus( eWeaponType, GetAI()->GetPosition() );
}

RangeStatus CAIBrain::GetRangeStatus(EnumAIWeaponType eWeaponType, const LTVector& vSourcePos )
{
	LTFLOAT fRangeMinSqr = 0.f;
	LTFLOAT fRangeMaxSqr = 0.f;

	switch ( eWeaponType )
	{
		case kAIWeap_Ranged:
			fRangeMinSqr = m_pBrain->fAttackRangedRangeMin*m_pBrain->fAttackRangedRangeMin;
			fRangeMaxSqr = (m_pBrain->fAttackRangedRangeMax + m_fRandomRangedVariation)*(m_pBrain->fAttackRangedRangeMax + m_fRandomRangedVariation);
			break;

		case kAIWeap_Melee:
			fRangeMinSqr = m_pBrain->fAttackMeleeRangeMin*m_pBrain->fAttackMeleeRangeMin;
			fRangeMaxSqr = m_pBrain->fAttackMeleeRangeMax*m_pBrain->fAttackMeleeRangeMax;
			break;

		case kAIWeap_Thrown:
			fRangeMinSqr = m_pBrain->fAttackThrownRangeMin*m_pBrain->fAttackThrownRangeMin;
			fRangeMaxSqr = m_pBrain->fAttackThrownRangeMax*m_pBrain->fAttackThrownRangeMax;
			break;
	}

	LTFLOAT fDistanceSqr = vSourcePos.DistSqr(GetAI()->GetTarget()->GetVisiblePosition());

	RangeStatus eRangeStatus;

	if ( fDistanceSqr < fRangeMinSqr )
	{
		return eRangeStatus = eRangeStatusTooClose;
	}
	else if ( fDistanceSqr > fRangeMaxSqr )
	{
		return eRangeStatus = eRangeStatusTooFar;
	}
	else
	{
		return eRangeStatus = eRangeStatusOk;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIBrain::IsAmmoGenerationRestricted()
//              
//	PURPOSE:	Returns true if the passed in Ammo IDs ammo ammount is
//				restricted, false if it is not.
//              
//----------------------------------------------------------------------------
uint32 CAIBrain::IsAmmoGenerationRestricted( uint32 nAmmoID ) const 
{
	UBER_ASSERT( m_pBrain->AmmoGenerationRestrictionMask.size() >= 
		static_cast<unsigned int>(g_pWeaponMgr->GetNumAmmoIds()), 
		"Out of range value on ammo generation restriction test" );

	return m_pBrain->AmmoGenerationRestrictionMask.test( nAmmoID );
}


LTFLOAT CAIBrain::GetAIData(EnumAIDataType eAIDataType)
{
	AIDATA_MAP::iterator it = m_pBrain->mapAIData.find( eAIDataType );
	if( it != m_pBrain->mapAIData.end() )
	{
		return it->second;
	}

	AIASSERT1( 0, m_pAIHuman->m_hObject, "CAIBrain::GetAIData: Could not find AIDataType in map: %s", s_aszAIDataTypes[eAIDataType] );
	return 0.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIBrain::GetAIDataExist()
//              
//	PURPOSE:	Support checking for the existance of a particular DataType
//				enumeration for a given brain.
//              
//----------------------------------------------------------------------------
LTBOOL CAIBrain::GetAIDataExist(EnumAIDataType eAIDataType)
{
	AIDATA_MAP::iterator it = m_pBrain->mapAIData.find( eAIDataType );
	if( it != m_pBrain->mapAIData.end() )
	{
		return LTTRUE;
	}

	return LTFALSE;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIBrain::Get*()
//              
//	PURPOSE:	Simple accessor functions.
//
//	NOTE:		Accessor functions pulled out of the header to remove header 
//				dependancy on macros
//              
//----------------------------------------------------------------------------
LTFLOAT	CAIBrain::GetAttackFromCoverCoverTime() const
{
	return LOWER_BY_DIFFICULTY(m_pBrain->fAttackFromCoverCoverTime);
}
LTFLOAT	CAIBrain::GetAttackFromCoverCoverTimeRandom() const
{
	return LOWER_BY_DIFFICULTY(m_pBrain->fAttackFromCoverCoverTimeRandom);
}
LTFLOAT	CAIBrain::GetAttackFromCoverUncoverTime() const
{
	return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromCoverUncoverTime);
}
LTFLOAT	CAIBrain::GetAttackFromCoverUncoverTimeRandom() const
{
	return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromCoverUncoverTimeRandom);
}
LTFLOAT	CAIBrain::GetAttackFromCoverReactionTime() const
{
	return LOWER_BY_DIFFICULTY(m_pBrain->fAttackFromCoverReactionTime);
}
LTFLOAT	CAIBrain::GetAttackFromVantageAttackTime() const
{
	return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromVantageAttackTime);
}
LTFLOAT	CAIBrain::GetAttackFromVantageAttackTimeRandom() const
{
	return RAISE_BY_DIFFICULTY(m_pBrain->fAttackFromVantageAttackTimeRandom);
}
