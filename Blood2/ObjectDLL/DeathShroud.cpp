
// ----------------------------------------------------------------------- //
//
// MODULE  : DeathShroud.cpp
//
// PURPOSE : DeathShroud - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "DeathShroud.h"
#include "cpp_server_de.h"
#include "SoundTypes.h"

BEGIN_CLASS(DeathShroud)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 20.0f, 48.0f, 20.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(DeathShroud, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		DeathShroud::m_bLoadAnims = DTRUE;
CAnim_Sound	DeathShroud::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DeathShroud::DeathShroud() : AI_Mgr()
{
	m_fHearingDist	= 3000.0f;
	m_fSensingDist	= 3000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 3000.0f;

	m_fWalkSpeed	= 100.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"SKULL");

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_bCabal = DFALSE;

	// [blg]
	m_fAIHitPoints   = 800;
	m_fAIRandomHP    = 00;
	m_fAIArmorPoints = 200;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DeathShroud::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			
			VEC_SET(m_vScale, 1.5f, 1.5f, 1.5f);

			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_SHADOW;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

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

void DeathShroud::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\DeathShroud.abc";
	char* pSkin = "Skins\\Enemies\\DeathShroud.dtx";
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

DBOOL DeathShroud::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\DeathShroud");
		m_bLoadAnims = DFALSE;
	}

	m_InventoryMgr.AddDamageMultiplier(0.5f);	

	AI_Mgr::InitStatics(&m_Anim_Sound);

	//play ambient looping sound
	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB;
	playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_CTRL_VOL;
	
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)"sounds\\enemies\\deathshroud\\de_loop_2.wav", _MAX_PATH );
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_AI_MEDIUM;
	playSoundInfo.m_fOuterRadius = 1000;
	playSoundInfo.m_fInnerRadius = 1000 * 0.1f;
	playSoundInfo.m_nVolume = 50;
	
	g_pServerDE->PlaySound( &playSoundInfo );
	m_hLoopSound = playSoundInfo.m_hSound;

	//fade the deathshroud
	DVector vColor;
	DFLOAT fAlpha = 0.0f;

	m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
	fAlpha = 0.90f;
	m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DeathShroud::MC_Fade
// DESCRIPTION	: Run an fade out/in animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DeathShroud::MC_Fade(DBOOL bFade)
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_TAUNT_BOLD)
    {
		DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_TAUNT[4]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_TAUNT_BOLD;

		if(bFade)
		{
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_SOLID;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
		}
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 0.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		if(bFade)
		{
			fAlpha -= 0.05f;
			if(fAlpha <= 0.5f)
				fAlpha = 0.5f;

			m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

			if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE && fAlpha <= 0.5f)
			{
				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}
		}
		else
		{
			fAlpha += 0.05f;
			if(fAlpha >= 0.90f)
				fAlpha = 0.90f;

			m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

			if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE && fAlpha >= 0.90f)
			{
				DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
				dwFlags |= FLAG_SOLID;
				m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DeathShroud::MC_Fire_Stand
// DESCRIPTION	: Run the energy blast, shockwave, or fire animation and effect
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DeathShroud::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
	    DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[8]);

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
// ROUTINE		: DeathShroud::MC_Dead
// DESCRIPTION	: Run death animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DeathShroud::MC_Dead()
{
	DVector vColor;
	DFLOAT fAlpha = 0.0f;

	m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DEAD)
    {	
		if(m_hLoopSound)
		{
			m_pServerDE->KillSound(m_hLoopSound);
			m_hLoopSound = DNULL;
		}

		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, 1.0f);
	
		int nSideHit = m_damage.GetSideHit();

		switch(m_damage.GetNodeHit())
		{
			case NODE_NECK:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[0 + nSideHit]; break;
			case NODE_TORSO:	m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[1 + nSideHit]; break;
			case NODE_RARM:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[2 + nSideHit]; break;
			case NODE_LARM:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[3 + nSideHit]; break;
			case NODE_LLEG:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[4 + nSideHit]; break;
			case NODE_RLEG:		m_nCorpseType = m_pAnim_Sound->m_nAnim_DEATH[5 + nSideHit]; break;
		}

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DEAD;
	}
	else
	{
		VEC_MULSCALAR(m_vScale, m_vScale, 0.9f);
		VEC_MULSCALAR(m_vDims, m_vDims, 0.9f);

		m_pServerDE->ScaleObject(m_hObject, &m_vScale);
		m_pServerDE->SetObjectDims2(m_hObject, &m_vDims);

		fAlpha -= 0.05f;
		if(fAlpha <= 0.0f)		fAlpha = 0.0f;

		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			m_bAnimating = DFALSE;
			Metacmd++;

			m_bRemoveMe = DTRUE;
		}
	}

	return;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: DeathShroud::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DeathShroud::ComputeState(int nStimType)
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
		SetNewState(STATE_AttackFar);				
	}
	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DeathShroud::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DeathShroud::AI_STATE_AttackFar()
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

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	
	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_Fade(DFALSE);	break;	
		case 2:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.66) || m_nCurMetacmd == MC_FIRE_STAND
						|| fHeight > m_vDims.y)
					{
						MC_Fire_Stand();
					}
					else
					{
						Metacmd++;
					}

					break;
		case 3:		MC_Fade(DTRUE);		break;
		case 4:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.33) || fHeight > m_vDims.y)
						Metacmd++;
					else
						MC_Walk();

					break;
		case 5:		ComputeState();		break;
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

void DeathShroud::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\deathshroud");

	CacheSoundFileRange("de_attack_", 1, 3);
	CacheSoundFileRange("de_idle_", 1, 3);
	CacheSoundFileRange("de_loop_", 1, 3);
	CacheSoundFileRange("de_pain_", 1, 3);

	CacheSoundFile("de_death_3verbed");
	CacheSoundFile("flyskull");
}


