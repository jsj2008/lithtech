// ----------------------------------------------------------------------- //
//
// MODULE  : SoulDrudge.cpp
//
// PURPOSE : Soul Drudge - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "SoulDrudge.h"
#include "cpp_server_de.h"

#include <windows.h>

BEGIN_CLASS(SoulDrudge)
    ADD_REALPROP(RandomHitPoints, 50.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 21.0f, 36.0f, 18.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(SoulDrudge, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		SoulDrudge::m_bLoadAnims = DTRUE;
CAnim_Sound	SoulDrudge::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoulDrudge::SoulDrudge() : AI_Mgr()
{
	m_fHearingDist	= 1000.0f;
	m_fSensingDist	= 100.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1000.0f;

	m_fWalkSpeed	= 60.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	switch(g_pServerDE->IntRandom(1,3))
	{
		case 1:		_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"SOUL_CROWBAR" );	break;
		case 2:		_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"SOUL_AXE" );		break;
		case 3:		_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"SOUL_PIPE" );		break;
	}

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_NOAMMOCOLLECT;

	m_bCabal = DFALSE;

	// [blg]
	m_fAIHitPoints   = 120;
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

DDWORD SoulDrudge::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

void SoulDrudge::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename = "Models\\Enemies\\SoulDrudge.abc";
	char* pSkin = DNULL;

	switch(pServerDE->IntRandom(1,3))
	{
		case 1:		pSkin = "Skins\\Enemies\\SoulDrudge.dtx";	break;
		case 2:		pSkin = "Skins\\Enemies\\SoulDrudge2.dtx";	break;
		case 3:		pSkin = "Skins\\Enemies\\SoulDrudge3.dtx";	break;
	}

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

DBOOL SoulDrudge::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\souldrudge");	
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: SoulDrudge::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void SoulDrudge::ComputeState(int nStimType)
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
		if(m_fStimuli[SIGHT] > 0.5f)
			SetNewState(STATE_AttackClose);
		else
			SetNewState(STATE_AttackFar);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: SoulDrudge::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void SoulDrudge::MC_Fire_Stand()
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
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: SoulDrudge::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void SoulDrudge::AI_STATE_AttackClose()
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

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
			
					if(m_fStimuli[SIGHT] <= 75.0f || m_nCurMetacmd == MC_FIRE_STAND)
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
// ROUTINE		: SoulDrudge::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void SoulDrudge::AI_STATE_AttackFar()
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

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
			
					if(m_fStimuli[SIGHT] <= 75.0f || m_nCurMetacmd == MC_FIRE_STAND)
						MC_Fire_Stand();
					else
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y)
						{
							MC_Walk();
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
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void SoulDrudge::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\SoulDrudge");

	CacheSoundFileRange("bodyfall_", 1, 3);
	CacheSoundFileRange("sd_attackgroan", 1, 3);
	CacheSoundFileRange("sd_death_", 1, 3);
	CacheSoundFileRange("sd_firestandattack", 1, 3);
	CacheSoundFileRange("sd_foot_", 1, 3);
	CacheSoundFileRange("sd_idle", 1, 3);
	CacheSoundFileRange("sd_idlebreath", 1, 3);
	CacheSoundFileRange("sd_idlegroan", 1, 3);
	CacheSoundFileRange("sd_pain_", 1, 3);
	CacheSoundFileRange("sd_runknife1-", 1, 3);
	CacheSoundFileRange("sd_runknife2-", 1, 3);
	CacheSoundFileRange("sd_scratching", 1, 3);
	CacheSoundFileRange("sd_sniffgrunt", 1, 3);
	CacheSoundFileRange("sd_sniffing", 1, 3);
	CacheSoundFileRange("sd_spotgrunt", 1, 3);
	CacheSoundFileRange("sd_walk", 1, 3);
	CacheSoundFileRange("sd_walkgroan", 1, 3);
	CacheSoundFileRange("sd_walkntalk", 1, 3);
	CacheSoundFileRange("sd_walkwet", 1, 3);
}


