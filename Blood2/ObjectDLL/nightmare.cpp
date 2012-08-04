
// ----------------------------------------------------------------------- //
//
// MODULE  : Nightmare.cpp
//
// PURPOSE : Nightmare - Definition
//
// CREATED : 10/07/97
//
// ----------------------------------------------------------------------- //

#ifdef _ADD_ON		// Add-on pack only.
#include <stdio.h>
#include "Nightmare.h"
#include "cpp_server_de.h"


BEGIN_CLASS(Nightmare)
	ADD_BASEAI_AGGREGATE()
    ADD_REALPROP(HitPoints, 200000.0f)     \
    ADD_REALPROP(RandomHitPoints, 400.0f) \
    ADD_REALPROP(ArmorPoints, 0.0f)     \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIBrain, "STRONG")   \
	ADD_STRINGPROP(AIWeapon1, "NIGHTMARE_BITE")   \
	ADD_STRINGPROP(AIWeapon2, "NIGHTMARE_FIREBALLS")   \
	ADD_STRINGPROP(AIWeapon3, "BEHEMOTH_SHOCKWAVE")   \
END_CLASS_DEFAULT(Nightmare, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		Nightmare::m_bLoadAnims = DTRUE;
CAnim_Sound	Nightmare::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Nightmare::Nightmare() : AI_Mgr()
{
	m_fHearingDist	= 2000.0f;
	m_fSensingDist	= 6000.0f;
	m_fSmellingDist	= 4000.0f;
	m_fSeeingDist	= 2200.0f;

	m_fWalkSpeed	= 800.0f;
	m_fRunSpeed		= 1200.0f;
	m_fJumpSpeed	= 900.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIHitPoints   = 30000;
	m_fAIRandomHP    = 400;
	m_fAIArmorPoints = 0;

	m_fAIMass		= AI_DEFAULT_MASS * 15.0f;

	m_nAIStrength	= 9;

	m_fFullHealth	= 0;
	m_bCreateHealth = DTRUE;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"NIGHTMARE_BITE" );
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"NIGHTMARE_FIREBALLS" );
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"BEHEMOTH_SHOCKWAVE" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_JUMP | FLAG_NIGHTMAREDEATH | FLAG_NEVERGIB;

	m_damage.SetApplyDamagePhysics(DFALSE);

//	m_fWaitForIdleTime = 45.0f;
	m_bShockwave = DFALSE;

	m_bCabal = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Nightmare::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}

DDWORD Nightmare::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_DAMAGE:
		{
			if (!m_pServerDE) break;
			
			CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
			
			if (m_fFullHealth != 0) 
			{
				DFLOAT fTemp = m_damage.GetHitPoints() / m_fFullHealth;	

				if (fTemp > m_fFullHealth) fTemp = 1.0f;
				if (fTemp < 0) fTemp = 0;

				HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
				m_pServerDE->WriteToMessageFloat(hWrite,fTemp);
				m_pServerDE->EndMessage(hWrite);
			}
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

void Nightmare::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models_ao\\Enemies_ao\\Nightmare.abc";
	char* pSkin = "Skins_ao\\Enemies_ao\\Nightmare.dtx";
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

DBOOL Nightmare::InitialUpdate(DVector *pMovement)
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
	    m_Anim_Sound.SetSoundRoot("sounds_ao\\enemies\\Nightmare");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Nightmare::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Nightmare::ComputeState(int nStim)
{
	if(!ComputeStimuli())
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
		if(m_fStimuli[SIGHT] > 0.75f)
			SetNewState(STATE_AttackClose);
		else
			SetNewState(STATE_AttackFar);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Nightmare::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Nightmare::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		if(m_nState == STATE_AttackClose) 
		{
			if (m_bShockwave) 
			{
				m_InventoryMgr.ChangeWeapon(WEAP_BEHEMOTH_SHOCKWAVE);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[5]);
			}
			else
			{
				m_InventoryMgr.ChangeWeapon(WEAP_NIGHTMARE_BITE);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[4]);
			}
		}
		else
		{
			m_InventoryMgr.ChangeWeapon(WEAP_NIGHTMARE_FIREBALLS);
	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[2]);
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
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Nightmare::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Nightmare::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
			
					if(m_fStimuli[SIGHT] < 600.0f || m_nCurMetacmd == MC_FIRE_STAND)
					{
						m_bShockwave = DTRUE;
						MC_Fire_Stand();
					}
					else
					{
						if (m_fStimuli[SIGHT] < 900.f)
						{
							MC_FaceTarget();
							Metacmd--;

							MC_Walk();
						}
						else
							SetNewState(STATE_AttackFar);
					}
					
					break;
		case 3:		m_bShockwave = DFALSE;
					if (m_fStimuli[SIGHT] < 250.0f)
					{
						MC_Fire_Stand();			
					}
					else
					{
						if (m_fStimuli[SIGHT] < 900.f)
						{
							MC_FaceTarget();
							Metacmd--;

							MC_Walk();
						}
						else
							SetNewState(STATE_AttackFar);
					}

					break;
		case 4:		ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Nightmare::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Nightmare::AI_STATE_AttackFar()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);

	if (m_fStimuli[SIGHT] < 400.0f)
		SetNewState(STATE_AttackClose);

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		if(m_fStimuli[SIGHT] <= 1500.0f || m_nCurMetacmd == MC_FIRE_STAND)
						MC_Fire_Stand();
					else
					{
						m_InventoryMgr.ChangeWeapon(WEAP_NIGHTMARE_BITE);
						MC_FaceTarget();
						Metacmd--; 

						MC_Run();
					}
					
					break;

					// Now we need to be vulnerable before we move on.
		case 3:		MC_Taunt_Bold();	break;
		case 4:		{
						m_InventoryMgr.ChangeWeapon(WEAP_NIGHTMARE_BITE);
						MC_FaceTarget();
						Metacmd--;

						MC_Run();
					}
					break;

		case 5:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Nightmare::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Nightmare::AI_STATE_Idle()
{
	if(ComputeStimuli())
	{
		ComputeState();
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_Idle();			break;
		case 2:		ComputeState();		break;
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

void Nightmare::CacheFiles()
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

	SetCacheDirectory("sounds_ao\\enemies\\nightmare");

	CacheSoundFileRange("attack", 1, 3);
	CacheSoundFileRange("death", 1, 3);
	CacheSoundFileRange("idle", 1, 3);
	CacheSoundFileRange("pain", 1, 3);
	CacheSoundFileRange("spot", 1, 3);
	CacheSoundFileRange("step", 1, 3);
}

#endif // _ADD_ON