// ----------------------------------------------------------------------- //
//
// MODULE  : CPVWeaponModel.cpp
//
// PURPOSE : CPVWeaponModel implementation
//
// CREATED : 10/31/97
//
// ----------------------------------------------------------------------- //

#include "PVWeaponModel.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "PlayerObj.h"
#include "Weapon.h"
#include <stdio.h>

extern char g_tokenSpace[];
extern char *g_pTokens[];
extern char *g_pCommandPos;

BEGIN_CLASS(CPVWeaponModel)
END_CLASS_DEFAULT_FLAGS(CPVWeaponModel, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::CPVWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CPVWeaponModel::CPVWeaponModel() : BaseClass(OT_MODEL)
{
	m_pParent			= DNULL;
	m_hParentObject		= DNULL;
	m_hFlashObject		= DNULL;
	m_fFlashStartTime	= 0.0f;
	m_bFired			= DFALSE;

	VEC_INIT(m_vFlashOffset);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::~CPVWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPVWeaponModel::~CPVWeaponModel()
{
	RemoveFlash();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::~CPVWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::RemoveFlash()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hFlashObject)
	{
		if (m_hParentObject)
		{
			HATTACHMENT hAttachment;
			if (pServerDE->FindAttachment(m_hParentObject, m_hFlashObject, &hAttachment) == DE_OK)
			{
				pServerDE->RemoveAttachment(hAttachment);
			}
		}
		pServerDE->RemoveObject(m_hFlashObject);
		m_hFlashObject = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CPVWeaponModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = FLAG_FORCEOPTIMIZEOBJECT | FLAG_SHADOW | FLAG_MODELKEYS | FLAG_MODELGOURAUDSHADE;
			}
			break;
		}

		case MID_MODELSTRINGKEY:
		{
			StringKey((ArgList*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
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

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hParentObject)
				{
					m_hParentObject = DNULL;
				}
			}
		}
		break;

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pServerDE->SetNextUpdate(m_hObject, 0.0f);

	// Set the dims based on the current animation...

	DVector vDims;
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetAnimIndex(m_hObject, "Fire"));
	pServerDE->SetObjectDims(m_hObject, &vDims);

	pServerDE->SetModelLooping(m_hObject, DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::Update()
//
//	PURPOSE:	Update (muzzle flash)
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hFlashObject) return;

	if (m_bFired)
	{
		StartFlash();
		m_bFired = DFALSE;
	}

	if (UpdateFlash())
	{
		pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}


	// Make sure the flash goes away if our parent's object is dead...

	if (m_hParentObject)
	{
		CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject(m_hParentObject);
		if (!pChar || pChar->IsDead())
		{
			if (m_hFlashObject)
			{
				pServerDE->SetObjectFlags(m_hFlashObject, 0);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::CreateFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::CreateFlash()
{
	CServerDE* pServerDE = GetServerDE();
	if (m_hFlashObject || !pServerDE || !m_pParent) return;

	m_vFlashOffset = GetHandWeaponFlashOffset(m_pParent->GetId(), m_pParent->GetSize());

	char* pFlashName = GetFlashFilename(m_pParent->GetId());
	if (!pFlashName) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	
	theStruct.m_ObjectType = OT_SPRITE;
	SAFE_STRCPY(theStruct.m_Filename, pFlashName);
	theStruct.m_Flags = 0;

	HCLASS hClass = pServerDE->GetClass("BaseClass");
	LPBASECLASS pClass = pServerDE->CreateObject(hClass, &theStruct);

	if (pClass)
	{
		DVector vScale = GetFlashScale(m_pParent->GetId(), m_pParent->GetSize());
		VEC_MULSCALAR(vScale, vScale, 2.0f);
		m_hFlashObject = pClass->m_hObject;
		pServerDE->ScaleObject(m_hFlashObject, &vScale);
	}

	AttachFlash();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::AttachFlash()
//
//	PURPOSE:	Attach the flash
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::AttachFlash() 
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_pParent || !m_hFlashObject || !m_hParentObject) return;

	// Attach the flash to the hand-held weapon (well, actually we can't attach
	// the flash to the weapon model since it is attached to the base character
	// and attachments don't support that.  So, we'll just attach the flash to
	// the base character hand)...

	CBaseCharacter* pChar = (CBaseCharacter*) pServerDE->HandleToObject(m_hParentObject);

	if (!pChar)
	{
		pServerDE->RemoveObject(m_hFlashObject);
		m_hFlashObject = DNULL;
		return;
	}

	DRotation rOffset;
	ROT_INIT(rOffset);

	char* pHandName = pChar->GetHandName();

	HATTACHMENT	hAttachment;
	DRESULT dRes = pServerDE->CreateAttachment(m_hParentObject, m_hFlashObject, 
		pHandName, &m_vFlashOffset, &rOffset, &hAttachment);

	if (dRes != DE_OK)
	{
		pServerDE->RemoveObject(m_hFlashObject);
		m_hFlashObject = DNULL;
	}
	else
	{
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hFlashObject);
		pServerDE->SetObjectUserFlags(m_hFlashObject, dwUsrFlags | USRFLG_ATTACH_HIDE1);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::UpdateFlash()
//
//	PURPOSE:	Update muzzle flash state
//
// ----------------------------------------------------------------------- //

DBOOL CPVWeaponModel::UpdateFlash()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_pParent || !m_hFlashObject) return DFALSE;

	char* pFlashName = GetFlashFilename(m_pParent->GetId());
	if (!pFlashName) return DFALSE;

	if (m_hParentObject)
	{
		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hParentObject);
		if (!(dwFlags & FLAG_VISIBLE))
		{
			pServerDE->SetObjectFlags(m_hFlashObject, 0);
			return DFALSE;
		}
	}

	DFLOAT fCurTime = pServerDE->GetTime();

	DFLOAT fFlashDuration = GetFlashDuration(m_pParent->GetId());
	if (fCurTime >= m_fFlashStartTime + fFlashDuration)
	{
		pServerDE->SetObjectFlags(m_hFlashObject, 0);
		return DFALSE;
	}	
	else
	{
		pServerDE->SetObjectFlags(m_hFlashObject, FLAG_VISIBLE);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::StartFlash()
//
//	PURPOSE:	Start the muzzle flash
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::StartFlash()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_pParent || !m_hFlashObject) return;

	char* pFlashName = GetFlashFilename(m_pParent->GetId());
	if (!pFlashName) return;

	DFLOAT fCurTime = pServerDE->GetTime();

	DFLOAT fFlashDuration = GetFlashDuration(m_pParent->GetId());
	if (fCurTime >= m_fFlashStartTime + fFlashDuration)
	{
		m_fFlashStartTime = pServerDE->GetTime();
	}	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::SetParent()
//
//	PURPOSE:	Set our parent data member
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::SetParent(CWeapon* pParent) 
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_pParent = pParent;

	SetupParent();

	if (m_pParent)
	{
		m_hParentObject = m_pParent->GetObject();
		if (m_hParentObject)
		{
			pServerDE->CreateInterObjectLink(m_hObject, m_hParentObject);
		}

		RemoveFlash();
		CreateFlash();
	}
	else if (m_hFlashObject)
	{
		// Hide flash...

		pServerDE->SetObjectFlags(m_hFlashObject, 0);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::StringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::StringKey(ArgList* pArgList)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, WEAPON_KEY_FIRE) == 0)
	{
		Fire();
	}
	else if (stricmp(pKey, WEAPON_KEY_SOUND) == 0)
	{
		if (pArgList->argc > 1)
		{
			char* pSound = pArgList->argv[1];
			if (pSound)
			{
				char buf[100];
				sprintf(buf,"Sounds\\Weapons\\%s.wav", pSound);
				PlaySoundFromObject(m_hObject, buf, 1000, SOUNDPRIORITY_PLAYER_HIGH, 
					DFALSE, DFALSE, DFALSE, 100, DTRUE );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::Fire()
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::Fire()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_pParent) return;

	m_bFired = m_pParent->Fire();
	pServerDE->SetNextUpdate(m_hObject, 0.001f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::SetupParent()
//
//	PURPOSE:	Set up our parent weapon's timing
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::SetupParent()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_pParent || !m_hObject) return;

	char commandStr[100];
	if (pServerDE->GetModelCommandString(m_hObject, commandStr, 100) != DE_OK)
	{
		return;
	}

	int nArgs;
	pServerDE->Parse(commandStr, &g_pCommandPos, g_tokenSpace, g_pTokens, &nArgs);
	if (nArgs < 4) return;

	DFLOAT fMinDur, fMaxDur, fMinRest, fMaxRest;
	fMinDur  = (DFLOAT)atof(g_pTokens[0]);
	fMaxDur  = (DFLOAT)atof(g_pTokens[1]);
	fMinRest = (DFLOAT)atof(g_pTokens[2]);
	fMaxRest = (DFLOAT)atof(g_pTokens[3]);

	m_pParent->SetMinFireDuration(fMinDur);
	m_pParent->SetMaxFireDuration(fMaxDur);
	m_pParent->SetMinFireRest(fMinRest);
	m_pParent->SetMaxFireRest(fMaxRest);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hFlashObject);
	pServerDE->WriteToMessageVector(hWrite, &m_vFlashOffset);
	pServerDE->WriteToMessageFloat(hWrite, m_fFlashStartTime);
	pServerDE->WriteToMessageByte(hWrite, m_bFired);
	pServerDE->WriteToMessageByte(hWrite, DFALSE);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVWeaponModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPVWeaponModel::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hFlashObject);
	pServerDE->ReadFromMessageVector(hRead, &m_vFlashOffset);
	m_fFlashStartTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_bFired				= pServerDE->ReadFromMessageByte(hRead);
	DBOOL bPlayerViewWeapon = pServerDE->ReadFromMessageByte(hRead);
}