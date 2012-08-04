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
#include "ServerSoundMgr.h"
#include <stdio.h>
#include "ParsedMsg.h"

extern char g_tokenSpace[];
extern char *g_pTokens[];
extern char *g_pCommandPos;

#define HHW_SOUND_MAX_DURATION	0.5f

LINKFROM_MODULE( HHWeaponModel );


#pragma force_active on
BEGIN_CLASS(CHHWeaponModel)
END_CLASS_DEFAULT_FLAGS(CHHWeaponModel, GameBase, NULL, NULL, CF_HIDDEN)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::CHHWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CHHWeaponModel::CHHWeaponModel() : GameBase(OT_MODEL)
{
    m_hParentObject     = LTNULL;
	m_pParentWeapon		= NULL;
	m_nLoopSoundId		= PSI_INVALID;
	m_hLoopSound		= LTNULL;
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

uint32 CHHWeaponModel::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
		g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, dwAni);
		g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

		g_pLTServer->SetModelAnimation(m_hObject, dwAni);
	}

	g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::Setup()
//
//	PURPOSE:	Setup our weapon data.
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::Setup(CWeapon* pParent, WEAPON const *pWeaponData)
{
	SetParent(pParent);

	if (pWeaponData->szHHModel[0])
	{
		SetObjectFilenames(m_hObject, pWeaponData->szHHModel, pWeaponData->blrHHSkins.GetItem(0));

		if( pParent )
			pParent->HideWeapon( false );
	}
	else if( pParent )
	{
		pParent->HideWeapon( true );
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

	LTFLOAT fRadius = WEAPON_SOUND_RADIUS;

	if( tok == s_cTok_WeaponKeySound )
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
            uint8 nWeaponId = m_pParentWeapon->GetId();
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if (!pWeapon) return;

            char* pFilename = LTNULL;

			PlayerSoundId nId = (PlayerSoundId)atoi(pArgList->argv[1]);
			switch (nId)
			{
				case PSI_RELOAD:
				case PSI_RELOAD2:
				case PSI_RELOAD3:
					pFilename = pWeapon->szReloadSounds[nId - PSI_RELOAD];
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				break;
				case PSI_SELECT:
					pFilename = pWeapon->szSelectSound;
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				break;
				case PSI_DESELECT:
					pFilename = pWeapon->szDeselectSound;
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				break;
				
				case PSI_WEAPON_MISC1:
				case PSI_WEAPON_MISC2:
				case PSI_WEAPON_MISC3:
				case PSI_WEAPON_MISC4:
				case PSI_WEAPON_MISC5:
					pFilename = pWeapon->szMiscSounds[nId - PSI_WEAPON_MISC1];
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				break;

				case PSI_INVALID:
				default : break;
			}

			if (pFilename && pFilename[0])
			{
                LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
                g_pServerSoundMgr->PlaySoundFromPos(vPos, pFilename, fRadius, SOUNDPRIORITY_AI_HIGH);
            }
		}
	}
	else if( tok == s_cTok_WeaponKeyButeSound )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pServerSoundMgr->PlaySoundFromPos( vPos, pArgList->argv[1] );
		}
	}
	else if( tok == s_cTok_WeaponKeyLoopSound )
	{
		// Handle a looping sound key

		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			uint8 nWeaponId = m_pParentWeapon->GetId();
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if (!pWeapon) return;
			
			if( 0 == stricmp( pArgList->argv[1], "STOP" ))
			{
				// Stop the looping sound from playing...

				KillLoopSound();
			}

			char* pBuf = 0;

			PlayerSoundId nId = static_cast< PlayerSoundId >( atoi( pArgList->argv[ 1 ] ) );
			switch( nId )
			{
				case PSI_RELOAD:												// 1
				case PSI_RELOAD2:												// 2
				case PSI_RELOAD3:												// 3
				{
					pBuf = pWeapon->szReloadSounds[ ( nId - PSI_RELOAD ) ];
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				}
				break;

				case PSI_SELECT:												// 4
				{
					pBuf = pWeapon->szSelectSound;			
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				}
				break;	
				
				case PSI_DESELECT:												// 5		
				{
					pBuf = pWeapon->szDeselectSound;
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				}
				break;
				
				case PSI_FIRE:													// 6
				{
					pBuf = pWeapon->szFireSound;			
					fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
				}
				break;
				
				case PSI_DRY_FIRE:												// 7		
				{	
					pBuf = pWeapon->szDryFireSound;			
					fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
				}	
				break;
				
				case PSI_ALT_FIRE:												// 8
				{	
					pBuf = pWeapon->szAltFireSound;			
					fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
				}	
				break;
				
				case PSI_SILENCED_FIRE:											// 9
				{
					pBuf = pWeapon->szSilencedFireSound;	
					fRadius = (LTFLOAT) pWeapon->nFireSoundRadius;
				}	
				break;
				
				case PSI_WEAPON_MISC1:											// 10
				case PSI_WEAPON_MISC2:											// 11
				case PSI_WEAPON_MISC3:											// 12
				case PSI_WEAPON_MISC4:											// 13
				case PSI_WEAPON_MISC5:											// 14
				{
					pBuf = pWeapon->szMiscSounds[nId - PSI_WEAPON_MISC1];
					fRadius = (LTFLOAT)pWeapon->nWeaponSoundRadius;
				}
				break; 
				
				case PSI_INVALID:
				default:
				{
				}
				break;
			}

			if( pBuf && pBuf[0] )
			{
				if( !m_hLoopSound || (nId != m_nLoopSoundId) )
				{
					// Stop any previous looping sound...

					KillLoopSound();
					
					// Play the sound immediately localy 
					
					m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject( m_hObject, pBuf, fRadius, SOUNDPRIORITY_PLAYER_HIGH,
																			PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE );

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
		m_hLoopSound = LTNULL;
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
		float fTimeDelta = g_pLTServer->GetTime() - m_pParentWeapon->GetLastFireTime();
		
		if( HHW_SOUND_MAX_DURATION < fTimeDelta )
		{
			m_pParentWeapon->PlayAnimation( m_pParentWeapon->GetPostFireAni(), true, false, true );
		}
	
		SetNextUpdate( UPDATE_NEXT_FRAME, eControlUpdateOnly );
	}
	else
	{
		SetNextUpdate( UPDATE_NEVER, eControlUpdateOnly );
	}
}