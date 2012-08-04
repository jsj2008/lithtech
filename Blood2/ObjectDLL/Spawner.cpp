// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.cpp
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 6/24/98
//
// ----------------------------------------------------------------------- //

#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "Spawner.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include <mbstring.h>
#include "SoundTypes.h"


BEGIN_CLASS(Spawner)
	ADD_STRINGPROP(SpawnObject, "")
	ADD_VECTORPROP_VAL(MinVelocity, 0, 500, 0)
	ADD_VECTORPROP_VAL(MaxVelocity, 0, 500, 0)
	ADD_STRINGPROP(Sound, "")
	ADD_REALPROP(SoundRadius, 500.0f)
	ADD_BOOLPROP(UseTriggerObjPos, DTRUE)
	ADD_BOOLPROP(StartActive, DFALSE)
	ADD_LONGINTPROP(RespawnCount, -1)
	ADD_REALPROP(RespawnRate, 1)
	ADD_BOOLPROP(CreateRiftEffect, DFALSE)
END_CLASS_DEFAULT(Spawner, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Spawner
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Spawner::Spawner() : B2BaseClass(OT_NORMAL)
{
	VEC_SET(m_vMinVelocity, -100.0f, 400.0f, -100.0f);
	VEC_SET(m_vMaxVelocity, 100.0f, 600.0f, 100.0f);

	m_hstrSpawnObject   = DNULL;
	m_hstrSound			= DNULL;
	m_fSoundRadius		= 500.0f;
	m_bUseTriggerObjPos	= DFALSE;
	m_bStartActive      = DFALSE;
	m_dwRespawnCount    = -1;
	m_fRespawnRate      = 1;
	m_bCreateRiftEffect = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::~Spawner
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Spawner::~Spawner()
{
	if (m_hstrSpawnObject)
		g_pServerDE->FreeString( m_hstrSpawnObject );

	if( m_hstrSound )
 		g_pServerDE->FreeString( m_hstrSound );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::EngineMessageFn
//
//	PURPOSE:	Handler for engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Spawner::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == 1.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			break;
		}
		case MID_INITIALUPDATE:
		{
			if (m_bStartActive)
			{
				g_pServerDE->SetNextUpdate(m_hObject, m_fRespawnRate);
			}
			else
			{
				g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
			}
			DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_SAVEABLE);

			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}


	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ReadProp
//
//	PURPOSE:	Reads spawner properties
//
// ----------------------------------------------------------------------- //

DBOOL Spawner::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("SpawnObject", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrSpawnObject = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("MinVelocity", &genProp) == DE_OK)
	{
		VEC_COPY(m_vMinVelocity, genProp.m_Vec)
	}

	if (g_pServerDE->GetPropGeneric("MaxVelocity", &genProp) == DE_OK)
	{
		VEC_COPY(m_vMaxVelocity, genProp.m_Vec)
	}

	if (g_pServerDE->GetPropGeneric("Sound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrSound = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("SoundRadius", &genProp) == DE_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("UseTriggerObjPos", &genProp) == DE_OK)
	{
		m_bUseTriggerObjPos = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("StartActive", &genProp) == DE_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("CreateRiftEffect", &genProp) == DE_OK)
	{
		m_bCreateRiftEffect = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("RespawnCount", &genProp) == DE_OK)
	{
		m_dwRespawnCount = genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("RespawnRate", &genProp) == DE_OK)
	{
		m_fRespawnRate = genProp.m_Float;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ObjectMessageFn
//
//	PURPOSE:	Handle messages from objects
//
// ----------------------------------------------------------------------- //

DDWORD Spawner::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	switch( messageID )
	{
		case MID_TRIGGER:
		{
			// Read the message's hstring, discard
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString( hRead );

			if (hMsg)
			{
				char* sMsg = g_pServerDE->GetStringData(hMsg);

				// Look for an "activate" message
				if (sMsg)
				{
					if (_mbsicmp((const unsigned char*)sMsg, (const unsigned char*)"START") == 0)
					{
						g_pServerDE->SetNextUpdate(m_hObject, m_fRespawnRate);
						g_pServerDE->FreeString(hMsg);
						return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
					}
					else if (_mbsicmp((const unsigned char*)sMsg, (const unsigned char*)"STOP") == 0)
					{
						g_pServerDE->SetNextUpdate(m_hObject, 0);
						g_pServerDE->FreeString(hMsg);
						return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
					}
					else if (_mbsicmp((const unsigned char*)sMsg, (const unsigned char*)"SINGLE") == 0)
					{
						if (m_dwRespawnCount > 0)
						{
							m_dwRespawnCount--;
							SpawnObject(NULL);
						}

						g_pServerDE->FreeString(hMsg);
						return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
					}
				}

				g_pServerDE->FreeString(hMsg);
			}

			// Spawn the object
			SpawnObject(hSender);

			break;
		}

		default : break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObject
//
//	PURPOSE:	Spawns an object of any class
//
// ----------------------------------------------------------------------- //

HOBJECT SpawnObject( char *pszSpawnString, DVector *pvPos, DRotation *prRot, DVector *pvVel )
{
	char *pszObjClass = NULL;

	if( !g_pServerDE || !pszSpawnString )
		return DNULL;

	// Class name should be the first token in the string, validate it
	pszObjClass = strtok( pszSpawnString, " " );
	if( !pszObjClass )	return DNULL;

	HCLASS hClass = g_pServerDE->GetClass( pszObjClass );
	if( !hClass ) return DNULL;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	VEC_COPY( ocStruct.m_Pos, *pvPos );
	ROT_COPY( ocStruct.m_Rotation, *prRot );

	// Remainder of the string could be string props
	pszSpawnString = strtok( NULL, "" );

	// Create the object...
	HOBJECT hObj = DNULL;
	if (BaseClass *pObj = g_pServerDE->CreateObjectProps( hClass, &ocStruct, pszSpawnString ))
		hObj = g_pServerDE->ObjectToHandle(pObj);

	// Set it's initial velocity
	if (hObj && pvVel) 
		g_pServerDE->SetVelocity(hObj, pvVel);

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Update
//
//	PURPOSE:	Handle updating the spawner
//
// ----------------------------------------------------------------------- //

void Spawner::Update()
{
	// Update the respawn counter...

	if (m_dwRespawnCount >= 1)
	{
		m_dwRespawnCount--;
	}


	// Spawn the object...

	SpawnObject(NULL);


	// Set the next update as necessary...

	if (m_dwRespawnCount != 0)
	{
		g_pServerDE->SetNextUpdate(m_hObject, m_fRespawnRate);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::SpawnObject
//
//	PURPOSE:	Spawns an object
//
// ----------------------------------------------------------------------- //

DBOOL Spawner::SpawnObject(HOBJECT hSender)
{
	// Sanity checks
	if (!m_hstrSpawnObject) return(DFALSE);

	// Local variables
	DVector vPos;
	DRotation rRot;
	DVector vVel;

	VEC_INIT(vVel);

	char szSpawn[MAX_GP_STRING_LEN+1];

	szSpawn[0] = '\0';

	if (m_bUseTriggerObjPos && hSender)
	{
		g_pServerDE->GetObjectPos( hSender, &vPos );
		g_pServerDE->GetObjectRotation( hSender, &rRot );
	}
	else
	{
		g_pServerDE->GetObjectPos( m_hObject, &vPos );
		g_pServerDE->GetObjectRotation( m_hObject, &rRot );
	}

	// Now spawn objects specified in the properties
	if (m_hstrSpawnObject)
	{
		char *pString = g_pServerDE->GetStringData(m_hstrSpawnObject);
		if (!pString) return(DFALSE);

		_mbsncpy((unsigned char*)szSpawn, (const unsigned char*)pString, MAX_GP_STRING_LEN);
		DVector vVel;
		vVel.x = g_pServerDE->Random(m_vMinVelocity.x, m_vMaxVelocity.x);
		vVel.y = g_pServerDE->Random(m_vMinVelocity.y, m_vMaxVelocity.y);
		vVel.z = g_pServerDE->Random(m_vMinVelocity.z, m_vMaxVelocity.z);
		::SpawnObject(szSpawn, &vPos, &rRot, &vVel);
	}

	if (m_bCreateRiftEffect)
	{
		DVector		vU, vR, vF;
		g_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&vPos);
		g_pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

		g_pServerDE->WriteToMessageVector(hMessage, &vPos);
		g_pServerDE->WriteToMessageVector(hMessage, &vF);
		g_pServerDE->WriteToMessageDWord(hMessage, EXP_RIFT_1);

		g_pServerDE->EndMessage(hMessage);
	}

	// Play spawn sound
	if( m_hstrSound )
	{
		char* pSound = g_pServerDE->GetStringData(m_hstrSound);
		if (pSound)
		{
			PlaySoundFromPos( &vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW );
		}
	}

	// All done
	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Save
//
//	PURPOSE:	Saves the spawner object
//
// ----------------------------------------------------------------------- //

void Spawner::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSound);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrSpawnObject);

	g_pServerDE->WriteToMessageVector(hWrite, &m_vMinVelocity);
	g_pServerDE->WriteToMessageVector(hWrite, &m_vMaxVelocity);

	g_pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	g_pServerDE->WriteToMessageFloat(hWrite, m_fRespawnRate);

	g_pServerDE->WriteToMessageByte(hWrite, m_bUseTriggerObjPos);
	g_pServerDE->WriteToMessageByte(hWrite, m_bStartActive);
	g_pServerDE->WriteToMessageByte(hWrite, m_bCreateRiftEffect);

	g_pServerDE->WriteToMessageDWord(hWrite, m_dwRespawnCount);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Load
//
//	PURPOSE:	Loads the spawner object
//
// ----------------------------------------------------------------------- //

void Spawner::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	m_hstrSound       = g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpawnObject = g_pServerDE->ReadFromMessageHString(hRead);

	g_pServerDE->ReadFromMessageVector(hRead, &m_vMinVelocity);
	g_pServerDE->ReadFromMessageVector(hRead, &m_vMaxVelocity);

	m_fSoundRadius = g_pServerDE->ReadFromMessageFloat(hRead);
	m_fRespawnRate = g_pServerDE->ReadFromMessageFloat(hRead);

	m_bUseTriggerObjPos = g_pServerDE->ReadFromMessageByte(hRead);
	m_bStartActive      = g_pServerDE->ReadFromMessageByte(hRead);
	m_bCreateRiftEffect = g_pServerDE->ReadFromMessageByte(hRead);

	m_dwRespawnCount = g_pServerDE->ReadFromMessageDWord(hRead);
}

