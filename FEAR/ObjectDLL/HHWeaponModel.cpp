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

#include "Stdafx.h"
#include "HHWeaponModel.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "Weapon.h"
#include "SoundMgr.h"
#include "ServerSoundMgr.h"
#include <stdio.h>
#include "ParsedMsg.h"
#include "AIUtils.h"

#define HHW_SOUND_MAX_DURATION	0.5f

LINKFROM_MODULE( HHWeaponModel );


BEGIN_CLASS(CHHWeaponModel)
END_CLASS_FLAGS(CHHWeaponModel, GameBase, CF_HIDDEN, "Hand Held weapon model support")


CMDMGR_BEGIN_REGISTER_CLASS( CHHWeaponModel )
CMDMGR_END_REGISTER_CLASS( CHHWeaponModel, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::CHHWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CHHWeaponModel::CHHWeaponModel() : GameBase(OT_MODEL)
{
    m_hParentObject     = NULL;
	m_pParentWeapon		= NULL;
	m_nLoopSoundId		= PSI_INVALID;
	m_hLoopSound		= NULL;
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
	KillLoopSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CHHWeaponModel::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = FLAG_NOTINWORLDTREE | FLAG_MODELKEYS;
				pStruct->m_eGroup = ePhysicsGroup_NonSolid;
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

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
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
    // If we are going to recieve MID_MODELSTRINGKEY we must be active

	SetNextUpdate(UPDATE_NEVER, eControlUpdateOnly);
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
	HMODELANIM	dwAni = INVALID_ANI;
	m_hParentObject = NULL;
	m_pParentWeapon = pParent;

	if (m_pParentWeapon)
	{
		m_hParentObject = pParent->GetObject();

		// Use the same timer as our owner.  If they don't have one, then 
		// just use the default and don't associate a object specific timer.
		EngineTimer engineTimer = (HOBJECT)m_hParentObject;
		if( engineTimer.IsValid( ))
		{
			engineTimer.ApplyTimerToObject( m_hObject );
		}

		// The player HHWeapon doesn't need to recieve MID_MODELSTRINGKEY.

		if( IsPlayer( m_hParentObject ))
			SetNextUpdate( UPDATE_NEVER );

		m_pParentWeapon->SetModelObject(m_hObject);
		dwAni = m_pParentWeapon->GetHandAni();
	}

	// Set the dims based on the hand-held ani...

 	if (dwAni != INVALID_ANI)
	{
		LTVector vDims;
		g_pModelLT->GetModelAnimUserDims(m_hObject, dwAni, &vDims);
		g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

		g_pLTServer->GetModelLT()->SetCurAnim(m_hObject, MAIN_TRACKER, dwAni, true);
	}

	g_pLTServer->GetModelLT()->SetLooping(m_hObject, MAIN_TRACKER, false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::Setup()
//
//	PURPOSE:	Setup our weapon data.
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::Setup( CWeapon* pParent, HWEAPON hWeapon, const char *pszDBStructName )
{
	ObjectCreateStruct ocs;

	HATTRIBUTE hAttrib = NULL;
	bool bUseAIData = pParent ? IsAI( pParent->GetObject() ) : !USE_AI_DATA;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, bUseAIData );
	HATTRIBUTE hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, pszDBStructName );
	if( !hWeaponModelStruct )
	{
		LTERROR( "Failed to retrieve the weapon model struct." );
		return;
	}

	hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
	const char *pszHHModel = g_pWeaponDB->GetString( hAttrib );
	if( pszHHModel[0] )
	{
		ocs.SetFileName( pszHHModel );
	}

	// Get all specified materials...
	char szCompleteName[256];
	LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", pszDBStructName, WDB_WEAPON_sHHMaterial );
	g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName, ocs.m_Materials[0],
								   LTARRAYSIZE(ocs.m_Materials), LTARRAYSIZE(ocs.m_Materials[0]) );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// SetParent will assign animation indices which depend on having the correct model filename set...
	SetParent( pParent );

	if( pParent )
	{
		// Set the weapon invisible if no model was specified...
		pParent->HideWeapon( LTStrEmpty( pszHHModel ));
	}
}

void CHHWeaponModel::SetupDualWeaponModel( CWeapon *pParent, HWEAPON hWeapon, const char *pszDBStructName )
{
	// Setup parents dual weapon...
	{
		HMODELANIM	dwAni = INVALID_ANI;
		m_hParentObject = NULL;
		m_pParentWeapon = pParent;

		if (m_pParentWeapon)
		{
			m_hParentObject = pParent->GetObject();

			// Use the same timer as our owner.  If they don't have one, then 
			// just use the default and don't associate a object specific timer.
			EngineTimer engineTimer = (HOBJECT)m_hParentObject;
			if( engineTimer.IsValid( ))
			{
				engineTimer.ApplyTimerToObject( m_hObject );
			}

			// The player HHWeapon doesn't need to recieve MID_MODELSTRINGKEY.

			if( IsPlayer( m_hParentObject ))
				SetNextUpdate( UPDATE_NEVER );

			m_pParentWeapon->SetDualWeaponModelObject(m_hObject);
			dwAni = m_pParentWeapon->GetHandAni();
		}

		// Set the dims based on the hand-held ani...

		if (dwAni != INVALID_ANI)
		{
			LTVector vDims;
			g_pModelLT->GetModelAnimUserDims(m_hObject, dwAni, &vDims);
			g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

			g_pLTServer->GetModelLT()->SetCurAnim(m_hObject, MAIN_TRACKER, dwAni, true);
		}

		g_pLTServer->GetModelLT()->SetLooping(m_hObject, MAIN_TRACKER, false);
	}

	ObjectCreateStruct ocs;

	HATTRIBUTE hAttrib = NULL;
	bool bUseAIData = pParent ? IsAI( pParent->GetObject() ) : !USE_AI_DATA;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, bUseAIData );
	HATTRIBUTE hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, pszDBStructName );
	if( !hWeaponModelStruct )
	{
		LTERROR( "Failed to retrieve the weapon model struct." );
		return;
	}

	hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
	const char *pszHHModel = g_pWeaponDB->GetString( hAttrib );
	if( pszHHModel[0] )
	{
		ocs.SetFileName( pszHHModel );
	}

	// Get all specified materials...
	char szCompleteName[256];
	LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", pszDBStructName, WDB_WEAPON_sHHMaterial );
	g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName, ocs.m_Materials[0],
		LTARRAYSIZE(ocs.m_Materials), LTARRAYSIZE(ocs.m_Materials[0]) );

	if( pszHHModel[0] && ocs.m_Materials[0][0] )
	{
		g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );
	}
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
	static CParsedMsg::CToken s_cTok_WeaponKeySound(WEAPON_KEY_SOUND);
	static CParsedMsg::CToken s_cTok_WeaponKeyButeSound(WEAPON_KEY_BUTE_SOUND);
	static CParsedMsg::CToken s_cTok_WeaponKeyLoopSound(WEAPON_KEY_LOOPSOUND);

	if (!m_hParentObject || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	if( !m_pParentWeapon )
		return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	float fRadius = WEAPON_SOUND_RADIUS;

	if( tok == s_cTok_WeaponKeySound )
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			HWEAPONDATA hWpnData = m_pParentWeapon->GetWeaponData();
			if( !hWpnData )
				return;

			HATTRIBUTE hWeaponSoundStruct;
			HRECORD hSR = NULL;
			HATTRIBUTE   hWeaponSound = NULL;
			uint32 nValueIndex = 0;

			// this (currently?) can't be the player, so get the nonlocal sounds...
			// regardless, can't determine if it's the client player so we'll just
			// have to use the nonlocal sound..
			hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWpnData, WDB_WEAPON_NonLocalSoundInfo);

			PlayerSoundId nId = (PlayerSoundId)atoi(pArgList->argv[1]);
			switch (nId)
			{
			case PSI_RELOAD:
			case PSI_RELOAD2:
			case PSI_RELOAD3:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rReloadSnd );
				nValueIndex = nId - PSI_RELOAD;
				break;
			case PSI_SELECT:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSelectSnd );
				break;
			case PSI_DESELECT:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDeselectSnd );
				break;

			case PSI_WEAPON_MISC1:
			case PSI_WEAPON_MISC2:
			case PSI_WEAPON_MISC3:
			case PSI_WEAPON_MISC4:
			case PSI_WEAPON_MISC5:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rMiscSnd );
				nValueIndex = nId - PSI_WEAPON_MISC1;
				break;
			case PSI_FIRE_LOOP:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopSnd );
				break;
			case PSI_FIRE_LOOP_END:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopEndSnd );
				break;

			case PSI_INVALID:
			default : break;
			}
			if (hWeaponSound != NULL)
			{
				hSR = g_pWeaponDB->GetRecordLink(hWeaponSound, nValueIndex);
			}

			if (hSR)
			{
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				g_pServerSoundMgr->PlayDBSoundFromPos(vPos, hSR, SMGR_INVALID_RADIUS, SOUNDPRIORITY_AI_HIGH,
					0, SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPON_IMPACTS);
			}
		}
	}
	else if( tok == s_cTok_WeaponKeyButeSound )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pServerSoundMgr->PlaySoundFromPos( vPos, pArgList->argv[1], NULL, 
				-1.0f, SOUNDPRIORITY_MISC_LOW,  PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER);
		}
	}
	else if( tok == s_cTok_WeaponKeyLoopSound )
	{
		// Handle a looping sound key

		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			HWEAPONDATA hWpnData = m_pParentWeapon->GetWeaponData();
			if( !hWpnData )
				return;

			if( LTStrIEquals( pArgList->argv[1], "STOP" ))
			{
				// Stop the looping sound from playing...

				KillLoopSound();
			}

			HATTRIBUTE hWeaponSoundStruct;
			HRECORD hSR = NULL;
			HATTRIBUTE   hWeaponSound = NULL;
			uint32 nValueIndex = 0;

			// this (currently?) can't be the player, so get the nonlocal sounds...
			// regardless, can't determine if it's the client player so we'll just
			// have to use the nonlocal sound..
			hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWpnData, WDB_WEAPON_NonLocalSoundInfo);


			PlayerSoundId nId = static_cast< PlayerSoundId >( atoi( pArgList->argv[ 1 ] ) );
			switch (nId)
			{
			case PSI_RELOAD:
			case PSI_RELOAD2:
			case PSI_RELOAD3:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, nId - PSI_RELOAD, WDB_WEAPON_rReloadSnd );
				break;
			case PSI_SELECT:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSelectSnd );
				break;
			case PSI_DESELECT:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDeselectSnd );
				break;

			case PSI_WEAPON_MISC1:
			case PSI_WEAPON_MISC2:
			case PSI_WEAPON_MISC3:
			case PSI_WEAPON_MISC4:
			case PSI_WEAPON_MISC5:
				{
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rMiscSnd );
					nValueIndex = nId - PSI_WEAPON_MISC1;
					break;
				}
			case PSI_FIRE_LOOP:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopSnd );
				break;
			case PSI_FIRE_LOOP_END:
				hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopEndSnd );
				break;

			case PSI_INVALID:
			default : break;
			}
			if (hWeaponSound != NULL)
			{
				hSR = g_pWeaponDB->GetRecordLink(hWeaponSound, nValueIndex);
			}

			if( hSR )
			{
				if( !m_hLoopSound || (nId != m_nLoopSoundId) )
				{
					// Stop any previous looping sound...

					KillLoopSound();
					
					// Play the sound immediately localy 
					
					m_hLoopSound = g_pServerSoundMgr->PlayDBSoundFromObject( m_hObject, hSR, SMGR_INVALID_RADIUS, SOUNDPRIORITY_PLAYER_HIGH,
						PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE, SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
						DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER);

					SetNextUpdate( HHW_SOUND_MAX_DURATION, eControlUpdateOnly );
					m_nLoopSoundId = nId;
				}
				
			}

		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHHWeaponModel::KillLoopSound
//
//  PURPOSE:	Stop the looping sound from playing...
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::KillLoopSound( )
{
	m_nLoopSoundId = PSI_INVALID;
	if( m_hLoopSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hLoopSound );
		m_hLoopSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHHWeaponModel::Update
//
//  PURPOSE:	Update to make sure the looping sounds stops if playing for too long...
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::Update()
{
	if( m_hLoopSound )
	{
		double fTimeDelta = m_pParentWeapon->GetLastFireTime().GetElapseTime();
		
		if( HHW_SOUND_MAX_DURATION < fTimeDelta )
		{
			// AI control the logic of which weapon animations to play from 
			// AIWeaponAbstract::UpdateWeaponAnimation().  Changing the animation
			// externally from other code confuses the AI, so AI need to skip this.

			if( !IsAI( m_hParentObject ) )
			{
				m_pParentWeapon->PlayAnimation( m_pParentWeapon->GetPostFireAni(), true, false );
			}
		}
	
		SetNextUpdate( UPDATE_NEXT_FRAME, eControlUpdateOnly );
	}
	else
	{
		SetNextUpdate( UPDATE_NEVER, eControlUpdateOnly );
	}
}
