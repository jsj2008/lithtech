// ----------------------------------------------------------------------- //
//
// MODULE  : Lightning.cpp
//
// PURPOSE : Lightning - Implementation
//
// CREATED : 4/17/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Lightning.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"

LINKFROM_MODULE( Lightning );

#pragma force_active on
BEGIN_CLASS(Lightning)
	ADD_VECTORPROP_VAL_FLAG(EndPos, 0.0f, 0.0f, 0.0f, 0)
    ADD_BOOLPROP(StartOn, LTTRUE)
    ADD_BOOLPROP(OneTimeOnly, LTFALSE)
	ADD_REALPROP(Lifetime, 0.5f)
	ADD_REALPROP(AlphaLifetime, 0.1f)
	ADD_REALPROP(MinDelayTime, 0.5f)
	ADD_REALPROP(MaxDelayTime, 2.0f)
	ADD_REALPROP(Perturb, 100.0f)
	ADD_LONGINTPROP(NumSegments, 15)

    ADD_BOOLPROP(PlaySound, LTTRUE)
	ADD_REALPROP_FLAG(SoundRadius, 3000.0f, PF_RADIUS)

	ADD_STRINGPROP_FLAG(Texture, "sfx\\test\\fxtest35.dtx", PF_FILENAME)
	ADD_COLORPROP_FLAG(InnerColorStart, 255.0f, 255.0f, 255.0f, 0)
	ADD_COLORPROP_FLAG(InnerColorEnd, 255.0f, 255.0f, 255.0f, 0)
	ADD_COLORPROP_FLAG(OuterColorStart, 255.0f, 255.0f, 255.0f, 0)
	ADD_COLORPROP_FLAG(OuterColorEnd, 255.0f, 255.0f, 255.0f, 0)
	ADD_REALPROP_FLAG(AlphaStart, 1.0f, 0)
	ADD_REALPROP_FLAG(AlphaEnd, 0.0f, 0)
    ADD_BOOLPROP(Additive, LTTRUE)
    ADD_BOOLPROP(Multiply, LTFALSE)

	PROP_DEFINEGROUP(DynamicLight, PF_GROUP(1))
        ADD_BOOLPROP_FLAG(CreateLight, LTTRUE, PF_GROUP(1))
		ADD_COLORPROP_FLAG(LightColor, 255.0f, 255.0f, 255.0f, PF_GROUP(1))
		ADD_REALPROP_FLAG(LightRadius, 2000.0f, PF_GROUP(1) | PF_RADIUS)

	PROP_DEFINEGROUP(WidthInfo, PF_GROUP(2))
		ADD_REALPROP_FLAG(MinWidth, 50.0f, PF_GROUP(2))
		ADD_REALPROP_FLAG(MaxWidth, 100.0f, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(BigToSmall, LTFALSE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(SmallToBig, LTFALSE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(SmallToSmall, LTTRUE, PF_GROUP(2))
        ADD_BOOLPROP_FLAG(Constant, LTFALSE, PF_GROUP(2))

END_CLASS_DEFAULT(Lightning, CClientSFX, NULL, NULL)
#pragma force_active off


CMDMGR_BEGIN_REGISTER_CLASS( Lightning )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( REMOVE, 1, NULL, "REMOVE" )

CMDMGR_END_REGISTER_CLASS( Lightning, CClientSFX )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::Lightning
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Lightning::Lightning() : CClientSFX()
{
    m_bOn = LTTRUE;

	m_vEndPos.Init();
	m_vLightColor.Init(255, 255, 255);

	m_vInnerColorStart.Init();
	m_vInnerColorEnd.Init();
	m_vOuterColorStart.Init();
	m_vOuterColorEnd.Init();

	m_hstrTexture			= LTNULL;

	m_fAlphaStart			= 1.0f;
	m_fAlphaEnd				= 1.0f;

	m_fMinWidth				= 50.0f;
	m_fMaxWidth				= 100.0f;
	m_fLifeTime				= 0.5f;
	m_fAlphaLifeTime		= 0.1f;
	m_fMinDelayTime			= 0.5f;
	m_fMaxDelayTime			= 2.0f;
	m_fPerturb				= 100.0f;
	m_fLightRadius			= 2000.0f;
	m_fSoundRadius			= 3000.0f;
    m_bPlaySound            = LTTRUE;
    m_bOneTimeOnly          = LTFALSE;
    m_bDynamicLight         = LTFALSE;
    m_nWidthStyle           = (uint8) PLWS_SMALL_TO_SMALL;
	m_nNumSegments			= 15;
    m_bAdditive             = LTTRUE;
	m_bMultiply				= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::Lightning
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Lightning::~Lightning()
{
	FREE_HSTRING(m_hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Lightning::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	Lightning::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Lightning::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("Texture", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrTexture = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("InnerColorStart", &genProp) == LT_OK)
		m_vInnerColorStart = genProp.m_Color;

    if (g_pLTServer->GetPropGeneric("InnerColorEnd", &genProp) == LT_OK)
		m_vInnerColorEnd = genProp.m_Color;

    if (g_pLTServer->GetPropGeneric("OuterColorStart", &genProp) == LT_OK)
		m_vOuterColorStart = genProp.m_Color;

    if (g_pLTServer->GetPropGeneric("OuterColorEnd", &genProp) == LT_OK)
		m_vOuterColorEnd = genProp.m_Color;

   if (g_pLTServer->GetPropGeneric("AlphaStart", &genProp) == LT_OK)
		m_fAlphaStart = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("AlphaEnd", &genProp) == LT_OK)
		m_fAlphaEnd = genProp.m_Float;

     if (g_pLTServer->GetPropGeneric("Additive", &genProp) == LT_OK)
		m_bAdditive = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("Multiply", &genProp) == LT_OK)
		m_bMultiply = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
		m_bOn = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("EndPos", &genProp) == LT_OK)
		m_vEndPos = genProp.m_Vec;

    if (g_pLTServer->GetPropGeneric("LightColor", &genProp) == LT_OK)
		m_vLightColor = genProp.m_Color;

    if (g_pLTServer->GetPropGeneric("NumSegments", &genProp) == LT_OK)
        m_nNumSegments = genProp.m_Long > 255 ? 255 : (uint8)genProp.m_Long;

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp) == LT_OK)
		m_fSoundRadius = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("LightRadius", &genProp) == LT_OK)
		m_fLightRadius = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("Perturb", &genProp) == LT_OK)
		m_fPerturb = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("MinDelayTime", &genProp) == LT_OK)
		m_fMinDelayTime = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("MaxDelayTime", &genProp) == LT_OK)
		m_fMaxDelayTime = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("MinWidth", &genProp) == LT_OK)
		m_fMinWidth = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("MaxWidth", &genProp) == LT_OK)
		m_fMaxWidth = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("Lifetime", &genProp) == LT_OK)
		m_fLifeTime = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("AlphaLifetime", &genProp) == LT_OK)
		m_fAlphaLifeTime = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("OneTimeOnly", &genProp) == LT_OK)
		m_bOneTimeOnly = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("CreateLight", &genProp) == LT_OK)
		m_bDynamicLight = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("PlaySound", &genProp) == LT_OK)
		m_bPlaySound = genProp.m_Bool;

	m_nWidthStyle = PLWS_SMALL_TO_SMALL;

    if (g_pLTServer->GetPropGeneric("BigToSmall", &genProp) == LT_OK)
        if (genProp.m_Bool) m_nWidthStyle = (uint8) PLWS_BIG_TO_SMALL;

    if (g_pLTServer->GetPropGeneric("SmallToBig", &genProp) == LT_OK)
        if (genProp.m_Bool) m_nWidthStyle = (uint8) PLWS_SMALL_TO_BIG;

    if (g_pLTServer->GetPropGeneric("SmallToSmall", &genProp) == LT_OK)
        if (genProp.m_Bool) m_nWidthStyle = (uint8) PLWS_SMALL_TO_SMALL;

    if (g_pLTServer->GetPropGeneric("Constant", &genProp) == LT_OK)
        if (genProp.m_Bool) m_nWidthStyle = (uint8) PLWS_CONSTANT;


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void Lightning::InitialUpdate(int nInfo)
{
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, m_bOn ? USRFLG_VISIBLE : 0, FLAGMASK_ALL);

	// Tell the clients about the Lightning, and remove thyself if
	// a one-time only lightning...

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_LIGHTNING_ID);

	cMsg.WriteLTVector(vPos);
    cMsg.WriteLTVector(m_vEndPos);
    cMsg.WriteLTVector(m_vLightColor);

    cMsg.WriteLTVector(m_vInnerColorStart);
    cMsg.WriteLTVector(m_vInnerColorEnd);
    cMsg.WriteLTVector(m_vOuterColorStart);
    cMsg.WriteLTVector(m_vOuterColorEnd);

    cMsg.Writefloat(m_fAlphaStart);
    cMsg.Writefloat(m_fAlphaEnd);

    cMsg.Writefloat(m_fMinWidth);
    cMsg.Writefloat(m_fMaxWidth);

    cMsg.Writefloat(m_fLifeTime);
    cMsg.Writefloat(m_fAlphaLifeTime);

    cMsg.Writefloat(m_fMinDelayTime);
    cMsg.Writefloat(m_fMaxDelayTime);

    cMsg.Writefloat(m_fPerturb);
    cMsg.Writefloat(m_fLightRadius);
    cMsg.Writefloat(m_fSoundRadius);

    cMsg.Writeuint8(m_nWidthStyle);
    cMsg.Writeuint8(m_nNumSegments);

    cMsg.Writeuint8(m_bOneTimeOnly);
    cMsg.Writeuint8(m_bDynamicLight);
    cMsg.Writeuint8(m_bPlaySound);
    cMsg.Writeuint8(m_bAdditive);
    cMsg.Writeuint8(m_bMultiply);

    cMsg.WriteHString(m_hstrTexture);


	if (m_bOneTimeOnly)
	{
		g_pLTServer->SendSFXMessage(cMsg.Read(), vPos, MESSAGE_GUARANTEED);
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
	{
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::OnTrigger()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

bool Lightning::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if ((cMsg.GetArg(0) == s_cTok_On) && !m_bOn)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
        m_bOn = LTTRUE;
	}
	else if ((cMsg.GetArg(0) == s_cTok_Off) && m_bOn)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);
        m_bOn = LTFALSE;
	}
	else if (cMsg.GetArg(0) == s_cTok_Remove)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
		return CClientSFX::OnTrigger(hSender, cMsg);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Lightning::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Lightning::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_BOOL(m_bOn);
}