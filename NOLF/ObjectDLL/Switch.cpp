// ----------------------------------------------------------------------- //
//
// MODULE  : Switch.CPP
//
// PURPOSE : A Switch object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Switch.h"
#include "ServerUtilities.h"

BEGIN_CLASS(Switch)
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PortalName, "", PF_HIDDEN)
    ADD_BOOLPROP_FLAG(LoopSounds, LTFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpenSound, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CloseSound, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(OpenWaitTime, 4.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(CloseWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ClosingSpeed, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP(AITriggerable, 1)
	ADD_REALPROP_FLAG(Speed, 200.0f, PF_HIDDEN)
	PROP_DEFINEGROUP(StateFlags, PF_GROUP4)
        ADD_BOOLPROP_FLAG(ActivateTrigger, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(StartOpen, LTFALSE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(TriggerClose, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(RemainsOpen, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(ForceMove, LTTRUE, PF_GROUP4)
	ADD_REALPROP(MoveDelay, 0.0f)
	ADD_REALPROP(MoveDist, 4.0f)
	ADD_VECTORPROP_VAL(MoveDir, 0.0f, 0.0f, 0.0f)
	ADD_STRINGPROP(OnSound, "Snd\\Switch\\01On.wav")
	ADD_STRINGPROP(OnTriggerTarget, "")
	ADD_STRINGPROP(OnTriggerMessage, "TRIGGER")
	ADD_STRINGPROP(OffSound, "Snd\\Switch\\01Off.wav")
	ADD_STRINGPROP(OffTriggerTarget, "")
	ADD_STRINGPROP(OffTriggerMessage, "TRIGGER")
	ADD_REALPROP(OnWaitTime, 0.0f)
	ADD_REALPROP(OnMoveSpeed, 300.0f)
	ADD_REALPROP(OffMoveSpeed, 0.0f)
	ADD_REALPROP(OffWaitTime, 0.0f)
END_CLASS_DEFAULT(Switch, Door, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::Switch()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Switch::Switch() : Door()
{
    m_hstrOnTriggerTarget   = LTNULL;
    m_hstrOnTriggerMessage  = LTNULL;
    m_hstrOffTriggerTarget  = LTNULL;
    m_hstrOffTriggerMessage = LTNULL;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::~Switch()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Switch::~Switch()
{
    ILTServer* pServerDE = GetServerDE();

	if (m_hstrOnTriggerTarget)
	{
		pServerDE->FreeString(m_hstrOnTriggerTarget);
		m_hstrOnTriggerTarget = NULL;
	}
	if (m_hstrOnTriggerMessage)
	{
		pServerDE->FreeString(m_hstrOnTriggerMessage);
		m_hstrOnTriggerMessage = NULL;
	}
	if (m_hstrOffTriggerTarget)
	{
		pServerDE->FreeString(m_hstrOffTriggerTarget);
		m_hstrOffTriggerTarget = NULL;
	}
	if (m_hstrOffTriggerMessage)
	{
		pServerDE->FreeString(m_hstrOffTriggerMessage);
		m_hstrOffTriggerMessage = NULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 Switch::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call ReadProp()

            uint32 dwRet = Door::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				ReadProp(pStruct);

				pStruct->m_Flags |= FLAG_RAYHIT;
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
                uint32 dwRet = Door::EngineMessageFn(messageID, pData, fData);
				InitialUpdate();
				return dwRet;
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint8)fData);
		}
		break;

		default : break;
	}

	return Door::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::ReadProp()
//
//	PURPOSE:	Reads Switch properties
//
// --------------------------------------------------------------------------- //

LTBOOL Switch::ReadProp(ObjectCreateStruct *)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("OnTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOnTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OnTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOnTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OffTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOffTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OffTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOffTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}


	// Just new names for door data members...

    if (g_pLTServer->GetPropGeneric("OnSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOpenSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OffSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrCloseSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	pServerDE->GetPropReal("OnWaitTime", &m_fOpenWaitTime);
	pServerDE->GetPropReal("OnMoveSpeed", &m_fSpeed);
	pServerDE->GetPropReal("OffWaitTime", &m_fCloseWaitTime);
	pServerDE->GetPropReal("OffMoveSpeed", &m_fClosingSpeed);

	m_fClosingSpeed = m_fClosingSpeed ? m_fClosingSpeed : m_fSpeed;

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::SetOpening()
//
//	PURPOSE:	Sets the switch opening state
//
// --------------------------------------------------------------------------- //

void Switch::SetOpening()
{
	if (m_hstrOnTriggerTarget && m_hstrOnTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrOnTriggerTarget, m_hstrOnTriggerMessage);
	}

	Door::SetOpening();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::SetClosing()
//
//	PURPOSE:	Sets the switch closing state
//
// --------------------------------------------------------------------------- //

void Switch::SetClosing()
{
	if (m_hstrOffTriggerTarget && m_hstrOffTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrOffTriggerTarget, m_hstrOffTriggerMessage);
	}

	Door::SetClosing();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void Switch::InitialUpdate()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

    uint32 dwUsrFlgs = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_CAN_ACTIVATE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Switch::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrOnTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOnTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOffTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrOffTriggerMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Switch::Load(HMESSAGEREAD hRead, uint8 nType)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrOnTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrOnTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrOffTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrOffTriggerMessage = pServerDE->ReadFromMessageHString(hRead);
}