// ----------------------------------------------------------------------- //
//
// MODULE  : GremlinAI.cpp
//
// PURPOSE : GremlinAI - Definition
//
// CREATED : 1/29/99
//
// ----------------------------------------------------------------------- //

#ifdef _ADD_ON

#include <stdio.h>
#include "Gremlin.h"
#include "cpp_server_de.h"
#include "smellhint.h"


BEGIN_CLASS(GremlinAI)
    ADD_REALPROP(RandomHitPoints, 10.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 19.0f, 30.0f, 24.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(GremlinAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		GremlinAI::m_bLoadAnims = DTRUE;
CAnim_Sound	GremlinAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GremlinAI::GremlinAI() : AI_Mgr()
{
	m_fHearingDist	= 2000.0f;
	m_fSensingDist	= 300.0f;
	m_fSmellingDist	= 1200.0f;
	m_fSeeingDist	= 700.0f;

	m_fWalkSpeed	= 320.0f;
	m_fRunSpeed		= 640.0f;
	m_fRollSpeed	= 320.0f;
	m_fJumpSpeed	= 640.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"SHIKARI_CLAW" );
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"ROCK" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_JUMP | FLAG_NOAMMOCOLLECT;

	m_bCabal = DFALSE;

	// [blg]
	m_fAIHitPoints   = 65;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD GremlinAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			CacheFiles();
			break;
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

void GremlinAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models_ao\\Enemies_ao\\Gremlin.abc";
	char* pSkin = "Skins_ao\\Enemies_ao\\Gremlin.dtx";
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

DBOOL GremlinAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	m_hObject = m_pServerDE->ObjectToHandle(this);
	if (!m_hObject) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds_ao\\enemies\\Gremlin");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::MC_Jump
// DESCRIPTION	: Run the jump animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::MC_Jump()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_JUMP)
    {
        SetAnimation(m_pAnim_Sound->m_nAnim_JUMP[4]);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel;

		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed * 3;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed * 3;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_JUMP;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		DVector vVel;

		m_pServerDE->GetVelocity(m_hObject,&vVel);
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed * 3;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed * 3;
		Move(vVel, MATH_EPSILON);

		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if (collisionInfo.m_hObject)
		{
			StopVelocity();

			m_bAnimating = DFALSE; 
			Metacmd++;
		}
    }               
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
        DBOOL bRet = DFALSE;
		DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);
		DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

		if(m_nState == STATE_AttackFar || m_nState == STATE_GuardLocation || (bAbove && fHeight > m_vDims.y))
		{
			m_InventoryMgr.ChangeWeapon(WEAP_GREMLIN_ROCK);
	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[4]);
		}
		else
		{
			m_InventoryMgr.ChangeWeapon(WEAP_SHIKARI_CLAW);
	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[4]);
		}

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_STAND;
    }
    else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			m_InventoryMgr.ChangeWeapon(WEAP_SHIKARI_CLAW);

			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::MC_Dodge_Left
// DESCRIPTION	: Run the dodge animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::MC_Dodge_Left()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_LEFT)
    {
        SetAnimation(m_pAnim_Sound->m_nAnim_DODGE_RIGHT);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel, vLeft;
		VEC_MULSCALAR(vLeft, m_MoveObj.GetRightVector(), -1.0f);
		
		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = vLeft.x * m_fRunSpeed;
		vVel.z = vLeft.z * m_fRunSpeed;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_DODGE_LEFT;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if (collisionInfo.m_hObject)
		{
			DVector vVel;
			VEC_INIT(vVel);

			Move(vVel, MATH_EPSILON);

			m_bAnimating = DFALSE; 
			Metacmd++;
		}
    }               
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::MC_Dodge_Right
// DESCRIPTION	: Run the dodge animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::MC_Dodge_Right()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_RIGHT)
    {
        SetAnimation(m_pAnim_Sound->m_nAnim_DODGE_RIGHT);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel;

		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = m_MoveObj.GetRightVector().x * m_fRunSpeed;
		vVel.z = m_MoveObj.GetRightVector().z * m_fRunSpeed;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_DODGE_RIGHT;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if (collisionInfo.m_hObject)
		{
			DVector vVel;
			VEC_INIT(vVel);

			Move(vVel, MATH_EPSILON);

			m_bAnimating = DFALSE; 
			Metacmd++;
		}
    }               
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(nStim == 0)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);			break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);			break;
			case STATE_SearchSmellTarget:	SetNewState(STATE_Idle);			break;
			default:						SetNewState(STATE_SearchSmellTarget);	break;
		}
	}
	else
	{
		if(nStimType == STIM_SMELL)
			SetNewState(STATE_SearchSmellTarget);
		else if(m_fStimuli[SIGHT] <= 0.75f)
			SetNewState(STATE_AttackFar);
		else
			SetNewState(STATE_AttackClose);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::AI_STATE_AttackClose()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	
	MC_FaceTarget();
	Metacmd--;

	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		if(m_fStimuli[SIGHT] <= 75.0f || m_nCurMetacmd == MC_FIRE_STAND 
						|| (bAbove && fHeight > m_vDims.y))
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.33))
							ComputeState();
						else
							MC_Run();		
					} 
					
					break;
		case 3:		ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::AI_STATE_AttackFar()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		ComputeState();

		return;
	}

	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);
	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;
	
	MC_FaceTarget();
	Metacmd--;
					
	switch(Metacmd)
	{
		case 1:		{
						if (m_nCurMetacmd != MC_TAUNT_BOLD) {
							if (IsRandomChance(40))
							{
								MC_Taunt_Bold();
								Metacmd--;
							}
							else
								Metacmd++;
						}
						else
						{
							if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)	
								Metacmd++;
						}

						break;
					}
		case 2:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND 
						|| (bAbove && fHeight > m_vDims.y))
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.33))
							ComputeState();
						else
							MC_Walk();		
					}
					
					break;
		case 3:		if((m_fStimuli[SIGHT] < (m_fSeeingDist * 0.33)) && m_nCurMetacmd != MC_JUMP)
					{
						ComputeState();
					}
					else if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.4) || m_nCurMetacmd == MC_JUMP
							|| IsLedge(m_MoveObj.GetForwardVector()))
					{
						MC_Jump();
					} else Metacmd++;

					break;
		default:	ComputeState();					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::AI_STATE_Dodge
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::AI_STATE_Dodge()
{
	switch(Metacmd)
	{
		case 1:		m_nDodgeFlags = CalculateDodge(m_vTrackObjPos);
					Metacmd++;		break;
		case 2:		if(m_nDodgeFlags & RIGHT)
						MC_Dodge_Right();
					else if(m_nDodgeFlags & LEFT)
						MC_Dodge_Left();
					else
						Metacmd++;

					break;
		case 3:		SetNewState(m_nLastState);	break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::AI_STATE_GuardLocation
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::AI_STATE_GuardLocation()
{
	int nStimType = ComputeStimuli();

	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		if(nStimType != STIM_SMELL && nStimType > 0)
						Metacmd++;

					break;
		case 2:		MC_Taunt_Bold();		break;
		case 3:		if(m_fStimuli[SIGHT] <= m_fSeeingDist || m_nCurMetacmd == MC_FIRE_STAND)
					{
						MC_Fire_Stand();
					}
					else
						Metacmd++;

					break;
		case 4:		if(m_fStimuli[HEALTH] <= 0.50f)
						ComputeState();
					else
						Metacmd = 1;			
					
					break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: GremlinAI::AI_STATE_Special1
// DESCRIPTION	: EATING state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void GremlinAI::AI_STATE_Special1()
{
	if(m_damage.GetLastDamager())
	{
		ComputeState();
		return;
	}

	m_pServerDE->SetNextUpdate(m_hObject, 0.1f);

	switch(Metacmd)
	{
		case 1:		MC_Special(0);		break;
		case 2:		Metacmd = 1;		break;
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

void GremlinAI::CacheFiles()
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

	SetCacheDirectory("sounds_ao\\enemies\\gremlin");

	CacheSoundFileRange("attack", 1, 3);
	CacheSoundFileRange("death", 1, 3);
	CacheSoundFileRange("idle", 1, 3);
	CacheSoundFileRange("pain", 1, 3);
	CacheSoundFileRange("taunt", 1, 3);
}

#endif // _ADD_ON

