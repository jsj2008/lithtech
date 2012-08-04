// ----------------------------------------------------------------------- //
//
// MODULE  : AncientOne.cpp
//
// PURPOSE : Soul Drudge - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "AncientOne.h"
#include "cpp_server_de.h"

BEGIN_CLASS(AncientOne)
    ADD_REALPROP(RandomHitPoints, 00.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
END_CLASS_DEFAULT(AncientOne, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		AncientOne::m_bLoadAnims = DTRUE;
CAnim_Sound	AncientOne::m_Anim_Sound;

#define TENTACLE_SPAWN_AREA		500.0f
#define TENTACLE_SPAWN_MIN		300.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructord
//
// ----------------------------------------------------------------------- //

AncientOne::AncientOne() : AI_Mgr()
{
	m_fHearingDist	= 10000.0f;
	m_fSensingDist	= 10000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 18000.0f;

	m_fWalkSpeed	= 0.0f;
	m_fRunSpeed		= 0.0f;
	m_fRollSpeed	= 0.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"ANCIENTONE_BEAM" );
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"ANCIENTONE_TENTACLE" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_fLastFire = 0;

	m_bCabal = DFALSE;

	m_dwFlags = FLAG_ALWAYSCLEAR;

	m_damage.SetApplyDamagePhysics(DFALSE);

	// [blg]
	m_fAIHitPoints   = 22000;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 2000;

	m_fFullHealth = 0.0f;

	m_bCreateHealth = DTRUE;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD AncientOne::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

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

		case MID_UPDATE:
		{
			if (m_bCreateHealth)
			{
				if (!m_fFullHealth)
				{
					HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
					m_pServerDE->WriteToMessageFloat(hWrite,1.0f);
					m_pServerDE->EndMessage(hWrite);
					m_bCreateHealth = DFALSE;
					m_fFullHealth = m_damage.GetHitPoints();
				}
				else
				{
					DFLOAT fTemp = m_damage.GetHitPoints() / m_fFullHealth;	
					HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
					m_pServerDE->WriteToMessageFloat(hWrite,fTemp);
					m_pServerDE->EndMessage(hWrite);
				}
			}
		}
		break;

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

DDWORD AncientOne::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_DAMAGE:
		{
			if (!m_pServerDE) break;
			
			CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
			
			DFLOAT fTemp = m_damage.GetHitPoints() / m_fFullHealth;	
			HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
			m_pServerDE->WriteToMessageFloat(hWrite,fTemp);
			m_pServerDE->EndMessage(hWrite);
		}

		default: break;
	}

	return AI_Mgr::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void AncientOne::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename = "Models\\Enemies\\AncientOne.abc";
	char* pSkin = "Skins\\Enemies\\AncientOne.dtx";	

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

DBOOL AncientOne::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	VEC_SET(m_vScale,5.0f,5.0f,5.0f);

	m_bMoveToGround = DTRUE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\AncientOne");	
		m_bLoadAnims = DFALSE;
	}

	//play ambient looping sound
/*	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE;
	playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_CTRL_VOL;
	
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)"sounds\\enemies\\ancientone\\ao_loop_1.wav", _MAX_PATH );
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_nVoiceType = SOUNDTYPE_AI;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MEDIUM;
	playSoundInfo.m_fOuterRadius = 5000;
	playSoundInfo.m_fInnerRadius = 5000 * 0.2f;
	playSoundInfo.m_nVolume = 75;
	
	g_pServerDE->PlaySound( &playSoundInfo );
	m_hLoopSound = playSoundInfo.m_hSound;
*/
	AI_Mgr::InitStatics(&m_Anim_Sound);
/*
	// Spawn in some extra tentacles
	int iTemp = m_pServerDE->IntRandom(2,5);

	for (int loop = 0; loop < iTemp; loop++)
	{
		SpawnTentacle();
	}
	*/

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOne::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOne::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
		if (m_fStimuli[SIGHT] < 300.0f)
		{
			m_InventoryMgr.ChangeWeapon(WEAP_ANCIENTONE_TENTACLE);
		}
		else
		{
			m_InventoryMgr.ChangeWeapon(WEAP_ANCIENTONE_BEAM);
		}

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();		
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);

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
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOne::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOne::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_SearchVisualTarget);	break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);				break;
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
// ROUTINE		: AncientOne::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOne::AI_STATE_AttackFar()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	MC_FaceTarget();
	Metacmd--;
	
	switch(Metacmd)
	{
		case 1:		MC_Idle();			break;
		case 2:		MC_Fire_Stand();	break;
		case 3:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOne::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOne::AI_STATE_Idle()
{
	int nStimType = ComputeStimuli();

	if(nStimType)
	{
		ComputeState(nStimType);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_Idle();			break;
		case 2:		ComputeState();		break;
	}

	return;
}

void AncientOne::SpawnTentacle()
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return;

	DVector vPos;
	DFLOAT fTemp;

	m_pServerDE->GetObjectPos(m_hObject,&vPos);

	fTemp = m_pServerDE->Random(0,TENTACLE_SPAWN_AREA);
	fTemp = (m_pServerDE->IntRandom(0,1)) ? fTemp : -fTemp;

	vPos.x = vPos.x + fTemp;

	fTemp = m_pServerDE->Random(0,TENTACLE_SPAWN_AREA);
	fTemp = (m_pServerDE->IntRandom(0,1)) ? fTemp : -fTemp;

	vPos.z = vPos.z + fTemp;

	if ((abs((int)(vPos.x)) < TENTACLE_SPAWN_MIN) && (abs((int)(vPos.z)) < TENTACLE_SPAWN_MIN))
	{
		// Move outward from the origin
		vPos.x += TENTACLE_SPAWN_MIN * ((vPos.x < 0) ? -1 : 1);
		vPos.z += TENTACLE_SPAWN_MIN * ((vPos.z < 0) ? -1 : 1);
		vPos.y += 200;
	}

	ObjectCreateStruct ocStruct;

	INIT_OBJECTCREATESTRUCT(ocStruct);

	VEC_COPY(ocStruct.m_Pos,vPos);
	ocStruct.m_ObjectType = OT_MODEL;
	
	HCLASS hClass = m_pServerDE->GetClass("AncientOneTentacle");
	m_pServerDE->CreateObject(hClass,&ocStruct);
}

void AncientOne::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fLastFire);
	pServerDE->WriteToMessageFloat(hWrite, m_fFullHealth);
}

void AncientOne::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_fLastFire = pServerDE->ReadFromMessageFloat(hRead);
	m_fFullHealth = m_pServerDE->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void AncientOne::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\ancientone");

	CacheSoundFileRange("ancient", 1, 3);
	CacheSoundFileRange("ao_beam_", 1, 3);
	CacheSoundFileRange("ao_loop_", 1, 3);
	CacheSoundFileRange("ao_tent_", 1, 3);
	CacheSoundFileRange("ao_vox_", 1, 3);

	CacheSoundFile("dying");
}


