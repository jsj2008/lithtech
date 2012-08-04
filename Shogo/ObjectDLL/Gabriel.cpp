// ----------------------------------------------------------------------- //
//
// MODULE  : Gabriel.cpp
//
// PURPOSE : Gabriel - Implementation
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#include "Gabriel.h"
#include "CharacterMgr.h"

BEGIN_CLASS(Gabriel)
	ADD_LONGINTPROP(State, BaseAI::AGGRESSIVE)
	PROP_DEFINEGROUP(AvailableStates, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateIdle, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateDefensive, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateAggressive, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateRetreating, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateGuarding, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePanicked, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePsycho, 0, PF_GROUP3)

	ADD_LONGINTPROP(Marksmanship, BaseAI::MARKSMANSHIP5)
	ADD_LONGINTPROP(Evasive, BaseAI::NON_EVASIVE)
	ADD_LONGINTPROP(Condition, BaseAI::HEALTHY)
	ADD_REALPROP_FLAG(VisibleRange, 10000.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(HearingRange, 10000.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(SoundRadius, 10000.0f, PF_RADIUS)	

	ADD_LONGINTPROP(WeaponId, GUN_ENERGYBATON_ID)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_GABRIEL_ID), PF_DIMS | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Gabriel, MajorCharacter, NULL, NULL)

#define MAX_ATTACK_DIST		350.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::Gabriel()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Gabriel::Gabriel() : MajorCharacter()
{
	m_nModelId	 = MI_AI_GABRIEL_ID;
	m_bIsMecha	 = DTRUE;
	m_nWeaponId	 = GUN_ENERGYBATON_ID;
	m_cc		 = FALLEN;
	m_fWalkVel	 = 200.0f;
	m_fRunVel	 = 200.0f;

	m_fNextTargetTime		= 1.0f;
	m_bCreateHandHeldWeapon	= DFALSE;
	m_bCanDamageBody		= DFALSE;
	m_hShield				= DNULL;
	m_hShieldSound			= DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::~Gabriel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Gabriel::~Gabriel()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hShield)
	{
		HATTACHMENT hAttachment;
		if (pServerDE->FindAttachment(m_hObject, m_hShield, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}
		pServerDE->RemoveObject(m_hShield);
		m_hShield = DNULL;
	}

	StopShieldSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Gabriel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = MajorCharacter::EngineMessageFn(messageID, pData, fData);
			
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if (pStruct)
				{
					SAFE_STRCPY(pStruct->m_Name, "Gabriel");
				}
			}
			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				CreateShield();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return MajorCharacter::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::ObjectMessageFn()
//
//	PURPOSE:	Handler for object to object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Gabriel::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch (messageID)
	{
		case MID_DAMAGE:
		{
			DDWORD dwRet = MajorCharacter::ObjectMessageFn(hSender, messageID, hRead);
			ProcessDamageMsg(hRead);
			return dwRet;
		}
	}
	
	return MajorCharacter::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void Gabriel::ProcessDamageMsg(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;
	
	DVector vDir;
	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DFLOAT fDamage   = pServerDE->ReadFromMessageFloat(hRead);
	DamageType eType = (DamageType)pServerDE->ReadFromMessageByte(hRead);
	HOBJECT hHeHitMe = pServerDE->ReadFromMessageObject(hRead);

	if (IsPlayer(hHeHitMe) && !m_hCurDlgSnd && !m_damage.GetCanDamage())
	{
		char* pSounds[] = { "Sounds\\MajorCharacter\\Gabriel\\laugh1.wav",
							"Sounds\\MajorCharacter\\Gabriel\\laugh2.wav" };

		PlayDialogSound(pSounds[GetRandom(0,1)]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::UpdateSenses
//
//	PURPOSE:	Update AI senses (sight, hearing, smell)
//
// ----------------------------------------------------------------------- //

void Gabriel::UpdateSenses()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	// Until we've spotted a player, keep looking for one...

	if (!m_bSpottedPlayer)
	{
		HOBJECT hObj = pCharMgr->FindVisiblePlayer(this);
		if (hObj)
		{
			if (CheckAlignment(HATE, hObj))
			{
				SetNewTarget(hObj);
				TargetObject(hObj);
			}
			
			SpotPlayer(hObj);
		}
	}

	if (m_hTarget)
	{
		UpdateTargetPos();
		TargetObject(m_hTarget);
	}

	pServerDE->SetNextUpdate(m_hObject, 0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::InitializeWeapons()
//
//	PURPOSE:	Initialize the weapons
//
// ----------------------------------------------------------------------- //

void Gabriel::InitializeWeapons()
{
	MajorCharacter::InitializeWeapons();

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;

	// Adjust our weapon range to that of MAX_ATTACK_DIST...

	DFLOAT fRange = pWeapon->GetRange();
	if (fRange < 1.0f) return;

	DFLOAT fMultiplier = MAX_ATTACK_DIST / fRange;
	pWeapon->SetRangeAdjust(fMultiplier);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::InitializeWeapons()
//
//	PURPOSE:	Initialize the weapons
//
// ----------------------------------------------------------------------- //

void Gabriel::AdjustDamageAggregate()
{
	MajorCharacter::AdjustDamageAggregate();

	SetCanDamage(DFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::UpdateAggressive()
//
//	PURPOSE:	Implement the attacking actions
//
// ----------------------------------------------------------------------- //

void Gabriel::UpdateAggressive()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !m_hTarget) return;

	CBaseCharacter* pB = (CBaseCharacter*) pServerDE->HandleToObject(m_hTarget);
	if (!pB) return;

	if (pB->IsDead()) 
	{
		// Stop coming after Sanjuro...

		SetNewTarget(DNULL);
		m_bSpottedPlayer = DFALSE;

		// Set to idle (after script)...

		SetState(IDLE);

		BuildTauntScript();
		SetScript();

		PlayDialogSound("Sounds\\MajorCharacter\\Gabriel\\sanjurodead.wav");
	} 
	

	// Try to shoot sanjuro.....

	if (SanjuroInRange())
	{
		ShootTarget();
	}
	

	// Always try to get a better shot...

	ApproachSanjuro();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::ApproachSanjuro()
//
//	PURPOSE:	Move towards Sanjuro
//
// ----------------------------------------------------------------------- //

void Gabriel::ApproachSanjuro()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !m_hTarget) return;

	TargetObject(m_hTarget);

	// If we can't see Sanjuro, see if we can find him...

	if (!IsObjectVisibleToAI(m_hTarget))
	{
		SearchForPlayer(m_hTarget);
		return;
	}
	
	WalkForward();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::SanjuroInRange()
//
//	PURPOSE:	Make sure Sanjuro is close enough to attack
//
// ----------------------------------------------------------------------- //

DBOOL Gabriel::SanjuroInRange()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hTarget) return DFALSE;

	DVector vTargetPos;
	pServerDE->GetObjectPos(m_hTarget, &vTargetPos);

	if (VEC_DIST(vTargetPos, m_vPos) <= MAX_ATTACK_DIST)
	{
		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::BuildTauntScript()
//
//	PURPOSE:	Build our taunt script 
//
// ----------------------------------------------------------------------- //

void Gabriel::BuildTauntScript()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_scriptCmdList.RemoveAll();

	// Play our pose ani...

	AISCRIPTCMD* pCmd = new AISCRIPTCMD;
	if (!pCmd) return;

	pCmd->command = AI_SCMD_PLAYANIMATION;
	strncpy(pCmd->args, "POSE1", MAX_AI_ARGS_LENGTH);

	m_scriptCmdList.Add(pCmd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::SetCanDamage()
//
//	PURPOSE:	Set up gabriel so he can be damaged...
//
// ----------------------------------------------------------------------- //

void Gabriel::SetCanDamage(DBOOL bCanDamage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_damage.SetCanDamage(bCanDamage);

	// Turn shield on/off...

	if (m_hShield)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hShield);
		if (bCanDamage)
		{
			dwFlags &= ~FLAG_VISIBLE;
			StopShieldSound();
		}
		else
		{
			dwFlags |= FLAG_VISIBLE;
			PlayShieldSound();
		}

		pServerDE->SetObjectFlags(m_hShield, dwFlags);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::PlayShieldSound()
//
//	PURPOSE:	Play shield sound
//
// ----------------------------------------------------------------------- //

void Gabriel::PlayShieldSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_hShieldSound) return;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB | PLAYSOUND_ATTACHED | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	
	strncpy(playSoundInfo.m_szSoundName, "Sounds\\MajorCharacter\\Gabriel\\Shield.wav", _MAX_PATH);
	playSoundInfo.m_hObject		 = m_hObject;
	playSoundInfo.m_fOuterRadius = 2000.0f;
	playSoundInfo.m_fInnerRadius = m_fSoundRadius * 0.5f;

	pServerDE->PlaySound(&playSoundInfo);
	m_hShieldSound = playSoundInfo.m_hSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::StopShieldSound()
//
//	PURPOSE:	Stop shield sound
//
// ----------------------------------------------------------------------- //

void Gabriel::StopShieldSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hShieldSound) return;

	if (m_hShieldSound)
	{
		pServerDE->KillSound(m_hShieldSound);
		m_hShieldSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::CreateShield()
//
//	PURPOSE:	Create Gabriel's shield...
//
// ----------------------------------------------------------------------- //

void Gabriel::CreateShield()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_hShield) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, "Models\\PV_Weapons\\SpiderExplosionCore.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\Explosions\\SpiderCore.dtx");

	theStruct.m_ObjectType  = OT_MODEL;
	theStruct.m_Flags		= FLAG_VISIBLE | FLAG_GOTHRUWORLD;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	m_hShield = pModel->m_hObject;

	DVector vScale;
	VEC_SET(vScale, 150.0f, 300.0f, 150.0f);
	pServerDE->ScaleObject(m_hShield, &vScale);

	pServerDE->SetObjectColor(m_hShield, 1.0f, 0.5f, 0.0f, 0.5f);

	// Attach the sheild model to Gabriel...

	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hShield, DNULL, 
											   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hShield);
		m_hShield = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void Gabriel::SetAnimationIndexes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	BaseAI::SetAnimationIndexes();

	m_hKnifeIdle2Ani = m_hKnifeIdle1Ani;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::SetDeathAnimation()
//
//	PURPOSE:	Set animation to death
//
// ----------------------------------------------------------------------- //

void Gabriel::SetDeathAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || m_bStartedDeath) return;

	StartDeath();
	m_eDeathType = CD_NORMAL;
	SetAnimation(pServerDE->GetAnimIndex(m_hObject, "DEATH1"), DFALSE);
			
	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	dwFlags &= ~FLAG_GRAVITY;
	pServerDE->SetObjectFlags(m_hObject, dwFlags);

	HandleDead(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Gabriel::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hShield);

	if (m_hShield)
	{
		if (pServerDE->GetObjectFlags(m_hShield) & FLAG_VISIBLE)
		{			
			PlayShieldSound();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gabriel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Gabriel::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hShield);
}