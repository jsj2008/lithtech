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
#include "ParsedMsg.h"
#include "Destructible.h"

LINKFROM_MODULE( Steam );

#pragma force_active on
BEGIN_CLASS(Steam)
	ADD_REALPROP_FLAG(Lifetime, -1.0f, 0)
    ADD_BOOLPROP_FLAG(StartActive, LTFALSE, 0)
	ADD_REALPROP_FLAG(Damage, 0.0f, 0)
	ADD_STEAM_PROPS()
END_CLASS_DEFAULT(Steam, GameBase, NULL, NULL)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( Steam )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS( Steam, GameBase )

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

	return GameBase::EngineMessageFn(messageID, pData, fData);
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
		SetNextUpdate(UPDATE_NEVER);
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

    SetNextUpdate(UPDATE_NEXT_FRAME);
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
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

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
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_STEAM_ID);
    m_SteamStruct.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::OnTrigger()
//
//	PURPOSE:	Trigger function to turn Steam on/off
//
// --------------------------------------------------------------------------- //

bool Steam::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if (cMsg.GetArg(0) == s_cTok_On)
	{
		TurnOn();
	}
	else if (cMsg.GetArg(0) == s_cTok_Off)
	{
		TurnOff();
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
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

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);
    SetNextUpdate(UPDATE_NEVER);

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

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);

	// If we do damage, make sure we do updates...

	if (m_fDamage > 0.0f || m_fLifetime > 0.0f)
	{
		if (m_fLifetime > 0.0f)
		{
			m_Timer.Start(m_fLifetime);
		}

        SetNextUpdate(UPDATE_NEXT_FRAME);
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

void Steam::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fDamage);
    SAVE_FLOAT(m_fLifetime);
    SAVE_BOOL(m_bStartActive);
    SAVE_BOOL(m_bOn);

	m_Timer.Save(pMsg);
	m_SteamStruct.Save(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Steam::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    LOAD_FLOAT(m_fDamage);
    LOAD_FLOAT(m_fLifetime);
    LOAD_BOOL(m_bStartActive);
    LOAD_BOOL(m_bOn);

	m_Timer.Load(pMsg);
	m_SteamStruct.Load(pMsg);
}