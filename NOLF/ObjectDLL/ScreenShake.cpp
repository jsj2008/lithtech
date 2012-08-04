// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenShake.cpp
//
// PURPOSE : ScreenShake class - implementation
//
// CREATED : 1/25/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenShake.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "PlayerObj.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"

const int c_nINFINITE_SHAKES = -1;
const LTFLOAT c_fENTIRE_LEVEL = -1.0f;


BEGIN_CLASS(ScreenShake)
	ADD_VECTORPROP_VAL(Amount, 1.0f, 1.0f, 1.0f)
	ADD_LONGINTPROP(NumShakes, c_nINFINITE_SHAKES)
	ADD_REALPROP(Frequency, 1.0f)
	ADD_REALPROP_FLAG(AreaOfEffect, c_fENTIRE_LEVEL, PF_RADIUS)
	ADD_STRINGPROP_FLAG(ShakeSound, "", PF_FILENAME)
	ADD_REALPROP_FLAG(SoundRadius, 500.0f, PF_RADIUS)
END_CLASS_DEFAULT(ScreenShake, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::ScreenShake
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

ScreenShake::ScreenShake() : BaseClass(OT_NORMAL)
{
	VEC_SET(m_vAmount, 1.0f, 1.0f, 1.0f);
	m_nNumShakes	= c_nINFINITE_SHAKES;
	m_fAreaOfEffect = c_fENTIRE_LEVEL;
	m_fFrequency	= 1.0f;
    m_hstrSound     = LTNULL;
	m_fSoundRadius	= 500.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::~ScreenShake
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ScreenShake::~ScreenShake()
{
	if (m_hstrSound)
	{
        g_pLTServer->FreeString(m_hstrSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ScreenShake::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
			CacheFiles();
		}
		break;

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
//	ROUTINE:	ScreenShake::Update
//
//	PURPOSE:	Do updating
//
// ----------------------------------------------------------------------- //

void ScreenShake::Update()
{
	if (--m_nNumShakes > 0)
	{
        SetNextUpdate(m_hObject, m_fFrequency);
	}

    LTVector vMyPos;
    g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

	// Play sound...

	if (m_hstrSound)
	{
        char* pSound = g_pLTServer->GetStringData(m_hstrSound);
		g_pServerSoundMgr->PlaySoundFromPos(vMyPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
	}

	// Time to shake.  Get all the players in the area of effect...

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
	int numObjects = objArray.NumObjects();

	if (!numObjects) return;

	for (int i = 0; i < numObjects; i++)
	{
		HOBJECT hObject = objArray.GetObject(i);

		if (hObject && IsPlayer(hObject))
		{
			// Make sure object is in area of effect...

            LTVector vPlayerPos;
            g_pLTServer->GetObjectPos(hObject, &vPlayerPos);

			if (m_fAreaOfEffect == c_fENTIRE_LEVEL ||
				VEC_DIST(vPlayerPos, vMyPos) <= m_fAreaOfEffect)
			{
                CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);
				if (pPlayer)
				{
					HCLIENT hClient = pPlayer->GetClient();
					if (hClient)
					{
                        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_SHAKE_SCREEN);
                        g_pLTServer->WriteToMessageVector(hMessage, &m_vAmount);
                        g_pLTServer->EndMessage(hMessage);
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

LTBOOL ScreenShake::InitialUpdate()
{
    SetNextUpdate(m_hObject, 0.0f);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::ReadProp
//
//	PURPOSE:	Reads properties from level info
//
// ----------------------------------------------------------------------- //

LTBOOL ScreenShake::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("ShakeSound", &genProp ) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Amount", &genProp ) == LT_OK)
	{
		m_vAmount = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("NumShakes", &genProp ) == LT_OK)
	{
		m_nNumShakes = genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("Frequency", &genProp ) == LT_OK)
	{
		m_fFrequency = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("AreaOfEffect", &genProp ) == LT_OK)
	{
		m_fAreaOfEffect = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp ) == LT_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 ScreenShake::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if (_stricmp(szMsg, "ON") == 0)
			{
                SetNextUpdate(m_hObject, 0.001f);
			}
			else if (_stricmp(szMsg, "OFF") == 0)
			{
                SetNextUpdate(m_hObject, 0.0f);
			}
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ScreenShake::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vAmount);
    pServerDE->WriteToMessageFloat(hWrite, (LTFLOAT) m_nNumShakes);
	pServerDE->WriteToMessageFloat(hWrite, m_fAreaOfEffect);
	pServerDE->WriteToMessageFloat(hWrite, m_fFrequency);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ScreenShake::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vAmount);
	m_nNumShakes	= (int) pServerDE->ReadFromMessageFloat(hRead);
	m_fAreaOfEffect	= pServerDE->ReadFromMessageFloat(hRead);
	m_fFrequency	= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius	= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrSound		= pServerDE->ReadFromMessageHString(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScreenShake::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void ScreenShake::CacheFiles()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

    char* pFile = LTNULL;
	if (m_hstrSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}