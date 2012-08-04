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
#include "ParsedMsg.h"
#include "CommandMgr.h"

LINKFROM_MODULE( Fire );

#pragma force_active on
BEGIN_CLASS(Fire)
	ADD_REALPROP_FLAG(Radius, 100.0f, PF_RADIUS)
    ADD_BOOLPROP(StartOn, LTTRUE)
	PROP_DEFINEGROUP(Light, PF_GROUP(1))
        ADD_BOOLPROP_FLAG(CreateLight, LTTRUE, PF_GROUP(1))
		ADD_COLORPROP_FLAG(LightColor, 255.0, 128.0, 64.0, PF_GROUP(1))
		ADD_REALPROP_FLAG(LightRadius, 125.0f, PF_RADIUS | PF_GROUP(1))
		ADD_REALPROP_FLAG(LightPhase, 0.0f, PF_GROUP(1))
		ADD_REALPROP_FLAG(LightFrequency, 3.0f, PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(LightOffset, 0.0f, 1.0f, 0.0f, PF_GROUP(1))
	PROP_DEFINEGROUP(Sound, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(CreateSound, LTTRUE, PF_GROUP(2))
		ADD_REALPROP_FLAG(SoundRadius, 300.0f, PF_RADIUS | PF_GROUP(2))
	PROP_DEFINEGROUP(Smoke, PF_GROUP(3))
        ADD_BOOLPROP_FLAG(CreateSmoke, LTTRUE, PF_GROUP(3))
        ADD_BOOLPROP_FLAG(SmokeOnly, LTFALSE, PF_GROUP(3))
        ADD_BOOLPROP_FLAG(BlackSmoke, LTFALSE, PF_GROUP(3))
    ADD_BOOLPROP(CreateSparks, LTTRUE)
END_CLASS_DEFAULT(Fire, CClientSFX, NULL, NULL)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( Fire )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( REMOVE, 1, NULL, "REMOVE" )

CMDMGR_END_REGISTER_CLASS( Fire, CClientSFX )
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

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
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

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, m_bOn ? USRFLG_VISIBLE : 0, FLAGMASK_ALL);

	// Tell the clients about the Fire...

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_FIRE_ID);
	cMsg.Writefloat(m_fRadius);
    cMsg.Writefloat(m_fSoundRadius);
    cMsg.Writefloat(m_fLightRadius);
    cMsg.Writefloat(m_fLightPhase);
    cMsg.Writefloat(m_fLightFreq);
    cMsg.WriteLTVector(m_vLightOffset);
    cMsg.WriteLTVector(m_vLightColor);
    cMsg.Writeuint8(m_bSmoke);
    cMsg.Writeuint8(m_bLight);
    cMsg.Writeuint8(m_bSparks);
    cMsg.Writeuint8(m_bSound);
    cMsg.Writeuint8(m_bBlackSmoke);
    cMsg.Writeuint8(m_bSmokeOnly);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::OnTrigger()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

bool Fire::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if ((cMsg.GetArg(0) == s_cTok_On) && !m_bOn)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, FLAGMASK_ALL);
        m_bOn = LTTRUE;
	}
	else if ((cMsg.GetArg(0) == s_cTok_Off) && m_bOn)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, FLAGMASK_ALL);
        m_bOn = LTFALSE;
	}
	else if (cMsg.GetArg(0) == s_cTok_Remove)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Fire::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	SAVE_BOOL(m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Fire::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
    LOAD_BOOL(m_bOn);
}