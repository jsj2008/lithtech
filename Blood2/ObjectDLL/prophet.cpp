
// ----------------------------------------------------------------------- //
//
// MODULE  : Prophet.cpp
//
// PURPOSE : Prophet - Definition
//
// CREATED : 11/11.97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "Prophet.h"
#include "cpp_server_de.h"

BEGIN_CLASS(Prophet)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIWeapon1, "BERETTA")   \
	ADD_STRINGPROP(AIWeapon2, "COMBAT")   \
	ADD_STRINGPROP(AIWeapon3, "ASSAULTRIFLE")   \
	ADD_STRINGPROP(AIWeapon4, "COMBAT")   \
	ADD_STRINGPROP(AIWeapon5, "ASSAULTRIFLE")   \
    ADD_REALPROP(Bullets, 750.0f)     \
    ADD_REALPROP(BMG, 150.0f)     \
    ADD_REALPROP(Shells, 150.0f)     \
	ADD_REALPROP(Grenades, 50.0f)	\
    ADD_REALPROP(Rockets, 100.0f)     \
    ADD_REALPROP(Flares, 100.0f)     \
    ADD_REALPROP(Cells, 600.0f)     \
    ADD_REALPROP(Charges, 400.0f)     \
    ADD_REALPROP(Fuel, 200.0f)     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 19.0f, 43.0f, 15.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Prophet, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		Prophet::m_bLoadAnims = DTRUE;
CAnim_Sound	Prophet::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Prophet::Prophet() : AI_Mgr()
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
	m_dwFlags = FLAG_CRAWL | FLAG_JUMP | FLAG_LIMP | FLAG_DODGE;

	m_bCabal = DTRUE;

	// [blg]
	m_fAIHitPoints   = 200;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 100;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"BERETTA");
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"COMBAT");
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"ASSAULTRIFLE");
	_mbscpy((unsigned char*)m_szAIWeapon[3], (const unsigned char*)"COMBAT");
	_mbscpy((unsigned char*)m_szAIWeapon[4], (const unsigned char*)"ASSAULTRIFLE");

	m_fAIBullets  = 750;
	m_fAIShells   = 150;
	m_fAIGrenades = 50;
	m_fAIRockets  = 100;
	m_fAIFlares   = 100;
	m_fAICells    = 600;
	m_fAICharges  = 400;
	m_fAIFuel	  = 200;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Prophet::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			if(fData != PRECREATE_SAVEGAME)
				PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			AI_Mgr::EngineMessageFn(messageID, pData, fData);

			m_InventoryMgr.ObtainWeapon(WEAP_MELEE);

			CacheFiles();

			return 0;
		}

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prophet::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\Prophet.abc";
	char* pSkin = "Skins\\Enemies\\Prophet.dtx";
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL Prophet::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\Prophet");
		m_bLoadAnims = DFALSE;
	}

	m_InventoryMgr.AddDamageMultiplier(0.5f);	
    
	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Prophet::Fire
// DESCRIPTION	: Fire current weapon forward
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

DBOOL Prophet::Fire(DBOOL bAltFire)
{
	DBOOL bFire = DFALSE;
	CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();
	
	if(pW == DNULL)
	{
		SetNewState(STATE_Idle);
		return DFALSE;
	}

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
// ROUTINE		: Prophet::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Prophet::MC_Fire_Stand()
{
	CWeapon* pW = m_InventoryMgr.GetCurrentWeapon();

	if(pW == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	if(pW->GetType() == WEAP_MELEE)
	{
		if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
		{
			DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[m_pServerDE->IntRandom(4,6)]);

			//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
			if(!bRet)
			{
				SetNewState(STATE_CrawlUnderObj);
				return;
			}

			m_pServerDE->SetModelLooping(m_hObject, DFALSE);
    
			m_bAnimating = DTRUE; 
			m_nCurMetacmd = MC_FIRE_STAND;
			m_pServerDE->SetNextUpdate(m_hObject, 0.1f);
		}
		else
		{
			if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
			{
				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}
		}      
	}
	else
	{
		AI_Mgr::MC_Fire_Stand();
	}

    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Prophet::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Prophet::ComputeState(int nStimType)
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
			default:						if(m_nLastState == STATE_GuardLocation)
												SetNewState(STATE_GuardLocation);
											else if(fHeight <= m_vDims.y && m_fStimuli[HEALTH] < 0.30f)
												SetNewState(STATE_SearchVisualTarget);
											else
												SetNewState(STATE_Idle);

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
		else if(m_nState == STATE_Escape_RunAway)
		{
			SetNewState(STATE_FindAmmo);
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
// ROUTINE		: Prophet::STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Prophet::AI_STATE_AttackClose()
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

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_BestWeapon();	break;
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.33) || m_nCurMetacmd == MC_FIRE_STAND
						|| m_nCurMetacmd == MC_FIRE_CROUCH)
					{
						if(m_nDodgeFlags & FORWARD || m_nDodgeFlags & BACKWARD || m_fStimuli[HEALTH] < 0.5f 
							|| m_nCurMetacmd == MC_FIRE_CROUCH || (fHeight > m_vDims.y && !bAbove))
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
							&& m_nCurMetacmd != MC_FIRE_CROUCH)
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
// ROUTINE		: Prophet::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Prophet::AI_STATE_AttackFar()
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
	
	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_BestWeapon();	break;
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND
						|| m_nCurMetacmd == MC_FIRE_CROUCH || fHeight > m_vDims.y)
					{
						if((fHeight > m_vDims.y && !bAbove) || m_nCurMetacmd == MC_FIRE_CROUCH
							|| m_fStimuli[HEALTH] < 0.5f)
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
							&& m_nCurMetacmd != MC_FIRE_CROUCH)
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
// ROUTINE		: Prophet::AI_STATE_Escape_Runaway
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Prophet::AI_STATE_Escape_RunAway()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	
					m_InventoryMgr.ChangeWeapon(WEAP_MELEE);

					break;
		case 2:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
			
					if(m_fStimuli[SIGHT] <= 75.0f)
						MC_Fire_Stand();
					else
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y)
						{
							MC_Run();
						}
						else
						{
							MC_Idle();
						}
					}
					
					break;
		case 3:		ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Prophet::AI_STATE_Dodge
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Prophet::AI_STATE_Dodge()
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
						Metacmd++;
					}

					break;
		case 3:		ComputeState();	break;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void Prophet::CacheFiles()
{
	// Sanity checks...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if(!(pServerDE->GetServerFlags() & SS_CACHING))
	{
		return;
	}

	if (!m_hObject) return;


	// Get the model filenames...

	char sModel[256] = { "" };
	char sSkin[256]  = { "" };

	pServerDE->GetModelFilenames(m_hObject, sModel, 255, sSkin, 255);


	// Cache models...

	pServerDE->CacheFile(FT_MODEL, sModel);


	// Cache textures...

	pServerDE->CacheFile(FT_TEXTURE, sSkin);


	// Cache sounds...

	SetCacheDirectory("sounds\\enemies\\prophet");

	CacheSoundFileRange("anger", 1, 3);
	CacheSoundFileRange("death", 1, 3);
	CacheSoundFileRange("idle", 1, 3);
	CacheSoundFileRange("laugh", 1, 3);
	CacheSoundFileRange("onfire", 1, 3);
	CacheSoundFileRange("pain", 1, 3);
	CacheSoundFileRange("spot", 1, 3);
}


