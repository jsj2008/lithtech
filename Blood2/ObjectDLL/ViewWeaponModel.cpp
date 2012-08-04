// ----------------------------------------------------------------------- //
//
// MODULE  : CViewWeaponModel.cpp
//
// PURPOSE : CViewWeaponModel implementation
//
// CREATED : 10/31/97
//
// ----------------------------------------------------------------------- //

#include "ViewWeaponModel.h"
#include "cpp_server_de.h"
#include "Weapon.h"
#include <mbstring.h>
#include "SoundTypes.h"


#define WEAPON_KEY_FIRE			"FIRE_KEY"
#define WEAPON_KEY_SOUND		"SOUND_KEY"
#define WEAPON_KEY_SOUNDLOOP	"SOUNDLOOP_KEY"
#define WEAPON_KEY_SOUNDSTOP	"SOUNDSTOP_KEY"
#define WEAPON_KEY_HIDE			"HIDE_KEY"
#define WEAPON_KEY_SHOW			"SHOW_KEY"


BEGIN_CLASS(CViewWeaponModel)
END_CLASS_DEFAULT_FLAGS(CViewWeaponModel, BaseClass, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeaponModel::CViewWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CViewWeaponModel::CViewWeaponModel() : BaseClass(OT_MODEL)
{
	m_pParent		= DNULL;
	m_hLoopSound	= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeaponModel::Init()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DBOOL CViewWeaponModel::Init(CWeapon* pParent)
{
	if (!pParent) return DFALSE;

	m_pParent = pParent;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CViewWeaponModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_UPDATE:
		{
			// Don't handle updates
//			pServerDE->SetNextUpdate( m_hObject, (DFLOAT)0.001 );
			break;
		}

		case MID_INITIALUPDATE:
		{
			pServerDE->SetObjectFlags( m_hObject, FLAG_MODELGOURAUDSHADE | FLAG_REALLYCLOSE | FLAG_FORCECLIENTUPDATE );
//			pServerDE->SetNextUpdate( m_hObject, (DFLOAT)0.001 );
			break;
		}

		case MID_MODELSTRINGKEY:
		{
			OnStringKey((ArgList*)pData);
			break;
		}

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, lData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CViewWeaponModel::OnStringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CViewWeaponModel::OnStringKey(ArgList* pArgList)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char*	pKey;
	int		count = pArgList->argc;

	for(int i = 0; i < count; i++)
	{
		pKey = pArgList->argv[i];
		if(!pKey) break;

		if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_FIRE) == 0)
		{
			if(m_pParent) 
				m_pParent->OnFireKey();
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SOUND) == 0)
		{
			char*	pSound = pArgList->argv[++i];
			char	pDir[100];

			if(pSound)
			{
				_mbscpy((unsigned char*)pDir, (const unsigned char*)"Sounds\\Weapons\\");
				_mbscat((unsigned char*)pDir, (const unsigned char*)pSound);
				PlaySoundFromObject(m_pParent->GetOwner( ), pDir, 1000, SOUNDPRIORITY_PLAYER_HIGH,
					DFALSE, DFALSE, DFALSE, 100, DFALSE, DTRUE );
			}
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SOUNDLOOP) == 0)
		{
			char*	pSound = pArgList->argv[++i];
			char	pDir[100];

			if(pSound && !m_hLoopSound)
			{
				_mbscpy((unsigned char*)pDir, (const unsigned char*)"Sounds\\Weapons\\");
				_mbscat((unsigned char*)pDir, (const unsigned char*)pSound);
				m_hLoopSound = PlaySoundFromObject( m_pParent->GetOwner( ), pDir, 1000, SOUNDPRIORITY_PLAYER_HIGH, DTRUE, DTRUE);
			}
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SOUNDSTOP) == 0)
		{
			if(m_hLoopSound)
				pServerDE->KillSound(m_hLoopSound);
			m_hLoopSound = DNULL;
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_HIDE) == 0)
		{
			char*	pNode = pArgList->argv[++i];
			DBOOL	nodeStat;
			pServerDE->GetModelNodeHideStatus(this->m_hObject, pNode, &nodeStat);

			if(!nodeStat)
				pServerDE->SetModelNodeHideStatus(this->m_hObject, pNode, DTRUE);
		}
		else if(_mbsicmp((const unsigned char*)pKey, (const unsigned char*)WEAPON_KEY_SHOW) == 0)
		{
			char*	pNode = pArgList->argv[++i];
			DBOOL	nodeStat;
			pServerDE->GetModelNodeHideStatus(this->m_hObject, pNode, &nodeStat);

			if(nodeStat)
				pServerDE->SetModelNodeHideStatus(this->m_hObject, pNode, DFALSE);
		}
	}
}