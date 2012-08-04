// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.cpp
//
// PURPOSE : Implementation of the alarm
//
// CREATED : 3/27/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Alarm.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "ObjectMsgs.h"

// Statics

static char *s_szActivate	= "ACTIVATE";
static char *s_szLock		= "LOCK";
static char *s_szUnlock		= "UNLOCK";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Alarm
//
//	PURPOSE:	An alarm object
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Alarm)

	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Alarm.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Alarm.dtx", PF_FILENAME)
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, 0)
    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

    ADD_BOOLPROP(PlayerUsable, LTFALSE)
    ADD_STRINGPROP_FLAG(ActivateTarget, LTFALSE, PF_OBJECTLINK)
    ADD_STRINGPROP(ActivateMessage, LTFALSE)

END_CLASS_DEFAULT(Alarm, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Alarm()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Alarm::Alarm() : Prop ()
{
	m_eState = eStateOff;
    m_bPlayerUsable = LTFALSE;
    m_hstrActivateMessage = LTNULL;
    m_hstrActivateTarget = LTNULL;
	m_bLocked = LTFALSE;

    m_damage.m_bRemoveOnDeath = LTFALSE;

	m_pDebrisOverride = "Metal small";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::~Alarm()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Alarm::~Alarm()
{
    if( !g_pLTServer ) return;

	m_eState = eStateOff;
    m_bPlayerUsable = LTFALSE;
	FREE_HSTRING(m_hstrActivateMessage);
	FREE_HSTRING(m_hstrActivateTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Alarm::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
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

			CacheFiles();
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if ( g_pLTServer->GetPropGeneric( "ActivateMessage", &genProp ) == LT_OK )
	if ( genProp.m_String[0] )
        m_hstrActivateMessage = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "ActivateTarget", &genProp ) == LT_OK )
	if ( genProp.m_String[0] )
        m_hstrActivateTarget = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric("PlayerUsable", &genProp ) == LT_OK )
		m_bPlayerUsable = genProp.m_Bool;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Alarm::PostPropRead(ObjectCreateStruct *pStruct)
{
	if ( !pStruct ) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Alarm::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
            const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			if ( szMsg && !_stricmp(szMsg, s_szActivate) )
			{
                if ( m_bPlayerUsable || g_pLTServer->GetObjectClass(hSender) != g_pLTServer->GetClass("CPlayerObj") )
				{
					// We were triggered, now send our message

					if ( m_hstrActivateTarget && m_hstrActivateMessage )
					{
						SendTriggerMsgToObjects(this, m_hstrActivateTarget, m_hstrActivateMessage);
						FREE_HSTRING(m_hstrActivateMessage);
						FREE_HSTRING(m_hstrActivateTarget);
					}
				}
			}
			else if ( szMsg && !_stricmp(szMsg, s_szLock) )
			{
				m_bLocked = LTTRUE;
			}
			else if ( szMsg && !_stricmp(szMsg, s_szUnlock) )
			{
				m_bLocked = LTFALSE;
			}

			break;
		}

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if ( m_damage.IsDead() )
			{
				m_eState = eStateDestroyed;
			}

			// TODO: Check to see if we have been disbled

			return dwRet;
		}

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Alarm::InitialUpdate()
{
	// Make sure we're non-solid...	

	uint dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_SOLID);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Alarm::Save(HMESSAGEWRITE hWrite)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

    SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bPlayerUsable);
    SAVE_HSTRING(m_hstrActivateMessage);
    SAVE_HSTRING(m_hstrActivateTarget);
	SAVE_BOOL(m_bLocked);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Alarm::Load(HMESSAGEREAD hRead)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

    LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bPlayerUsable);
    LOAD_HSTRING(m_hstrActivateMessage);
    LOAD_HSTRING(m_hstrActivateTarget);
	LOAD_BOOL(m_bLocked);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Alarm::CacheFiles()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;
}