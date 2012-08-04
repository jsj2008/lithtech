// ----------------------------------------------------------------------- //
//
// MODULE  : DrudgeLord.cpp
//
// PURPOSE : Drudge Lord - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "DrudgeLord.h"
#include "cpp_server_de.h"

BEGIN_CLASS(DrudgeLord)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 31.0f, 51.0f, 25.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(DrudgeLord, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		DrudgeLord::m_bLoadAnims = DTRUE;
CAnim_Sound	DrudgeLord::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DrudgeLord::DrudgeLord() : AI_Mgr()
{
	m_fHearingDist	= 2000.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1200.0f;

	m_fWalkSpeed	= 70.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"SOUL_HOOK");
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"FIREBALL");

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_bCabal = DFALSE;

	// [blg]
	m_fAIHitPoints   = 500;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 100;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DrudgeLord::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

void DrudgeLord::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\drudge_lord.abc";
	char* pSkin = "Skins\\Enemies\\drudge_lord.dtx";
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

DBOOL DrudgeLord::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\drudgelord");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgeLord::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgeLord::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);			break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);			break;
			case STATE_SearchSmellTarget:	SetNewState(STATE_Idle);			break;
			default:						SetNewState(STATE_SearchVisualTarget);	break;
		}
	}
	else
	{
		//if health is low or threat is high, attack close
		if(m_fStimuli[HEALTH] < 0.25f)
		{
			SetNewState(STATE_AttackFar);
			return;
		}

		if(m_fStimuli[SIGHT] > 0.70f)
			SetNewState(STATE_AttackClose);
		else
			SetNewState(STATE_AttackFar);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgeLord::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgeLord::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;
		DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

		if(m_nState == STATE_AttackFar || fHeight > m_vDims.y)
		{
			m_InventoryMgr.ChangeWeapon(WEAP_DRUDGE_FIREBALL);
	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[0]);
		}
		else
		{
			m_InventoryMgr.ChangeWeapon(WEAP_SOUL_HOOK);
	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[m_pServerDE->IntRandom(4,6)]);
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
			m_InventoryMgr.ChangeWeapon(WEAP_SOUL_HOOK);
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgeLord::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgeLord::AI_STATE_AttackClose()
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

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		if(m_fStimuli[SIGHT] <= 100.0f || m_nCurMetacmd == MC_FIRE_STAND)
						MC_Fire_Stand();
					else
					{
						MC_FaceTarget();
						Metacmd--;

						if(m_fStimuli[SIGHT] >= (m_fSeeingDist * 0.30))
							ComputeState();
						else
						{
							if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
								&& m_nCurMetacmd != MC_FIRE_STAND)
							{
								MC_Walk();
							}
							else
							{
								MC_Fire_Stand();
							}
						}
					}
					
					break;
		case 3:		ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgeLord::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgeLord::AI_STATE_AttackFar()
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
						
	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();
					Metacmd--;

					if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND)
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.30))
							ComputeState();
						else
						{
							if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
								&& m_nCurMetacmd != MC_FIRE_STAND)
							{
								MC_Walk();
							}
							else
							{
								MC_Fire_Stand();
							}
						}
					}
					
					break;
		case 2:		MC_Taunt_Bold();			break;
		case 3:		MC_FaceTarget();
					Metacmd--;
		
					if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.30))
						ComputeState();
					else
						{
							if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
								&& m_nCurMetacmd != MC_FIRE_STAND)
							{
								MC_Walk();
							}
							else
							{
								MC_Fire_Stand();
							}
						}

					break;
		case 4:		ComputeState();									break;
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

void DrudgeLord::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\drudgelord");

	CacheSoundFileRange("bodyfall_", 1, 3);
	CacheSoundFileRange("dl_angry", 1, 3);
	CacheSoundFileRange("dl_blade", 1, 3);
	CacheSoundFileRange("dl_death", 1, 3);
	CacheSoundFileRange("dl_idle", 1, 3);
	CacheSoundFileRange("dl_idleblade", 1, 3);
	CacheSoundFileRange("dl_pain", 1, 3);
	CacheSoundFileRange("dl_spot", 1, 3);
	CacheSoundFileRange("dl_walk", 1, 3);
	CacheSoundFileRange("lordfoot_", 1, 3);
	CacheSoundFileRange("lordfoot_drag", 1, 3);

	CacheSoundFile("babyfacefall");
}

