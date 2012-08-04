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

LINKFROM_MODULE( RandomSpawner );

#pragma force_active on
BEGIN_CLASS(RandomSpawner)

	ADD_STRINGPROP(Spawner, "")
	ADD_LONGINTPROP(Number, 1)

END_CLASS_DEFAULT(RandomSpawner, BaseClass, NULL, NULL)
#pragma force_active off

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

	m_ahSpawners = debug_newa(LTObjRef, kMaxSpawners);

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

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
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
    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);

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

       SetNextUpdate(m_hObject, UPDATE_NEVER);
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

void RandomSpawner::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bFirstUpdate);
	SAVE_HSTRING(m_hstrSpawner);
    SAVE_DWORD(m_cSpawn);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
        SAVE_HOBJECT(m_ahSpawners[iSpawner]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RandomSpawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RandomSpawner::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_BOOL(m_bFirstUpdate);
    LOAD_HSTRING(m_hstrSpawner);
    LOAD_DWORD(m_cSpawn);

	for ( int iSpawner = 0 ; iSpawner < kMaxSpawners ; iSpawner++ )
	{
        LOAD_HOBJECT(m_ahSpawners[iSpawner]);
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