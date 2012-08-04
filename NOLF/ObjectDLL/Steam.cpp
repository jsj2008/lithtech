// ----------------------------------------------------------------------- //
//
// MODULE  : Steam.cpp
//
// PURPOSE : Steam implementation
//
// CREATED : 10/19/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Steam.h"
#include "SFXMsgIds.h"
#include "ObjectMsgs.h"
#include "Destructible.h"

BEGIN_CLASS(Steam)
	ADD_REALPROP_FLAG(Lifetime, -1.0f, 0)
    ADD_BOOLPROP_FLAG(StartActive, LTFALSE, 0)
	ADD_REALPROP_FLAG(Damage, 0.0f, 0)
	ADD_STEAM_PROPS()
END_CLASS_DEFAULT(Steam, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Steam()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Steam::Steam() : GameBase(OT_NORMAL)
{
	m_fDamage		= 0.0f;
	m_fLifetime		= -1.0f;
    m_bStartActive  = LTFALSE;
    m_bOn           = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::~Steam()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Steam::~Steam()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Steam::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES;
			}

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProps();
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Steam::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Steam::ReadProps()
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("Damage", &genProp) == LT_OK)
	{
		m_fDamage = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Lifetime", &genProp) == LT_OK)
	{
		m_fLifetime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

	m_SteamStruct.ReadProps();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::InitialUpdate
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Steam::InitialUpdate()
{
	// Tell the clients about us...

	CreateSFXMsg();

	if (m_bStartActive)
	{
		TurnOn();
	}
	else
	{
		SetNextUpdate(0.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Setup
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Steam::Setup(STEAMCREATESTRUCT* pSC, LTFLOAT fLifetime, LTBOOL bStartActive)
{
	if (!pSC) return;

	m_SteamStruct = *pSC;

	m_bStartActive	= bStartActive;
	m_fLifetime		= fLifetime;

	// Make sure we reset any necessary values...

	InitialUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Update
//
//	PURPOSE:	Update the damage
//
// ----------------------------------------------------------------------- //

void Steam::Update()
{
	if (m_fDamage > 0.0f)
	{
		// Do damage...
        DoDamage(m_fDamage * g_pLTServer->GetFrameTime());
	}

	// See if we are timed...

	if (m_fLifetime > 0.0f)
	{
		if (m_Timer.Stopped())
		{
			TurnOff();
			return;
		}
	}

    SetNextUpdate(0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::DoDamage
//
//	PURPOSE:	Damage objects (if necessary)
//
// ----------------------------------------------------------------------- //

void Steam::DoDamage(LTFLOAT fDamageAmount)
{
	if (fDamageAmount <= 0.0f || m_SteamStruct.fRange <= 0.0f) return;

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    LTVector vU, vR, vF;
    g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// See if there is an object to damage...

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vPos;
	IQuery.m_To		= vPos + (vF * m_SteamStruct.fRange);
	IQuery.m_Flags	= INTERSECT_OBJECTS | IGNORE_NONSOLID;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject)
		{
			DamageStruct damage;

			damage.eType	= DT_BURN;
			damage.fDamage	= fDamageAmount;
			damage.hDamager = m_hObject;
			damage.vDir		= vF;

			damage.DoDamage(this, IInfo.m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::CreateSFXMsg
//
//	PURPOSE:	Create our special fx message
//
// ----------------------------------------------------------------------- //

void Steam::CreateSFXMsg()
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_STEAM_ID);
    m_SteamStruct.Write(g_pLTServer, hMessage);
    g_pLTServer->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::TriggerMsg()
//
//	PURPOSE:	Trigger function to turn Steam on/off
//
// --------------------------------------------------------------------------- //

void Steam::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (_stricmp(szMsg, "ON") == 0)
	{
		TurnOn();
	}
	else if (_stricmp(szMsg, "OFF") == 0)
	{
		TurnOff();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::TurnOff()
//
//	PURPOSE:	Turn Steam off
//
// --------------------------------------------------------------------------- //

void Steam::TurnOff()
{
	if (!m_bOn) return;

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);
    SetNextUpdate(0.0f);

    m_bOn = LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::TurnOn()
//
//	PURPOSE:	Turn Steam on
//
// --------------------------------------------------------------------------- //

void Steam::TurnOn()
{
	if (m_bOn) return;

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUsrFlags |= USRFLG_VISIBLE;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);

	// If we do damage, make sure we do updates...

	if (m_fDamage > 0.0f || m_fLifetime > 0.0f)
	{
		if (m_fLifetime > 0.0f)
		{
			m_Timer.Start(m_fLifetime);
		}

        SetNextUpdate(0.001f);
	}

    m_bOn = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Steam::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, m_fDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLifetime);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartActive);
    g_pLTServer->WriteToMessageByte(hWrite, m_bOn);

	m_Timer.Save(hWrite);
	m_SteamStruct.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Steam::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    m_fDamage       = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLifetime     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bStartActive  = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bOn           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

	m_Timer.Load(hRead);
	m_SteamStruct.Load(hRead);
}