// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSpawner.h
//
// PURPOSE : RandomSpawner - Implementation
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RandomSpawner.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(RandomSpawner)

	ADD_STRINGPROP(Spawner, "")
	ADD_LONGINTPROP(Number, 1)

END_CLASS_DEFAULT(RandomSpawner, BaseClass, NULL, NULL)

// Statics

const char s_szSpawnTrigger[] = "DEFAULT";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::RandomSpawner()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RandomSpawner::RandomSpawner() : BaseClass()
{
    m_bFirstUpdate = LTTRUE;

    m_hstrSpawner = LTNULL;
	m_cSpawn = 0;

	m_ahSpawners = debug_newa(HOBJECT, kMaxSpawners);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
        m_ahSpawners[iSpawner] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::~RandomSpawner()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

RandomSpawner::~RandomSpawner()
{
	FREE_HSTRING(m_hstrSpawner);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
		if ( m_ahSpawners[iSpawner] )
		{
            g_pLTServer->BreakInterObjectLink(m_hObject, m_ahSpawners[iSpawner]);
		}
	}

	debug_deletea(m_ahSpawners);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 RandomSpawner::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
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
			HOBJECT hObject = (HOBJECT)pData;

			for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
			{
				if ( hObject == m_ahSpawners[iSpawner] )
				{
                    m_ahSpawners[iSpawner] = LTNULL;
				}
			}

			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 RandomSpawner::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::TriggerMsg()
//
//	PURPOSE:	Handler for RandomSpawner trigger messages.
//
// --------------------------------------------------------------------------- //

void RandomSpawner::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL RandomSpawner::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric( "Spawner", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrSpawner = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "Number", &genProp ) == LT_OK )
		m_cSpawn = genProp.m_Long;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void RandomSpawner::PostPropRead(ObjectCreateStruct *pStruct)
{
	if ( !pStruct ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL RandomSpawner::InitialUpdate()
{
    g_pLTServer->SetNextUpdate(m_hObject, 0.01f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL RandomSpawner::Update()
{
	if ( m_bFirstUpdate )
	{
		Setup();
		Spawn();

        m_bFirstUpdate = LTFALSE;

        g_pLTServer->SetNextUpdate(m_hObject, 0.00f);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Save(HMESSAGEWRITE hWrite)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

    g_pLTServer->WriteToMessageDWord(hWrite, m_bFirstUpdate);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpawner);
    g_pLTServer->WriteToMessageDWord(hWrite, m_cSpawn);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
        g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_ahSpawners[iSpawner]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Load(HMESSAGEREAD hRead)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

    m_bFirstUpdate = (LTBOOL)g_pLTServer->ReadFromMessageDWord(hRead);
    m_hstrSpawner = g_pLTServer->ReadFromMessageHString(hRead);
    m_cSpawn = g_pLTServer->ReadFromMessageDWord(hRead);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
        g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_ahSpawners[iSpawner]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Setup()
//
//	PURPOSE:	Setup
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Setup()
{
    HCLASS  hSpawner = g_pLTServer->GetClass("Spawner");
	_ASSERT(hSpawner);
	if  ( !hSpawner ) return;

    const char* szSpawnerBase = g_pLTServer->GetStringData(m_hstrSpawner);
	_ASSERT(szSpawnerBase);
	if ( !szSpawnerBase ) return;

	int cSpawners = 0;

	while ( cSpawners <= kMaxSpawners )
	{
		char szSpawner[256];
		sprintf(szSpawner, "%s%2.2d", szSpawnerBase, cSpawners);

		ObjArray <HOBJECT, 1> objArray;
        g_pLTServer->FindNamedObjects(szSpawner,objArray);
		if(!objArray.NumObjects()) break;

		HOBJECT hObject = objArray.GetObject(0);

		if ( hObject )
		{
			m_ahSpawners[cSpawners++] = hObject;
            g_pLTServer->CreateInterObjectLink(m_hObject, hObject);
		}
		else
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Spawn()
//
//	PURPOSE:	Spawn
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Spawn()
{
    LTBOOL abSpawned[kMaxSpawners];
    memset(abSpawned, LTFALSE, sizeof(abSpawned));

	for ( int iSpawn = 0 ; iSpawn < m_cSpawn ; iSpawn++ )
	{
		// Keep picking a random one until we have one that hasn't spawned yet

		int iSafety = 50000;
		int iSpawner = GetRandom(0, kMaxSpawners-1);
		while ( (abSpawned[iSpawner] || !m_ahSpawners[iSpawner]) && (--iSafety > 0) )
		{
			iSpawner = GetRandom(0, kMaxSpawners-1);
		}

        abSpawned[iSpawner] = LTTRUE;

		// Trigger the spawner to spawn the default object

        SendTriggerMsgToObject(this, m_ahSpawners[iSpawner], LTFALSE, (char*)s_szSpawnTrigger);
	}
}