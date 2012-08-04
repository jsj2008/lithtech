// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObject.cpp
//
// PURPOSE : Item that any player can walk across and potentially pick up - 
//			 Implementation
//
// CREATED : 10/27/97
//
// ----------------------------------------------------------------------- //

#include "generic_msg_de.h"
#include "PickupObject.h"
#include "cpp_server_de.h"
#include "trigger.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"
#include "SfxMsgIds.h"
#include "BloodServerShell.h"
#include <mbstring.h>
#include "SoundTypes.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		PickupObject
//
//	PURPOSE:	Any in-game object that the player can pick up
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(PickupObject)
	ADD_STRINGPROP(PickupTriggerTarget, "")
	ADD_STRINGPROP(PickupTriggerMessage, "")
END_CLASS_DEFAULT(PickupObject, B2BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PickupObject()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PickupObject::PickupObject() : B2BaseClass (OT_MODEL)
{
	m_nType						= 0;
	m_fValue					= 0;
	m_fValueMult				= 1.0f;
	m_fRespawnTime				= 0.0f;
	m_bRotate					= DFALSE;
	m_bBounce					= DFALSE;
	m_fYaw						= 0.0f;
	m_fLastTime					= 0.0f;
	m_fBounce					= 0.0f;
	m_bBouncing					= DFALSE;

	m_hstrPickupTriggerTarget	= DNULL;
	m_hstrPickupTriggerMessage	= DNULL;
	m_hstrObjectName			= DNULL;
	m_hstrDisplayName			= DNULL;
	m_szObjectName				= DNULL;
	m_nNameID					= 0;
	m_szFile					= DNULL;

	// Set respawn sound...
	m_szRespawnSound = "sounds\\powerups\\powerup2.wav";

	// Set pickup sound...
	m_szPickupSound = "sounds\\powerups\\powerup4.wav";

	m_bFirstUpdate				= DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::~PickupObject()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PickupObject::~PickupObject()
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

	if (m_hstrObjectName)
	{
		g_pServerDE->FreeString(m_hstrObjectName);
	}

	if( m_hstrDisplayName )
	{
		g_pServerDE->FreeString( m_hstrDisplayName );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD PickupObject::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	if( g_pServerDE )
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
				ObjectTouch((HOBJECT)pData);
				break;
			}

			case MID_PRECREATE:
			{
				DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

				if (fData == PRECREATE_WORLDFILE )
					ReadProp((ObjectCreateStruct*)pData);
				if( fData == PRECREATE_STRINGPROP)
				{
					ReadProp((ObjectCreateStruct*)pData);
				}

				PostPropRead((ObjectCreateStruct*)pData);
				return dwRet;
			}

			case MID_INITIALUPDATE:
			{
				if (fData != INITIALUPDATE_SAVEGAME)
				{
					InitialUpdate((DVector *)pData);

				}
				if (fData == INITIALUPDATE_WORLDFILE)
				{
					MoveObjectToGround(m_hObject);
				}
				else if( fData == PRECREATE_STRINGPROP)
				{
					DVector vPos;
					g_pServerDE->GetObjectPos(m_hObject, &vPos);
					SpawnItem( &vPos );
				}

				CacheFiles();
				
				break;
			}

			case MID_SAVEOBJECT:
			{
				Save((HMESSAGEWRITE)pData, (DDWORD)fData);
				break;
			}

			case MID_LOADOBJECT:
			{
				Load((HMESSAGEREAD)pData, (DDWORD)fData);
				break;
			}

			default : break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD PickupObject::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
/*
			case MID_TRIGGER:
			{
				HandleTrigger(hSender, hRead);
				break;
			}
*/
			default: break;
		}
	}

	return B2BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL PickupObject::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("PickupTriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrPickupTriggerTarget = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("PickupTriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrPickupTriggerMessage = g_pServerDE->CreateString(genProp.m_String);
	}
/*
	if (g_pServerDE->GetPropGeneric("Rotate", &genProp) == DE_OK)
	{
		m_bRotate = genProp.m_Bool;
	}
*/
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PickupObject::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_VISIBLE | FLAG_GRAVITY;
	pStruct->m_Flags &= ~FLAG_SHADOW;
	m_dwFlags = pStruct->m_Flags;

	if (!m_hstrObjectName && m_szObjectName)
		m_hstrObjectName = g_pServerDE->CreateString(m_szObjectName);

#ifdef _ADD_ON
	if(m_nType == WEAP_COMBATSHOTGUN || m_nType == WEAP_FLAYER)
	{
		if (m_szFile)
		{
			_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models_ao/Powerups_ao/");
			_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)m_szFile);
			_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)".abc");
		}

		// Set the skin filename
		if (m_szFile)
		{
			_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins_ao/Powerups_ao/");
			_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)m_szFile);
			_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)".dtx");
		}
	}
	else
	{
		if (m_szFile)
		{
			_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models/Powerups/");
			_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)m_szFile);
			_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)".abc");
		}

		// Set the skin filename
		if (m_szFile)
		{
			_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins/Powerups/");
			_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)m_szFile);
			_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)".dtx");
		}
	}
#else
	// Set the model filename
	if (m_szFile)
	{
		_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models/Powerups/");
		_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)m_szFile);
		_mbscat((unsigned char*)pStruct->m_Filename, (const unsigned char*)".abc");
	}

	// Set the skin filename
	if (m_szFile)
	{
		_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins/Powerups/");
		_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)m_szFile);
		_mbscat((unsigned char*)pStruct->m_SkinName, (const unsigned char*)".dtx");
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL PickupObject::InitialUpdate(DVector* pMovement)
{
	if (m_bBouncing)
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}

	// Mark this object as savable
	DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_SAVEABLE | USERFLG_NIGHTGOGGLESGLOW;

	// If we are a rotator, set a flag
	if(m_bRotate)
		dwFlags |= USRFLG_PICKUPOBJ_ROTATE;
	
	g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	// Set some minimum dims, and make sure x and z are the same.
	DVector vDims;
	g_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, g_pServerDE->GetModelAnimation(m_hObject));
	if (vDims.x < 10.0f)
		vDims.x = 10.0f;
	if (vDims.x > vDims.z)
		vDims.z = vDims.x;
//	g_pServerDE->BPrint("Setting PU dims %f,%f,%f", VEC_EXPAND(vDims));
	if (g_pServerDE->SetObjectDims2(m_hObject, &vDims) == DE_ERROR)
		g_pServerDE->SetObjectDims(m_hObject, &vDims);

	SendEffectMessage();

	g_pServerDE->SetDeactivationTime(m_hObject, 1.0f);

	if( m_hstrDisplayName )
		g_pServerDE->FreeString( m_hstrDisplayName );

	// Get the name from the resources if there is one, otherwise use the object name
	if(m_nNameID)
	{
//		g_pServerDE->FreeString(m_hstrObjectName);
		m_hstrDisplayName = g_pServerDE->FormatString(m_nNameID);
	}
	else
		m_hstrDisplayName = g_pServerDE->CopyString( m_hstrObjectName );

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL PickupObject::Update(DVector* pMovement)
{
	if (!g_pServerDE) return DFALSE;

	// Only need update for bouncers...

	if (m_bBouncing)
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
	}

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	// If we aren't visible we've been hit and we're now respawning...
	if (!(dwFlags & FLAG_VISIBLE))
	{
		g_pServerDE->SetObjectFlags(m_hObject, m_dwFlags );
		if (m_szRespawnSound)
		{
			DVector vPos;
			g_pServerDE->GetObjectPos(m_hObject, &vPos);
			PlaySoundFromPos( &vPos, m_szRespawnSound, 300.0f, SOUNDPRIORITY_MISC_HIGH);
		}
	}

	// Check if we've been bouncing...
	if( m_bBouncing )
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

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PickedUp()
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void PickupObject::PickedUp (HMESSAGEREAD hRead)
{
	// get the override respawn time - if it's -1.0, use the default

	float nRespawn = g_pServerDE->ReadFromMessageFloat (hRead);
	if (nRespawn == -1.0f) nRespawn = m_fRespawnTime;

	// make the item invisible for the correct amount of time

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE & ~FLAG_TOUCH_NOTIFY );

	if( nRespawn <= 0.0f || g_pBloodServerShell->GetGameType() == GAMETYPE_SINGLE)
		g_pServerDE->RemoveObject( m_hObject );
	else
		g_pServerDE->SetNextUpdate(m_hObject, (DFLOAT) nRespawn);

	// Let the world know what happened...
		
	if (m_szPickupSound)
	{
		DVector vPos;
		g_pServerDE->GetObjectPos( m_hObject, &vPos );
		PlaySoundFromPos( &vPos, m_szPickupSound, 300.0f, SOUNDPRIORITY_MISC_HIGH);
	}

	// if we're supposed to trigger something, trigger it here
	
	if (m_hstrPickupTriggerTarget && m_hstrPickupTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrPickupTriggerTarget, m_hstrPickupTriggerMessage);
	}
}
	

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

void PickupObject::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

	if( _mbsicmp((const unsigned char*) pszMessage, (const unsigned char*)"PropSpawn" ) == 0 )
	{
		g_pServerDE->GetObjectPos( hSender, &m_vRestPos );
		SpawnItem( &m_vRestPos );
	}
	
	g_pServerDE->FreeString( hMsg );
}
*/
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::SpawnItem()
//
//	PURPOSE:	Spawn item.
//
// ----------------------------------------------------------------------- //

void PickupObject::SpawnItem( DVector *pvRestPos )
{
	DVector vPos;

	if( m_bBounce )
	{
		VEC_COPY( m_vRestPos, *pvRestPos );

		// Bounce the item up a little, just to catch attention...
		m_fBounce = 500.0f;
		m_bBouncing = DTRUE;
		VEC_SET( vPos, m_vRestPos.x, m_vRestPos.y + 10.0f, m_vRestPos.z ); 
		g_pServerDE->MoveObject( m_hObject, &vPos );
	}

	// Only spawn once...
	m_fRespawnTime = 0.0f;

	// Show yourself!
	m_dwFlags |= FLAG_VISIBLE | FLAG_TOUCH_NOTIFY; // | FLAG_GRAVITY /* | FLAG_SHADOW */;
	g_pServerDE->SetObjectFlags(m_hObject, m_dwFlags );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void PickupObject::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// {MD 9/23/98}
	if(!(pServerDE->GetServerFlags() & SS_CACHING))
		return;

	if (m_szPickupSound)
	{
		g_pServerDE->CacheFile(FT_SOUND, m_szPickupSound);
	}

	if (m_szRespawnSound)
	{
		g_pServerDE->CacheFile(FT_SOUND ,m_szRespawnSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::SendEffectMessage
//
//	PURPOSE:	Sends a pickupobject special effect to the client
//
// ----------------------------------------------------------------------- //

void PickupObject::SendEffectMessage()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_POWERUP_ID);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PickupObject::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();

	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageByte(hWrite, m_nType);
	pServerDE->WriteToMessageFloat(hWrite, m_fValue);
	pServerDE->WriteToMessageFloat(hWrite, m_fRespawnTime);
	pServerDE->WriteToMessageByte(hWrite, m_bRotate);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime - fTime);
	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageFloat(hWrite, m_fBounce);
	pServerDE->WriteToMessageByte(hWrite, m_bBouncing);
	pServerDE->WriteToMessageVector(hWrite, &m_vRestPos);

	pServerDE->WriteToMessageHString(hWrite, m_hstrPickupTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPickupTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrObjectName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDisplayName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupObject::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	DFLOAT fTime = pServerDE->GetTime();

	m_bFirstUpdate				= pServerDE->ReadFromMessageByte(hRead);
	m_nType						= pServerDE->ReadFromMessageByte(hRead);
	m_fValue					= pServerDE->ReadFromMessageFloat(hRead);
	m_fRespawnTime				= pServerDE->ReadFromMessageFloat(hRead);
	m_bRotate					= pServerDE->ReadFromMessageByte(hRead);
	m_fYaw						= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastTime					= pServerDE->ReadFromMessageFloat(hRead) + fTime;
	m_dwFlags					= pServerDE->ReadFromMessageDWord(hRead);
	m_fBounce					= pServerDE->ReadFromMessageFloat(hRead);
	m_bBouncing					= pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vRestPos);

	m_hstrPickupTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrPickupTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrObjectName			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDisplayName			= pServerDE->ReadFromMessageHString(hRead);
}
