// ----------------------------------------------------------------------- //
//
// MODULE  : Controller.cpp
//
// PURPOSE : Controller - Implementation
//
// CREATED : 4/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "controller.h"
#include "iltmessage.h"
#include "commonutilities.h"
#include "serverutilities.h"
#include "ObjectMsgs.h"


BEGIN_CLASS(Controller)
	ADD_STRINGPROP_FLAG(Target0, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target1, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target2, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target3, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target4, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target5, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target6, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target7, "", PF_OBJECTLINK)
END_CLASS_DEFAULT(Controller, GameBase, NULL, NULL)




// ----------------------------------------------------------------------------------------------- //
// ParamValue functions.
// ----------------------------------------------------------------------------------------------- //

void ParamValue::Load(HMESSAGEREAD hRead)
{
	m_Color = hRead->ReadVector();
}

void ParamValue::Save(HMESSAGEWRITE hWrite)
{
	hWrite->WriteVector(m_Color);
}


// ----------------------------------------------------------------------------------------------- //
// Controller functions.
// ----------------------------------------------------------------------------------------------- //

Controller::Controller()
{
	m_fStartTime = 0.0f;
	m_fDuration = 0.0f;
	m_State = CState_Off;
    m_bFirstUpdate = LTTRUE;
}


uint32 Controller::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				PreCreate((ObjectCreateStruct*)pData);
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

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


uint32 Controller::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


void Controller::PreCreate(ObjectCreateStruct *pStruct)
{
	char buf[256];
    uint32 i;
	GenericProp gProp;


	// Get target object names.
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		sprintf(buf, "Target%d", i);
        if(g_pLTServer->GetPropGeneric(buf, &gProp) == LT_OK)
		{
			SAFE_STRCPY(m_Fades[i].m_ObjectName, gProp.m_String);
		}
	}
}


void Controller::InitialUpdate()
{
    SetNextUpdate(0.001f);
}


void Controller::FirstUpdate()
{
	// Find target objects and make interlinks.

    for (uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

        g_pLTServer->FindNamedObjects(m_Fades[i].m_ObjectName, objArray);

		if (objArray.NumObjects())
		{
			HOBJECT hObject = objArray.GetObject(0);

			m_Fades[i].m_hTarget = hObject;
            g_pLTServer->CreateInterObjectLink(m_hObject, m_Fades[i].m_hTarget);
		}
	}
}


void Controller::Update()
{
    uint32 i;
	FadeState *pState;
	float curTime, t;

	if(m_bFirstUpdate)
	{
		FirstUpdate();
        m_bFirstUpdate = LTFALSE;
	}

	if(m_State == CState_Fade)
	{
		// Find out if we're even interpolating.
        curTime = g_pLTServer->GetTime();
		if(curTime > (m_fStartTime + m_fDuration))
			return;

		t = (curTime - m_fStartTime) / m_fDuration;
		t = GetWaveFn(m_WaveType)(t); // Apply wave function.

		for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
		{
			pState = &m_Fades[i];

			if(!pState->m_hTarget)
				continue;

			InterpolateValue(pState, t);
		}

        SetNextUpdate(0.001f);
	}
	else if(m_State == CState_Flicker)
	{
        if(g_pLTServer->GetTime() > m_fNextFlickerTime)
		{
			// Send the message.
			for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
			{
				pState = &m_Fades[i];

				if(!pState->m_hTarget)
					continue;

                SendTriggerMsgToObject(this, pState->m_hTarget, LTFALSE, m_FlickerMsg);
			}

			// Go again?
			if(m_FlickerCounter != FLICKER_FOREVER)
				--m_FlickerCounter;

			if(m_FlickerCounter == 0)
				HandleOffCommand(LTNULL, LTNULL);

            m_fNextFlickerTime = g_pLTServer->GetTime() + GetRandom(m_fIntervalMin, m_fIntervalMax);
		}

        SetNextUpdate(0.001f);
	}
	else
	{
		SetNextUpdate(0.0f);
	}
}


void Controller::OnLinkBroken(HOBJECT hObj)
{
    uint32 i;

	// Disable the fade with this object.
	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		if(m_Fades[i].m_hTarget == hObj)
		{
            m_Fades[i].m_hTarget = LTNULL;
		}
	}
}


void Controller::Load(HMESSAGEREAD hRead, uint32 dwSaveFlags)
{
	FadeState *pState;
    uint32 i;

	m_bFirstUpdate = (LTBOOL) hRead->ReadByte();

	m_State = (CState)hRead->ReadDWord();

	// Read FLICKER vars.
	m_fNextFlickerTime = hRead->ReadFloat();
	m_fIntervalMin = hRead->ReadFloat();
	m_fIntervalMax = hRead->ReadFloat();
	m_FlickerCounter = hRead->ReadDWord();
	m_FlickerMsg[0] = 0;
	hRead->ReadStringFL(m_FlickerMsg, sizeof(m_FlickerMsg));

	// Read FADE vars.
	m_fStartTime = hRead->ReadFloat();
	m_fDuration = hRead->ReadFloat();
	m_WaveType = (WaveType)hRead->ReadDWord();
	m_ParamType = (ParamType)hRead->ReadDWord();
	m_DestValue.Load(hRead);

	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		pState = &m_Fades[i];

		pState->m_StartVal.Load(hRead);
		pState->m_hTarget = hRead->ReadObject();
	}
}


void Controller::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	FadeState *pState;
    uint32 i;

    hWrite->WriteByte((uint8)m_bFirstUpdate);

    hWrite->WriteDWord((uint32)m_State);

	// Write FLICKER vars.
	hWrite->WriteFloat(m_fNextFlickerTime);
	hWrite->WriteFloat(m_fIntervalMin);
	hWrite->WriteFloat(m_fIntervalMax);
	hWrite->WriteDWord(m_FlickerCounter);
	hWrite->WriteString(m_FlickerMsg);

	// Write FADE vars.
	hWrite->WriteFloat(m_fStartTime);
	hWrite->WriteFloat(m_fDuration);
    hWrite->WriteDWord((uint32)m_WaveType);
    hWrite->WriteDWord((uint32)m_ParamType);
	m_DestValue.Save(hWrite);

	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		pState = &m_Fades[i];

		pState->m_StartVal.Save(hWrite);
		hWrite->WriteObject(pState->m_hTarget);
	}
}


void Controller::HandleTrigger(HOBJECT hSender, const char *szMsg)
{
	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

    if(g_pLTServer->Common()->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs == 0)
		{
			ShowTriggerError(szMsg);
			return;
		}

		char* szCmd = parse.m_Args[0];
		if(stricmp(szCmd, "FADE") == 0)
		{
			HandleFadeCommand(szMsg, &parse);
		}
		else if(stricmp(szCmd, "FLICKER") == 0)
		{
			HandleFlickerCommand(szMsg, &parse);
		}
		else if(stricmp(szCmd, "OFF") == 0)
		{
			HandleOffCommand(szMsg, &parse);
		}
		else
		{
			ShowTriggerError(szMsg);
		}
	}
}


void Controller::HandleFadeCommand(const char *szMsg, ConParse *pParse)
{
	char *pParamType, *pValue, *pDuration, *pWaveType;
	ParamType paramType;
	ParamValue paramValue;
	WaveType waveType;
	float duration;
    uint32 i;


	if(pParse->m_nArgs < 4)
	{
		ShowTriggerError(szMsg);
		return;
	}

	pParamType = pParse->m_Args[1];
	pValue = pParse->m_Args[2];
	pDuration = pParse->m_Args[3];

	// Parse everything.. it doesn't do anything if there's an error.
	if(stricmp(pParamType, "ALPHA") == 0)
	{
		paramType = Param_Alpha;
	}
	else if(stricmp(pParamType, "COLOR") == 0)
	{
		paramType = Param_Color;
	}
	else
	{
		ShowTriggerError(szMsg);
		return;
	}

	paramValue = ParseValue(paramType, pValue);
	duration = (float)atof(pDuration);
    duration = LTCLAMP(duration, 0.0f, 100000.0f);

	waveType = Wave_Sine;
	if(pParse->m_nArgs >= 5)
	{
		pWaveType = pParse->m_Args[4];
		waveType = ParseWaveType(pWaveType);
	}

	// Ok, configure...
    m_fStartTime = g_pLTServer->GetTime();
	m_fDuration = duration;
	m_ParamType = paramType;
	m_WaveType = waveType;
	m_DestValue = paramValue;

	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		SetupCurrentValue(&m_Fades[i]);
	}

	m_State = CState_Fade;
    SetNextUpdate(0.001f);
}


void Controller::HandleFlickerCommand(const char *szMsg, ConParse *pParse)
{
	char *pMin, *pMax, *pMessage;

	if (pParse->m_nArgs < 4)
	{
		ShowTriggerError(szMsg);
		return;
	}

	pMin = pParse->m_Args[1];
	pMax = pParse->m_Args[2];
	pMessage = pParse->m_Args[3];
	if(strlen(szMsg) > MAX_FLICKERMSG_LEN)
	{
        g_pLTServer->CPrint("Controller: Warning, msg '%s' greater than %d", szMsg, MAX_FLICKERMSG_LEN);
	}

	m_fIntervalMin = (float)atof(pMin);
	m_fIntervalMax = (float)atof(pMax);
	SAFE_STRCPY(m_FlickerMsg, pMessage);
	m_FlickerCounter = FLICKER_FOREVER;

	if(pParse->m_nArgs >= 5)
	{
        m_FlickerCounter = (uint32)atof(pParse->m_Args[4]);
	}

    m_fNextFlickerTime = g_pLTServer->GetTime() + GetRandom(m_fIntervalMin, m_fIntervalMax);
	m_State = CState_Flicker;
    SetNextUpdate(0.001f);
}


void Controller::HandleOffCommand(const char *szMsg, ConParse *pParse)
{
	m_State = CState_Off;
	SetNextUpdate(0.0f);
}


void Controller::ShowTriggerError(const char *szMsg)
{
    g_pLTServer->CPrint("Controller: Invalid msg: %s", szMsg);
}


ParamValue Controller::ParseValue(ParamType paramType, char *pValue)
{
	ParamValue ret;
    LTVector color;
	char colorStr[3][256];
    uint32 i;

	if(paramType == Param_Alpha)
	{
		ret.SetAlpha((float)atof(pValue));
	}
	else
	{
		sscanf(pValue, "%s %s %s", colorStr[0], colorStr[1], colorStr[2]);

		// X's mean to not interpolate that value.
		for(i=0; i < 3; i++)
		{
			color[i] = (float)atof(colorStr[i]);
			if(stricmp(colorStr[i], "X") == 0)
				color[i] = -1.0f;
		}

		ret.SetColor(color);
	}

	return ret;
}


void Controller::SetupCurrentValue(FadeState *pState)
{
	float r, g, b, a;

	if(!pState->m_hTarget)
		return;

	if(m_ParamType == Param_Alpha)
	{
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
		pState->m_StartVal.SetAlpha(a);
	}
	else
	{
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        pState->m_StartVal.SetColor(LTVector(r*255.0f, g*255.0f, b*255.0f));
	}
}


void Controller::InterpolateValue(FadeState *pState, float t)
{
	float newAlpha, r, g, b, a;
    LTVector newColor, destColor;
    uint32 i;

	if(m_ParamType == Param_Alpha)
	{
		// Alpha.
        newAlpha = LTLERP(pState->m_StartVal.GetAlpha(), m_DestValue.GetAlpha(), t);
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        g_pLTServer->SetObjectColor(pState->m_hTarget, r, g, b, newAlpha);
	}
	else
	{
		// Color.
		destColor = m_DestValue.GetColor();
		for(i=0; i < 3; i++)
		{
			if(destColor[i] == -1.0f)
				newColor[i] = pState->m_StartVal.m_Color[i];
			else
                newColor[i] = LTLERP(pState->m_StartVal.m_Color[i], m_DestValue.m_Color[i], t);
		}

        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        g_pLTServer->SetObjectColor(pState->m_hTarget,
			newColor.x / 255.0f,
			newColor.y / 255.0f,
			newColor.z / 255.0f,
			a);
	}
}