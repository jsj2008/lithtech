// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIBrain.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "ProjectileTypes.h"
#include "AIVolume.h"
#include "AINodeMgr.h"
#include "AIPathMgr.h"

IMPLEMENT_FACTORY(CAIBrain, 0)

void CAIBrain::Constructor()
{
	m_szName[0] = 0;
	m_pBrain = LTNULL;
	m_pAIHuman = LTNULL;

	m_fRangeStatusCheckTime = 0.0f;
	m_eRangeStatusLast = eRangeStatusOk;

	m_eDodgeStatusLast = eDodgeStatusOk;
	m_hstrDodgeProjectileName = LTNULL;
}

void CAIBrain::Destructor()
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
}

void CAIBrain::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eRangeStatusLast, RangeStatus);
	LOAD_FLOAT(m_fRangeStatusCheckTime);

	LOAD_DWORD_CAST(m_eDodgeStatusLast, DodgeStatus);
	LOAD_FLOAT(m_fDodgeStatusCheckTimeVector);
	LOAD_FLOAT(m_fDodgeStatusCheckTimeProjectile);
	LOAD_HSTRING(m_hstrDodgeProjectileName);
	LOAD_VECTOR(m_vDodgeProjectilePosition);
	LOAD_FLOAT(m_fGrenadeStatusCheckTime);
}

void CAIBrain::Save(HMESSAGEWRITE hWrite)
{
	SAVE_DWORD(m_eRangeStatusLast);
	SAVE_FLOAT(m_fRangeStatusCheckTime);

	SAVE_DWORD(m_eDodgeStatusLast);
	SAVE_FLOAT(m_fDodgeStatusCheckTimeVector);
	SAVE_FLOAT(m_fDodgeStatusCheckTimeProjectile);
	SAVE_HSTRING(m_hstrDodgeProjectileName);
	SAVE_VECTOR(m_vDodgeProjectilePosition);
	SAVE_FLOAT(m_fGrenadeStatusCheckTime);
}

void CAIBrain::DodgeDelay()
{
	m_fDodgeStatusCheckTimeVector = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeVectorDelayTime);
	m_fDodgeStatusCheckTimeProjectile = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeProjectileDelayTime);
}

void CAIBrain::GetDodgeStatus(DodgeStatus* peDodgeStatus, Direction* peDirection, DodgeAction* peDodgeAction, uint32* pdwNode)
{
	if ( !GetAI()->HasTarget() || !GetAI()->HasLastVolume() ) 
	{
		*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
		return;
	}

	if ( g_pLTServer->GetTime() >= m_fDodgeStatusCheckTimeVector )
	{
		m_fDodgeStatusCheckTimeVector = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fDodgeVectorCheckTime);

		if ( GetRandom(0.0f, 1.0f) <= RAISE_BY_DIFFICULTY(m_pBrain->fDodgeVectorCheckChance) )
		{
			if ( GetAI()->GetTarget()->IsVisiblePartially() )
			{
				CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(GetAI()->GetTarget()->GetObject());
				if ( pCharacter->HasDangerousWeapon() )
				{
					LTRotation rRot;
					LTVector vNull, vForward;
					g_pLTServer->GetObjectRotation(GetAI()->GetTarget()->GetObject(), &rRot);
					g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

					LTVector vDir;
					vDir = GetAI()->GetPosition() - GetAI()->GetTarget()->GetPosition();
					vDir.y = 0;
					vDir.Norm();

					// TODO: bute this

					const static LTFLOAT fThreshhold = 0.95f;

					if ( (vDir.Dot(vForward) > fThreshhold) && (GetAI()->GetForwardVector().Dot(vForward) < -fThreshhold) )
					{
						LTFLOAT fCheckDistance;

						LTFLOAT fRandom = GetRandom(0.0f, 1.0f);

						if ( fRandom > m_pBrain->fDodgeVectorCoverChance )
						{
							if ( fRandom > (m_pBrain->fDodgeVectorCoverChance + m_pBrain->fDodgeVectorRollChance) )
							{
								*peDodgeAction = eDodgeActionShuffle;
								fCheckDistance = 109.0f;
							}
							else
							{
								*peDodgeAction = eDodgeActionRoll;
								fCheckDistance = 140.0f;
							}

							// MAKE SURE WE WON'T DODGE OUT OF THE VOLUME 
							if ( GetAI()->GetLastVolume()->Inside2d(GetAI()->GetPosition()+GetAI()->GetRightVector()*fCheckDistance, GetAI()->GetRadius()) )
							{
								*peDirection = eDirectionRight;
								*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusVector;
								return;
							}
							else if ( GetAI()->GetLastVolume()->Inside2d(GetAI()->GetPosition()-GetAI()->GetRightVector()*fCheckDistance, GetAI()->GetRadius()) )
							{
								*peDirection = eDirectionLeft;
								*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusVector;
								return;
							}
							else
							{
								*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
								return;
							}
						}
						else
						{
							CAINode* pNode = g_pAINodeMgr->FindNearestCoverFromThreat(GetAI()->GetPosition(), GetAI()->GetTarget()->GetObject());

							if ( pNode )
							{
								*peDodgeAction = eDodgeActionCover;
								*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusVector;
								*pdwNode = pNode->GetID();
							}
							else
							{
								*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
							}
							return;
						}
					}
				}
			}
		}
	}

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
				m_hstrDodgeProjectileName = g_pLTServer->CreateString(g_pLTServer->GetObjectName(pGrenade->m_hObject));

				*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusProjectile;
				*peDodgeAction = eDodgeActionFlee;

				return;
			}
		}
	}

	*peDodgeStatus = m_eDodgeStatusLast = eDodgeStatusOk;
	return;
}

GrenadeStatus CAIBrain::GetGrenadeStatus()
{
	if ( g_pLTServer->GetTime() >= m_fGrenadeStatusCheckTime )
	{
		m_fGrenadeStatusCheckTime = g_pLTServer->GetTime() + LOWER_BY_DIFFICULTY(m_pBrain->fAttackGrenadeThrowTime);

		if ( GetRandom(0.0f, 1.0f) <= RAISE_BY_DIFFICULTY(m_pBrain->fAttackGrenadeThrowChance) )
		{
			LTFLOAT fRangeMinSqr = m_pBrain->fAttackProjectileRangeMin*m_pBrain->fAttackProjectileRangeMin;
			LTFLOAT fRangeMaxSqr = m_pBrain->fAttackProjectileRangeMax*m_pBrain->fAttackProjectileRangeMax;
			LTFLOAT fDistanceSqr = GetAI()->GetPosition().DistSqr(GetAI()->GetTarget()->GetPosition());
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
	if ( !GetAI()->HasTarget() || !pWeapon || pWeapon->GetAmmoData()->eType != VECTOR)
	{
		return eRangeStatusOk;
	}

	LTFLOAT fRangeMinSqr = m_pBrain->fAttackVectorRangeMin*m_pBrain->fAttackVectorRangeMin;
	LTFLOAT fRangeMaxSqr = m_pBrain->fAttackVectorRangeMax*m_pBrain->fAttackVectorRangeMax;
	LTFLOAT fDistanceSqr = GetAI()->GetPosition().DistSqr(GetAI()->GetTarget()->GetPosition());

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

	LTBOOL bCanReport = LTFALSE;

	if ( g_pLTServer->GetTime() > m_fRangeStatusCheckTime )
	{
		// It's time for a fresh report

		m_fRangeStatusCheckTime = g_pLTServer->GetTime() + 3.0f;
		bCanReport = LTTRUE;
	}

	if ( eRangeStatusTooClose == eRangeStatus )
	{
		// We always report too close

		m_eRangeStatusLast = eRangeStatus;
		return eRangeStatus;
	}

	if ( bCanReport )
	{
		// If it's time to report, report the latest status

		m_eRangeStatusLast = eRangeStatus;
		return eRangeStatus;
	}
	else
	{
		// Otherwise, keep it the same

		return m_eRangeStatusLast;
	}
}
