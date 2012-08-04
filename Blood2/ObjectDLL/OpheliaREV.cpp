// ----------------------------------------------------------------------- //
//
// MODULE  : OpheliaREV.cpp
//
// PURPOSE : OpheliaREV - Definition
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "OpheliaREV.h"
#include "cpp_server_de.h"

BEGIN_CLASS(OpheliaREV)
	ADD_STRINGPROP(AIState, "PASSIVE")
	ADD_STRINGPROP(AIWeapon1, "TESLA")   \
	ADD_STRINGPROP(AIWeapon2, "SNIPER")   \
	ADD_VECTORPROP_VAL_FLAG(Dims, 12.0f, 38.0f, 12.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(OpheliaREV, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		OpheliaREV::m_bLoadAnims = DTRUE;
CAnim_Sound	OpheliaREV::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OpheliaREV::OpheliaREV()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

OpheliaREV::OpheliaREV() : AI_Mgr()
{
	m_fHearingDist	= 1500.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1500.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 105.0f;
	m_fJumpSpeed	= 400.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_CRAWL | FLAG_JUMP | FLAG_DODGE;

	m_bCabal = DTRUE;

	// [blg]
	m_fAIHitPoints   = 400;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 400;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"PASSIVE");
	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"TESLA");
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"SNIPER");

	m_fAIBullets  = 500;
	m_fAIBMG	  = 100;
	m_fAIShells   = 150;
	m_fAIGrenades = 50;
	m_fAIRockets  = 100;
	m_fAIFlares   = 100;
	m_fAICells    = 500;
	m_fAICharges  = 500;
	m_fAIFuel	  = 500;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OpheliaREV::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD OpheliaREV::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we set the skin/filename...

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models\\Characters\\ophelia.abc");
				_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins\\Characters\\ophrev.dtx");
			}

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			FirstUpdate();
		}
		break;

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FirstUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL OpheliaREV::FirstUpdate()
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE || !m_hObject) return DFALSE;

	//Load up animation indexes if first model instance

    if (m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
		m_Anim_Sound.SetSoundRoot("sounds\\chosen\\ophelia");

		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::Fire
// DESCRIPTION	: Fire current weapon forward
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL OpheliaREV::Fire(DBOOL bAltFire)
{
	DBOOL bFire = DFALSE;
	CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();
	
	DVector vTargetVel, vVel;
	m_pServerDE->GetVelocity(m_hTarget, &vTargetVel);
	m_pServerDE->GetVelocity(m_hObject, &vVel);

	DFLOAT fTargetVel = VEC_MAG(vTargetVel);
	DFLOAT fVel = VEC_MAG(vVel);

	if(fTargetVel <= 5.0f && fVel <= 5.0f && pW->GetType() != WEAP_ASSAULTRIFLE
		&& pW->GetType() != WEAP_NAPALMCANNON && pW->GetType() != WEAP_SNIPERRIFLE)
	{
		if(m_InventoryMgr.GetAmmoCount(pW->GetAmmoType(DFALSE)) >= pW->GetAltAmmoUse())
			bFire = DTRUE;
	}

//	DDWORD m_nFiredWeapon = m_InventoryMgr.FireCurrentWeapon(&m_MoveObj.GetPos(), &m_MoveObj.GetRotation(), bFire);

	return AI_Mgr::Fire(bFire);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::MC_BestWeapon
// DESCRIPTION	: Choose best weapon for attacking
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::MC_BestWeapon()
{
	CWeapon* pW	= m_InventoryMgr.GetCurrentWeapon();

	if(pW == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

    if (m_bAnimating == DFALSE)
    {
		if(pW)
		{
			if(m_InventoryMgr.GetAmmoCount(pW->GetAmmoType(DFALSE)) <= 0.0)
			{
				SetAnimation( m_pAnim_Sound->m_nAnim_SWITCH_WEAPON[pW->GetFireType()]);

				m_pServerDE->SetModelLooping(m_hObject, DFALSE);

				m_bAnimating = DTRUE;
			}
			else
			{
				m_bAnimating = DFALSE;
				Metacmd++;
			}
		}
	}
	else
	{
		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			if(pW->GetType() == WEAP_TESLACANNON)
			{
				m_InventoryMgr.ChangeWeapon(WEAP_SNIPERRIFLE);
				pW = m_InventoryMgr.GetCurrentWeapon();
				pW->ShowHandModel(DFALSE);
				SetAnimation( m_pAnim_Sound->m_nAnim_SWITCH_WEAPON[pW->GetFireType()]);
			}
			else
			{
				if(m_InventoryMgr.GetAmmoCount(pW->GetAmmoType(DFALSE)) <= 0.0)
				{
					SetNewState(STATE_FindAmmo);
				}
				else
				{
					m_bAnimating = DFALSE;
					Metacmd++;
				}
			}
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);			break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);			break;
			case STATE_SearchSmellTarget:	SetNewState(STATE_Idle);			break;
			case STATE_FindAmmo:			SetNewState(m_nLastState);			break;
			case STATE_FindHealth:			SetNewState(m_nLastState);			break;
			case STATE_Escape_RunAway:		SetNewState(STATE_Idle);
			case STATE_AttackFar:			
			case STATE_AttackClose:			
			default:						SetNewState(STATE_SearchVisualTarget);
											break;
		}
	}
	else
	{
		if(m_fStimuli[HEALTH] < 0.15f)
		{
			SetNewState(STATE_FindHealth);
			return;
		}
		else if(m_fStimuli[HEALTH] < 0.50f)
		{
			SetNewState(STATE_AttackFar);
			return;
		}
		else
		{
			if(m_fStimuli[SIGHT] <= 0.5f || fHeight > m_vDims.y)
				SetNewState(STATE_AttackFar);
			else
				SetNewState(STATE_AttackClose);
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::AI_STATE_AttackClose()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

	CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();

	if(pW == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();		break;
		case 2:		MC_BestWeapon();		break;
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.33) || m_nCurMetacmd == MC_FIRE_STAND
						|| m_nCurMetacmd == MC_FIRE_CROUCH)
					{
						if(m_nDodgeFlags & FORWARD || m_nDodgeFlags & BACKWARD || m_fStimuli[HEALTH] < 0.5f 
							|| m_nCurMetacmd == MC_FIRE_CROUCH || (fHeight > m_vDims.y && !bAbove)
							|| pW->GetType() == WEAP_SNIPERRIFLE)
						{
							MC_Fire_Crouch();
						}
						else
						{
							MC_Fire_Stand();
						}
					}
					else
						Metacmd++;
					
					break;
		case 4:		if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.33) && m_fStimuli[SIGHT] < (m_fSeeingDist * 0.5))
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
							&& m_nCurMetacmd != MC_FIRE_CROUCH && pW->GetType() != WEAP_SNIPERRIFLE)
						{
							if(m_nInjuredLeg)
								MC_Walk();
							else
							{
								if(m_nDodgeFlags & FORWARD || m_nDodgeFlags & BACKWARD
									|| m_fStimuli[HEALTH] < 0.5f)
									MC_Fire_Crawl();
								else
									MC_Fire_Run();
							}
						}
						else
						{
							MC_Fire_Crouch();
						}
					}
					else
						Metacmd++;

					break;
		case 5:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::AI_STATE_AttackFar()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;
	
	CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();

	if(pW == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_BestWeapon();	break;
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND
						|| m_nCurMetacmd == MC_FIRE_CROUCH || fHeight > m_vDims.y)
					{
						if((fHeight > m_vDims.y && !bAbove) || m_nCurMetacmd == MC_FIRE_CROUCH
							|| m_fStimuli[HEALTH] < 0.5f || pW->GetType() == WEAP_SNIPERRIFLE)
						{
							MC_Fire_Crouch();
						}
						else
						{
							MC_Fire_Stand();
						}
					}
					else
						Metacmd++;

					break;
		case 4:		if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.75))
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
							&& m_nCurMetacmd != MC_FIRE_CROUCH && pW->GetType() != WEAP_SNIPERRIFLE)
						{
							if(m_nInjuredLeg)
								MC_Walk();
							else
								MC_Fire_Run();
						}
						else
						{
							MC_Fire_Crouch();
						}
					}
					else
						Metacmd++;

					break;
		case 5:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::AI_STATE_Escape
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::AI_STATE_Escape_RunAway()
{
	if(Metacmd > 1)
	{
		int nStimType = ComputeStimuli();

		if(nStimType > 0)
		{
			ComputeState(nStimType);
			return;
		}
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		m_hTrackObject = FindObjectInRadius(m_pServerDE->GetClass("ExitHint"), m_fSeeingDist, FIND_VISIBLE | FIND_AVOID_TARGET);

					if(m_hTrackObject)
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

						MC_FacePos(m_vTrackObjPos);

						if(!m_MoveObj.CalculatePath(m_vTrackObjPos))
							SetNewState(STATE_Idle);
					}
					else
						SetNewState(STATE_AttackFar);
					
					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_Escape_RunAway);

					break;
		}
		case 3:		
		{
					DVector vPoint = *m_MoveObj.GetNextPathPoint();
					vPoint.y = m_MoveObj.GetPos().y;

					if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
					{
						if(!m_MoveObj.MoveToNextPathPoint())
						{
							MC_FacePos(m_vTargetPos);
						}
						else
						{
							Metacmd = 2;
						}
					}
					else
						MC_Run();

					break;
		}
		case 4:		ComputeState();			break;
	}

	return;}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::AI_STATE_Escape
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::AI_STATE_Escape_Hide()
{
	switch(Metacmd)
	{
		case 1:		m_hTrackObject = FindObjectInRadius(m_pServerDE->GetClass("ExitHint"), m_fSeeingDist);

					if(m_hTrackObject)
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vDestPos);
						SetNewState(STATE_RunToPos);
					}
					else
						SetNewState(STATE_AttackClose);
					
					break;
	}

	return;

}

// ----------------------------------------------------------------------- //
// ROUTINE		: OpheliaREV::AI_STATE_Dodge
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void OpheliaREV::AI_STATE_Dodge()
{
	switch(Metacmd)
	{
		case 1:		m_nDodgeFlags = CalculateDodge(m_vTrackObjPos);
					Metacmd++;		break;
		case 2:		if(m_nDodgeFlags & ROLL)
					{
						if(m_nDodgeFlags & RIGHT)
							MC_Roll_Right();
						else if(m_nDodgeFlags & LEFT)
							MC_Roll_Left();
						else if(m_nDodgeFlags & FORWARD)
							MC_Roll_Forward();
						else
							MC_Roll_Backward();
					}
					else
					{
						if(m_nDodgeFlags & RIGHT)
							MC_Dodge_Right();
						else if(m_nDodgeFlags & LEFT)
							MC_Dodge_Left();
						else
							Metacmd++;
					}

					break;
		case 3:		ComputeState();	break;
	}

	return;
}