// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.cpp
//
// PURPOSE : Item that any player can walk across and potentially pick up -
//			 Implementation
//
// CREATED : 10/27/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupItem.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "CVarTrack.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CharacterHitBox.h"
#include "ServerSoundMgr.h"
#include "PropTypeMgr.h"
#include "VersionMgr.h"

LINKFROM_MODULE( PickupItem );

extern CGameServerShell* g_pGameServerShell;

CVarTrack g_RespawnScaleTrack;

const char* c_aWorldAnimations[] =
{
	"World",
	"BindPos",
};
#define PICKUP_MAX_WORLD_ANIS	2

#define DEFAULT_PICKUP_SOUND	"DefaultItemPickup"
#define DEFAULT_RESPAWN_SOUND	"DefaultItemRespawn"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		PickupItem
//
//	PURPOSE:	Any in-game object that the player can pick up
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(PickupItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_LOCALDIMS | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL(Scale, 1.0f, 1.0f, 1.0f)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SHADOW_FLAG(1, 0)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupCommand, "", PF_NOTIFYCHANGE)
	ADD_BOOLPROP(Rotate, 0)
	ADD_BOOLPROP(Bounce, 0)
	ADD_BOOLPROP(SPRespawn, 0)
	ADD_BOOLPROP(MPRespawn, 1)
	ADD_REALPROP(RespawnTime, 10.0f)
	ADD_STRINGPROP_FLAG(RespawnSound, DEFAULT_RESPAWN_SOUND, PF_FILENAME)
	ADD_STRINGPROP_FLAG(PickupSound, DEFAULT_PICKUP_SOUND, PF_FILENAME)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
	ADD_BOOLPROP_FLAG(TouchPickup, LTFALSE, PF_HIDDEN ) // Hidden for TO2 (we want all pickups to be activated only)
	ADD_BOOLPROP_FLAG(ActivatePickup, LTTRUE, PF_HIDDEN ) // Hidden for TO2 (we want all pickups to be activated only) 
	ADD_BOOLPROP_FLAG(DMTouchPickup, LTTRUE, PF_HIDDEN ) // Hidden for TO2 (we want pickups to be touchable in DM)

	// The Model override specifies a prop type model to use instead of the normal pick up
	// model.  NOTE: it would be nice to get the dims from the model override if specified
	// however currently model dims can only be associated with one attribute and our
	// sub-classes often use this functionality.
	ADD_BOOLPROP_FLAG(UseModelOverride, LTFALSE, 0) 
	ADD_STRINGPROP_FLAG(ModelOverride, "", PF_STATICLIST /*| PF_DIMS | PF_LOCALDIMS*/)
	
	ADD_STRINGPROP_FLAG( WorldAnimation, "World", PF_DYNAMICLIST )
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST)

END_CLASS_DEFAULT_FLAGS_PLUGIN(PickupItem, GameBase, NULL, NULL, CF_HIDDEN, CPickupItemPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( PickupItem )

	CMDMGR_ADD_MSG( ACTIVATE,	1,	NULL,	"ACTIVATE" )
	CMDMGR_ADD_MSG( TEAM, 2, NULL, "TEAM <0, 1, -1>" )

CMDMGR_END_REGISTER_CLASS( PickupItem, GameBase )


LTRESULT CPickupItemPlugin::PreHook_PropChanged( const char *szObjName, 
												 const char *szPropName,
												 const int nPropType,
												 const GenericProp &gpPropValue,
												 ILTPreInterface *pInterface,
												 const char *szModifiers )
{
	// See if we can handle this property...

	if( !_stricmp( "PickupCommand", szPropName ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}
	return LT_UNSUPPORTED;
}

LTRESULT CPickupItemPlugin::PreHook_EditStringList(const char* szRezPath,
												   const char* szPropName,
												   char** aszStrings,
												   uint32* pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLength)
{

	if (_strcmpi("ModelOverride", szPropName) == 0)
	{
		if (m_PropTypeMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );

			return LT_OK;
		}
	}
	else if( _strcmpi( "WorldAnimation", szPropName ) == 0 )
	{
		for( int i = 0; i < PICKUP_MAX_WORLD_ANIS; ++i )
		{
			strcpy( aszStrings[(*pcStrings)++], c_aWorldAnimations[i] );
		}

		return LT_OK;
	}
	else if( _stricmp( "Team", szPropName ) == 0 )
	{
		char szTeam[32] = {0};

		_ASSERT(cMaxStrings > (*pcStrings) + 1);
		strcpy( aszStrings[(*pcStrings)++], "NoTeam" );
		
		for( int i = 0; i < MAX_TEAMS; ++i )
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			sprintf( szTeam, "Team%i", i );
			strcpy( aszStrings[(*pcStrings)++], szTeam );
		}

		return LT_OK;
	}


	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickupItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PickupItem::PickupItem() : GameBase(OT_MODEL)
{
	MakeTransitionable();

	m_fRespawnDelay		= 10.0f;
    m_bRotate			= LTFALSE;
    m_bBounce			= LTFALSE;
	m_dwFlags			= 0;
    m_bMoveToFloor		= LTTRUE;
	m_bTouchPickup		= LTFALSE;
	m_bActivatePickup	= LTTRUE;
	m_bWasPickedUp		= false;

	m_bRespawn = (IsMultiplayerGame( ) ? LTTRUE : LTFALSE);

	// Pick up items glow and can be seen with spy vision.  Also we 
	// assume they are moveable so they won't show bullet holes...

	m_dwUserFlags	= USRFLG_GLOW | /*USRFLG_SPY_VISION |*/ USRFLG_MOVEABLE;  

    m_hstrPickupCommand		= LTNULL;
    m_hstrSoundFile			= LTNULL;
    m_hstrRespawnSoundFile	= LTNULL;
	m_hstrModelOverride		= LTNULL;

    m_hPlayerObj = LTNULL;

	m_vScale.Init(1.0f, 1.0f, 1.0f);

	m_nTeamId = INVALID_TEAM;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::~PickupItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
PickupItem::~PickupItem()
{
	FREE_HSTRING(m_hstrPickupCommand);
	FREE_HSTRING(m_hstrSoundFile);
	FREE_HSTRING(m_hstrRespawnSoundFile);
	FREE_HSTRING(m_hstrModelOverride);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PickupItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			if( !m_bTouchPickup ) break;

			HOBJECT hObj = (HOBJECT)pData;

			// If this is a character hit box, use its object...

			CCharacterHitBox* pHitBox = LTNULL;
			if (IsCharacterHitBox(hObj))
			{
				pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
				if (pHitBox)
				{
					hObj = pHitBox->GetModelObject();
				}
			}


			// If the object is dead, it can't pick up stuff...

			if (IsPlayer(hObj))
			{
                CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);

				if (!pPlayer || pPlayer->IsDead()) break;

				SetPlayerObj(hObj);
			}
			else
			{
                SetPlayerObj(LTNULL);
				break;
			}

			ObjectTouch(hObj);
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pInfo = (ObjectCreateStruct*)pData;
	
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(pInfo);
			}
			else if (fData == PRECREATE_STRINGPROP)
			{
				ReadProp(pInfo);

				// Show ourself...

				pInfo->m_Flags |= FLAG_VISIBLE;
			}

			PostPropRead(pInfo);
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
			
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			
			// We need to reset our sfx message since values
			// could have changed across save versions.

			CreateSpecialFX( );

			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 PickupItem::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_PICKEDUP:
		{
			PickedUp(pMsg);
		}
		break;

		default: break;
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool PickupItem::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");
	static CParsedMsg::CToken s_cTok_Team( "TEAM" );

	if( cMsg.GetArg(0) == s_cTok_Activate )
	{
		// Can't activate before we've respawned.
		if( m_bWasPickedUp )
			return true;

		// If the object is dead, it can't pick up stuff...

		if (IsPlayer(hSender))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);

			if (!pPlayer || pPlayer->IsDead()) return true;

			SetPlayerObj(hSender);

			ObjectTouch(hSender);
		}
		else
		{
			SetPlayerObj(LTNULL);
			return true;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				SetTeamId( nTeamId );
			}
			else
			{
				SetTeamId( INVALID_TEAM );
			}

			return true;
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::ReadProp(ObjectCreateStruct *pInfo)
{
	GenericProp genProp;

    if (!pInfo) return LTFALSE;

    if (g_pLTServer->GetPropGeneric("PickupCommand", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
            m_hstrPickupCommand = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PickupSound", &genProp)  == LT_OK)
	{
		if(genProp.m_String[0])
		{
            m_hstrSoundFile = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("RespawnSound", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
            m_hstrRespawnSoundFile = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	// Only get the value of the ModelOverride attribute if we're using
	// it...
    if (g_pLTServer->GetPropGeneric("UseModelOverride", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			if (g_pLTServer->GetPropGeneric("ModelOverride", &genProp) == LT_OK)
			{
				if(genProp.m_String[0])
				{
					m_hstrModelOverride = g_pLTServer->CreateString(genProp.m_String);
				}
			}
		}
	}

    if (g_pLTServer->GetPropGeneric("Rotate", &genProp) == LT_OK)
	{
		m_bRotate = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Bounce", &genProp) == LT_OK)
	{
		m_bBounce = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		 m_bMoveToFloor = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "TouchPickup", &genProp ) == LT_OK )
	{
		if( IsCoopMultiplayerGameType() )
		{
			m_bTouchPickup = genProp.m_Bool;
		}
	}

	if( g_pLTServer->GetPropGeneric( "DMTouchPickup", &genProp ) == LT_OK )
	{
		if( !IsCoopMultiplayerGameType() )
		{
			m_bTouchPickup = genProp.m_Bool;
		}
	}


	if( g_pLTServer->GetPropGeneric( "ActivatePickup", &genProp ) == LT_OK )
	{
		m_bActivatePickup = genProp.m_Bool;
	}

	if( !IsMultiplayerGame( ))
	{
		if( g_pLTServer->GetPropGeneric( "SPRespawn", &genProp ) == LT_OK )
		{
			m_bRespawn = genProp.m_Bool;
		}
	}
	else
	{
		if( g_pLTServer->GetPropGeneric( "MPRespawn", &genProp ) == LT_OK )
		{
			m_bRespawn = genProp.m_Bool;
		}
	}

	if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK)
	{
		m_fRespawnDelay = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Scale", &genProp) == LT_OK)
	{
		 m_vScale = genProp.m_Vec;
	}

	if( g_pLTServer->GetPropGeneric( "WorldAnimation", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_sWorldAniName = genProp.m_String;
		}
	}

	// Get the team this object belongs to.
	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &genProp ) == LT_OK )
		{
			m_nTeamId = TeamStringToTeamId( genProp.m_String );
		}
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PickupItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags |= (m_bActivatePickup ? FLAG_RAYHIT : 0);
	//pStruct->m_Flags |= (m_bTouchPickup ? FLAG_TOUCH_NOTIFY : 0);

	// [RP] 8/31/02 The above line is commented out because if the object doesn't 
	//		have touch notify set it won't get added to containers and therefore won't 
	//		transition :(  Since we won't even process the touch message unless m_bTouchPickup
	//		is set it should be fine to set this flag to allow pickups to transition.

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;

	m_dwFlags |= pStruct->m_Flags;

	m_dwUserFlags |= (m_bActivatePickup ? USRFLG_CAN_ACTIVATE : 0);

	// Make sure we have a pickedup sound...

	if (!m_hstrSoundFile)
	{
		m_hstrSoundFile = g_pLTServer->CreateString(DEFAULT_PICKUP_SOUND);
	}

	if (!m_hstrRespawnSoundFile)
	{
		m_hstrRespawnSoundFile = g_pLTServer->CreateString(DEFAULT_RESPAWN_SOUND);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::CheckForOverrideModel()
//
//	PURPOSE:	See if we have an override model and if so set us up
//				to use it
//
// ----------------------------------------------------------------------- //

void PickupItem::CheckForOverrideModel(ObjectCreateStruct *pStruct)
{
	// See if we have an override model prop-type, if so get the model
	// information and set it in pStruct

	if (!m_hstrModelOverride) return;

	const char* pName = g_pLTServer->GetStringData(m_hstrModelOverride);
	if (pName)
	{
		PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType((char*)pName);

		if (pPropType && !pPropType->sFilename.empty())
		{
			pPropType->SetupModel(pStruct);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEVER);

	if (!g_RespawnScaleTrack.IsInitted())
	{
        g_RespawnScaleTrack.Init(GetServerDE(), "RespawnScale", LTNULL, 1.0f);
	}


	// Set up our user flags...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, m_dwUserFlags, m_dwUserFlags);


	// set a special fx message so that the client knows about the pickup item...
	// (so it can handle bouncing, rotating, and displaying the proper targeting text...)

	CreateSpecialFX();

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, m_dwFlags);

	// Look for an animation specified in DEdit...

	uint32 dwAni = INVALID_ANI;
	if( !m_sWorldAniName.empty())
	{
		dwAni = g_pLTServer->GetAnimIndex(m_hObject, m_sWorldAniName.c_str());
	}
	
	if (dwAni != INVALID_ANI)
	{
		g_pLTServer->SetModelAnimation(m_hObject, dwAni);
	}
	else
	{
		// If we couldn't find the ani look for the world ani.  ALL PickUps should have a "World" ani...

		dwAni = g_pLTServer->GetAnimIndex( m_hObject, "World" );
		if( dwAni != INVALID_ANI )
		{
			g_pLTServer->SetModelAnimation( m_hObject, dwAni );
		}
	}

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_vScale.x * vDims.x;
	vNewDims.y = m_vScale.y * vDims.y;
	vNewDims.z = m_vScale.z * vDims.z;

    g_pLTServer->ScaleObject(m_hObject, &m_vScale);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);

	// Make sure object starts on floor if the gravity flag is set...

	if(m_bMoveToFloor)
	{
		MoveObjectToFloor(m_hObject);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::Update()
{
   SetNextUpdate(UPDATE_NEVER);

	// If we aren't visible it must be time to respawn...

	if( m_bWasPickedUp )
	{
		Respawn( );
		
		m_bWasPickedUp = false;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickedUp()
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void PickupItem::PickedUp(ILTMessage_Read *)
{
	// Let the world know what happened...

	PlayPickedupSound();


	// Clear our player obj, we no longer need this link...

    SetPlayerObj(LTNULL);


	// If we're supposed to process a command, do it here...

	if (m_hstrPickupCommand)
	{
		const char *pCmd = g_pLTServer->GetStringData( m_hstrPickupCommand );

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}

	if (!m_bRespawn)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
	{
		// Make the item invisible until the next update

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);

		//if (m_bTouchPickup)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY);
		}
	
		// If we're activateable, turn of the relative flags...

		if (m_bActivatePickup)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_RAYHIT);
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
		}

	    SetNextUpdate(m_fRespawnDelay / g_RespawnScaleTrack.GetFloat(1.0f));
	}

	// Consider ourselves picked up.
	m_bWasPickedUp = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PlayPickedupSound()
//
//	PURPOSE:	Play the picked up sound
//
// ----------------------------------------------------------------------- //

void PickupItem::PlayPickedupSound()
{
	if (m_hstrSoundFile)
	{
		if (m_hPlayerObj)
		{
			// Play it in the player's head...

            g_pServerSoundMgr->PlaySoundFromObject(m_hPlayerObj, g_pLTServer->GetStringData(m_hstrSoundFile), 600.0f, SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_CLIENTLOCAL);
		}
		else  // Play from pos, more efficient...
		{
            LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pServerSoundMgr->PlaySoundFromPos(vPos, g_pLTServer->GetStringData(m_hstrSoundFile), 600.0f, SOUNDPRIORITY_MISC_HIGH);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetPlayerObj
//
//	PURPOSE:	Set our player obj data member
//
// ----------------------------------------------------------------------- //

void PickupItem::SetPlayerObj(HOBJECT hObj)
{
	m_hPlayerObj = hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::CreateSpecialFX
//
//	PURPOSE:	Send the special fx message for this object
//
// ----------------------------------------------------------------------- //

void PickupItem::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_PICKUPITEM_ID);
		cMsg.Writebool(!!m_bRotate);
		cMsg.Writebool(!!m_bBounce);
		cMsg.WriteString( m_sClientFX.c_str() );
		cMsg.Writeuint8(m_nTeamId);
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetClientFX
//
//	PURPOSE:	Set the clientfx for this pickupitem...
//
// ----------------------------------------------------------------------- //

void PickupItem::SetClientFX( const char *pszFX )
{
	m_sClientFX = pszFX;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_CLIENTFX );
	cMsg.WriteString( m_sClientFX.c_str() );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

	CreateSpecialFX( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetTeamId
//
//	PURPOSE:	Set the teamid for this pickupitem...
//
// ----------------------------------------------------------------------- //

void PickupItem::SetTeamId( uint8 nTeamId )
{
	m_nTeamId = nTeamId;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_TEAMID );
	cMsg.Writeuint8( m_nTeamId );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

	CreateSpecialFX( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Respawn
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void PickupItem::Respawn( )
{
	// Make us visible...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, 0, FLAG2_FORCETRANSLUCENT );

	// If we're touch activateable, make us touchable...

	// [RP] 8/31/02 We need this flag to transition...

	//if (m_bTouchPickup)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY);
	}

	// If we're activateable, turn on the relative flags...

	if (m_bActivatePickup)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
	}

 	// Let the world know what happened...

	if (m_hstrRespawnSoundFile)
	{
        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
        g_pServerSoundMgr->PlaySoundFromPos(vPos, g_pLTServer->GetStringData(m_hstrRespawnSoundFile),
						 600.0f, SOUNDPRIORITY_MISC_HIGH);
    }
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_HOBJECT(m_hPlayerObj);

	SAVE_FLOAT(m_fRespawnDelay);
	SAVE_BOOL(m_bRotate);
	SAVE_BOOL(m_bBounce);
	SAVE_BOOL(m_bRespawn);
	SAVE_DWORD(m_dwUserFlags);
	SAVE_DWORD(m_dwFlags);
	SAVE_HSTRING(m_hstrPickupCommand);
    SAVE_HSTRING(m_hstrSoundFile);
    SAVE_HSTRING(m_hstrRespawnSoundFile);
	SAVE_HSTRING(m_hstrModelOverride);
	SAVE_VECTOR(m_vScale);
	SAVE_BOOL(m_bTouchPickup);
	SAVE_BOOL(m_bActivatePickup);
	SAVE_bool(m_bWasPickedUp);
	SAVE_CHARSTRING(m_sWorldAniName.c_str());
	SAVE_BYTE(m_nTeamId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hPlayerObj);
    LOAD_FLOAT(m_fRespawnDelay);
    LOAD_BOOL(m_bRotate);
    LOAD_BOOL(m_bBounce);
	LOAD_BOOL(m_bRespawn);
    LOAD_DWORD(m_dwUserFlags);
    LOAD_DWORD(m_dwFlags);
    LOAD_HSTRING(m_hstrPickupCommand);
    LOAD_HSTRING(m_hstrSoundFile);
    LOAD_HSTRING(m_hstrRespawnSoundFile);
	LOAD_HSTRING(m_hstrModelOverride);
    LOAD_VECTOR(m_vScale);
	LOAD_BOOL(m_bTouchPickup);
	LOAD_BOOL(m_bActivatePickup);
	LOAD_bool(m_bWasPickedUp);

	char szString[1024] = {0};
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sWorldAniName = szString;

	if( g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_2 )
	{
		LOAD_BYTE(m_nTeamId);
	}
}

