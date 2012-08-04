// ----------------------------------------------------------------------- //
//
// MODULE  : Naga.cpp
//
// PURPOSE : Naga - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "Naga.h"
#include "cpp_server_de.h"

BEGIN_CLASS(Naga)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
END_CLASS_DEFAULT(Naga, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		Naga::m_bLoadAnims = DTRUE;
CAnim_Sound	Naga::m_Anim_Sound;

#define NAGA_MAX_WHOMPS		10

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Naga::Naga() : AI_Mgr()
{
	m_fHearingDist	= 4000.0f;
	m_fSensingDist	= 4000.0f;
	m_fSmellingDist	= 4000.0f;
	m_fSeeingDist	= 4000.0f;

	m_fWalkSpeed	= 0.0f;
	m_fRunSpeed		= 0.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"NAGA_SPIKE" );
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"NAGA_BLAST" );
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"NAGA_DEBRIS" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_ALWAYSCLEAR;

	m_bCabal = DFALSE;
	m_iCeilingWhomps = 0;

	m_damage.SetApplyDamagePhysics(DFALSE);

	// [blg]
	m_fAIHitPoints   = 10000;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 1000;

	m_fFullHealth = 0.0f;
	m_bCreateHealth = DTRUE;

	m_bTurning = DFALSE;
	m_fRadsLeft = 0.0f;
	m_fRadsTurned = 0.0f;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Naga::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				// Remove shadow for 'dramatic purposes' at Bill V's request.
				DDWORD dwFlags;
				dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
				dwFlags &= ~FLAG_SHADOW;
				dwFlags &= ~FLAG_SOLID;
				m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
			}

			CacheFiles();
			
			return dwRet;

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

DDWORD Naga::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

void Naga::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename = "Models\\Enemies\\Naga.abc";
	char* pSkin = "Skins\\Enemies\\Naga.dtx";

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

DBOOL Naga::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\Naga");	
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	// Create the spike ahead of time; we don't want to create 
	// a spike each time, just to have one and hide/show it.
	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_MODEL;
	m_pServerDE->GetModelNodeTransform(m_hObject,"l_gun",&ocStruct.m_Pos,&ocStruct.m_Rotation);
	sprintf(ocStruct.m_Filename,"Models\\Ammo\\Nagaspike.abc");
	sprintf(ocStruct.m_SkinName,"Skins\\Powerups\\Nagaspike_pu.dtx");

	HCLASS hClass = m_pServerDE->GetClass("Model");
	BaseClass* pObj = m_pServerDE->CreateObject(hClass, &ocStruct);
	m_hSpike = pObj->m_hObject;

	// Hide the Naga
	DVector vColor;
	DFLOAT fAlpha;

	m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
	fAlpha = 0.0f;
	m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

	DVector dVec;
	DRotation dRot;

	VEC_INIT(dVec);
	m_pServerDE->SetupEuler(&dRot, 0.0f, 0.0f, 0.0f);

	m_pServerDE->CreateAttachment(m_hObject, m_hSpike, "l_gun", &dVec, &dRot, &m_hSpikeAttach);

	m_bCreateHealth = DTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Naga::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Naga::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);				break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);				break;
			default:						SetNewState(STATE_SearchVisualTarget);	break;
		}
	}
	else
	{
		if(m_fStimuli[SENSE] > 0.0f && m_fStimuli[SIGHT] <= 0.0f)
		{
			SetNewState(STATE_AttackFar);
			return;
		}

		//if health is low or threat is high, attack close
		if(m_fStimuli[HEALTH] < 0.25f || m_fStimuli[THREAT] > 0.75f)
		{
			SetNewState(STATE_AttackClose);
			return;
		}

		if(m_fStimuli[SIGHT] > 0.85f)
			SetNewState(STATE_AttackClose);
		else
			SetNewState(STATE_AttackFar);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Naga::MC_Extra
// DESCRIPTION	: Create or remove (and fire) the spike
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //
void Naga::MC_Extra(const char *lpszParam)
{
	if (m_bAnimating && lpszParam) 
	{
		DDWORD dwFlags;

		if (_mbsicmp((const unsigned char*)lpszParam,(const unsigned char*)"take_spike") == 0)
		{
			// Make the spike visible
			dwFlags = m_pServerDE->GetObjectFlags(m_hSpike);
			dwFlags |= FLAG_VISIBLE;
			m_pServerDE->SetObjectFlags(m_hSpike, dwFlags);
		}

		if (_mbsicmp((const unsigned char*)lpszParam,(const unsigned char*)"fire_spike") == 0)
		{
			// Hide the hand-spike...
			dwFlags = m_pServerDE->GetObjectFlags(m_hSpike);
			dwFlags &= ~FLAG_VISIBLE;
			m_pServerDE->SetObjectFlags(m_hSpike, dwFlags);

			// ...and fire the spike weapon.
			Fire();
		}
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Naga::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Naga::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		if (m_nState == STATE_AttackFar)
		{
			if (m_fStimuli[SIGHT] > 0.0f && IsRandomChance(50))
			{
				m_InventoryMgr.ChangeWeapon(WEAP_NAGA_EYEBEAM);
				DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
			}
			else
			{
				m_iCeilingWhomps++;

				if (m_iCeilingWhomps > NAGA_MAX_WHOMPS)
				{
					// Too many whomps, we use the eye beam for variety.
					m_iCeilingWhomps = 0;

					m_InventoryMgr.ChangeWeapon(WEAP_NAGA_EYEBEAM);
					DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
				}
				else
				{
					m_InventoryMgr.ChangeWeapon(WEAP_NAGA_DEBRIS);
					DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);
				}
			}

		} else
		{
			if (IsRandomChance(25))
			{
				m_InventoryMgr.ChangeWeapon(WEAP_NAGA_EYEBEAM);
				DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
			}
			else
			{
				m_InventoryMgr.ChangeWeapon(WEAP_NAGA_SPIKE);
				DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MAGIC]);			
			}
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
// ROUTINE		: Naga::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Naga::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_Fire_Stand();	break;
		case 3:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Naga::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Naga::AI_STATE_AttackFar()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_Fire_Stand();	break;
		case 3:		ComputeState();		break;
	}

	return;
}

void Naga::SetNewState(int nState)
{
	DDWORD dwFlags;

	// Hide the spike, just in case
	dwFlags = m_pServerDE->GetObjectFlags(m_hSpike);
	dwFlags &= ~FLAG_VISIBLE;
	m_pServerDE->SetObjectFlags(m_hSpike, dwFlags);

	AI_Mgr::SetNewState(nState);	
}

DBOOL Naga::Fire(DBOOL bAltFire)
{
	DVector vPos, vDir;
	DRotation rRot;

	// Sanity check (GK 9/18/98)
	if (!m_InventoryMgr.GetCurrentWeapon())
		return DFALSE;

	switch(m_InventoryMgr.GetCurrentWeapon()->GetType())
	{
		case WEAP_NAGA_EYEBEAM:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"head_gun",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		case WEAP_NAGA_SPIKE:
		{
			m_pServerDE->GetModelNodeTransform(m_hObject,"l_gun",&vPos,&rRot);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}

		default:
		{	
			m_pServerDE->GetObjectPos(m_hObject,&vPos);
			VEC_SUB(vDir, m_vTargetPos, vPos);
			break;
		}
	}

	VEC_NORM(vDir);
	m_pServerDE->AlignRotation(&rRot, &vDir, DNULL);

	m_InventoryMgr.FireCurrentWeapon(&vPos, &rRot, bAltFire);
	return DTRUE;
}

void Naga::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{

	DVector vCol;
	DFLOAT fAlpha;

	m_pServerDE->GetObjectColor(m_hObject, &vCol.x,&vCol.y,&vCol.z,&fAlpha);

	m_pServerDE->WriteToMessageDWord(hWrite, (DDWORD)m_iCeilingWhomps);
	m_pServerDE->WriteToMessageObject(hWrite, m_hSpike);
	m_pServerDE->WriteToMessageFloat(hWrite, m_fFullHealth);

	m_pServerDE->WriteToMessageFloat(hWrite, fAlpha);
}

void Naga::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	DVector vCol;
	DFLOAT fAlpha;

	m_iCeilingWhomps = (int)(m_pServerDE->ReadFromMessageFloat(hRead));
	m_hSpike = m_pServerDE->ReadFromMessageObject(hRead);
	m_fFullHealth = m_pServerDE->ReadFromMessageFloat(hRead);

	m_pServerDE->GetObjectColor(m_hObject, &vCol.x,&vCol.y,&vCol.z, &fAlpha);
	fAlpha = m_pServerDE->ReadFromMessageFloat(hRead);
	m_pServerDE->SetObjectColor(m_hObject, vCol.x,vCol.y,vCol.z,fAlpha);
}


void Naga::MC_FadeIn()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FADEIN)
    {        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FADEIN;

		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags = dwFlags | FLAG_SOLID;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 0.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		fAlpha += 1.0/64;
		if(fAlpha >= 1.0f)
			fAlpha = 1.0f;

		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

		if(fAlpha >= 1.0f)
		{
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;	
}

void Naga::MC_Dead()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DEAD)
    {        
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_SOLID;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);

		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_DEAD;
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 1.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		fAlpha -= 1.0/64;
		if(fAlpha <= 0.0f)
		{
			fAlpha = 0.0f;

			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_VISIBLE;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
		}

		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

		if(fAlpha <= 0.0f)
		{
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;	
}

void Naga::AI_STATE_Special1()
{
	switch(Metacmd)
	{
		case 1:
			MC_FadeIn();
			break;

		case 2:
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags = dwFlags | FLAG_SOLID;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
			SetNewState(STATE_Idle);
			break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void Naga::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\naga");

	CacheSoundFileRange("na_attack_", 1, 3);
	CacheSoundFileRange("na_beam_", 1, 3);
	CacheSoundFileRange("na_death_", 1, 3);
	CacheSoundFileRange("na_howl_", 1, 3);
	CacheSoundFileRange("na_idle_", 1, 3);
	CacheSoundFileRange("na_pain_", 1, 3);
	CacheSoundFileRange("na_taunt_", 1, 3);
}


