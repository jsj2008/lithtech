// ----------------------------------------------------------------------- //
//
// MODULE  : Lightning.cpp
//
// PURPOSE : Lightning - Implementation
//
// CREATED : 4/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Lightning.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"

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

	PROP_DEFINEGROUP(DynamicLight, PF_GROUP1)
        ADD_BOOLPROP_FLAG(CreateLight, LTTRUE, PF_GROUP1)
		ADD_COLORPROP_FLAG(LightColor, 255.0f, 255.0f, 255.0f, PF_GROUP1)
		ADD_REALPROP_FLAG(LightRadius, 2000.0f, PF_GROUP1 | PF_RADIUS)

	PROP_DEFINEGROUP(WidthInfo, PF_GROUP2)
		ADD_REALPROP_FLAG(MinWidth, 50.0f, PF_GROUP2)
		ADD_REALPROP_FLAG(MaxWidth, 100.0f, PF_GROUP2)
        ADD_BOOLPROP_FLAG(BigToSmall, LTFALSE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(SmallToBig, LTFALSE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(SmallToSmall, LTTRUE, PF_GROUP2)
        ADD_BOOLPROP_FLAG(Constant, LTFALSE, PF_GROUP2)

END_CLASS_DEFAULT(Lightning, CClientSFX, NULL, NULL)


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
//	ROUTINE:	Lightning::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Lightning::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
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

    uint32 dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Tell the clients about the Lightning, and remove thyself if
	// a one-time only lightning...

	HMESSAGEWRITE hMessage;

	if (m_bOneTimeOnly)
	{
        hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vPos);
	}
	else
	{
        hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	}

    g_pLTServer->WriteToMessageByte(hMessage, SFX_LIGHTNING_ID);
    g_pLTServer->WriteToMessageVector(hMessage, &vPos);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vEndPos);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vLightColor);

    g_pLTServer->WriteToMessageVector(hMessage, &(m_vInnerColorStart));
    g_pLTServer->WriteToMessageVector(hMessage, &(m_vInnerColorEnd));
    g_pLTServer->WriteToMessageVector(hMessage, &(m_vOuterColorStart));
    g_pLTServer->WriteToMessageVector(hMessage, &(m_vOuterColorEnd));

    g_pLTServer->WriteToMessageFloat(hMessage, m_fAlphaStart);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fAlphaEnd);

    g_pLTServer->WriteToMessageFloat(hMessage, m_fMinWidth);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fMaxWidth);

    g_pLTServer->WriteToMessageFloat(hMessage, m_fLifeTime);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fAlphaLifeTime);

    g_pLTServer->WriteToMessageFloat(hMessage, m_fMinDelayTime);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fMaxDelayTime);

    g_pLTServer->WriteToMessageFloat(hMessage, m_fPerturb);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fLightRadius);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fSoundRadius);

    g_pLTServer->WriteToMessageByte(hMessage, m_nWidthStyle);
    g_pLTServer->WriteToMessageByte(hMessage, m_nNumSegments);

    g_pLTServer->WriteToMessageByte(hMessage, m_bOneTimeOnly);
    g_pLTServer->WriteToMessageByte(hMessage, m_bDynamicLight);
    g_pLTServer->WriteToMessageByte(hMessage, m_bPlaySound);
    g_pLTServer->WriteToMessageByte(hMessage, m_bAdditive);
    g_pLTServer->WriteToMessageByte(hMessage, m_bMultiply);

    g_pLTServer->WriteToMessageHString(hMessage, m_hstrTexture);

    g_pLTServer->EndMessage(hMessage);

	if (m_bOneTimeOnly)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void Lightning::HandleMsg(HOBJECT hSender, const char* szMsg)
{
	if (_stricmp(szMsg, "ON") == 0 && !m_bOn)
	{
        uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_VISIBLE);
        m_bOn = LTTRUE;
	}
	else if (_stricmp(szMsg, "OFF") == 0 && m_bOn)
	{
        uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_VISIBLE);
        m_bOn = LTFALSE;
	}
	else if (_stricmp(szMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Lightning::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lightning::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Lightning::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_bOn = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
}