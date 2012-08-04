// ----------------------------------------------------------------------- //
//
// MODULE  : Fire.cpp
//
// PURPOSE : Fire - Implementation
//
// CREATED : 5/6/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Fire.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(Fire)
	ADD_REALPROP_FLAG(Radius, 100.0f, PF_RADIUS)
    ADD_BOOLPROP(StartOn, LTTRUE)
	PROP_DEFINEGROUP(Light, PF_GROUP1)
        ADD_BOOLPROP_FLAG(CreateLight, LTTRUE, PF_GROUP1)
		ADD_COLORPROP_FLAG(LightColor, 255.0, 128.0, 64.0, PF_GROUP1)
		ADD_REALPROP_FLAG(LightRadius, 125.0f, PF_RADIUS | PF_GROUP1)
		ADD_REALPROP_FLAG(LightPhase, 0.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(LightFrequency, 3.0f, PF_GROUP1)
		ADD_VECTORPROP_VAL_FLAG(LightOffset, 0.0f, 1.0f, 0.0f, PF_GROUP1)
	PROP_DEFINEGROUP(Sound, PF_GROUP2)
        ADD_BOOLPROP_FLAG(CreateSound, LTTRUE, PF_GROUP2)
		ADD_REALPROP_FLAG(SoundRadius, 300.0f, PF_RADIUS | PF_GROUP2)
	PROP_DEFINEGROUP(Smoke, PF_GROUP3)
        ADD_BOOLPROP_FLAG(CreateSmoke, LTTRUE, PF_GROUP3)
        ADD_BOOLPROP_FLAG(SmokeOnly, LTFALSE, PF_GROUP3)
        ADD_BOOLPROP_FLAG(BlackSmoke, LTFALSE, PF_GROUP3)
    ADD_BOOLPROP(CreateSparks, LTTRUE)
END_CLASS_DEFAULT(Fire, CClientSFX, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Fire
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Fire::Fire() : CClientSFX()
{
	m_fRadius		= 100.0f;
	m_fLightPhase	= 0.0f;
	m_fLightFreq	= 3.0f;
	m_fSoundRadius	= 300.0f;
	m_fLightRadius	= 125.0f;
	m_vLightOffset.Init(0.0f, 1.0f, 0.0f);
	m_vLightColor.Init(1.0f, 0.5f, 0.25f);

    m_bOn           = LTTRUE;
    m_bSmoke        = LTTRUE;
    m_bLight        = LTTRUE;
    m_bSparks       = LTTRUE;
    m_bSound        = LTTRUE;
    m_bBlackSmoke   = LTFALSE;
    m_bSmokeOnly    = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Fire
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Fire::~Fire()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Fire::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct *)pData;
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pStruct);
			}

			pStruct->m_Flags = FLAG_FULLPOSITIONRES | FLAG_FORCECLIENTUPDATE;
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
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

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Fire::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return CClientSFX::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Fire::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("Radius", &genProp) == LT_OK)
	{
		m_fRadius = genProp.m_Float;
	}
    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp) == LT_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}
    if (g_pLTServer->GetPropGeneric("LightRadius", &genProp) == LT_OK)
	{
		m_fLightRadius = genProp.m_Float;
	}
    if (g_pLTServer->GetPropGeneric("LightPhase", &genProp) == LT_OK)
	{
		m_fLightPhase = genProp.m_Float;
	}
    if (g_pLTServer->GetPropGeneric("LightFrequency", &genProp) == LT_OK)
	{
		m_fLightFreq = genProp.m_Float;
	}
    if (g_pLTServer->GetPropGeneric("LightOffset", &genProp) == LT_OK)
	{
		m_vLightOffset = genProp.m_Vec;
	}
    if (g_pLTServer->GetPropGeneric("LightColor", &genProp) == LT_OK)
	{
		m_vLightColor = (genProp.m_Vec / 255.0f);
	}
    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}
    if (g_pLTServer->GetPropGeneric("CreateSmoke", &genProp) == LT_OK)
	{
		m_bSmoke = genProp.m_Bool;
	}
    if (g_pLTServer->GetPropGeneric("SmokeOnly", &genProp) == LT_OK)
	{
		m_bSmokeOnly = genProp.m_Bool;
	}
    if (g_pLTServer->GetPropGeneric("BlackSmoke", &genProp) == LT_OK)
	{
		m_bBlackSmoke = genProp.m_Bool;
	}
    if (g_pLTServer->GetPropGeneric("CreateSparks", &genProp) == LT_OK)
	{
		m_bSparks = genProp.m_Bool;
	}
    if (g_pLTServer->GetPropGeneric("CreateLight", &genProp) == LT_OK)
	{
		m_bLight = genProp.m_Bool;
	}
    if (g_pLTServer->GetPropGeneric("CreateSound", &genProp) == LT_OK)
	{
		m_bSound = genProp.m_Bool;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void Fire::InitialUpdate(int nInfo)
{
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    uint32 dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Tell the clients about the Fire...

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_FIRE_ID);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fRadius);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fSoundRadius);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fLightRadius);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fLightPhase);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fLightFreq);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vLightOffset);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vLightColor);
    g_pLTServer->WriteToMessageByte(hMessage, m_bSmoke);
    g_pLTServer->WriteToMessageByte(hMessage, m_bLight);
    g_pLTServer->WriteToMessageByte(hMessage, m_bSparks);
    g_pLTServer->WriteToMessageByte(hMessage, m_bSound);
    g_pLTServer->WriteToMessageByte(hMessage, m_bBlackSmoke);
    g_pLTServer->WriteToMessageByte(hMessage, m_bSmokeOnly);
    g_pLTServer->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void Fire::HandleMsg(HOBJECT hSender, const char* szMsg)
{
	if (_stricmp(szMsg, "ON") == 0 && !m_bOn)
	{
        uint32 dwUserFlags = USRFLG_VISIBLE;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTTRUE;
	}
	else if (_stricmp(szMsg, "OFF") == 0 && m_bOn)
	{
        uint32 dwUserFlags = 0;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTFALSE;
	}
	else if (_stricmp(szMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Fire::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    g_pLTServer->WriteToMessageByte(hWrite, m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Fire::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    m_bOn = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
}