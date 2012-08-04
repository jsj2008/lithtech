// ----------------------------------------------------------------------- //
//
// MODULE  : EnhancedGideon.cpp
//
// PURPOSE : EnhancedGideon AI - Implementation
//
// CREATED : 10/1/98 (version 3!)
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "EnhancedGideon.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(EnhancedGideon)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_BOOLPROP(Enhanced,DFALSE)	\
END_CLASS_DEFAULT(EnhancedGideon, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		EnhancedGideon::m_bLoadAnims = DTRUE;
CAnim_Sound	EnhancedGideon::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EnhancedGideon::EnhancedGideon() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 10000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 10000.0f;

	m_fWalkSpeed	= 175.0f;
	m_fRunSpeed		= 380.0f;
	m_fRollSpeed	= 200.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 10;

	strcpy(m_szAIWeapon[0], "GIDEON_SHIELD" );
	strcpy(m_szAIWeapon[1], "ENERGY_BLAST" );
	strcpy(m_szAIWeapon[2], "NAGA_BLAST");
	strcpy(m_szAIWeapon[3], "GIDEON_WIND");

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_ALWAYSRECOIL;

	m_bCabal = DFALSE; // Should this be true?
	m_bMoveToGround = DFALSE; // We can fly, don't move to floor on creation.

	sprintf(m_szFireNode,"rr_gun");

	// [blg]
	m_fAIHitPoints   = 9000;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;

	m_fFullHealth = 0.0f;

	m_bCreateHealth = DTRUE;

	m_fShieldDuration = 60.0f; // We can only shield ourselves once every 60 seonds
	m_fLastShield = 0.0f;
	m_fLastPanic = 0.0f;

	m_damage.SetApplyDamagePhysics(DFALSE);

	m_bNoFire = DFALSE;

	strcpy(m_szAIState, "IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD EnhancedGideon::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

	// Store result of parent class function
	DDWORD dwResult = AI_Mgr::EngineMessageFn(messageID, pData, fData);

	// If this is initial update, we need to turn off gravity AFTER the
	// MID_INITIALUPDATE call in AI_Mgr...
	if (messageID == MID_INITIALUPDATE) 
	{
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		m_pServerDE->SetObjectFlags(m_hObject, dwFlags);	
	}

	// ...and return the stored result.
	return dwResult;
}

DDWORD EnhancedGideon::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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

void EnhancedGideon::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\Gideon.abc";
	char *pSkin = "Skins\\Enemies\\GideonE.dtx";

	strcpy(pStruct->m_Filename, pFilename);
	strcpy(pStruct->m_SkinName, pSkin);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL EnhancedGideon::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\EnhancedGideon");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	m_bCreateHealth = DTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: EnhancedGideon::MC_Dodge_Left
// DESCRIPTION	: run the dodge left animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void EnhancedGideon::MC_Dodge_Left()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_LEFT)
    {
		m_fTimeStart = m_pServerDE->GetTime();

        SetAnimation( m_pAnim_Sound->m_nAnim_DODGE_LEFT);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vLeft;
		VEC_MULSCALAR(vLeft, m_MoveObj.GetRightVector(), -1.0f);
		Move(vLeft, m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DODGE_LEFT;
	}
	else
	{
		DVector vLeft;
		VEC_MULSCALAR(vLeft, m_MoveObj.GetRightVector(), -1.0f);
		Move(vLeft, m_fRollSpeed);

		//Are we done?
		if(m_pServerDE->GetTime() - m_fTimeStart >= 1.0f)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: EnhancedGideon::MC_Dodge_Right
// DESCRIPTION	: run the dodge right animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void EnhancedGideon::MC_Dodge_Right()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_RIGHT)
    {
		m_fTimeStart = m_pServerDE->GetTime();

        SetAnimation( m_pAnim_Sound->m_nAnim_DODGE_RIGHT);
        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DODGE_RIGHT;
	}
	else
	{
		Move(m_MoveObj.GetRightVector(),m_fRollSpeed);

		//Are we done?
		if(m_pServerDE->GetTime() - m_fTimeStart >= 1.0f)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: EnhancedGideon::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void EnhancedGideon::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Special1);			break;
			case STATE_Teleport:			SetNewState(STATE_SearchVisualTarget);	break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);				break;
			default:						SetNewState(STATE_Teleport);			break;
		}
	}
	else
	{
		//if health is low or threat is high...
		if(m_fStimuli[HEALTH] < 0.25f || m_fStimuli[THREAT] > 0.75f)
		{
			// ...and we can shield...
			if (m_pServerDE->GetTime() - m_fLastShield > m_fShieldDuration)
			{
				// ...do so.
				SetNewState(STATE_AttackClose);
			} 
			else
			{
				// ...or run like hell.
				if (m_fStimuli[HEALTH] < 0.05f)
				{
					if ((m_fLastPanic + 15.0f) >= m_pServerDE->GetTime())
					{
						m_fLastPanic = m_pServerDE->GetTime();
						SetNewState(STATE_Teleport);
					}
					else
					{
						SetNewState(STATE_AttackClose);
					}
				}
				else
				{
					// ...or use the wind attack.
					SetNewState(STATE_AttackClose);
				}
			}
			return;
		}

		switch(m_nState)
		{
			case STATE_SearchVisualTarget:
			{
				SetNewState(STATE_Special1);
				break;
			}

			default:
			{
				if(m_fStimuli[SIGHT] > 0.94f)
				{
					SetNewState(STATE_AttackClose);
				}
				else
				{
					if (m_fStimuli[SIGHT] > 0.75f)
					{
						SetNewState(STATE_AttackFar);
					}
					else
					{
						SetNewState(STATE_Special1);
					}
				}
				
				break;
			}
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: EnhancedGideon::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void EnhancedGideon::MC_Fire_Stand()
{
	// Hack.
	if (m_bNoFire)
	{
		SetNewState(STATE_Passive);
		return;
	}

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		DFLOAT fArmor = m_damage.GetArmorPoints();

		if ((fArmor <= 25) && ((m_pServerDE->GetTime() - m_fLastShield) > m_fShieldDuration))
		{
			m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_SHIELD);
			m_damage.SetArmorPoints(fArmor + 100);
			m_fLastShield = m_pServerDE->GetTime();
			bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
		} 
		else if(m_nState == STATE_AttackFar)
		{
			if (IsRandomChance(70))
			{
				m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_ENERGYBLAST);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MAGIC]);
			} else
			{
				m_InventoryMgr.ChangeWeapon(WEAP_NAGA_EYEBEAM);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
			}
		}
		else
		{
			if (IsRandomChance(50))
			{
				m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_WIND);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
			}
			else
			{
				m_InventoryMgr.ChangeWeapon(WEAP_NAGA_EYEBEAM);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
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
			m_InventoryMgr.ChangeWeapon(WEAP_SOUL_HOOK);
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: EnhancedGideon::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void EnhancedGideon::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	if (m_bNoFire)
	{
		SetNewState(STATE_Passive);
		return;
	}

	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);

					if((m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.90f)) || m_nCurMetacmd == MC_FIRE_STAND)
						MC_Fire_Stand();
					else
					{
						if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.40))
						{
							SetNewState(STATE_Special1);		
						}
						else
						{
							MC_Walk();
						}
					}
					
					break;
		case 3:		if(m_nCurMetacmd == MC_DODGE_RIGHT)
						MC_Dodge_Right();
					else if(m_nCurMetacmd == MC_DODGE_LEFT)
						MC_Dodge_Left();
					else if(IsRandomChance(50))
						MC_Dodge_Right();
					else
						MC_Dodge_Left();

					break;
		case 4:		ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: EnhancedGideon::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void EnhancedGideon::AI_STATE_AttackFar()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	if (m_bNoFire)
	{
		SetNewState(STATE_Passive);
		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
						
	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();
					Metacmd--;

					if((m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.30f)) || m_nCurMetacmd == MC_FIRE_STAND)
					{
						MC_Fire_Stand();
					}
					else
					{
						SetNewState(STATE_Special1);
					}
					
					break;
		case 2:		MC_Taunt_Bold();			break;
		case 3:		MC_FaceTarget();
					Metacmd--;
		
					if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.40f))
					{
						SetNewState(STATE_Special1);
					}
					else
					{
						MC_Walk();
					}


					break;
		case 4:		ComputeState();									break;
	}

	return;
}


void EnhancedGideon::MagicPowerup()
{
	DVector		offset;
	VEC_SET(offset, 0.0f, 0.0f, 0.0f);

	if (m_bNoFire)
	{
		return;
	}

	// Display the 'powerup' effect around EnhancedGideon
	HMESSAGEWRITE hMessage = m_pServerDE->StartInstantSpecialEffectMessage(&offset);
	m_pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);

	m_pServerDE->WriteToMessageObject(hMessage, m_hObject);
	m_pServerDE->WriteToMessageVector(hMessage, &offset);
	m_pServerDE->WriteToMessageFloat(hMessage, 0.0f);
	m_pServerDE->WriteToMessageDWord(hMessage, 0);
	m_pServerDE->WriteToMessageDWord(hMessage, OBJFX_POWERUP_1);
	m_pServerDE->WriteToMessageDWord(hMessage, 0);

	m_pServerDE->EndMessage(hMessage);	
}

void EnhancedGideon::MC_FadeOut()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FADEOUT)
    {        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FADEOUT;

		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_SOLID;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 0.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		fAlpha -= 1.0/16;
		if(fAlpha <= 0.0f)
			fAlpha = 0.0f;

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

void EnhancedGideon::AI_STATE_Teleport()
{
	switch(Metacmd)
	{
		case 1:		MC_FadeOut();		break;
		case 2:
		{
					m_pAnim_Sound->PlaySound(m_hObject, "teleport.wav", 1000.0f, 100);					
					Metacmd++;			break;
		}
		case 3:
		{
					IntersectQuery IQuery;
					IntersectInfo IInfo;
					DDWORD dwFlags;

					IQuery.m_Flags	  = INTERSECT_OBJECTS;
					IQuery.m_FilterFn = DNULL;

					DVector vDir;
					VEC_SET(vDir, m_pServerDE->Random(-1.0f,1.0f), 0.0f, m_pServerDE->Random(-1.0f,1.0f));

					VEC_COPY(IQuery.m_From, m_MoveObj.GetPos());
					VEC_ADDSCALED(IQuery.m_To, IQuery.m_From, vDir, 1500.0f);

					dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
					dwFlags |= FLAG_SOLID;
					m_pServerDE->SetObjectFlags(m_hObject,dwFlags);

					if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
					{
						VEC_ADDSCALED(IInfo.m_Point, IInfo.m_Point, vDir, -100.0f);
						m_pServerDE->MoveObject(m_hObject, &IInfo.m_Point);
					}
					else
					{
						VEC_ADDSCALED(IInfo.m_Point, IQuery.m_To, vDir, -100.0f);
						m_pServerDE->MoveObject(m_hObject, &IQuery.m_To);
					}

					Metacmd++;
					break;
		}
		case 4:		
		{	
					DVector vColor;
					DFLOAT	fAlpha = 1.0f;

					m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
					m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, 1.0f);

					Metacmd++;
					break;
		}
		case 5:		MC_FaceTarget();	break;
		case 6:		ComputeState();		break;
	}

	return;
}

void EnhancedGideon::AI_STATE_Special1()
{
	if (!m_hTarget)
	{
		SetNewState(STATE_Teleport);
	}

	switch(Metacmd)
	{
		case 1:		MC_FadeOut();		break;
		case 2:
		{
//					char szSound[256];
//					strcpy(szSound, SOUND_LAUGH);
//					m_pAnim_Sound->GetSoundPath(szSound,m_pServerDE->IntRandom(1,3));

//					PlayAISound(szSound, 1000.0f, PLAY_WAIT);					

					Metacmd++;			break;
		}
		case 3:
		{
					IntersectQuery IQuery;
					IntersectInfo IInfo;
					DDWORD dwFlags;

					IQuery.m_Flags	  = INTERSECT_OBJECTS;
					IQuery.m_FilterFn = DNULL;

					DVector vDir, vTarget;
					VEC_SET(vDir, m_pServerDE->Random(-1.0f,1.0f), 0.2f, m_pServerDE->Random(-1.0f,1.0f));

					m_pServerDE->GetObjectPos(m_hTarget,&vTarget);

					dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
					dwFlags |= FLAG_SOLID;
					m_pServerDE->SetObjectFlags(m_hObject,dwFlags);

					VEC_COPY(IQuery.m_From, vTarget);
					VEC_ADDSCALED(IQuery.m_To, IQuery.m_From, vDir, m_pServerDE->Random(400.0f,600.0f));
					if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
					{
						VEC_ADDSCALED(IInfo.m_Point, IInfo.m_Point, vDir, -300.0f);
						m_pServerDE->MoveObject(m_hObject, &IInfo.m_Point);
					}
					else
					{
						VEC_ADDSCALED(IInfo.m_Point, IQuery.m_To, vDir, -300.0f);
						m_pServerDE->MoveObject(m_hObject, &IQuery.m_To);
					}

					Metacmd++;
					break;
		}
		case 4:		
		{	
					DVector vColor;
					DFLOAT	fAlpha = 1.0f;

					m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
					m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, 1.0f);

					Metacmd++;
					break;
		}
		case 5:		MC_FaceTarget();	break;
		case 6:		ComputeState();		break;
	}

	return;
}

void EnhancedGideon::MC_Extra(const char *lpszText)
{
	if(strncmp(lpszText,"powerup",7) == 0)
	{
		MagicPowerup();
	} 
	else
	{
		sprintf(m_szFireNode,"%s",lpszText);
		Fire();
	}
}

DBOOL EnhancedGideon::Fire(DBOOL bAltFire)
{
	DVector vPos, vDir;
	DRotation rRot;

	if (m_bNoFire)
	{
		return DFALSE;
	}

	// Sanity check (GK 9/18/98)
	if (!m_InventoryMgr.GetCurrentWeapon())
		return DFALSE;

	if(!m_pServerDE->GetModelNodeTransform(m_hObject,m_szFireNode,&vPos,&rRot))
	{
		m_pServerDE->GetObjectPos(m_hObject,&vPos);
		VEC_COPY(vDir, m_MoveObj.GetForwardVector());
	}
	else
	{
		VEC_SUB(vDir, m_vTargetPos, vPos);
	}

	VEC_NORM(vDir);
	m_pServerDE->AlignRotation(&rRot, &vDir, DNULL);

	DDWORD m_nFiredWeapon = m_InventoryMgr.FireCurrentWeapon(&vPos, &rRot, bAltFire);

	return DTRUE;
}


void EnhancedGideon::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fShieldDuration);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastShield);
	pServerDE->WriteToMessageFloat(hWrite, m_fFullHealth);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastPanic);
}

void EnhancedGideon::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_fShieldDuration = pServerDE->ReadFromMessageFloat(hRead);
	m_fLastShield = pServerDE->ReadFromMessageFloat(hRead);
	m_fFullHealth = m_pServerDE->ReadFromMessageFloat(hRead);
	m_fLastPanic = m_pServerDE->ReadFromMessageFloat(hRead);
}

void EnhancedGideon::Script_Walk()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_WALK)
    {
		DBOOL bRet = DFALSE;

		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags = dwFlags | FLAG_GRAVITY;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);

    	m_fTimeStart = m_pServerDE->GetTime();

		bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		if(m_nInjuredLeg)
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed/3);
		else
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
			
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_WALK;
    }
    else
    {   
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
   
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
        }
    }
}

void EnhancedGideon::Script_Run()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RUN)
    {
		DBOOL bRet = DFALSE;

    	m_fTimeStart = m_pServerDE->GetTime();

        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_RUN[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_RUN;
    }
    else
    {   
		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
    
		//Are we done running?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
         }
    }
}


void EnhancedGideon::MC_Walk()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_WALK)
    {
		DBOOL bRet = DFALSE;

		if (m_nState == STATE_Script)
		{
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags = dwFlags | FLAG_GRAVITY;
			m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
		}

    	m_fTimeStart = m_pServerDE->GetTime();

		bRet = SetAnimation( m_pAnim_Sound->m_nAnim_WALK[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		if(m_nInjuredLeg)
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed/3);
		else
			Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
			
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_WALK;
    }
    else
    {   
		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
   
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
        }
    }
}

void EnhancedGideon::MC_Run()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RUN)
    {
		DBOOL bRet = DFALSE;

		if (m_nState == STATE_Script)
		{
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_GRAVITY;
			m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
		}

    	m_fTimeStart = m_pServerDE->GetTime();

        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_RUN[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fWalkSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_RUN;
    }
    else
    {   
		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
    
		//Are we done running?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
			StopVelocity();

            m_bAnimating = DFALSE; 
         }
    }
}


void EnhancedGideon::AI_STATE_Script()
{
	m_bNoFire = DTRUE;

	if (m_bUpdateScriptCmd) 
	{
		UpdateScriptCommand();
	}

	DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

	if (!(dwFlags & FLAG_GRAVITY))
	{
		dwFlags = dwFlags | FLAG_GRAVITY;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
	}

	switch(m_curScriptCmd.command)
	{
		case AI_SCMD_SETMOVEMENT:
		case AI_SCMD_PLAYSOUND:
		case AI_SCMD_SETSTATE:
		case AI_SCMD_TARGET:
			m_bUpdateScriptCmd = DTRUE;
		break;
		
		case AI_SCMD_FOLLOWPATH:
			UpdateFollowPathCmd();
		break;

		case AI_SCMD_WAIT:
			UpdateWaitCmd();
		break;
		
		case AI_SCMD_PLAYANIMATION:
			UpdatePlayAnimationCmd();
		break;
		
		case AI_SCMD_MOVETOOBJECT:
			UpdateMoveToObjectCmd();
		break;

		case AI_SCMD_DONE:
		default: 
			m_dwScriptFlags = 0;
			m_bNoFire = DFALSE;
			ComputeState();
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

void EnhancedGideon::CacheFiles()
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
}


