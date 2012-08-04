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
#include "ParsedMsg.h"

LINKFROM_MODULE( LaserTrigger );


#pragma force_active on
BEGIN_CLASS(LaserTrigger)
	ADD_VECTORPROP_VAL_FLAG(Dims, 5.0f, 5.0f, 200.0f, PF_DIMS)
	ADD_COLORPROP(Color, 255.0f, 0.0f, 0.0f)
	ADD_REALPROP(Alpha, 0.7f)
	ADD_BOOLPROP(CreateSprites, 1)
	ADD_STRINGPROP_FLAG(SpriteFilename, "SFX\\FLARES\\RedSoft1.spr", PF_FILENAME)
	ADD_REALPROP(SpriteScale, 0.1f)
END_CLASS_DEFAULT(LaserTrigger, Trigger, NULL, NULL)
#pragma force_active off

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( LaserTrigger )
CMDMGR_END_REGISTER_CLASS( LaserTrigger, Trigger )

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
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
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

bool LaserTrigger::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Lock("LOCK");
	static CParsedMsg::CToken s_cTok_Unlock("UNLOCK");

	// See if we hide/show ourself :)

	if (cMsg.GetArg(0) == s_cTok_Lock)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);
	}
	else if (cMsg.GetArg(0) == s_cTok_Unlock)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
	}

	return Trigger::OnTrigger(hSender, cMsg);
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

	// If object isn't visible, it won't get the special fx msg...

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

	m_ltcs.vDims = m_vDims;

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_LASERTRIGGER_ID);
    m_ltcs.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void LaserTrigger::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	m_ltcs.Save(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LaserTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void LaserTrigger::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_ltcs.Load(pMsg);
}