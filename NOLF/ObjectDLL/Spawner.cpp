// ----------------------------------------------------------------------- //
//
// MODULE  : Spawner.cpp
//
// PURPOSE : Spawner class - implementation
//
// CREATED : 12/31/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Spawner.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnObject
//
//	PURPOSE:	Spawns an object
//
// ----------------------------------------------------------------------- //

BaseClass *SpawnObject(char *pszSpawn, const LTVector& vPos, const LTRotation& rRot)
{
    if (!g_pLTServer || !pszSpawn) return LTNULL;

	// Pull the class name out of the spawn string...
	char* pszClassName = strtok(pszSpawn, " ");
    if (!pszClassName) return LTNULL;

    HCLASS hClass = g_pLTServer->GetClass(pszClassName);
    if (!hClass) return LTNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, vPos);
    theStruct.m_Rotation = rRot;

	// Set the string to be the rest of the string...
	char* pszProps = strtok(NULL, "");

	// Allocate an object...
    return (BaseClass *)g_pLTServer->CreateObjectProps(hClass, &theStruct, pszProps);
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
    m_hstrDefaultSpawn = LTNULL;
    m_hstrSpawnSound = LTNULL;
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
        g_pLTServer->FreeString( m_hstrDefaultSpawn );

	if( m_hstrSpawnSound )
        g_pLTServer->FreeString( m_hstrSpawnSound );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Spawner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
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

LTBOOL Spawner::InitialUpdate()
{
    g_pLTServer->SetNextUpdate(m_hObject, (LTFLOAT)0.0f);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric( "DefaultSpawn", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrDefaultSpawn = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "SpawnSound", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrSpawnSound = g_pLTServer->CreateString( genProp.m_String );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::PostPropRead
//
//	PURPOSE:	Modifies properties after reading from level info.
//
// ----------------------------------------------------------------------- //

LTBOOL Spawner::PostPropRead( ObjectCreateStruct *pStruct )
{
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Spawner::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Spawner::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
    LTVector vPos;
    LTRotation rRot;
	char szSpawn[501];

	switch( messageID )
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if (_stricmp(szMsg, "default") == 0)
			{
                szMsg = g_pLTServer->GetStringData(m_hstrDefaultSpawn);
			}

			strncpy( szSpawn, szMsg, 500 );

            g_pLTServer->GetObjectPos( m_hObject, &vPos );
            g_pLTServer->GetObjectRotation( m_hObject, &rRot );

			// Play spawn sound...
			if ( m_hstrSpawnSound )
			{
                char* pSound = g_pLTServer->GetStringData(m_hstrSpawnSound);
                g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
            }

			SpawnObject( szSpawn, vPos, rRot );
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

void Spawner::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    ILTServer* pServerDE = GetServerDE();
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

void Spawner::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    ILTServer* pServerDE = GetServerDE();
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
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

    char* pFile = LTNULL;
	if (m_hstrSpawnSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSpawnSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}