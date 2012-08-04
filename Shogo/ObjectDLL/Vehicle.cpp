// ----------------------------------------------------------------------- //
//
// MODULE  : Vehicle.cpp
//
// PURPOSE : Vehicle base class - Implementation
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#include "Vehicle.h"
#include "cpp_server_de.h"
#include "PVWeaponModel.h"
#include "Spawner.h"

BEGIN_CLASS(Vehicle)
	ADD_LONGINTPROP(State, BaseAI::DEFENSIVE)
	ADD_BOOLPROP(Lights, DTRUE)
	ADD_BOOLPROP(Exhaust, DTRUE)
	PROP_DEFINEGROUP(AvailableStates, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateRetreating, 0, PF_GROUP3 | PF_HIDDEN)
		ADD_BOOLPROP_FLAG(StatePanicked, 0, PF_GROUP3 | PF_HIDDEN)
		ADD_BOOLPROP_FLAG(StatePsycho, 0, PF_GROUP3 | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(Evasive, BaseAI::NON_EVASIVE, PF_HIDDEN)
END_CLASS_DEFAULT(Vehicle, BaseAI, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::Vehicle()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Vehicle::Vehicle() : BaseAI()
{
	m_fWalkVel		= 100.0f;
	m_fRunVel		= 200.0f;
	m_bOkAdjustVel	= DFALSE;

	m_eEvasive		= NON_EVASIVE;

	m_bSpawnWeapon		= DFALSE;
	m_pHandName			= "turret_node";

	m_pLLightName		= "L_Headlight_node";
	m_pRLightName		= "R_Headlight_node";
	m_pExhaustName		= "Muffler_node";

	m_bHeadLights		= DTRUE;
	m_bExhaust			= DTRUE;

	m_hExhaust				= DNULL;
	m_hLHeadLight			= DNULL;
	m_hRHeadLight			= DNULL;

	m_pIdleSound			= "Sounds\\Enemies\\Vehicle\\Idle.wav";
	m_pRunSound				= "Sounds\\Enemies\\Vehicle\\Run.wav";

	m_hIdleSound			= DNULL;
	m_hRunSound				= DNULL;

	// Don't use hit detection for vector weapons
	m_bUsingHitDetection = DFALSE;

	ROT_INIT(m_rTurretRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::~Vehicle()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Vehicle::~Vehicle()
{	
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hIdleSound)
	{
		pServerDE->KillSound(m_hIdleSound);
		m_hIdleSound = DNULL;
	}

	if (m_hRunSound)
	{
		pServerDE->KillSound(m_hRunSound);
		m_hRunSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Vehicle::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f || fData == 2.0f )
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
			break;
		}

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

	return BaseAI::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void Vehicle::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
			
	DFLOAT fRadius = 2000.0f;
	fRadius *= (m_eModelSize == MS_SMALL ? 0.2f : (m_eModelSize == MS_LARGE ? 5.0f : 1.0f));

	// Kludge kludgleson and his kludgy kludgeness...(refering to sub-class
	// in base-class is a no no...however we're about a week from going gold...)

	if (m_eModelSize == MS_SMALL || m_nModelId == MI_AI_VANDAL_ID)
	{
		// m_dwFlags &= ~FLAG_SHADOW;
	}

	// Play idle sound as long as we're alive...

	m_hIdleSound = PlaySoundFromObject(m_hObject, m_pIdleSound, fRadius, 
									   SOUNDPRIORITY_AI_HIGH, 
									   DTRUE, DTRUE);

	if (m_bExhaust) CreateExhaust();
	if (m_bHeadLights) CreateHeadLights();

	// Don't rag-doll vehicles...

	m_damage.SetApplyDamagePhysics(DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Vehicle::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	if (pServerDE->GetPropGeneric("HeadLights", &genProp) == DE_OK)
	{
		m_bHeadLights = genProp.m_Bool;
	}
	if (pServerDE->GetPropGeneric("Exhaust", &genProp) == DE_OK)
	{
		m_bExhaust = genProp.m_Bool;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::HandleWeaponChange()
//
//	PURPOSE:	Handle our weapon changing (i.e., creation)...
//
// ----------------------------------------------------------------------- //
	
void Vehicle::HandleWeaponChange()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pTurretFilename = GetTurretFilename(m_nModelId, m_eModelSize);
	char* pTurretSkin	  = GetSkin(m_nModelId, m_cc, m_eModelSize);

	if (!pTurretFilename) return;

	DVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(vScale, vScale, m_fDimsScale[m_eModelSize]);


	// If we don't have a turret, try to create one...

	if (!m_hHandHeldWeapon) 
	{
		CreateHandHeldWeapon(pTurretFilename, pTurretSkin);
	}
	if (!m_hHandHeldWeapon) return;


	pServerDE->SetModelFilenames(m_hHandHeldWeapon, pTurretFilename, pTurretSkin);
	pServerDE->ScaleObject(m_hHandHeldWeapon, &vScale);

	DDWORD dwHandHeldFlags = pServerDE->GetObjectFlags(m_hHandHeldWeapon);
	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	if (dwFlags & FLAG_VISIBLE)
	{
		dwHandHeldFlags |= FLAG_VISIBLE;
	}
	else
	{
		dwHandHeldFlags &= ~FLAG_VISIBLE;
	}

	if (m_eModelSize == MS_SMALL)
	{
		dwHandHeldFlags &= ~FLAG_SHADOW;
	}

	pServerDE->SetObjectFlags(m_hHandHeldWeapon, dwHandHeldFlags);



	if (m_hHandHeldWeapon)
	{
		CWeapon* pWeapon = m_weapons.GetCurWeapon();
		if (!pWeapon) return;
	
		// Associated our hand held weapon with our weapon...

		CPVWeaponModel* pModel = (CPVWeaponModel*)pServerDE->HandleToObject(m_hHandHeldWeapon);
		if (pModel) 
		{
			pModel->SetParent(pWeapon);
			pWeapon->SetModelObject(m_hHandHeldWeapon);
			pWeapon->InitAnimations();

			SetWeaponDamageFactor(pWeapon);
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::TargetPos()
//
//	PURPOSE:	Turn turret to target a specific pos
//
// ----------------------------------------------------------------------- //

void Vehicle::TargetPos(DVector vTargetPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (!m_hHandHeldWeapon)
	{
		BaseAI::TargetPos(vTargetPos);
		return;
	}


	// Remove the weapon attachment...

	HATTACHMENT hAttachment;
	if (pServerDE->FindAttachment(m_hObject, m_hHandHeldWeapon, &hAttachment) == DE_OK)
	{
		pServerDE->RemoveAttachment(hAttachment);
	}


	// Determine the direction to point...

	DVector vMyPos, vNewTargetPos, vDir;
	DVector vOffset, vUp;
	DRotation rRot;
	DMatrix m1, m2, m3;

	pServerDE->GetModelNodeTransform( m_hObject, m_pHandName, &vMyPos, &rRot );

	VEC_COPY(vNewTargetPos, vTargetPos);
	VEC_SUB(vDir, vNewTargetPos, vMyPos);
	VEC_NORM(vDir);

	// Rotate the turret to the correct position...

	VEC_INIT(vOffset);

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	pServerDE->AlignRotation(&m_rTurretRot, &vDir, &vUp);

	pServerDE->SetupRotationMatrix(&m1, &rRot);
	pServerDE->SetupRotationMatrix(&m2, &m_rTurretRot);
	MatTranspose3x3(&m1);
	MatMul(&m3, &m2, &m1);
	pServerDE->SetupRotationFromMatrix(&m_rTurretOffset, &m3);
	
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hHandHeldWeapon, m_pHandName, 
											   &vOffset, &m_rTurretOffset, &hAttachment);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::HandHeldWeaponFirePos()
//
//	PURPOSE:	Get the fire position of the turret
//
// ----------------------------------------------------------------------- //
	
DVector	Vehicle::HandHeldWeaponFirePos()
{
	DVector vVehiclePos, vMuzzlePos;
	DRotation rMuzzleRot;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	CServerDE* pServerDE = GetServerDE();

	pServerDE->GetObjectPos( m_hObject, &vVehiclePos );
	if ( !m_hHandHeldWeapon || !pWeapon ) return vVehiclePos;
	char* pNodeName = GetTurretFireNodeName();
	if ( !pNodeName ) return vVehiclePos;

	HATTACHMENT hAttachment;
	if (pServerDE->FindAttachment(m_hObject, m_hHandHeldWeapon, &hAttachment) != DE_OK)
	{
		return vVehiclePos;
	}

	if (pServerDE->GetAttachedModelNodeTransform(hAttachment, pNodeName, &vMuzzlePos, &rMuzzleRot) != DE_OK)
	{
		return vVehiclePos;
	}

	return vMuzzlePos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::GetTargetDir()
//
//	PURPOSE:	Get direction to our target
//
// ----------------------------------------------------------------------- //
	
DVector Vehicle::GetTargetDir(DVector & vFirePos, DVector & vTargetPos)
{
	// Make sure we don't aim too far up or too far down...

	// NOT IMPLEMENTED YET


	DVector vDir;
	VEC_SUB(vDir, vTargetPos, vFirePos);
	return vDir;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::HandleDead()
//
//	PURPOSE:	Okay, death animation is done...
//
// ----------------------------------------------------------------------- //

void Vehicle::HandleDead(DBOOL bRemoveObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	RemoveAttachments();

	BaseAI::HandleDead(bRemoveObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::SetRemoveCmd()
//
//	PURPOSE:	Remove the AI
//
// ----------------------------------------------------------------------- //

void Vehicle::SetRemoveCmd()
{
	RemoveAttachments();
	BaseAI::SetRemoveCmd();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::RemoveAttachments()
//
//	PURPOSE:	Remove attachments
//
// ----------------------------------------------------------------------- //

void Vehicle::RemoveAttachments()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HATTACHMENT hAttachment;
	if (m_hExhaust)
	{
		if (pServerDE->FindAttachment(m_hObject, m_hExhaust, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hExhaust);
		m_hExhaust = DNULL;
	}
	
	if (m_hLHeadLight)
	{
		if (pServerDE->FindAttachment(m_hObject, m_hLHeadLight, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hLHeadLight);
		m_hLHeadLight = DNULL;
	}

	if (m_hRHeadLight)
	{
		if (pServerDE->FindAttachment(m_hObject, m_hRHeadLight, &hAttachment) == DE_OK)
		{
			pServerDE->RemoveAttachment(hAttachment);
		}

		pServerDE->RemoveObject(m_hRHeadLight);
		m_hRHeadLight = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::CreateExhaust()
//
//	PURPOSE:	Create exhaust object
//
// ----------------------------------------------------------------------- //

void Vehicle::CreateExhaust()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

#ifdef EXHAUST_WORKING
	m_hExhaust = pObj->m_hObject;

	DRotation rOffset;
	ROT_INIT(rOffset);

	DVector vOffset;
	VEC_INIT(vOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hExhaust, m_pExhaustName, 
											   &vOffset, &rOffset, &hAttachment);

	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hExhaust);
		m_hExhaust = DNULL;
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::CreateHeadLights()
//
//	PURPOSE:	Create head light objects
//
// ----------------------------------------------------------------------- //

void Vehicle::CreateHeadLights()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);

	// Kludge kludgleson and his kludgy kludgeness...(refering to sub-class
	// in base-class is a no no...however we're about a week from going gold...)

	if (m_eModelSize == MS_SMALL || m_nModelId == MI_AI_VANDAL_ID)
	{
		VEC_MULSCALAR(vScale, vScale, 0.1f);
	}

	// Create the left head light...

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, "Sprites\\vehicle.spr");
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GLOWSPRITE;
	theStruct.m_ObjectType  = OT_SPRITE;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pSprite = pServerDE->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hLHeadLight = pSprite->m_hObject;

	pServerDE->ScaleObject(m_hLHeadLight, &vScale);

	// Attach the sprite to the model...

	DVector vOffset;
	VEC_INIT(vOffset);

	vOffset.z += (m_eModelSize == MS_SMALL) ? 1.0f : 5.0f;

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hObject, m_hLHeadLight, m_pLLightName, 
											   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hLHeadLight);
		m_hLHeadLight = DNULL;
	}


	// Create the right head light now...

	pSprite = pServerDE->CreateObject(hClass, &theStruct);
	if (!pSprite) return;

	m_hRHeadLight = pSprite->m_hObject;

	pServerDE->ScaleObject(m_hRHeadLight, &vScale);

	dRes = pServerDE->CreateAttachment(m_hObject, m_hRHeadLight, m_pRLightName, 
									   &vOffset, &rOffset, &hAttachment);
	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hRHeadLight);
		m_hRHeadLight = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Vehicle::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageRotation(hWrite, &m_rTurretOffset);
	pServerDE->WriteToMessageRotation(hWrite, &m_rTurretRot);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hExhaust);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hLHeadLight);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hRHeadLight);
	pServerDE->WriteToMessageByte(hWrite, m_bHeadLights);
	pServerDE->WriteToMessageByte(hWrite, m_bExhaust);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Vehicle::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageRotation(hRead, &m_rTurretOffset);
	pServerDE->ReadFromMessageRotation(hRead, &m_rTurretRot);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hExhaust);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hLHeadLight);
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hRHeadLight);
	m_bHeadLights	= pServerDE->ReadFromMessageByte(hRead);
	m_bExhaust		= pServerDE->ReadFromMessageByte(hRead);


	if (m_pIdleSound)
	{
		DFLOAT fRadius = 2000.0f;
		fRadius *= (m_eModelSize == MS_SMALL ? 0.2f : (m_eModelSize == MS_LARGE ? 5.0f : 1.0f));

		m_hIdleSound = PlaySoundFromObject(m_hObject, m_pIdleSound, fRadius, 
									   SOUNDPRIORITY_AI_HIGH, DTRUE, DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Vehicle::CacheFiles
//
//	PURPOSE:	Cache resources used by this the object
//
// ----------------------------------------------------------------------- //

void Vehicle::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_pIdleSound)
	{
		pServerDE->CacheFile(FT_SOUND, m_pIdleSound);
	}

	if (m_pRunSound)
	{
		pServerDE->CacheFile(FT_SOUND, m_pRunSound);
	}

	pServerDE->CacheFile(FT_SPRITE, "Sprites\\glow.spr");
}