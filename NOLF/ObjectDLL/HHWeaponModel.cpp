// ----------------------------------------------------------------------- //
//
// MODULE  : CHHWeaponModel.cpp
//
// PURPOSE : CHHWeaponModel implementation
//
// CREATED : 10/31/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HHWeaponModel.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "Weapon.h"
#include "SoundMgr.h"
#include <stdio.h>

extern char g_tokenSpace[];
extern char *g_pTokens[];
extern char *g_pCommandPos;

#define HHW_SOUND_KEY_RADIUS	1000.0f

BEGIN_CLASS(CHHWeaponModel)
END_CLASS_DEFAULT_FLAGS(CHHWeaponModel, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::CHHWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CHHWeaponModel::CHHWeaponModel() : BaseClass(OT_MODEL)
{
    m_pParent           = LTNULL;
    m_hParentObject     = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::~CHHWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHHWeaponModel::~CHHWeaponModel()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CHHWeaponModel::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = FLAG_FORCEOPTIMIZEOBJECT | FLAG_SHADOW | FLAG_MODELKEYS;
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

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hParentObject)
				{
                    m_hParentObject = LTNULL;
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
//	ROUTINE:	CHHWeaponModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::InitialUpdate()
{
    SetNextUpdate(m_hObject, 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::SetParent()
//
//	PURPOSE:	Set our parent data member
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::SetParent(CWeapon* pParent)
{
	m_pParent = pParent;

	if (m_pParent)
	{
		m_hParentObject = m_pParent->GetObject();
		if (m_hParentObject)
		{
            g_pLTServer->CreateInterObjectLink(m_hObject, m_hParentObject);
		}

		m_pParent->SetModelObject(m_hObject);
		m_pParent->CacheFiles();
	}

	// Set the dims based on the hand-held ani...

	uint32 dwAni = g_pLTServer->GetAnimIndex(m_hObject, "Hand");

 	if (dwAni != INVALID_ANI)
	{
		LTVector vDims;
		g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, dwAni);
		g_pLTServer->SetObjectDims(m_hObject, &vDims);

		g_pLTServer->SetModelAnimation(m_hObject, dwAni);
	}

	g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::SetParent()
//
//	PURPOSE:	Set our parent data member
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::Setup(CWeapon* pParent, WEAPON* pWeaponData)
{
    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if (pWeaponData->szHHModel[0])
	{
		g_pLTServer->SetModelFilenames(m_hObject, pWeaponData->szHHModel, pWeaponData->szHHSkin);
		dwFlags |= FLAG_VISIBLE;
	}
	else
	{
		dwFlags &= ~FLAG_VISIBLE;
	}
	
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	SetParent(pParent);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::StringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::StringKey(ArgList* pArgList)
{
	if (!m_pParent || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, WEAPON_KEY_FIRE) == 0)
	{
		//if (m_pParent) m_pParent->Fire();
	}
	else if (stricmp(pKey, WEAPON_KEY_SOUND) == 0)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
            uint8 nWeaponId = m_pParent->GetId();
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if (!pWeapon) return;

            char* pFilename = LTNULL;

			PlayerSoundId nId = (PlayerSoundId)atoi(pArgList->argv[1]);
			switch (nId)
			{
				case PSI_RELOAD:
				case PSI_RELOAD2:
				case PSI_RELOAD3:
					pFilename = pWeapon->szReloadSounds[nId - PSI_RELOAD];
				break;
				case PSI_SELECT:
					pFilename = pWeapon->szSelectSound;
				break;
				case PSI_DESELECT:
					pFilename = pWeapon->szDeselectSound;
				break;

				case PSI_INVALID:
				default : break;
			}

			if (pFilename && pFilename[0])
			{
                LTVector vPos;
                g_pLTServer->GetObjectPos(m_hObject, &vPos);
                g_pServerSoundMgr->PlaySoundFromPos(vPos, pFilename, HHW_SOUND_KEY_RADIUS, SOUNDPRIORITY_AI_HIGH);
            }
		}
	}
}
