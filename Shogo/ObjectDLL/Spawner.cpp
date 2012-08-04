// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.cpp
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 12/31/97
//
// ----------------------------------------------------------------------- //

#include "Spawner.h"
#include "cpp_server_de.h"
#include "RiotMsgIds.h"
#include "RiotObjectUtilities.h"



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObject
//
//	PURPOSE:	Spawns an object
//
// ----------------------------------------------------------------------- //

BaseClass *SpawnObject( char *pszSpawn, DVector *pvPos, DRotation *prRot )
{
	HCLASS hClass;
	char *pszClassName;

	if( !g_pServerDE || !pszSpawn )
		return DNULL;

	// Pull the class name out of the spawn string...
	pszClassName = strtok( pszSpawn, " " );
	if( !pszClassName )
		return DNULL;
	hClass = g_pServerDE->GetClass( pszClassName );
	if( !hClass )
		return DNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY( theStruct.m_Pos, *pvPos );
	ROT_COPY( theStruct.m_Rotation, *prRot );

	// Set the string to be the rest of the string...
	pszSpawn = strtok( NULL, "" );

	// Allocate an object...
	return ( BaseClass * )g_pServerDE->CreateObjectProps( hClass, &theStruct, pszSpawn );
}



BEGIN_CLASS(Spawner)
	ADD_STRINGPROP( DefaultSpawn, "" )
	ADD_STRINGPROP( SpawnSound, "" )
	ADD_REALPROP( SoundRadius, 500.0f )
END_CLASS_DEFAULT(Spawner, BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Spawner
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Spawner::Spawner() : BaseClass(OT_NORMAL)
{
	m_hstrDefaultSpawn = DNULL;
	m_hstrSpawnSound = DNULL;
	m_fSoundRadius = 500.0f;
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
	if( m_hstrDefaultSpawn )
		g_pServerDE->FreeString( m_hstrDefaultSpawn );

	if( m_hstrSpawnSound )
 		g_pServerDE->FreeString( m_hstrSpawnSound );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Spawner::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			break;
		}
		case MID_INITIALUPDATE:
		{
			InitialUpdate();
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
//	ROUTINE:	Spawner::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL Spawner::InitialUpdate()
{
	g_pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.0f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

DBOOL Spawner::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	if ( g_pServerDE->GetPropGeneric( "DefaultSpawn", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrDefaultSpawn = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "SpawnSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSpawnSound = g_pServerDE->CreateString( genProp.m_String );

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::PostPropRead
//
//	PURPOSE:	Modifies properties after reading from level info.
//
// ----------------------------------------------------------------------- //

DBOOL Spawner::PostPropRead( ObjectCreateStruct *pStruct )
{
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Spawner::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	DVector vPos;
	DRotation rRot;
	char szSpawn[501];

	switch( messageID )
	{
		case MID_TRIGGER:
		{
			szSpawn[0] = '\0';
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString( hRead );
			if (!hMsg) break;

			char* pMsg = g_pServerDE->GetStringData(hMsg);
			if (_stricmp(pMsg, "default") == 0)
			{
				pMsg = g_pServerDE->GetStringData(m_hstrDefaultSpawn);
			}

			strncpy( szSpawn, pMsg, 500 );

			g_pServerDE->FreeString(hMsg);

			g_pServerDE->GetObjectPos( m_hObject, &vPos );
			g_pServerDE->GetObjectRotation( m_hObject, &rRot );

			// Play spawn sound...
			if( m_hstrSpawnSound )
			{
				char* pSound = g_pServerDE->GetStringData(m_hstrSpawnSound);
				PlaySoundFromPos( &vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW );
			}
			
			SpawnObject( szSpawn, &vPos, &rRot );
			break;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Spawner::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpawnSound);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrDefaultSpawn);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Spawner::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_fSoundRadius		= pServerDE->ReadFromMessageFloat(hRead);	
	m_hstrSpawnSound	= pServerDE->ReadFromMessageHString(hRead);	
	m_hstrDefaultSpawn	= pServerDE->ReadFromMessageHString(hRead);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void Spawner::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;
	if (m_hstrSpawnSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSpawnSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}
