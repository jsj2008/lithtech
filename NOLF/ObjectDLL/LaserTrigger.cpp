// ----------------------------------------------------------------------- //
//
// MODULE  : LaserTrigger.h
//
// PURPOSE : LaserTrigger - Implementation
//
// CREATED : 4/30/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LaserTrigger.h"
#include "iltserver.h"
#include "ClientServerShared.h"
#include "stdio.h"

BEGIN_CLASS(LaserTrigger)
	ADD_VECTORPROP_VAL_FLAG(Dims, 5.0f, 5.0f, 200.0f, PF_DIMS)
	ADD_COLORPROP(Color, 255.0f, 0.0f, 0.0f)
	ADD_REALPROP(Alpha, 0.7f)
	ADD_BOOLPROP(CreateSprites, 1)
	ADD_STRINGPROP_FLAG(SpriteFilename, "SFX\\FLARES\\RedSoft1.spr", PF_FILENAME)
	ADD_REALPROP(SpriteScale, 0.1f)
END_CLASS_DEFAULT(LaserTrigger, Trigger, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::LaserTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

LaserTrigger::LaserTrigger() : Trigger()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::~LaserTrigger()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

LaserTrigger::~LaserTrigger()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

    if(m_ltcs.hstrSpriteFilename && g_pLTServer)
	{
        g_pLTServer->FreeString(m_ltcs.hstrSpriteFilename);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 LaserTrigger::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = Trigger::EngineMessageFn(messageID, pData, fData);
			InitialUpdate((int)fData);
			return dwRet;
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

	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void LaserTrigger::HandleTriggerMsg(HOBJECT hSender, const char* szMsg)
{
	// See if we hide/show ourself :)

	if (_stricmp(szMsg, "LOCK") == 0)
	{
        uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		dwUsrFlags &= ~USRFLG_VISIBLE;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}
	else if (_stricmp(szMsg, "UNLOCK") == 0)
	{
        uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_VISIBLE;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

	Trigger::HandleTriggerMsg(hSender, szMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL LaserTrigger::ReadProp(ObjectCreateStruct *pData)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE || !pData) return LTFALSE;

	GenericProp genProp;

    if (pServerDE->GetPropGeneric("Alpha", &genProp) == LT_OK)
	{
		m_ltcs.fAlpha = genProp.m_Float;
	}

    if (pServerDE->GetPropGeneric("Color", &genProp) == LT_OK)
	{
		m_ltcs.vColor = genProp.m_Vec;
	}

    if (pServerDE->GetPropGeneric("CreateSprite", &genProp) == LT_OK)
	{
		m_ltcs.bCreateSprite = genProp.m_Bool;
	}

    if (pServerDE->GetPropGeneric("SpriteScale", &genProp) == LT_OK)
	{
		m_ltcs.fSpriteScale = genProp.m_Float;
	}

    if (pServerDE->GetPropGeneric("SpriteFilename", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_ltcs.hstrSpriteFilename = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void LaserTrigger::InitialUpdate(int nInfo)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Don't need to create the model when restoring a save game...

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwFlags |= USRFLG_VISIBLE;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags);

	// If object isn't visible, it won't get the special fx msg...

    dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_VISIBLE;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	m_ltcs.vDims = m_vDims;

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_LASERTRIGGER_ID);
    m_ltcs.Write(g_pLTServer, hMessage);
    g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void LaserTrigger::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	m_ltcs.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void LaserTrigger::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_ltcs.Load(hRead);
}