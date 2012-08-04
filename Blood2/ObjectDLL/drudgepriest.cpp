// ----------------------------------------------------------------------- //
//
// MODULE  : DrudgePriest.cpp
//
// PURPOSE : Drudge Priest - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "DrudgePriest.h"
#include "cpp_server_de.h"
#include "SoundTypes.h"

BEGIN_CLASS(DrudgePriest)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 40.0f, 30.0f, 37.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(DrudgePriest, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		DrudgePriest::m_bLoadAnims = DTRUE;
CAnim_Sound	DrudgePriest::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DrudgePriest::DrudgePriest() : AI_Mgr()
{
	m_fHearingDist	= 3000.0f;
	m_fSensingDist	= 3000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1500.0f;

	m_fWalkSpeed	= 100.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"FIREBALL" );
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"LIGHTNING" );
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"SOUL_HOOK" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_bCabal = DFALSE;

	m_fLastSpawn = 0.0f;

	// [blg]
	m_fAIHitPoints   = 1000;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 100;

	m_damage.SetApplyDamagePhysics(DFALSE);

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
	m_hLoopSound = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DrudgePriest::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);
			
			//turn off gravity since he  flies constantly
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_GRAVITY;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

			CacheFiles();

			return dwRet;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

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

void DrudgePriest::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\drudgepriest.abc";
	char* pSkin = "Skins\\Enemies\\drudgepriest.dtx";
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

DBOOL DrudgePriest::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\drudgepriest");
		m_bLoadAnims = DFALSE;
	}

	m_bMoveToGround = DFALSE;

	AI_Mgr::InitStatics(&m_Anim_Sound);

	//play ambient looping sound
	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB;
	playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_CTRL_VOL;
	
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)"sounds\\enemies\\drudgepriest\\dp-looptry1.wav", _MAX_PATH );
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_AI_MEDIUM;
	playSoundInfo.m_fOuterRadius = 1000;
	playSoundInfo.m_fInnerRadius = 1000 * 0.1f;
	playSoundInfo.m_nVolume = 60;
	
	g_pServerDE->PlaySound( &playSoundInfo );
	m_hLoopSound = playSoundInfo.m_hSound;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgePriest::MC_Taunt_Bold
// DESCRIPTION	: Spawn boner leeches
// RETURN TYPE	: DBOOL 
// ----------------------------------------------------------------------- //

void DrudgePriest::MC_Taunt_Bold()
{
	HCLASS hClass = m_pServerDE->GetClass("BoneLeech");

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_TAUNT_BOLD)
    {
		if(m_pServerDE->GetTime() - m_fLastSpawn < 30.0f)
		{
            m_bAnimating = DFALSE;
            Metacmd++;
			return;
		}
		//have to change cabal setting to search the right list
		m_bCabal = DTRUE;

		if(FindObjectInRadius(hClass, m_fSeeingDist, FIND_SPECIFIC_OBJ))
		{
			m_bCabal = DFALSE;
            m_bAnimating = DFALSE;
            Metacmd++;
			return;
		}

		m_pServerDE->DebugOut("Spawning leeaches...\r\n");

		m_bCabal = DFALSE;

        SetAnimation(m_pAnim_Sound->m_nAnim_TAUNT[4]);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_TAUNT_BOLD;
	}
	else
	{
		if(m_pServerDE->GetTime() - m_fLastSpawn >= 1.5f)
		{
			ObjectCreateStruct ocStruct;

			INIT_OBJECTCREATESTRUCT(ocStruct);

			VEC_ADDSCALED( ocStruct.m_Pos, m_MoveObj.GetPos(), m_MoveObj.GetForwardVector(), 20.0f);
			ROT_COPY( ocStruct.m_Rotation, m_MoveObj.GetRotation());

			// Create the object...
			BaseClass *pObj = g_pServerDE->CreateObject( hClass, &ocStruct);

			m_fLastSpawn = m_pServerDE->GetTime();
		}

		//Are we done?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			m_fLastSpawn = m_pServerDE->GetTime();
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgePriest::MC_Dead
// DESCRIPTION	: Run death animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgePriest::MC_Dead()
{
	if(m_hLoopSound)
	{
		m_pServerDE->KillSound(m_hLoopSound);
		m_hLoopSound = DNULL;
	}

	AI_Mgr::MC_Dead();
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgePriest::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgePriest::ComputeState(int nStimType)
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
		if(m_fStimuli[HEALTH] < 0.25f || m_fStimuli[THREAT] > 0.75f)
		{
			SetNewState(STATE_AttackFar);
			return;
		}

		if(m_fStimuli[SIGHT] > 0.50f)
			SetNewState(STATE_AttackClose);
		else
			SetNewState(STATE_AttackFar);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgePriest::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgePriest::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		DVector vVel;
		VEC_INIT(vVel);

		Move(vVel, MATH_EPSILON);

		if(m_nState == STATE_AttackFar)
		{
			if(m_InventoryMgr.ChangeWeapon(WEAP_DRUDGE_FIREBALL) == CHWEAP_WEAPONBUSY)
				return;

	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[0]);
		}
		else
		{
			if(m_InventoryMgr.ChangeWeapon(WEAP_DRUDGE_LIGHTNING) == CHWEAP_WEAPONBUSY)
				return;

	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[5]);
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
			m_InventoryMgr.ChangeWeapon(WEAP_DRUDGE_FIREBALL);
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: DrudgePriest::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgePriest::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	
	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_Taunt_Bold();	break;
		case 2:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.33) || m_nCurMetacmd == MC_FIRE_STAND)
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] >= (m_fSeeingDist * 0.50))
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
// ROUTINE		: DrudgePriest::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void DrudgePriest::AI_STATE_AttackFar()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	
	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND)
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.50))
							ComputeState();
						else
							MC_Run();		
					}
					
					break;
		case 2:		if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.50))
						ComputeState();
					else
						MC_Run();		

					break;
		case 3:		ComputeState();									break;
	}

	return;
}

void DrudgePriest::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	m_pServerDE->WriteToMessageFloat(hWrite, m_fLastSpawn);
}

void DrudgePriest::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	m_fLastSpawn = m_pServerDE->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void DrudgePriest::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\drudgepriest");

	CacheSoundFileRange("dp_attack_", 1, 3);
	CacheSoundFileRange("dp_death_", 1, 3);
	CacheSoundFileRange("dp_idle_", 1, 3);
	CacheSoundFileRange("dp_pain_", 1, 3);
	CacheSoundFileRange("dp_spot", 1, 3);
	CacheSoundFileRange("dp_taunt_", 1, 3);
	CacheSoundFileRange("priestsplat_", 1, 3);

	CacheSoundFile("dp_attacktesla");
	CacheSoundFile("dp_looptry1");
}

