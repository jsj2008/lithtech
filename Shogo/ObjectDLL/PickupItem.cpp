// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.cpp
//
// PURPOSE : Item that any player can walk across and potentially pick up - 
//			 Implementation
//
// CREATED : 10/27/97
//
// ----------------------------------------------------------------------- //

#include "PickupItem.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "ClientServerShared.h"
#include "RiotServerShell.h"
#include "PlayerObj.h"
#include "CVarTrack.h"

extern CRiotServerShell* g_pRiotServerShellDE;
static CVarTrack g_RespawnScaleTrack;

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		PickupItem
//
//	PURPOSE:	Any in-game object that the player can pick up
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(PickupItem)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SHADOW_FLAG(1, 0)
	ADD_SOLID_FLAG(0, 0)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_STRINGPROP(PickupTriggerTarget, "")
	ADD_STRINGPROP(PickupTriggerMessage, "")
	ADD_BOOLPROP(Rotate, 0)
	ADD_BOOLPROP(Bounce, 0)
	ADD_REALPROP(RespawnTime, 10.0f)
	ADD_STRINGPROP(RespawnSound, "Sounds\\Powerups\\Respawn.wav")
	ADD_STRINGPROP(SoundFile, "")
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS)
	ADD_STRINGPROP(Skin, "")
	ADD_LONGINTPROP(UserFlags, USRFLG_GLOW)
END_CLASS_DEFAULT(PickupItem, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickupItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PickupItem::PickupItem() : BaseClass (OT_MODEL)
{
	m_fRespawnDelay = 10.0f;
	m_bRotate		= DFALSE;
	m_bBounce		= DFALSE;
	m_fYaw			= 0.0f;
	m_fLastTime		= 0.0f;
	m_fBounce		= 0.0f;
	m_bBouncing		= DFALSE;

	m_dwUserFlags	= USRFLG_GLOW;  // Pick up items glow

	m_hstrPickupTriggerTarget	= DNULL;
	m_hstrPickupTriggerMessage	= DNULL;
	m_hstrSoundFile				= DNULL;
	m_hstrRespawnSoundFile		= DNULL;

	m_dwFlags		= 0;

	// Don't need to load/save these...

	m_hClient		= DNULL;
	m_eType			= PIT_UNKNOWN;
	m_bInformClient = DTRUE;
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
	if( !g_pServerDE ) return;

	if (m_hstrPickupTriggerTarget)
	{
		g_pServerDE->FreeString(m_hstrPickupTriggerTarget);
	}
	
	if (m_hstrPickupTriggerMessage)
	{
		g_pServerDE->FreeString(m_hstrPickupTriggerMessage);
	}

	if (m_hstrSoundFile)
	{
		g_pServerDE->FreeString(m_hstrSoundFile);
	}	
	
	if (m_hstrRespawnSoundFile)
	{
		g_pServerDE->FreeString(m_hstrRespawnSoundFile);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD PickupItem::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update((DVector *)pData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HOBJECT hObj = (HOBJECT)pData;

			// If the object is dead, it can't pick up stuff...

			if (IsPlayer(hObj))
			{
				CPlayerObj* pPlayer = (CPlayerObj*) g_pServerDE->HandleToObject(hObj);
				
				if (!pPlayer || pPlayer->IsDead()) break;

				if (pPlayer)
				{
					m_hClient = pPlayer->GetClient();
				}
			}
			else
			{
				m_hClient = DNULL;
				break;
			}

			ObjectTouch(hObj);
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			else if (fData == PRECREATE_STRINGPROP)
			{
				ReadProp(( ObjectCreateStruct * )pData);
				Setup(( ObjectCreateStruct * )pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD PickupItem::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if( g_pServerDE )
	{
		switch(messageID)
		{
			case MID_PICKEDUP:
			{
				PickedUp (hRead);
				break;
			}

			default: break;
		}
	}

	return BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL PickupItem::ReadProp(ObjectCreateStruct *pInfo)
{
	GenericProp genProp;

	if (!pInfo) return DFALSE;

	if( g_pServerDE->GetPropGeneric("PickupTriggerTarget", &genProp ) == DE_OK )
		if( genProp.m_String[0] )
			m_hstrPickupTriggerTarget = g_pServerDE->CreateString( genProp.m_String );

	if( g_pServerDE->GetPropGeneric("PickupTriggerMessage", &genProp ) == DE_OK )
		if( genProp.m_String[0] )
			m_hstrPickupTriggerMessage = g_pServerDE->CreateString( genProp.m_String );

	if( g_pServerDE->GetPropGeneric("SoundFile", &genProp ) == DE_OK )
		if( genProp.m_String[0] )
			m_hstrSoundFile = g_pServerDE->CreateString( genProp.m_String );

	if( g_pServerDE->GetPropGeneric("RespawnSound", &genProp ) == DE_OK )
		if( genProp.m_String[0] )
			m_hstrRespawnSoundFile = g_pServerDE->CreateString( genProp.m_String );

	if( g_pServerDE->GetPropGeneric( "Rotate", &genProp ) == DE_OK )
		m_bRotate = genProp.m_Bool;

	if( g_pServerDE->GetPropGeneric( "Bounce", &genProp ) == DE_OK )
		m_bBounce = genProp.m_Bool;

	if( g_pServerDE->GetPropGeneric( "RespawnTime", &genProp ) == DE_OK )
		m_fRespawnDelay = genProp.m_Float;

	if( g_pServerDE->GetPropGeneric( "UserFlags", &genProp ) == DE_OK )
		m_dwUserFlags = genProp.m_Long;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Setup
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

DBOOL PickupItem::Setup( ObjectCreateStruct *pInfo )
{
	if (!pInfo) return DFALSE;

	// Only spawn once...
	m_fRespawnDelay = 0.0f;

#ifdef SERVER_SIDE_BOUNCING
	// Bounce the item up a little, just to catch attention...
	if (m_bBounce)
	{
		VEC_COPY(m_vRestPos, pInfo->m_Pos);
	
		m_fBounce = 500.0f;
		m_bBouncing = DTRUE;
		VEC_SET(pInfo->m_Pos, m_vRestPos.x, m_vRestPos.y - 10.0f, m_vRestPos.z); 
	}
#endif // SERVER_SIDE_BOUNCING

	// Show yourself!
	pInfo->m_Flags |= FLAG_VISIBLE | FLAG_TOUCH_NOTIFY;

	return DTRUE;
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

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;

	m_dwFlags |= pStruct->m_Flags;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL PickupItem::InitialUpdate(DVector* pMovement)
{
	HMESSAGEWRITE hMessage;

	if(!g_RespawnScaleTrack.IsInitted())
		g_RespawnScaleTrack.Init(GetServerDE(), "RespawnScale", DNULL, 1.0f);

	if (m_bBouncing)
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}


	// Set up our user flags...

	DDWORD dwUserFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_NIGHT_INFRARED;

	// If item rotates or bounces (or is multiplayer and thus respawns), set 
	// a special fx message to do these things on the client...

	if (m_bRotate || m_bBouncing || g_pRiotServerShellDE->GetGameType() != SINGLE)
	{
		hMessage = g_pServerDE->StartSpecialEffectMessage(this);
		g_pServerDE->WriteToMessageByte(hMessage, SFX_PICKUPITEM_ID);
		g_pServerDE->EndMessage(hMessage);
	}

	if (m_bRotate)
	{
		dwUserFlags |= USRFLG_PICKUP_ROTATE;
	}
	
	if (m_bBouncing)
	{
		dwUserFlags |= USRFLG_PICKUP_BOUNCE;
	}

	g_pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags | m_dwUserFlags);

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags | m_dwFlags);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL PickupItem::Update(DVector* pMovement)
{
	// Only need update for bouncers...

	if (m_bBouncing)
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}

	// If we aren't visible it must be time to respawn...

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	if (!(dwFlags & FLAG_VISIBLE))
	{
		g_pServerDE->SetObjectFlags(m_hObject, m_dwFlags | FLAG_VISIBLE);

		// Let the world know what happened...
			
		if (m_hstrRespawnSoundFile)
		{
			DVector vPos;
			g_pServerDE->GetObjectPos(m_hObject, &vPos);
			PlaySoundFromPos(&vPos, g_pServerDE->GetStringData(m_hstrRespawnSoundFile), 
						     600.0f, SOUNDPRIORITY_MISC_HIGH );
		}
	}

	// Check if we've been bouncing...

#ifdef SERVER_SIDE_BOUNCING
	if (m_bBouncing)
	{
		DVector vVel;
		g_pServerDE->GetVelocity( m_hObject, &vVel );

		// Check if bounce almost done...

		if( fabs( vVel.y ) < 1.0f && fabs( m_fBounce ) < 1.0f )
		{
			DVector vZero;
			
			VEC_INIT( vZero );
			m_fBounce = 0.0f;
			m_bBouncing = DFALSE;
			g_pServerDE->SetObjectPos( m_hObject, &m_vRestPos );
			g_pServerDE->SetVelocity( m_hObject, &vZero );
			g_pServerDE->SetAcceleration( m_hObject, &vZero );
		}
		// Continue bouncing...
		else
		{
			DVector vAccel, vPos;

			g_pServerDE->GetObjectPos( m_hObject, &vPos );
			VEC_SET( vAccel, 0.0f, m_fBounce, 0.0f );
			g_pServerDE->SetAcceleration( m_hObject, &vAccel );

			m_fBounce = ( 50.0f ) * ( m_vRestPos.y - vPos.y ) - ( 3.0f ) * vVel.y;
		}
	}
#endif // SERVER_SIDE_BOUNCING

	return DTRUE;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickedUp()
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void PickupItem::PickedUp(HMESSAGEREAD hRead)
{
	// make the item invisible for the correct amount of time

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE & ~FLAG_TOUCH_NOTIFY);


	// If we were touched by a player, our m_hClient data member will be
	// set.  Send a message to the client letting it know that it's been 
	// picked up...

	if (m_hClient && m_bInformClient)
	{
		HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(m_hClient, MID_POWERUP_PICKEDUP);
		g_pServerDE->WriteToMessageByte(hWrite, m_eType);
		g_pServerDE->EndMessage(hWrite);
	}
	

	// Let the world know what happened...
		
	if (m_hstrSoundFile)
	{
		PlaySoundFromObject(m_hObject, g_pServerDE->GetStringData(m_hstrSoundFile), 600.0f, 
			SOUNDPRIORITY_MISC_HIGH, DFALSE, DFALSE, DFALSE, 100, DTRUE);
	}


	// if we're supposed to trigger something, trigger it here
	
	if (m_hstrPickupTriggerTarget && m_hstrPickupTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrPickupTriggerTarget, m_hstrPickupTriggerMessage);
	}

	
	// get the override respawn time - if it's -1.0, use the default

	DFLOAT fRespawn = g_pServerDE->ReadFromMessageFloat (hRead);
	if (fRespawn == -1.0f) fRespawn = m_fRespawnDelay;

	if (fRespawn <= 0.0f || g_pRiotServerShellDE->GetGameType() == SINGLE)
	{
		g_pServerDE->RemoveObject(m_hObject);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, fRespawn / g_RespawnScaleTrack.GetFloat(1.0f));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fRespawnDelay);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fBounce);
	pServerDE->WriteToMessageByte(hWrite, m_bRotate);
	pServerDE->WriteToMessageByte(hWrite, m_bBounce);
	pServerDE->WriteToMessageByte(hWrite, m_bBouncing);
	pServerDE->WriteToMessageDWord(hWrite, m_dwUserFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPickupTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPickupTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSoundFile);
	pServerDE->WriteToMessageHString(hWrite, m_hstrRespawnSoundFile);
	pServerDE->WriteToMessageVector(hWrite, &m_vRestPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fRespawnDelay				= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw						= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastTime					= pServerDE->ReadFromMessageFloat(hRead);
	m_fBounce					= pServerDE->ReadFromMessageFloat(hRead);
	m_bRotate					= pServerDE->ReadFromMessageByte(hRead);
	m_bBounce					= pServerDE->ReadFromMessageByte(hRead);
	m_bBouncing					= pServerDE->ReadFromMessageByte(hRead);
	m_dwUserFlags				= pServerDE->ReadFromMessageDWord(hRead);
	m_dwFlags					= pServerDE->ReadFromMessageDWord(hRead);
	m_hstrPickupTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrPickupTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSoundFile				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrRespawnSoundFile		= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vRestPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void PickupItem::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSoundFile)
	{
		char* pFile = pServerDE->GetStringData(m_hstrSoundFile);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrRespawnSoundFile)
	{
		char* pFile = pServerDE->GetStringData(m_hstrRespawnSoundFile);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}
