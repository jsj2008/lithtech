// ----------------------------------------------------------------------- //
//
// MODULE  : Cothineal.cpp
//
// PURPOSE : Cothineal Mecha - Implementation
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#include "Cothineal.h"
#include "Gabriel.h"

BEGIN_CLASS(Cothineal)
	ADD_LONGINTPROP(State, MajorCharacter::IDLE)
	PROP_DEFINEGROUP(AvailableStates, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateIdle, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateDefensive, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateAggressive, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateRetreating, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateGuarding, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePanicked, 0, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePsycho, 0, PF_GROUP3)

	ADD_LONGINTPROP(Evasive, MajorCharacter::NON_EVASIVE)
	ADD_LONGINTPROP(Condition, MajorCharacter::HEALTHY)
	ADD_REALPROP_FLAG(VisibleRange, 10000.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(HearingRange, 10000.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(SoundRadius, 10000.0f, PF_RADIUS)	
	ADD_LONGINTPROP(WeaponId, GUN_NONE)
	ADD_STRINGPROP_FLAG(Filename, GetModel(MI_AI_COTHINEAL_ID), PF_DIMS | PF_HIDDEN )
	ADD_BOOLPROP_FLAG(Small, 0, PF_HIDDEN)
END_CLASS_DEFAULT(Cothineal, MajorCharacter, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::Cothineal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Cothineal::Cothineal() : MajorCharacter()
{
	m_nModelId	 = MI_AI_COTHINEAL_ID;
	m_bIsMecha	 = DFALSE;
	m_nWeaponId	 = GUN_NONE;
	m_cc		 = COTHINEAL;

	m_fWalkVel	 = 0.0f;
	m_fRunVel	 = 0.0f;

	m_fToggleEyeTime		= GetRandom(1.0f, 5.0f);
	m_fStartProtectionTime	= 0.0f;

	m_bEyeOpen		 = DFALSE;
	m_bAniTransition = DFALSE;
	m_hGabriel		 = DNULL;
	m_hBeam			 = DNULL;
	
	m_fDimsScale[0] = m_fDimsScale[1] = m_fDimsScale[2] = 3.0f;

	m_bCreateHandHeldWeapon	= DFALSE;
	m_bOkToProcessDamage	= DTRUE;

	m_hClosedEyeAni[0]	= INVALID_ANI;
	m_hClosedEyeAni[1]	= INVALID_ANI;
	m_hClosedEyeAni[2]	= INVALID_ANI;

	m_hOpenEyeAni[0]	= INVALID_ANI;
	m_hOpenEyeAni[1]	= INVALID_ANI;
	m_hOpenEyeAni[2]	= INVALID_ANI;

	m_hOpenToClose		= INVALID_ANI;
	m_hCloseToOpen		= INVALID_ANI;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::~Cothineal()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Cothineal::~Cothineal()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hBeam)
	{
		pServerDE->RemoveObject(m_hBeam);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::ObjectMessageFn()
//
//	PURPOSE:	Handler for object to object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Cothineal::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	Cothineal::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void Cothineal::ProcessDamageMsg(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead || !m_bOkToProcessDamage) return;
	
	DVector vDir;
	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DFLOAT fDamage   = pServerDE->ReadFromMessageFloat(hRead);
	DamageType eType = (DamageType)pServerDE->ReadFromMessageByte(hRead);
	HOBJECT hHeHitMe = pServerDE->ReadFromMessageObject(hRead);

	// Kato can't hurt us :)

	if (eType == DT_BURN || eType == DT_KATO) return;


	DFLOAT fTime = pServerDE->GetTime();

	// See if we can be damaged or not...

	if (m_bEyeOpen)
	{
		m_bOkToProcessDamage = DFALSE;

		PlayDamageSound(eType);

		if (m_hGabriel)
		{
			Gabriel* pGab = (Gabriel*)pServerDE->HandleToObject(m_hGabriel);
			if (!pGab) return;

			if (m_hBeam)
			{
				DDWORD dwFlags = pServerDE->GetObjectFlags(m_hBeam);
				dwFlags &= ~FLAG_VISIBLE;
				pServerDE->SetObjectFlags(m_hBeam, dwFlags);
			}
	
			pGab->SetCanDamage(DTRUE);

			m_fToggleEyeTime = - 1.0f;  // Close eye now!
			m_fStartProtectionTime = pServerDE->GetTime() + GetRandom(2.0f, 5.0f);  // Vulnerable time
		}
	}
	else
	{
		// Play looooo-ser, sound...
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Cothineal::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
					SAFE_STRCPY(pStruct->m_Name, "Cothineal");
				}
			}
			return dwRet;
			break;
		}

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				CreateControlBeam();
			}
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hGabriel)
				{
					m_hGabriel = DNULL;
				}
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


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::InitializeWeapons()
//
//	PURPOSE:	Initialize the weapons
//
// ----------------------------------------------------------------------- //

void Cothineal::AdjustDamageAggregate()
{
	MajorCharacter::AdjustDamageAggregate();

	m_damage.SetCanDamage(DFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void Cothineal::SetAnimationIndexes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	MajorCharacter::SetAnimationIndexes();

	// Set up our closed eye anis...

	m_hClosedEyeAni[0] = pServerDE->GetAnimIndex(m_hObject, "idleclose_1");	
	m_hClosedEyeAni[1] = pServerDE->GetAnimIndex(m_hObject, "idleclose_2");	
	m_hClosedEyeAni[2] = pServerDE->GetAnimIndex(m_hObject, "idleclose_3");	

	m_hOpenEyeAni[0] = pServerDE->GetAnimIndex(m_hObject, "idleopen_1");	
	m_hOpenEyeAni[1] = pServerDE->GetAnimIndex(m_hObject, "idleopen_2");	
	m_hOpenEyeAni[2] = pServerDE->GetAnimIndex(m_hObject, "idleopen_3");	

	m_hOpenToClose	 = pServerDE->GetAnimIndex(m_hObject, "open2close");
	m_hCloseToOpen	 = pServerDE->GetAnimIndex(m_hObject, "close2open");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::UpdateIdle()
//
//	PURPOSE:	Do Idle thang
//
// ----------------------------------------------------------------------- //

void Cothineal::UpdateIdle()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	
	// Always look at Gabriel...

	if (!m_hGabriel)
	{
		FindGabriel();
	}
	else
	{
		TargetObject(m_hGabriel);
	}


	// We're just standing here looking silly...

	ClearActionFlags();
	SetActionFlag(AI_AFLG_STAND);


	// See if it is time to change our idle animation...

	DFLOAT fTime = pServerDE->GetTime();
	if (fTime > m_fToggleEyeTime)
	{
		ToggleIdleAnis();
	}

	
	// See if it is time to raise our defenses...

	if (fTime > m_fStartProtectionTime && m_fStartProtectionTime >= 0.0f)
	{
		if (m_hBeam)
		{
			DDWORD dwFlags = pServerDE->GetObjectFlags(m_hBeam);
			dwFlags |= FLAG_VISIBLE;
			pServerDE->SetObjectFlags(m_hBeam, dwFlags);
		}
	
		if (m_hGabriel)
		{
			Gabriel* pGab = (Gabriel*)pServerDE->HandleToObject(m_hGabriel);
			if (!pGab) return;

			pGab->SetCanDamage(DFALSE);
		}

		m_fStartProtectionTime = -1.0f;
	}


	UpdateControlBeam();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::ToggleIdleAnis()
//
//	PURPOSE:	Toggle which idle animation is played
//
// ----------------------------------------------------------------------- //

void Cothineal::ToggleIdleAnis()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_bAniTransition = DTRUE;
	
	if (m_bEyeOpen)
	{
		SetAnimation(m_hOpenToClose, DFALSE);

		m_fToggleEyeTime = pServerDE->GetTime() + GetRandom(5.0f, 10.0f);
	}
	else
	{
		SetAnimation(m_hCloseToOpen, DFALSE);

		m_fToggleEyeTime = pServerDE->GetTime() + GetRandom(2.0f, 5.0f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::UpdateAnimation()
//
//	PURPOSE:	Update the current animation
//
// ----------------------------------------------------------------------- //

void Cothineal::UpdateAnimation()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DDWORD dwState	= pServerDE->GetModelPlaybackState(m_hObject);

	if (!(dwState & MS_PLAYDONE)) return;

	HMODELANIM hAni = INVALID_ANI;

	if (m_bAniTransition)
	{
		m_bAniTransition = DFALSE;
		m_bEyeOpen = !m_bEyeOpen;

		if (m_bEyeOpen)
		{
			m_bOkToProcessDamage = DTRUE;
		}
	}
	else if (m_bEyeOpen)
	{
		hAni = m_hOpenEyeAni[GetRandom(0, 2)];  
	}
	else  // Eye closed
	{
		hAni = m_hClosedEyeAni[GetRandom(0, 2)];
	}

	if (hAni != INVALID_ANI)
	{
		SetAnimation(hAni, DFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::FindGabriel()
//
//	PURPOSE:	Hook up our m_hGabriel data member
//
// ----------------------------------------------------------------------- //

void Cothineal::FindGabriel()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_hGabriel) return;

	ObjectList*	pList = pServerDE->FindNamedObjects("Gabriel");
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	if (pLink && pLink->m_hObject)
	{
		m_hGabriel = pLink->m_hObject;
		pServerDE->CreateInterObjectLink(m_hObject, m_hGabriel);
	}

	pServerDE->RelinquishList(pList);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::CreateControlBeam()
//
//	PURPOSE:	Create our m_hBeam data member
//
// ----------------------------------------------------------------------- //

void Cothineal::CreateControlBeam()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_hBeam) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	//strcpy(theStruct.m_Filename, "Models\\Powerups\\Beam.abc");
	SAFE_STRCPY(theStruct.m_Filename, "Models\\Characters\\CothinealBeam.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\Explosions\\CothinealBeam.dtx");
	theStruct.m_ObjectType = OT_MODEL;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_NOLIGHT;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pModel = pServerDE->CreateObject(hClass, &theStruct);

	if (pModel) 
	{
		m_hBeam = pModel->m_hObject;
	}

	pServerDE->SetObjectColor(m_hBeam, 1.0f, 1.0f, 1.0f, 0.7f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::UpdateControlBeam()
//
//	PURPOSE:	Update our m_hBeam data member
//
// ----------------------------------------------------------------------- //

void Cothineal::UpdateControlBeam()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hBeam) return;

	if (!m_hGabriel)
	{
		pServerDE->RemoveObject(m_hBeam);
		m_hBeam = DNULL;
		return;
	}


	// Make the beam point from our eye to Gabriel's noggin...

	DVector vPos, vGabPos, vDir;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	pServerDE->GetObjectPos(m_hGabriel, &vGabPos);

	vPos.y += 50.0f;
	vGabPos.y += 75.0f;

	VEC_SUB(vDir, vGabPos, vPos);
	DFLOAT fDist = VEC_MAG(vDir);

	VEC_NORM(vDir);

	DVector vTemp;
	VEC_MULSCALAR(vTemp, vDir, fDist/2.0f);
	VEC_ADD(vPos, vPos, vTemp);

	DRotation rRot;
	pServerDE->AlignRotation(&rRot, &vDir, DNULL);

	pServerDE->SetObjectPos(m_hBeam, &vPos);
	pServerDE->SetObjectRotation(m_hBeam, &rRot);

	DVector vScale;
	VEC_SET(vScale, 30.0f, 30.0f, fDist);
	pServerDE->ScaleObject(m_hBeam, &vScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Cothineal::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hGabriel);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hBeam);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartProtectionTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fToggleEyeTime);
	pServerDE->WriteToMessageByte(hWrite, m_bEyeOpen);
	pServerDE->WriteToMessageByte(hWrite, m_bAniTransition);
	pServerDE->WriteToMessageByte(hWrite, m_bOkToProcessDamage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cothineal::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Cothineal::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hGabriel);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hBeam);
	m_fStartProtectionTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fToggleEyeTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_bEyeOpen				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bAniTransition		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bOkToProcessDamage	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
}